#include "stdafx.h"
#include "KickableRock.h"

CKickableRock::CKickableRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CRigidObject{ pDevice, pContext }
{
}

CKickableRock::CKickableRock(const CKickableRock& rhs)
	: CRigidObject( rhs )
{
}

HRESULT CKickableRock::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKickableRock::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;
	}

	HRESULT  hr = __super::Initialize(pGameObjectDesc);
	CHECK_FAILED(hr);

	Add_Components();
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(-3.f, 8.f, -188.f, 1.f));

	return S_OK;
}

_int CKickableRock::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_bDead)
		return OBJ_DEAD;

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD6, KEY_DOWN))
	{
		_float3 force = _float3{ 0.5f, 3.f , 0.5f };
		m_pGameInstance->Kick_DynamicActor(force, 100.f);
	}

	return OBJ_NOEVENT;
}

void CKickableRock::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	//CGameObject* pCamera = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Camera"), 0);
	//_float4 vForce = static_cast<CTransform*>(pCamera->Get_Component(g_strTransformTag))->Get_State_Float4(CTransform::STATE_LOOK);
	//_float3 force = _float3{ vForce.x * 10000.f, vForce.y * 10000.f, vForce.z * 10000.f };
	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD7, KEY_DOWN))
	//	m_pRigidBodyCom->Add_Force(force);
	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD8, KEY_DOWN))
	//	m_pRigidBodyCom->Add_Torque(force);
	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9, KEY_DOWN))
	//	m_pRigidBodyCom->Add_Velocity(force);
	//m_pRigidBodyCom->Update_PhysX(m_pTransformCom);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CKickableRock::Render()
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

HRESULT CKickableRock::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CKickableRock::Render_IMGUI()
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

HRESULT CKickableRock::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_GsPebble"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_RigidBody */
	CRigidBody::RIGIDBODY_DESC rigidDesc {};
	rigidDesc.bTrigger = false;
	rigidDesc.bDynamic = true;
	rigidDesc.bKinematic = false;
	rigidDesc.eShapeType = RIGID_SPHERE;
	rigidDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
		TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
	CHECK_FAILED(hr);
	m_pRigidBodyCom->Set_Object(this);
	m_pRigidBodyCom->Activate(true);

	return S_OK;
}

HRESULT CKickableRock::Bind_ShaderResources()
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

CKickableRock* CKickableRock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKickableRock* pInstance = new CKickableRock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CKickableRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKickableRock::Clone(void* pArg)
{
	CKickableRock* pInstance = new CKickableRock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CKickableRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKickableRock::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidBodyCom);

}

