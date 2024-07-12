#include "stdafx.h"
#include "KirbyArmours.h"
#include "Bone.h"

#include "Light.h"

CKirbyArmours::CKirbyArmours(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CKirbyArmours::CKirbyArmours(const CKirbyArmours& rhs)
    : CPartObject{ rhs }
{
}

HRESULT CKirbyArmours::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CKirbyArmours::Initialize(void* pArg)
{
    KIRBYARMOURS_DESC* pArmourDesc = (KIRBYARMOURS_DESC*)pArg;

    m_pBoneMatrix = pArmourDesc->pBoneMatrix;
    m_pAbilityType = pArmourDesc->pAbilityType;
    m_pWhiteColorDiffuse = pArmourDesc->pWhite;
    m_fOverPowerColor = pArmourDesc->pOverPower;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    return S_OK;
}

_int CKirbyArmours::Tick(_float fTimeDelta)
{
    if (m_bDead)
        return OBJ_DEAD;

    Compute_MotionBlur();

    m_WorldMatrix = m_pTransformCom->Get_WorldMatrix() **m_pBoneMatrix **m_pParentMatrix;


    if (m_pLight == nullptr && *m_pAbilityType == ABILITY_CRASH)
    {
        LIGHT_DESC			LightDesc{};
        LightDesc.eType = LIGHT_DESC::TYPE_HORONG;
        _float4 vLightPos = CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION);
        LightDesc.vPosition = vLightPos;
        LightDesc.fRange = 5.f;
        LightDesc.vDiffuse = _float4(0.6f, 1.f, 1.f, 1.f);
        LightDesc.vAmbient = _float4(.5f, .5f, .5f, 1.f);
        LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
        if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
            return OBJ_NOEVENT;
        m_pLight = CGameInstance::Get_Instance()->Get_LightLastAddress();
        Safe_AddRef(m_pLight);
    }

    if (m_pLight != nullptr && *m_pAbilityType == ABILITY_CRASH)
    {
        m_pLight->Update_LightPos(CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION));
    }

    if (m_pLight != nullptr && *m_pAbilityType != ABILITY_CRASH)
    {
        m_pLight->Set_DeadLight(true);
        Safe_Release(m_pLight);
        m_pLight = nullptr;
    }


    return OBJ_NOEVENT;
}

void CKirbyArmours::Late_Tick(_float fTimeDelta)
{
    if (Block_Render() == false)
    {
        if (true == m_pGameInstance->isInFrustum_WorldSpace(XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, m_WorldMatrix._44), 2.0f))
        {
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
            m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);

            if (*m_pAbilityType == ABILITY_CRASH)
            {
                m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
                m_iRenderCount = 1;
            }
        }
    }
}

HRESULT CKirbyArmours::Render()
{
    if (Block_Render()) return S_OK;
    //if (*m_pCurrentLevelID == LEVEL_TOOL_ANIM) return S_OK;

    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[*m_pAbilityType]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        // For Crash
        if (m_iRenderCount == 0 && *m_pAbilityType == ABILITY_CRASH)
        {
            if (i == 0)
            {
                if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
                    return E_FAIL;
                if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
                    return E_FAIL;
                if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
                    return E_FAIL;
                _bool bEmissive = true;
                if (FAILED(m_pShaderCom->Bind_RawValue("g_bEmissive", &bEmissive, sizeof(_bool))))
                    return E_FAIL;
                m_fEmissivePower = CUtils::Make_RandomFloat(0.8f, 1.f);
                if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissivePower", &m_fEmissivePower, sizeof(_float))))
                    return E_FAIL;
            }
            else
                continue;
        }
        else
        {
            if (*m_pAbilityType == ABILITY_CRASH)
            {
                if (i == 0)
                    continue;
            }

            if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(m_pModelCom[*m_pAbilityType]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
                return E_FAIL;
            _bool bEmissive = false;
            if (FAILED(m_pShaderCom->Bind_RawValue("g_bEmissive", &bEmissive, sizeof(_bool))))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(MODEL_KIRBYPART)))
            return E_FAIL;
        m_pModelCom[*m_pAbilityType]->Render(i);
    }


    if (*m_pAbilityType == ABILITY_CRASH)
    {
        m_iRenderCount--;
    }

    return S_OK;
}

HRESULT CKirbyArmours::Render_LightDepth()
{
    if (FAILED(m_pGameInstance->Render_LightDepth_For_PartObject(m_pShaderCom, &m_WorldMatrix, m_pModelCom[*m_pAbilityType])))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirbyArmours::Render_DeferredInfo()
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

void CKirbyArmours::Large_Light(_float4 vDiffuse, _float fRange, _float fTime)
{
    if (m_pLight == nullptr)
        return;

    m_pLight->Interpolate_Light(vDiffuse, fRange, fTime);
}

HRESULT CKirbyArmours::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyArmour_Sword"),
        TEXT("Com_Model_Sword"), (CComponent**)&m_pModelCom[ABILITY_SWORD]);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyArmour_Boom"),
        TEXT("Com_Model_Boom"), (CComponent**)&m_pModelCom[ABILITY_BOMB]);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyArmour_Hammer"),
        TEXT("Com_Model_Hammer"), (CComponent**)&m_pModelCom[ABILITY_HAMMER]);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyArmour_Crash"),
        TEXT("Com_Model_Crash"), (CComponent**)&m_pModelCom[ABILITY_CRASH]);
    CHECK_FAILED(hr);

    return S_OK;
}

HRESULT CKirbyArmours::Bind_ShaderResources()
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

void CKirbyArmours::Compute_MotionBlur()
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

CKirbyArmours* CKirbyArmours::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirbyArmours* pInstance = new CKirbyArmours(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CKirbyArmours"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirbyArmours::Clone(void* pArg)
{
    CKirbyArmours* pInstance = new CKirbyArmours(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CKirbyArmours"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirbyArmours::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);

    for (auto& pModel : m_pModelCom)
        Safe_Release(pModel);

    if (m_pLight != nullptr)
        Safe_Release(m_pLight);

}
