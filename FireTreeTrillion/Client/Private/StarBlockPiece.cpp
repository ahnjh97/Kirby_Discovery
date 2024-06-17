#include "stdafx.h"
#include "StarBlockPiece.h"
#include "Trigger.h"

CStarBlockPiece::CStarBlockPiece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CRigidObject{ pDevice, pContext }
{
}

CStarBlockPiece::CStarBlockPiece(const CStarBlockPiece& rhs)
	: CRigidObject( rhs )
{
}

HRESULT CStarBlockPiece::Initialize_Prototype()
{
	//m_eCollisionGroup = KICKABLE;

	return S_OK;
}

HRESULT CStarBlockPiece::Initialize(void* pArg)
{
	PIECE_DESC tPieceDesc = *(PIECE_DESC*)pArg;

	HRESULT  hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, tPieceDesc.vInitialPos);
	m_pTransformCom->Rotation(CUtils::Make_Random_Vector(1.f), ToDegree(CUtils::Make_RandomFloat(0.f, 360.f)));

	Add_Components();

	//_float3 force = _float3{ 0.5f, 3.f , 0.5f };
	_float4 vRandomDir = CUtils::Make_Random_Vector(1.f);
	m_pRigidBodyCom->Kick_RigidBody((_float3)vRandomDir, 1.5f);

	m_fLifeTimeMax = CUtils::Make_RandomFloat(3.f, 5.f);
	m_fTurnSpeed = CUtils::Make_RandomFloat(-0.5f, 0.5f);

	return S_OK;
}

_int CStarBlockPiece::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	//if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4, KEY_DOWN))
	//{
	//	// 이 부분은 테스트가 끝나고 Collision_Hitbox에 넣기
	//	_float3 force = _float3{ 0.5f, 3.f , 0.5f };
	//	m_pRigidBodyCom->Kick_RigidBody(XMVector3Normalize(force), 400.f);
	//}

	return OBJ_NOEVENT;
}

void CStarBlockPiece::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	if (m_pRigidBodyCom->Is_Activated())
	{
		m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
		//m_pRigidBodyCom->Add_Torque(m_fTurnSpeed);

		m_fLifeTime += m_fTimeDelta;
		if (m_fLifeTime >= m_fLifeTimeMax)
		{
			m_bDead = true;
		}
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CStarBlockPiece::Render()
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

HRESULT CStarBlockPiece::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CStarBlockPiece::Render_IMGUI()
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


void CStarBlockPiece::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	/*_float3 force = _float3{ 0.5f, 3.f , 0.5f };
	m_pRigidBodyCom->Kick_RigidBody(XMVector3Normalize(force), 530.f);*/


}

HRESULT CStarBlockPiece::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	if (rand() % 2 == 0)
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockPiece"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	else
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockPieceStar"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}

	/* For.Com_RigidBody */
	CRigidBody::RIGIDBODY_DESC rigidDesc {};
	rigidDesc.bTrigger = false;
	rigidDesc.bDynamic = true;
	rigidDesc.bKinematic = false;
	rigidDesc.eShapeType = RIGID_BOX;
	rigidDesc.fOffsetSize = { 0.1f, 0.1f, 0.1f };
	rigidDesc.vMaterial = _float3(0.5f, 0.5f, 0.05f);
	rigidDesc.fDensity = 30.f;
	rigidDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
		TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
	CHECK_FAILED(hr);
	m_pRigidBodyCom->Set_Object(this);
	m_pRigidBodyCom->Activate(true);

	return S_OK;
}

HRESULT CStarBlockPiece::Bind_ShaderResources()
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

CStarBlockPiece* CStarBlockPiece::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStarBlockPiece* pInstance = new CStarBlockPiece(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CStarBlockPiece"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStarBlockPiece::Clone(void* pArg)
{
	CStarBlockPiece* pInstance = new CStarBlockPiece(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CStarBlockPiece"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStarBlockPiece::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidBodyCom);
} 

