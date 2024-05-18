#include "stdafx.h"
#include "TestTerrain.h"

CTestTerrain::CTestTerrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTestTerrain::CTestTerrain(const CTestTerrain& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CTestTerrain::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTestTerrain::Initialize(void* pArg)
{
    GAMEOBJECT_DESC		GameObjectDesc{};

    GameObjectDesc.fSpeedPerSec = 10.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    _vector vPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);


    return S_OK;
}

_int CTestTerrain::Tick(_float fTimeDelta)
{

    return OBJ_NOEVENT;
}

void CTestTerrain::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTestTerrain::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    // Component인 m_pModelCom에서 나의 Mesh의 개수를 파악한다.
    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    // 파악된 나의 Mesh 개수에 따라 여러번 그려서(왼팔, 오른팔, 무기) 하나의 객체를 만든다.
    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, 1)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, 5)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CTestTerrain::Add_Components()
{
    if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_TestMap"),
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CTestTerrain::Bind_ShaderResources()
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

CTestTerrain* CTestTerrain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestTerrain* pInstance = new CTestTerrain(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CTestTerrain"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestTerrain::Clone(void* pArg)
{
    CTestTerrain* pInstance = new CTestTerrain(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CTestTerrain"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestTerrain::Free()
{
    __super::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
