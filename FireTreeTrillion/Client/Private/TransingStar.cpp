#include "stdafx.h"
#include "TransingStar.h"

#include "Level_Loading.h"
#include "Utils.h"

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
   
    return S_OK;
}

_int CTransingStar::Tick(_float fTimeDelta)
{
    // FOR TEST
    if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1, KEY_DOWN))
        Activate();
    if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD2, KEY_DOWN))
        Deactivate();

    if (false == m_bActivate) return OBJ_NOEVENT;
    
    // activate상태일 경우만 별들이 움직임니다.
    if (CUtils::Get_Scaled_Matrix(m_arrayStarMatrix[0]).x != 0.f)
        Tick_AlphaStar(fTimeDelta);

    if (CUtils::Get_Scaled_Matrix(m_arrayStarMatrix[1]).x != 0.f)
        Tick_YeonDooStar(fTimeDelta);

    Tick_GreenStar(fTimeDelta);

    return OBJ_NOEVENT;
}

void CTransingStar::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SUPERUI, this);
}

HRESULT CTransingStar::Render()
{
    if (false == m_bActivate) return OBJ_NOEVENT;
    HRESULT hr(S_OK);

    // 고정 값들 >> 3D에서 가져온 것을 직교투영으로 바꾸
    CHECK_NULLPTR(m_pShaderCom);
    hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
    CHECK_FAILED(hr);
    hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);
    CHECK_FAILED(hr);

    // 텍스트들 돌면서 각자 다르게 값 주기
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
        hr = m_pShaderCom->Begin(16); //머지할때 17로 바꾸시오.
        CHECK_FAILED(hr);
        hr = m_pVIBufferCom->Bind_Buffers();
        CHECK_FAILED(hr);
        hr = m_pVIBufferCom->Render();
        CHECK_FAILED(hr);
    }

    return S_OK;
}

#ifdef _DEBUG
void CTransingStar::Render_IMGUI()
{
    //char ratio[16];
    //ImGui::DragFloat(ratio, (_float*)&m_fDecreaseOffset, 0.100f, 3000.f, 5000.f);
    //ImGui::Separator(); ImGui::NewLine();
}
#endif

/// <summary> 텍스쳐들의 위치를 초기화 시킨다. </summary>
void CTransingStar::Activate()
{
    // 활성화 시키는 부울값 ON
    m_bActivate = true;

    // Activate한 순간의 Player의 위치를 받아온다.
    CGameObject* pObj = m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_Player", L"Prototype_GameObject_Kirby");
    _float4 vPlayerPos = pObj->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);

    // 뷰-투영 변환 행렬
    _matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
    _matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _matrix ComMatrix = ViewMatrix * ProjMatrix;
    
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
    }
}

void CTransingStar::Deactivate()
{
    m_bActivate = false;
    m_fYeonDooTime = 1.f;
    m_fAlphaTimeRemains = 1.f;

    m_bDeadYeonDoo = false;
    m_fDecreaseValue = 0.f;
}

void CTransingStar::Tick_AlphaStar(_float fTimeDelta)
{
    if (m_fAlphaTimeRemains > 0.f)
        m_fAlphaTimeRemains -= fTimeDelta * TIMEDELTA_OFFSET;
    else
        m_fAlphaTimeRemains = 0.f;

    // 투명별 사이즈 조절
    if (m_bDeadYeonDoo)
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], 0.f, 0.f, 1.f);
    else if (m_InitialSize.x * m_fAlphaTimeRemains <= m_MediumSize.x)
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], m_MediumSize.x, m_MediumSize.y, 1.f);
    else
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[0], m_InitialSize.x * m_fAlphaTimeRemains, m_InitialSize.y * m_fAlphaTimeRemains, 1.f);
}

void CTransingStar::Tick_YeonDooStar(_float fTimeDelta)
{
    if (m_fAlphaTimeRemains <= 0.6f) // 투명별이 10% 진행되었을 때, 연두별 시작
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
        if (m_InitialSize.x < m_fDecreaseValue)
        {
            CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[1], 0.f, 0.f, 1.f);
            HRESULT hr = m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_RACING));
            CHECK_FAILED(hr);
        }
    }
    else // 연두별 배경 대기
        CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[1], m_InitialSize.x * 1.5f, m_InitialSize.y * 1.5f, 1.f);
}

void CTransingStar::Tick_GreenStar(_float fTimeDelta)
{
    CUtils::Set_Scaled_Matrix(m_arrayStarMatrix[2], m_InitialSize.x, m_InitialSize.y, 1.f);
}

HRESULT CTransingStar::Add_Components()
{
    HRESULT hr(S_OK);

    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

#pragma region 텍스쳐 컴포넌트
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
        TEXT("Com_Texture_AlphaStar"), (CComponent**)&m_arrTextures[0]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Star"),
        TEXT("Com_Texture_SubStar"), (CComponent**)&m_arrTextures[1]);
    CHECK_FAILED(hr);
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
    
    __super::Free();

}

