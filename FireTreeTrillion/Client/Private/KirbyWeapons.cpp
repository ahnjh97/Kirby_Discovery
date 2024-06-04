#include "stdafx.h"
#include "KirbyWeapons.h"
#include "Bone.h"

CKirbyWeapons::CKirbyWeapons(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CKirbyWeapons::CKirbyWeapons(const CKirbyWeapons& rhs)
    : CPartObject{ rhs }
{
}

HRESULT CKirbyWeapons::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CKirbyWeapons::Initialize(void* pArg)
{
    KIRBYWEAPON_DESC* pWeaponDesc = (KIRBYWEAPON_DESC*)pArg;

    m_pBoneMatrix = pWeaponDesc->pBoneMatrix;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    return S_OK;
}

_int CKirbyWeapons::Tick(_float fTimeDelta)
{
    if (m_bDead)
        return OBJ_DEAD;

    Compute_MotionBlur();

    m_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * *m_pBoneMatrix * *m_pParentMatrix;

    return OBJ_NOEVENT;
}

void CKirbyWeapons::Late_Tick(_float fTimeDelta)
{
    if (true == m_pGameInstance->isInFrustum_WorldSpace(XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, m_WorldMatrix._44), 2.0f))
    {
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);
    }
}

HRESULT CKirbyWeapons::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_X)))
            return E_FAIL;

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CKirbyWeapons::Render_LightDepth()
{
    if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirbyWeapons::Render_DeferredInfo()
{

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {

        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(MODEL_DEFERREDINFO)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;

}

HRESULT CKirbyWeapons::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyWeapon_Sword"),
        TEXT("Com_Model"), (CComponent**)&m_pModelCom);
    CHECK_FAILED(hr);

    return S_OK;
}

HRESULT CKirbyWeapons::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    // 추후 변경
    _bool bStencil = true;
    _bool bRimLight = true;
    _bool bMotionBlur = true;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
        return E_FAIL;

    return S_OK;
}

void CKirbyWeapons::Compute_MotionBlur()
{
    _vector vPos = CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION);
    _matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
    _float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
    _float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

    _float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

    m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
    m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
    m_vPreScreenPos = vCurScreenPos;
}

CKirbyWeapons* CKirbyWeapons::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirbyWeapons* pInstance = new CKirbyWeapons(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CKirbyWeapons"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirbyWeapons::Clone(void* pArg)
{
    CKirbyWeapons* pInstance = new CKirbyWeapons(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CKirbyWeapons"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirbyWeapons::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
