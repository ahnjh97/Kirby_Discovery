#include "stdafx.h"
#include "FinalePartical.h"

CFinalePartical::CFinalePartical(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CFinalePartical::CFinalePartical(const CFinalePartical& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CFinalePartical::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFinalePartical::Initialize(void* pArg)
{
    GAMEOBJECT_DESC* Desc = nullptr;

    if (pArg != nullptr)
        Desc = (GAMEOBJECT_DESC*)pArg;

    if (FAILED(__super::Initialize(Desc)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    m_bMotionBlur = true;
    m_bRimLight = true;
    m_bStencil = true;


    return S_OK;
}

_int CFinalePartical::Tick(_float fTimeDelta)
{
    if (m_bDead == true)
        return OBJ_DEAD;

    if (m_bActive == false)
        return OBJ_NOEVENT;

    m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
    Compute_MotionBlur();

    _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
    m_fGravity += 1.5f * m_fTimeDelta;
    _float4 vDelta = (m_vDir * m_fTimeDelta * m_fSpeed);
    _float4 vGravity = m_bNoGravity == false ? _float4(0.f, m_fGravity, 0.f, 0.f) : _float4(0.f, 0.f, 0.f, 0.f);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDelta - vGravity);
    m_pTransformCom->Turn(m_fTurnAxis, m_fTimeDelta, m_fTurn);

    m_fActiveTime += m_fTimeDelta;

    _float fActiveMaxTime = m_bNoGravity == false ? 5.f : 20.f;
    if (m_fActiveTime > fActiveMaxTime)
    {
        m_bActive = false;
    }


    return OBJ_NOEVENT;
}

void CFinalePartical::Late_Tick(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 5.0f))
    {
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
        if (m_bNoGravity == false)
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
    }

}

HRESULT CFinalePartical::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iRandomTexture * 3)))
            return E_FAIL;
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", m_iRandomTexture * 3 + 1)))
            return E_FAIL;
        if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", m_iRandomTexture * 3 + 2)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
            return E_FAIL;
        _float fWhiteColorDiffuse = 0.f;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
            return E_FAIL;

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CFinalePartical::Render_LightDepth()
{
    if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

void CFinalePartical::Set_Partical(_float4 vPos, _float fScale, _float4 vDir, _float fSpeed, _bool bNoGravity)
{
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
    m_vDir = vDir;
    m_fSpeed = fSpeed;
    m_bActive = true;
    m_fGravity = 0.f;
    m_pTransformCom->Set_Scaled(fScale, fScale, fScale);
    m_iRandomTexture = CUtils::Make_RandomInt(0, 2);
    m_fTurn = CUtils::Make_RandomFloat(10.f, 720.f);
    m_fTurnAxis = CUtils::Make_Random_Vector(1.f);
    m_fTurnAxis.w = 0.f;
    m_fActiveTime = 0.f;

    // 중력을 먹이지 않을 것인가?
    m_bNoGravity = bNoGravity;
}

HRESULT CFinalePartical::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(TEXT("Prototype_Component_Model_RoadParticle"),
        TEXT("Com_Model"), (CComponent**)&m_pModelCom);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_FinalePartical"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
    CHECK_FAILED(hr);

    // 텍스쳐 준비할 것.
    return S_OK;
}

HRESULT CFinalePartical::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    return S_OK;
}

void CFinalePartical::Compute_MotionBlur()
{
    _vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
    _matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
    _float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
    _float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

    _float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

    m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
    m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
    m_vMotionVelocity.z = 0.f;

    m_vPreScreenPos = vCurScreenPos;
}

CFinalePartical* CFinalePartical::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CFinalePartical* pInstance = new CFinalePartical(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CFinalePartical"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CFinalePartical::Clone(void* pArg)
{
    CFinalePartical* pInstance = new CFinalePartical(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CFinalePartical"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CFinalePartical::Free()
{
    __super::Free();
    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pTextureCom);
}
