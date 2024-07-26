#include "stdafx.h"
#include "TransingStar.h"

#include "Level_Loading.h"
#include "Kirby.h"
#include "Camera_Main.h"
#include "Utils.h"
#include "LoadingStart.h"
#include "LoadingFont.h"

const _float	g_fPosOffset        = 18.f;
const _float	g_fTurnOffset       = 72.f;
#define         TIMEDELTA_OFFSET    1.5f

CTransingStar::CTransingStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIObject{ pDevice, pContext }
{
}

CTransingStar::CTransingStar(const CTransingStar& rhs)
    : CUIObject{ rhs }
    , m_arrTextures(rhs.m_arrTextures)
    , m_arrayStarMatrix(rhs.m_arrayStarMatrix)
{
}

HRESULT CTransingStar::Initialize_Prototype()
{
    fill(m_arrTextures.begin(), m_arrTextures.end(), nullptr);
    fill(m_arrayStarMatrix.begin(), m_arrayStarMatrix.end(), _float4x4());
    return S_OK;
}

HRESULT CTransingStar::Initialize(void* pArg)
{
    HRESULT hr = __super::Initialize(pArg);
    CHECK_FAILED(hr);
    hr = Add_Components();
    CHECK_FAILED(hr);

    XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
   
    m_vSmallColor = _float3(160.f / 255.f, 212.f / 255.f, 104.f / 255.f);
    m_vLargeColor = _float3(91.f / 255.f,  121.f / 255.f, 59.f / 255.f);

    m_pLoadingStart = static_cast<CLoadingStart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_LoadingStart")));
    if (nullptr == m_pLoadingStart)
    {
        MSG_BOX(TEXT("Failed to CloneObject to TransingStar"));
        return E_FAIL;
    }

    return S_OK;
}

_int CTransingStar::Tick(_float fTimeDelta)
{
    m_fTimeDelta = m_pGameInstance->Get_SecondTimer() * 1.5f;

    // FOR TEST
    if (m_pGameInstance->Get_DIKeyState(DIK_LALT, KEY_PRESS))
    {
        if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1, KEY_DOWN))
            Activate(CTransingStar::CLOSE);
        if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD2, KEY_DOWN))
            Deactivate();
        if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD3, KEY_DOWN))
            Activate(CTransingStar::OPEN);
    }

	if (m_eActivateType == TYPE_END) return OBJ_NOEVENT;
    switch (m_eActivateType)
    {
    case CLOSE:
    {
        // activate상태일 경우만 별들이 움직임니다.
        if (CUtils::Get_Scaled_Matrix(m_arrayStarMatrix[0]).x != 0.f)
            Tick_AlphaStar(m_fTimeDelta);

        if (CUtils::Get_Scaled_Matrix(m_arrayStarMatrix[1]).x != 0.f)
            Tick_YeonDooStar(m_fTimeDelta);

        Tick_GreenStar(m_fTimeDelta);
    }
    break;
    case OPEN:
    {
        Tick_OpenAlphaStar(m_fTimeDelta);
    }
    break;
    }

    return OBJ_NOEVENT;
}

void CTransingStar::Late_Tick(_float fTimeDelta)
{
    //m_pLoadingStart->Late_Tick(m_fTimeDelta);

    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SUPERUI, this);
}

HRESULT CTransingStar::Render()
{
    if (m_eActivateType == TYPE_END) return S_OK;

    HRESULT hr(S_OK);

    // 고정 값들 >> 3D에서 가져온 것을 직교투영으로 바꾸
    CHECK_NULLPTR(m_pShaderCom);
    hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
    CHECK_FAILED(hr);
    hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);
    CHECK_FAILED(hr);

    m_pShaderCom->Bind_RawValue("g_vSmallStarColor", &m_vSmallColor, sizeof(_float3));
    m_pShaderCom->Bind_RawValue("g_vLargeStarColor", &m_vLargeColor, sizeof(_float3));

    if (m_eActivateType == CLOSE)
    {
        RenderClose();
    }
    else if (m_eActivateType == OPEN)
        RenderOpen();

    if (true == m_bLoadingStart)
    {
        m_fStartDelayTime += m_fTimeDelta;
        if(0.4f < m_fStartDelayTime)
            m_pLoadingStart->Render(m_fTimeDelta);
    }
    else
    {
        m_fStartDelayTime = 0.f;
        m_pLoadingStart->Set_Alpha(0.f);
    }

    return S_OK;
}

#ifdef _DEBUG
void CTransingStar::Render_IMGUI()
{
}
#endif

/// <summary> 텍스쳐들의 위치를 초기화 시킨다. </summary>
void CTransingStar::Activate(TYPE _eActivateType)
{
    if (_eActivateType == OPEN && m_eActivateType != CLOSE)
        return;
    
    // 활성화 시키는 부울값 ON
    m_eActivateType = _eActivateType;

    // 뷰-투영 변환 행렬
    _matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
    _matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _matrix ComMatrix = ViewMatrix * ProjMatrix;

    switch (m_eActivateType)
    {
    case OPEN:
    {
        fill(m_arrayStarMatrix.begin(), m_arrayStarMatrix.end(), _float4x4());

        for (_int i = 0; i < 3; ++i)
        {
            CUtils::Set_State_Matrix(m_arrayStarMatrix[i], CUtils::STATE_POSITION, _float4(0.f, 0.f, i * 0.1f, 1.f));
            //CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[i], m_InitialSize.x, m_InitialSize.y, 1.f);
            CUtils::Rotation(m_arrayStarMatrix[i], _float4(0.f, 0.f, 1.f, 0.f), 0.f);
        }
        CUtils::Set_State_Matrix(m_arrayStarMatrix[2], CUtils::STATE_POSITION, _float4(0.f, 0.f, 0.2f, 1.f));
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[2], m_InitialSize.x, m_InitialSize.y, 1.f);
    }
    break;
    case CLOSE:
    {
        // Activate한 순간의 Player의 위치를 받아온다.
        CGameObject* pObj = m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_Player", L"Prototype_GameObject_Kirby");
        _float4 vPlayerPos = pObj->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);

        // 3D 위치를 다시 2D 위치로 바꾸는 과정
        _float4 vPlayerNewPos = XMVector3TransformCoord(XMLoadFloat4(&vPlayerPos), ComMatrix);
        _float4 vFinalPos = _float4((vPlayerNewPos.x * g_iWinSizeX) * 0.5f,
                                    (vPlayerNewPos.y * g_iWinSizeY) * 0.5f + g_fPosOffset,
                                    0.f, 1.f);

        fill(m_arrayStarMatrix.begin(), m_arrayStarMatrix.end(), _float4x4());
        for (_int i = 0; i < 3; ++i)
        {
            _float4 vFinalPoswithZ = _float4(vFinalPos.x, vFinalPos.y, vFinalPos.z + i * 0.1f, 1.f);
            CUtils::Set_State_Matrix(m_arrayStarMatrix[i], CUtils::STATE_POSITION, vFinalPoswithZ);
            CUtils::Rotation(m_arrayStarMatrix[i], _float4(0.f, 0.f, 1.f, 0.f), 0.f);
        }
    }
    break;
    }
}

void CTransingStar::Deactivate()
{
    // TransingStar 종류 초기화
    m_eActivateType = TYPE_END;

    // 움직임에 대한 시간 초기화
    m_fAlphaTime = 1.f;
    m_fYeonDooTime = 1.f;
    m_fDecreaseValue = 0.f;

    // 연두별의 셰이더처리에 대한 초기화
    m_bDeadYeonDoo = false;

    // 준수 
    m_bLoadingStart = false;
    m_bFontRender = true;
}

void CTransingStar::Tick_AlphaStar(_float fTimeDelta)
{
    if (m_fAlphaTime > 0.f)
        m_fAlphaTime -= fTimeDelta * TIMEDELTA_OFFSET;
    else
        m_fAlphaTime = 0.f;

    // 투명별 사이즈 조절
    if (m_bDeadYeonDoo)
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], 0.f, 0.f, 1.f);
    else if (m_InitialSize.x * m_fAlphaTime <= m_MediumSize.x)
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], m_MediumSize.x, m_MediumSize.y, 1.f);
    else
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], m_InitialSize.x * m_fAlphaTime, m_InitialSize.y * m_fAlphaTime, 1.f);
}

void CTransingStar::Tick_YeonDooStar(_float fTimeDelta)
{
    if (m_fAlphaTime <= 0.6f) // 투명별이 10% 진행되었을 때, 연두별 시작
    {
        // 연두 돌아가유
        if (m_bDeadYeonDoo)
            CUtils::Turn_OtherMatrix(m_arrayStarMatrix[1], _float4(0.f, 0.f, 1.f, 0.f), fTimeDelta, 720.f);
        else
            CUtils::Turn_OtherMatrix(m_arrayStarMatrix[1], _float4(0.f, 0.f, 1.f, 0.f), fTimeDelta * TIMEDELTA_OFFSET, g_fTurnOffset);

        // 연두 사이즈 감소
        m_fDecreaseValue += fTimeDelta * m_fDecreaseOffset * TIMEDELTA_OFFSET;
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[1], m_InitialSize.x - m_fDecreaseValue, m_InitialSize.y - m_fDecreaseValue, 1.f);

        // 연두가 투명해지는 조건 == m_fYeonDooTime(1초)가 되었을때
        if (m_fYeonDooTime <= 0.f)
            m_bDeadYeonDoo = true;
        m_fYeonDooTime -= fTimeDelta * TIMEDELTA_OFFSET;

        // 사이즈가 다시 커지는 것에 대한 예외처리
        if (m_InitialSize.x <= m_fDecreaseValue)
        {
            CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[1], 0.f, 0.f, 1.f);
            On_Event();
        }
    }
    else // 연두별 배경 대기
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[1], m_InitialSize.x * 1.5f, m_InitialSize.y * 1.5f, 1.f);
}

void CTransingStar::Tick_GreenStar(_float fTimeDelta)
{
    CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[2], m_InitialSize.x, m_InitialSize.y, 1.f);
    if (m_eNextLevel != LEVEL_END) return;

    // 레벨이동 트리거가 아닌 커비 이동 트리거를 밟았을때 2초 뒤에 TransingStar는 오픈됩니다.
    if (*m_pCurrentLevelID == LEVEL_PARK || 
        *m_pCurrentLevelID == LEVEL_TOWN ||
        *m_pCurrentLevelID == LEVEL_FINALBOSS)
    {
        if (m_InitialSize.x <= m_fDecreaseValue)
        {
            m_fMovingTime += fTimeDelta;
            if (m_fMovingTime >= 2.f)
            {
                m_fMovingTime = 0.f;
                Activate(OPEN);
            }
        }
    }
}

void CTransingStar::RenderClose()
{
    HRESULT hr(S_OK);

    for (_uint i = 0; i < m_arrTextures.size(); ++i)
    {
        // 색 다르게 주는 곳
        if (m_bDeadYeonDoo && i == 1)
        {
            _int iTemp = 0;
            m_pShaderCom->Bind_RawValue("g_iMasking", &iTemp, sizeof(_int));
        }
        else
            m_pShaderCom->Bind_RawValue("g_iMasking", &i, sizeof(_int));
        // 사이즈, 위치 다르게 주는 곳
        hr = m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_arrayStarMatrix[i]);
        CHECK_FAILED(hr);
        // 텍스쳐는 같지만 다르게 붙여줍니다. >> >텍스쳐 같아도 될 것 같은데?
        hr = m_arrTextures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
        CHECK_FAILED(hr);

        // 실질적 render
        hr = m_pShaderCom->Begin(POSTEX_FOCUSINGPOSITION);
        CHECK_FAILED(hr);
        hr = m_pVIBufferCom->Bind_Buffers();
        CHECK_FAILED(hr);
        hr = m_pVIBufferCom->Render();
        CHECK_FAILED(hr);
    }
}

void CTransingStar::Tick_OpenAlphaStar(_float fTimeDelta)
{
    // ---- 투명한 알파 별에 대한 처리 ----------------------------
    // 위치 고정
    CUtils::Set_State_Matrix(m_arrayStarMatrix[0], CUtils::STATE_POSITION, _float4(0.f, 0.f, 0.f, 1.f));
    
    // 한바퀴 다 돌았으면 사이즈 고정
    if (m_fAlphaTime > 1.f)
    {
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], m_InitialSize.x * 1.5f, m_InitialSize.y * 1.5f, 1.f);
        Deactivate();
    }
    else // 커지면서 돌리기
    {
        m_fAlphaTime += m_fTimeDelta * TIMEDELTA_OFFSET;
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], m_InitialSize.x * m_fAlphaTime, m_InitialSize.y * m_fAlphaTime, 1.f);
        CUtils::Turn_OtherMatrix(m_arrayStarMatrix[0], _float4(0.f, 0.f, 1.f, 0.f), m_fTimeDelta, 135.f);
    }

    // ---- 초록 별에 대한 처리 ----------------------------
    _float4 vFinalPoswithZ = _float4(0.f, 0.f, 0.2f, 1.f);
    CUtils::Set_State_Matrix(m_arrayStarMatrix[2], CUtils::STATE_POSITION, vFinalPoswithZ);
    CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[2], m_InitialSize.x, m_InitialSize.y, 1.f);
}

void CTransingStar::RenderOpen()
{
    HRESULT hr(S_OK);
    // 텍스트들 돌면서 각자 다르게 값 주기
    for (_uint i = 0; i < m_arrTextures.size(); ++i)
    {
        if (i == 1) continue;

        m_pShaderCom->Bind_RawValue("g_iMasking", &i, sizeof(_int));

        // 사이즈, 위치 다르게 주는 곳
        hr = m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_arrayStarMatrix[i]);
        CHECK_FAILED(hr);

        hr = m_arrTextures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
        CHECK_FAILED(hr);

        // 실질적 render
        hr = m_pShaderCom->Begin(POSTEX_FOCUSINGPOSITION);
        CHECK_FAILED(hr);
        hr = m_pVIBufferCom->Bind_Buffers();
        CHECK_FAILED(hr);
        hr = m_pVIBufferCom->Render();
        CHECK_FAILED(hr);
    }
}

void CTransingStar::On_Event()
{
    if (m_eNextLevel != LEVEL_END)
    {
        // 스타트 로딩화면 렌더 켜기
        m_bLoadingStart = true;

        if (LEVEL_RACING == m_eNextLevel)
        {
            m_pLoadingStart->Set_TexIndex(0);

            CLoadingFont::LOADINGFONT_DESC LoadingFont_Desc{};
            HRESULT hr;
            LoadingFont_Desc.strTag = TEXT("Prototype_Component_Texture_UI_Forest_Font");
            _float fPosX = -60.f;
            _float fEndPos = -125.f;
            _float fDisappearPos = -205.f;
            for (_uint i = 0; i < 5; ++i)
            {
                LoadingFont_Desc.fPosX = fPosX;
                LoadingFont_Desc.fEndPos = fEndPos;
                LoadingFont_Desc.fDisappearPos = fDisappearPos;
                LoadingFont_Desc.iTexIndex = i;
                if (4 == i)
                    LoadingFont_Desc.bDeadRender = true;
                hr = m_pGameInstance->Add_Clone(LEVEL_STATIC, TEXT("Layer_UI"), TEXT("Prototype_GameObject_LoadingFont"), &LoadingFont_Desc);
                CHECK_FAILED(hr);

                fPosX += 20.f;
                if (i != 2)
                {
                    fEndPos += 50.f;
                    fDisappearPos += 90.f;
                }
                else
                {
                    fEndPos += 60.f;
                    fDisappearPos += 100.f;
                }
            }
        }
        else if (LEVEL_SIMBA == m_eNextLevel)
        {
            m_pLoadingStart->Set_TexIndex(2);
            m_bFontRender = false;
            CLoadingFont::LOADINGFONT_DESC LoadingFont_Desc{};
            HRESULT hr;
            LoadingFont_Desc.strTag = TEXT("Prototype_Component_Texture_UI_Lab_Font");
            _float fPosX = -60.f;
            _float fEndPos = -125.f;
            _float fDisappearPos = -205.f;
            for (_uint i = 0; i < 6; ++i)
            {
                LoadingFont_Desc.fPosX = fPosX;
                LoadingFont_Desc.fEndPos = fEndPos;
                LoadingFont_Desc.fDisappearPos = fDisappearPos;
                LoadingFont_Desc.iTexIndex = i;
                if (5 == i)
                    LoadingFont_Desc.bDeadRender = true;
                hr = m_pGameInstance->Add_Clone(LEVEL_STATIC, TEXT("Layer_UI"), TEXT("Prototype_GameObject_LoadingFont"), &LoadingFont_Desc);
                CHECK_FAILED(hr);

                fPosX += 20.f;
                if (i != 0)
                {
                    fEndPos += 50.f;
                    fDisappearPos += 90.f;
                }
                else
                {
                    fEndPos += 60.f;
                    fDisappearPos += 100.f;
                }
            }
        }
        else if (LEVEL_PARK == m_eNextLevel)
            m_bLoadingStart = false;

        m_pGameInstance->Reserve_Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, m_eNextLevel));
    }
    else
    {
        CCamera_Main*   pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
        CGameObject*    pPlayer = m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_Player", L"Prototype_GameObject_Kirby");
        CKirby*         pKirby = dynamic_cast<CKirby*>(pPlayer);

        switch (*CGameInstance::Get_Instance()->Get_CurrentLevelID())
        {
        case LEVEL_TOWN:
        {
            // 스타트 로딩화면 렌더 켜기
            m_bLoadingStart = true;
            m_pLoadingStart->Set_TexIndex(1);

            CLoadingFont::LOADINGFONT_DESC LoadingFont_Desc{};
            HRESULT hr;
            LoadingFont_Desc.strTag = TEXT("Prototype_Component_Texture_UI_Park_Font");
            _float fPosX = -80.f;
            _float fEndPos = -145.f;
            _float fDisappearPos = -225.f;
            for (_uint i = 0; i < 7; ++i)
            {
                LoadingFont_Desc.fPosX = fPosX;
                LoadingFont_Desc.fEndPos = fEndPos;
                LoadingFont_Desc.fDisappearPos = fDisappearPos;
                LoadingFont_Desc.iTexIndex = i;
                if (6 == i)
                    LoadingFont_Desc.bDeadRender = true;
                hr = m_pGameInstance->Add_Clone(LEVEL_STATIC, TEXT("Layer_UI"), TEXT("Prototype_GameObject_LoadingFont"), &LoadingFont_Desc);
                CHECK_FAILED(hr);

                fPosX += 20.f;
                if (i != 1 && i != 3)
                {
                    fEndPos += 50.f;
                    fDisappearPos += 90.f;
                }
                else
                {
                    fEndPos += 60.f;
                    fDisappearPos += 100.f;
                }
            }
            pCameraMain->Clear_Sequence();
            pCameraMain->Lock_All(_float3(109.9f, 25.2f, 108.5f), _float3(1.f, .07f, -.12f));
            pCameraMain->Unlock();
            pCameraMain->Set_FOVY(38);
            pCameraMain->Set_InterpolateSpeed(3.f);
            pCameraMain->Move_ForTrigger(m_fTimeDelta);
            m_pGameInstance->Set_ColorSet(CRenderer::COLORSET_PARKFRONT);
            pKirby->Set_ControllerPos(_float4(134.8f, 23.2f, 104.4f, 1.f));
        }
        break;
        case LEVEL_PARK:
        {
            // 스타트 로딩화면 렌더 끄기
            m_bLoadingStart = false;
            m_pGameInstance->Set_ColorSet(CRenderer::COLORSET_HORROR);
            pKirby->Set_ControllerPos(_float4(6.1f, 38.1f, -29.1f, 1.f));  // 엘베
            //pKirby->Set_ControllerPos(_float4(0.5f, 68.f, 165.f, 1.f));  // for test : 심바맵으로 이동하기 위한
            //pKirby->Set_ControllerPos(_float4(0.5f, 68.f, -10.f, 1.f));  // for test : 엘베 타고 이동한
            pCameraMain->Set_FOVY(30);
        }
        break;
        case LEVEL_FINALBOSS:
        {
            pKirby->Set_ControllerPos(_float4(-0.45f, -3.9f, -32.54f, 1.f));
            pCameraMain->Unlock();
            m_pGameInstance->Set_ColorSet(CRenderer::COLORSET_FINAL);
            pCameraMain->Move_ForTrigger(m_fTimeDelta);
        }
        break;
        }
	}
}

HRESULT CTransingStar::Add_Components()
{
    HRESULT hr(S_OK);

    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

#pragma region 텍스쳐 컴포넌트
    // 알파 스타
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
        TEXT("Com_Texture_AlphaStar"), (CComponent**)&m_arrTextures[0]);
    CHECK_FAILED(hr);
    // 연두 스타
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
        TEXT("Com_Texture_SubStar"), (CComponent**)&m_arrTextures[1]);
    CHECK_FAILED(hr);
    // 초록 스타
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
        TEXT("Com_Texture_LastStar"), (CComponent**)&m_arrTextures[2]);
    CHECK_FAILED(hr);
#pragma endregion

    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
        TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
    CHECK_FAILED(hr);

    return S_OK;
}

CTransingStar* CTransingStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTransingStar* pInstance = new CTransingStar(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CTransingStar"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTransingStar::Clone(void* pArg)
{
    CTransingStar* pInstance = new CTransingStar(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CTransingStar"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTransingStar::Free()
{
    for (auto& texure : m_arrTextures)
        Safe_Release(texure);

    Safe_Release(m_pLoadingStart);
    
    __super::Free();
}

