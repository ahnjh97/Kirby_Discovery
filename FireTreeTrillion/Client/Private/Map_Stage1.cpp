#include "stdafx.h"
#include "Map_Stage1.h"
#include "PartObject.h"

CMap_Stage1::CMap_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CMap_Stage1::CMap_Stage1(const CMap_Stage1& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CMap_Stage1::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMap_Stage1::Initialize(void* pArg)
{
    MAP_DESC		GameObjectDesc{};
    if (nullptr != pArg)
        GameObjectDesc = *(MAP_DESC*)pArg;

    GameObjectDesc.fSpeedPerSec = 10.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    wstring wstrModelTag = GameObjectDesc.wstrModelTag;

    if (FAILED(Add_Components(wstrModelTag)))
        return E_FAIL;

    if (wstrModelTag.substr(wstrModelTag.length() - 5) != TEXT("Blend")) {  
        if (FAILED(Add_BlendMap(wstrModelTag))) // BlendMap이 아닌 경우 PartObject 추가
            return E_FAIL;
    }
    else    // BlendMap이 맞는 경우
        m_eRenderGroup = CRenderer::RENDER_BLEND;

    m_vecPassIndices = GameObjectDesc.iPassIndices;  // 조건문으로 어떤모델인지에 따라서 그냥 이 클래스에서 직접 지정할지 Loader에서 넘겨줘서 지정할지 고민..

    if (FAILED(m_pModelCom->CreateStaticActor(m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION))))
        return E_FAIL;

    return S_OK;
}

_int CMap_Stage1::Tick(_float fTimeDelta)
{
    if (nullptr != m_pBlendMap)
        m_pBlendMap->Tick(fTimeDelta);

    return OBJ_NOEVENT;
}

void CMap_Stage1::Late_Tick(_float fTimeDelta)
{
    if (nullptr != m_pBlendMap)
        m_pBlendMap->Late_Tick(fTimeDelta);

    m_pGameInstance->Add_RenderGroup(m_eRenderGroup, this);
}

HRESULT CMap_Stage1::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    // Component인 m_pModelCom에서 나의 Mesh의 개수를 파악한다.
    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_HEIGHT)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;
        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CMap_Stage1::Add_Components(const wstring& _wstrModelTag)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMap_Stage1::Bind_ShaderResources()
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

HRESULT CMap_Stage1::Add_BlendMap(const wstring& _wstrModelTag)
{
    MAP_DESC tMapDesc{};
    tMapDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
    tMapDesc.wstrModelTag = _wstrModelTag + L"Blend"; // 맵 이름 규칙

    vector<_uint> vecPassIndices;
    vecPassIndices.resize(m_pModelCom->Get_NumMeshes()); // 수정 필요
    tMapDesc.iPassIndices = vecPassIndices; 

    m_pBlendMap = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Map_Stage1"), &tMapDesc);
    if (nullptr == m_pBlendMap)
        return E_FAIL;

    return S_OK;
}

CMap_Stage1* CMap_Stage1::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMap_Stage1* pInstance = new CMap_Stage1(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CMap_Stage1"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CMap_Stage1::Clone(void* pArg)
{
    CMap_Stage1* pInstance = new CMap_Stage1(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CMap_Stage1"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMap_Stage1::Free()
{
    __super::Free();
    Safe_Release(m_pBlendMap);

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}
