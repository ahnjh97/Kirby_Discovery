#include "RigidBody.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "Transform.h"

#define OVERLAP_MAX 8

CRigidBody::CRigidBody(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent{ pDevice, pContext }
{
}

CRigidBody::CRigidBody(const CRigidBody & rhs)
	: CComponent( rhs )
{
}

HRESULT CRigidBody::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRigidBody::Initialize(void * pArg)
{
	RIGIDBODY_DESC* pDesc = (RIGIDBODY_DESC*)pArg;
	m_bTrigger				= pDesc->bTrigger;
	m_eShapeType			= pDesc->eShapeType;
	m_OriginTransformMatrix = pDesc->matWorld;
	m_vMaterial				= pDesc->vMaterial;
	m_fOffsetSize			= pDesc->fOffsetSize;
	m_bDynamic				= pDesc->bDynamic;
	m_bKinematic			= pDesc->bKinematic;
	m_pActorObject			= pDesc->pObj;

	Create_Actor();
	return S_OK;
}

void CRigidBody::Update(_fmatrix matrix)
{
	if (!m_bKinematic)
		Set_PxWorldMatrix(matrix);
	else if (m_bKinematic && Is_Activated())
		m_pActor->setKinematicTarget(PxTransform{CUtils::To_Float4x4(matrix)});
}

void CRigidBody::Update(_float4 vPos)
{
	if (m_bKinematic && Is_Activated())
		m_pActor->setKinematicTarget(PxTransform{ vPos.x, vPos.y, vPos.z });
}

void CRigidBody::Update_PhysX(CTransform* pTransform)
{
	if (false == m_bKinematic && false == m_bTrigger)
	{
		pTransform->Set_WorldMatrix(Get_PxWorldMatrix());
	}
}

#ifdef _DEBUG

void CRigidBody::Render_IMGUI()
{
	__super::Render_IMGUI();

	// ReCreateActor(for change shape or scale)
	if (ImGui::Button("Update Changes"))
	{
		PxTransform globalPose = m_pActor->getGlobalPose();
		PxMat44 globalPoseMatrix = PxMat44(globalPose);
		m_OriginTransformMatrix = CUtils::To_Float4x4(globalPoseMatrix);
		
		Create_Actor();
		if (!Is_Activated())
			m_pGameInstance->Get_Scene()->addActor(*m_pActor);
	}
	ImGui::Checkbox("bTrigger",		&m_bTrigger);
	ImGui::Checkbox("bKinematic",	&m_bKinematic);
	ImGui::InputFloat("Density",	&m_fDensity);

}

#endif

void CRigidBody::Create_Actor()
{
	Release_Actor();

	auto pPhysics = m_pGameInstance->Get_Physics();
	
	// scale 긁어오기
	_matrix OriginMatrix = m_OriginTransformMatrix;
	_float3 vScale = _float3();
	if (m_fOffsetSize != _float3())
	{
		vScale = m_fOffsetSize;
	}
	else
	{
		vScale = _float3(XMVectorGetX(XMVector3Length(OriginMatrix.r[0])),
						 XMVectorGetX(XMVector3Length(OriginMatrix.r[1])),
						 XMVectorGetX(XMVector3Length(OriginMatrix.r[2])));
	}
	
	PxMaterial* pMtrl = m_pGameInstance->Get_Physics()->createMaterial(m_vMaterial.x, m_vMaterial.y, m_vMaterial.z);
	switch (m_eShapeType)
	{
	case RIGID_BOX:
		m_pShape = pPhysics->createShape(physx::PxBoxGeometry(vScale.x, vScale.y, vScale.z), *pMtrl);
		break;
	case RIGID_SPHERE:
		m_pShape = pPhysics->createShape(physx::PxSphereGeometry(0.5f * vScale.x), *pMtrl);
		break;
	case RIGID_CAPSULE:
		m_pShape = pPhysics->createShape(physx::PxCapsuleGeometry(0.5f * vScale.x, 1.f * vScale.y), *pMtrl);
		break;
	case RIGID_END:
		[[fallthrough]];
	default:
		NODEFAULT;
	}

	m_pShape->setSimulationFilterData(physx::PxFilterData{ 10, 0, 0, 0 });
	m_pShape->setQueryFilterData(physx::PxFilterData{ 10, 0, 0, 0 });

	// Transform 설정 (위치와 회전)
	PxMat44 pxMat = CUtils::To_Float4x4(OriginMatrix);
	PxTransform transform = CUtils::mat44ToTransform(pxMat);
	if (m_bDynamic)
	{
		m_pActor = pPhysics->createRigidDynamic(transform);
		SetUp_Actor();
		m_pActor->attachShape(*m_pShape);
		PxScene* scene = m_pGameInstance->Get_Scene();
		scene->addActor(*m_pActor);

		physx::PxRigidBodyExt::updateMassAndInertia(*m_pActor, m_fDensity);
	}
	else 
	{
		m_pStaticActor = pPhysics->createRigidStatic(transform);
		SetUp_Actor();

		m_pStaticActor->attachShape(*m_pShape);
		PxScene* scene = m_pGameInstance->Get_Scene();
		scene->addActor(*m_pStaticActor);
	}
}

/// <summary>
/// RigidBody를 어떻게 사용할 것인지 세팅 플래그 변경하는 함수
/// +) Create_Actor의 하단에 호출중
/// </summary>
void CRigidBody::SetUp_Actor()
{
	if (m_bTrigger)
	{
		if (m_bDynamic)
			m_pActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	}
	else
	{
		if (m_bDynamic)
			m_pActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, m_bKinematic);
	}

	if(nullptr != m_pActor)
		m_pActor->userData = this;
	if (nullptr != m_pStaticActor)
		m_pStaticActor->userData = this;

	if (m_bTrigger)
	{
		m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE,  false);
		m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE,	   true);
	}
	else
	{
		m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE,  true);
		m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE,	   false);
	}
}

void CRigidBody::Release_Actor()
{
	if (m_pActor)
	{
		m_pActor->userData = nullptr;
		if (m_pActor->getScene())
		{
			auto pScene = m_pGameInstance->Get_Scene();
			pScene->removeActor(*m_pActor);
		}

		if (m_pShape)
		{
			m_pActor->detachShape(*m_pShape);
			m_pShape->release();
			m_pShape = nullptr;
		}

		m_pActor->release();
		m_pActor = nullptr;
	}
	else if (m_pStaticActor)
	{
		m_pStaticActor->userData = nullptr;
		if (m_pStaticActor->getScene())
		{
			auto pScene = m_pGameInstance->Get_Scene();
			pScene->removeActor(*m_pStaticActor);
		}

		if (m_pShape)
		{
			m_pStaticActor->detachShape(*m_pShape);
			m_pShape->release();
			m_pShape = nullptr;
		}

		m_pStaticActor->release();
		m_pStaticActor = nullptr;
	}
}

/// <summary> physX의 RigidBody를 on/off해주는 함수 </summary>
void CRigidBody::Activate(_bool _bActive)
{
	if (_bActive)
	{
		if(m_bDynamic)
			m_pGameInstance->AddActor(*m_pActor);
		else
			m_pGameInstance->AddActor(*m_pStaticActor);

		if (m_bTrigger)
		{
			if (nullptr != m_pActor)
				m_pGameInstance->Register_Trigger(m_pActor, m_iTriggerType,  m_iTriggerIndex);
			else
				m_pGameInstance->Register_Trigger(m_pStaticActor, m_iTriggerType, m_iTriggerIndex);
		}
	}
	else
	{
		if (nullptr != m_pActor)
		{
			if (m_pActor->getScene())
				m_pGameInstance->RemoveActor(*m_pActor);
		}
		else
		{
			if (m_pStaticActor->getScene())
				m_pGameInstance->RemoveActor(*m_pStaticActor);
		}
	}
}

void CRigidBody::Add_Force(_float3 vForce)
{
	if (m_pActor == nullptr) return;

	if (false == m_bKinematic && false == m_bTrigger)
	{
		PxVec3 PxForce = physx::PxVec3(vForce.x, vForce.y, vForce.z);
		m_pActor->addForce(PxForce, physx::PxForceMode::eFORCE);
	}
}

void CRigidBody::Add_Torque(_float3 vTorque)
{
	if (false == (m_bTrigger && m_bKinematic))
	{
		PxVec3 PxToque = physx::PxVec3(vTorque.x, vTorque.y, vTorque.z);
		m_pActor->addTorque(PxToque, physx::PxForceMode::eFORCE);
	}
}

void CRigidBody::Add_Velocity(_float3 vVelocity)
{
	if (false == (m_bTrigger && m_bKinematic))
	{
		PxVec3 PxForce = PxVec3(vVelocity.x, vVelocity.y, vVelocity.z);
		m_pActor->addForce(PxForce, physx::PxForceMode::eVELOCITY_CHANGE);
	}
}

PxTransform CRigidBody::Get_PxTransform()
{
	return PxShapeExt::getGlobalPose(*m_pShape, *m_pActor);
}

// 현 actor의 physX에서의 행렬을 지정해준다.
void CRigidBody::Set_PxWorldMatrix(const _float4x4& _worldMatrix)
{
	if(m_pActor != nullptr)
		m_pActor->setGlobalPose(physx::PxTransform{CUtils::To_Float4x4(_worldMatrix)});
	if(m_pStaticActor != nullptr)
		m_pStaticActor->setGlobalPose(physx::PxTransform{CUtils::To_Float4x4(_worldMatrix)});
}

// physX에서의 행렬을 DX에서의 행렬로 변환하여 가져온다.
_float4x4 CRigidBody::Get_PxWorldMatrix()
{
	PxMat44 pos(PxShapeExt::getGlobalPose(*m_pShape, *m_pActor));
	return CUtils::To_Float4x4(pos);
}

/// <summary> Actor가 지정되어있으며, 해당 Actor가 존재할 Scene이 있는 경우 True를 반환
///	즉, physX의 영향을 받는 actor로 지정되어있는 경우 true를 반환 </summary>
_bool CRigidBody::Is_Activated()
{
	return m_pActor != nullptr && m_pActor->getScene() != nullptr;
}


CRigidBody * CRigidBody::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CRigidBody*		pInstance = new CRigidBody(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CRigidBody"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CRigidBody::Clone(void * pArg)
{
	CRigidBody*		pInstance = new CRigidBody(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CRigidBody"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRigidBody::Free()
{
	__super::Free();

	Safe_Release(m_pActorObject);
	Release_Actor();
}

