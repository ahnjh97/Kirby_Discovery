#include "stdafx.h"
#include "TunnelRock.h"
#include "Kirby.h"

CTunnelRock::CTunnelRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTunnelRock::CTunnelRock(const CTunnelRock& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CTunnelRock::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTunnelRock::Initialize(void* pArg)
{
    TUNNELROCK_DESC		GameObjectDesc{};
    if (nullptr != pArg)
        GameObjectDesc = *(TUNNELROCK_DESC*)pArg;

    GameObjectDesc.fSpeedPerSec = 10.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    if (FAILED(Add_Components(GameObjectDesc.wstrModelName)))
        return E_FAIL;

    //m_pModelCom = GameObjectDesc.pModel;
    //if (nullptr == m_pModelCom)
    //    MSG_BOX(TEXT("정현아 모델 분리생성 실패했다 이 멍청아"));

    m_bHide = true;

    m_fTotalTime = 3.f;

    m_fSamplingFactor = 0.1f;
    m_vRotationAxis = CUtils::Make_Random_Vector(1);

    m_fX = CUtils::Make_RandomFloat(0, 40);
    _int iRand = CUtils::Make_RandomInt(0, 1);
    if (0 == iRand)
        m_fX *= -1;

    m_fY = CUtils::Make_RandomFloat(25, 40);
    m_fZ = CUtils::Make_RandomFloat(0, 40);
    _int iRand2 = CUtils::Make_RandomInt(0, 1);
    if (0 == iRand2)
        m_fZ *= -1;

    return S_OK;
}

_int CTunnelRock::Tick(_float fTimeDelta)
{
    if (true == m_bDead)
        return OBJ_DEAD;

    if (false == m_bDirectionDetermined)
    {
        CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
        if (nullptr == pKirby)
            return OBJ_NOEVENT;

        CTransform* pTransform = pKirby->Get_TransformCom();
        if (nullptr == pTransform)
            return OBJ_NOEVENT;



        m_bDirectionDetermined = true;
    }

    m_fTime += fTimeDelta;

    if (m_fTime > m_fTotalTime)
        Set_Dead();
    
    m_pTransformCom->Turn(m_vRotationAxis, fTimeDelta * m_fTotalTime);

    _float4 vDir = _float4(m_fX * fTimeDelta, m_fY * cos(m_fTime * XM_PI / (m_fTotalTime * 2.f) ) * fTimeDelta, m_fZ * fTimeDelta, 0);
    m_pTransformCom->Move(vDir);

    return OBJ_NOEVENT;
}

void CTunnelRock::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTunnelRock::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(MAP_NONBLEND_NONDISCARD)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

HRESULT CTunnelRock::Add_Components(const wstring& _wstrModelTag)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CTunnelRock::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_fSamplingFactor, sizeof(m_fSamplingFactor))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fZero, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

CTunnelRock* CTunnelRock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTunnelRock* pInstance = new CTunnelRock(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CTunnelRock"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTunnelRock::Clone(void* pArg)
{
    CTunnelRock* pInstance = new CTunnelRock(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CTunnelRock"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTunnelRock::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}