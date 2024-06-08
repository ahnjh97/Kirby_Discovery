#include "stdafx.h"
#include "WasteCan.h"

CWasteCan::CWasteCan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CRigidObject{ pDevice, pContext }
{
}

CWasteCan::CWasteCan(const CWasteCan& rhs)
	: CRigidObject( rhs )
{
}

HRESULT CWasteCan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWasteCan::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;
	}

	HRESULT  hr = __super::Initialize(pGameObjectDesc);
	CHECK_FAILED(hr);

	Add_Components();
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(rand() % 20, 15.f, -180.f, 1.f));

	hr = m_pModelCom->CreateDynamicActor(m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION));
	CHECK_FAILED(hr);
	
	return S_OK;
}

_int CWasteCan::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CWasteCan::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	m_pModelCom->Update_ActorTransform(m_pTransformCom);
}

HRESULT CWasteCan::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);
		
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CWasteCan::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CWasteCan::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}
#endif

HRESULT CWasteCan::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_WasteCanYellow"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_RigidMesh */
	//hr = __super::Add_Component(TEXT("Prototype_Component_RigidMesh"),
	//	TEXT("Com_RigidMesh"), (CComponent**)&m_pModelCom);
	//CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CWasteCan::Bind_ShaderResources()
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

CWasteCan* CWasteCan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWasteCan* pInstance = new CWasteCan(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CWasteCan"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWasteCan::Clone(void* pArg)
{
	CWasteCan* pInstance = new CWasteCan(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CWasteCan"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWasteCan::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}

