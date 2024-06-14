#include "stdafx.h"
#include "Trigger.h"
#include "GameInstance.h"

CTrigger::CTrigger(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CTrigger::CTrigger(const CTrigger & rhs)
	: CPhysXObject( rhs )
{
}

HRESULT CTrigger::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger::Initialize(void * pArg)
{
	TRIGGER_DESC tTriggerDesc{};
	if (nullptr != pArg) {
		tTriggerDesc = *(TRIGGER_DESC*)pArg;
		m_eTriggerType		= TRIGGER(tTriggerDesc.iTriggerType);
		m_iTriggerIndex		= tTriggerDesc.iTriggerIndex;
		m_eCollisionGroup	= tTriggerDesc.eCollisionGroup;
		m_vSize				= tTriggerDesc.vTriggerSize;
		m_vInitialPos		= tTriggerDesc.vInitialPos;
	}

	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;	

	if (m_eTriggerType == TRIGGER_HITBOX)
		m_pRigidBodyCom->Set_PxWorldMatrix(m_vInitialPos);

	return S_OK;
}

_int CTrigger::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	// HITBOX용 트리거일경우 받아온 객체의 위치의 look방향의 앞쪽으로 따라다닌다.
	if ((m_eTriggerType == TRIGGER_HITBOX))
	{
		_float4 vRight = XMVector3Normalize(m_pOwnerTransform->Get_State_Float4(CTransform::STATE_RIGHT)) * (-0.05f);
		_float4 vLook = XMVector3Normalize(m_pOwnerTransform->Get_State_Float4(CTransform::STATE_LOOK)) * 0.8f;
		_float4 vPos = m_pOwnerTransform->Get_State_Float4(CTransform::STATE_POSITION) + vRight;

		_float4 vNewPos = vLook + _float4(vPos.x, vPos.y + 1.f, vPos.z, 1.f);
		m_pRigidBodyCom->Set_PxWorldMatrix(m_pOwnerTransform->Get_WorldFloat4x4());
	}
	else if (m_eTriggerType == TRIGGER_ITEM)
	{
		_float4 vPos = m_pOwnerTransform->Get_State_Float4(CTransform::STATE_POSITION);
		_float4 vNewPos = _float4(vPos.x, vPos.y + 1.f, vPos.z, 1.f);
		m_pRigidBodyCom->Set_PxWorldMatrix(m_pOwnerTransform->Get_WorldFloat4x4());
	}

	return OBJ_NOEVENT;
}

void CTrigger::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_TOOL_MAP)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTrigger::Render()
{
	if (m_eTriggerType == (TRIGGER_HITBOX || TRIGGER_ITEM))	return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pShaderCom->Begin(MODEL_TRIGGER))) // Trigger
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

#ifdef _DEBUG
void CTrigger::Render_IMGUI()
{
	//// Guizmo
	//_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
	//m_pGameInstance->EditTransform(matWorld);
	//m_pTransformCom->Set_WorldMatrix(matWorld);
	//
	//ImGui::Separator(); ImGui::NewLine();
}
#endif

void CTrigger::Collision_Overlap(CGameObject* pGameObject)
{
	// HitBox 충돌 처리
	if(m_bAlive)
		m_pOwner->Collision_Overlap(pGameObject);

	m_bAlive = false;
	m_pRigidBodyCom->Activate(false);
}

void CTrigger::Check_Collision()
{
	m_bAlive = true;
	m_pRigidBodyCom->Activate(true);
}

HRESULT CTrigger::Bind_ShaderResources()
{
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iTriggerType", &m_eTriggerType, sizeof(_uint))))
		return E_FAIL;
	return S_OK;
}

void CTrigger::Set_Owner(CGameObject* pObj)
{
	if (pObj == nullptr) return;

	m_pOwner = pObj;
	m_pOwnerTransform = m_pOwner->Get_TransformCom();
	m_pRigidBodyCom->Set_Object(this);
}

void CTrigger::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	CPhysXObject* pOBJ = static_cast<CPhysXObject*>(m_pOwner);
	pOBJ->Collision(eContent, pObject);
}

HRESULT CTrigger::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	if (m_eTriggerType != TRIGGER_HITBOX)
	{
		if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_Trigger"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
			return E_FAIL;
	}

	/* For.Com_RigidBody */
	if (m_eTriggerType == TRIGGER_CAM || m_eTriggerType == TRIGGER_SHADER || m_eTriggerType == TRIGGER_STAR)
	{
		CRigidBody::RIGIDBODY_DESC tRigidDesc(RIGID_BOX, m_pTransformCom->Get_WorldMatrix(), true, false);
		if (FAILED(__super::Add_Component(TEXT("Prototype_Component_RigidBody"),
			TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &tRigidDesc)))
			return E_FAIL;
		m_pRigidBodyCom->SetUp_TriggerType(m_eTriggerType);
		m_pRigidBodyCom->SetUp_TriggerIndex(m_iTriggerIndex);
	}
	else if (m_eTriggerType == TRIGGER_HITBOX)
	{
		CRigidBody::RIGIDBODY_DESC tRigidDesc;
		tRigidDesc.eShapeType = RIGID_BOX;
		tRigidDesc.matWorld = m_pTransformCom->Get_WorldMatrix();
		tRigidDesc.bTrigger = true;
		tRigidDesc.bDynamic = false;
		tRigidDesc.bKinematic = false;
		tRigidDesc.fOffsetSize = m_vSize;// _float3{ 1.f, 1.5f, 1.f };
		if(FAILED(__super::Add_Component(TEXT("Prototype_Component_RigidBody"),
										 TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &tRigidDesc)))
			return E_FAIL;
		m_pRigidBodyCom->Activate(false);
		m_pRigidBodyCom->SetUp_TriggerType(m_eTriggerType);
		m_pRigidBodyCom->SetUp_TriggerIndex(m_iTriggerIndex);
	}
	else if (m_eTriggerType == TRIGGER_ITEM)
	{
		CRigidBody::RIGIDBODY_DESC tRigidDesc;
		tRigidDesc.eShapeType = RIGID_BOX;
		tRigidDesc.matWorld = m_pTransformCom->Get_WorldMatrix();
		tRigidDesc.bTrigger = true;
		tRigidDesc.bDynamic = false;
		tRigidDesc.bKinematic = false;
		tRigidDesc.fOffsetSize = m_vSize;// _float3{ 1.f, 1.5f, 1.f };
		if (FAILED(__super::Add_Component(TEXT("Prototype_Component_RigidBody"),
			TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &tRigidDesc)))
			return E_FAIL;
		m_pRigidBodyCom->Activate(true);
		m_pRigidBodyCom->SetUp_TriggerType(m_eTriggerType);
		m_pRigidBodyCom->SetUp_TriggerIndex(m_iTriggerIndex);
	}

	return S_OK;
}

CTrigger * CTrigger::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTrigger*		pInstance = new CTrigger(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTrigger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CTrigger::Clone(void * pArg)
{
	CTrigger*		pInstance = new CTrigger(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CTrigger"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrigger::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);	
	if (m_eTriggerType != TRIGGER_HITBOX)
		Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidBodyCom);
}

