#include "stdafx.h"
#include "KickableRock.h"
#include "HitBox.h"

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
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;

	HRESULT  hr = __super::Initialize(pGameObjectDesc);
	CHECK_FAILED(hr);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, CUtils::Get_State_Vector_Matrix(pGameObjectDesc->matWorld, CUtils::STATE_POSITION));
	Add_Components();

	return S_OK;
}

_int CKickableRock::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (true == m_bDead)
		return Ready_Dead(0.9f);

	if (m_pGameInstance->Get_DIKeyState(DIK_4, KEY_DOWN))
	{
		m_pRigidBodyCom->Activate(true);

		_float3 force = _float3{ 0.5f, 3.f , 0.5f };
		m_pRigidBodyCom->Kick_RigidBody(XMVector3Normalize(force), 480.f);

		// 힘이 한번만 작용되게 한다.
		m_bLockCollision = true;
	}

	return OBJ_NOEVENT;
}

void CKickableRock::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	if (m_bLockCollision == true)
	{
		m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
		m_pRigidBodyCom->Add_Force(_float3(0.f, -0.5f, 0.f));

		m_fLifeTime += m_fTimeDelta;
		if (m_fLifeTime >= 1.5f)
		{
			m_fLifeTime = 0.f;
			m_bDead = true;
		}
	}

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


void CKickableRock::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (m_bLockCollision == false)
	{
		m_pRigidBodyCom->Activate(true);
		CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
		_float4 vPlayerPos = static_cast<CTransform*>(pPlayer->Get_TransformCom())->Get_State(CTransform::STATE_POSITION);
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		_float3 vDir = vPos - vPlayerPos;
		vDir.y = 0.f;
		vDir.Normalize();
		vDir.y += 1.f;
		vDir.Normalize();
		_float3 force = vDir;
		m_pRigidBodyCom->Kick_RigidBody(XMVector3Normalize(force), 480.f);

		// 힘이 한번만 작용되게 한다.
		m_bLockCollision = true;
	}
}

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
	rigidDesc.fOffsetSize = { 0.5f, 0.5f, 0.5f };
	rigidDesc.vMaterial = _float3(10.f, 1.f, 0.85f);
	rigidDesc.fDensity = 800.f;
	rigidDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
		TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
	CHECK_FAILED(hr);
	m_pRigidBodyCom->Set_Object(this);
	m_pRigidBodyCom->Activate(false);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 0.f, 1.f);


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

