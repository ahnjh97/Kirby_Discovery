#include "stdafx.h"
#include "KirbyWeapons.h"
#include "Bone.h"
#include "Kirby.h"

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
    m_pAbilityType = pWeaponDesc->pAbilityType;
    m_pWhiteColorDiffuse = pWeaponDesc->pWhite;
    m_fOverPowerColor = pWeaponDesc->pOverPower;


    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    m_ePreAnimType = m_eAnimType = GIANTSWING;
    m_pModelCom[ABILITY_HAMMER]->Set_Animation(m_eAnimType, 60.f, false, false);

    return S_OK;
}

_int CKirbyWeapons::Tick(_float fTimeDelta)
{
    if (m_bDead)
        return OBJ_DEAD;

    m_fTimeDelta = m_pGameInstance->Get_FirstTimer();

    CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
    m_isAnim = pKirby->Get_AbilityType() == ABILITY_HAMMER ? true : false;

    if (m_isAnim == true)
        Change_Animation(pKirby);

    Compute_MotionBlur();
    m_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * *m_pBoneMatrix * *m_pParentMatrix;

    return OBJ_NOEVENT;
}

void CKirbyWeapons::Late_Tick(_float fTimeDelta)
{
    if (Block_Render() == false)
    {
        if (m_isAnim == true)
            m_pModelCom[*m_pAbilityType]->Play_Animation(m_fTimeDelta);

        if (true == m_pGameInstance->isInFrustum_WorldSpace(XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, m_WorldMatrix._44), 2.0f))
        {
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);
        }
    }
}

HRESULT CKirbyWeapons::Render()
{
    if (Block_Render()) return S_OK;

    if (FAILED(m_isAnim == true ? Render_AnimWeapon() : Render_NonAnimWeapon()))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirbyWeapons::Render_LightDepth()
{

    if (FAILED(m_pGameInstance->Render_LightDepth_For_PartObject(m_pShaderCom, &m_WorldMatrix, m_pModelCom[*m_pAbilityType])))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirbyWeapons::Render_DeferredInfo()
{
    if (Block_Render()) return S_OK;
    //if (*m_pCurrentLevelID == LEVEL_TOOL_ANIM) return S_OK;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[*m_pAbilityType]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {

        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(MODEL_DEFERREDINFO)))
            return E_FAIL;

        if (FAILED(m_pModelCom[*m_pAbilityType]->Render(i)))
            return E_FAIL;
    }

    return S_OK;

}

HRESULT CKirbyWeapons::Render_NonAnimWeapon()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[*m_pAbilityType]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(MODEL_KIRBYPART)))
            return E_FAIL;

        m_pModelCom[*m_pAbilityType]->Render(i);
    }

    return S_OK;
}

HRESULT CKirbyWeapons::Render_AnimWeapon()
{
    if (FAILED(Bind_ShaderResources_For_Anim()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[*m_pAbilityType]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pAnimShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pAnimShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pAnimShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
            return E_FAIL;
        if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_BoneMatrices(m_pAnimShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pAnimShaderCom->Begin(ANIMMODEL_KIRBY_WEAPONS)))
            return E_FAIL;

        m_pModelCom[*m_pAbilityType]->Render(i);
    }

    return S_OK;
}

void CKirbyWeapons::Change_Animation(CKirby* pKirby)
{
    switch (pKirby->Get_State())
    {
    case 0:
        break;


    }



    if (m_eAnimType == m_ePreAnimType)
        return;

    m_pModelCom[*m_pAbilityType]->Set_Animation(m_eAnimType, 60.f, false, false);

    if (m_eAnimType != GIANTSWING)
    {
        if (m_pModelCom[*m_pAbilityType]->IsFinished())
            m_eAnimType = GIANTSWING;
    }

    m_ePreAnimType = m_eAnimType;
}

HRESULT CKirbyWeapons::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
        TEXT("Com_AnimShader"), (CComponent**)&m_pAnimShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyWeapon_Sword"),
        TEXT("Com_Model_Sword"), (CComponent**)&m_pModelCom[ABILITY_SWORD]);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyWeapon_Hammer"),
        TEXT("Com_Model_Hammer"), (CComponent**)&m_pModelCom[ABILITY_HAMMER]);
    CHECK_FAILED(hr);
        /* For.Com_Model */


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
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", m_pWhiteColorDiffuse, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fOverPowerColor", m_fOverPowerColor, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirbyWeapons::Bind_ShaderResources_For_Anim()
{
    if (nullptr == m_pAnimShaderCom)
        return E_FAIL;

    if (FAILED(m_pAnimShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;

    if (FAILED(m_pAnimShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(m_pAnimShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    // 추후 변경
    _bool bStencil = true;
    _bool bRimLight = true;
    _bool bMotionBlur = true;

    if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", m_pWhiteColorDiffuse, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_fOverPowerColor", m_fOverPowerColor, sizeof(_float))))
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
    Safe_Release(m_pAnimShaderCom);

    for (auto& pModel : m_pModelCom)
        Safe_Release(pModel);
}
