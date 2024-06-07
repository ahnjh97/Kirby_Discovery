#include "RigidBody.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "Transform.h"

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

void CRigidBody::Update(CTransform* pTransform)
{
	if(!m_bKinematic)
		Set_PxWorldMatrix(pTransform->Get_WorldFloat4x4());
	else if (m_bKinematic && Is_Activated())
		m_pActor->setKinematicTarget(physx::PxTransform{CUtils::To_Float4x4(pTransform->Get_WorldFloat4x4())});
}

void CRigidBody::Update(_fmatrix matrix)
{
	if (!m_bKinematic)
		Set_PxWorldMatrix(matrix);
	else if (m_bKinematic && Is_Activated())
		m_pActor->setKinematicTarget(physx::PxTransform{CUtils::To_Float4x4(matrix)});
}

void CRigidBody::Update_PhysX(CTransform* pTransform)
{
	if (Is_Activated() == false) return;

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
		Create_Actor();
		if (!Is_Activated())
			m_pGameInstance->Get_Scene()->addActor(*m_pActor);
	}
	ImGui::Checkbox("bTrigger",		&m_bTrigger);
	ImGui::Checkbox("bKinematic",	&m_bKinematic);
	ImGui::InputFloat("Density",	&m_fDensity);

	ImGui::Indent(20.f);
	if (ImGui::CollapsingHeader("Origin Trasnform"))
	{
	}
	ImGui::Unindent(20.f);
}

#endif

void CRigidBody::Create_Actor()
{
	Release_Actor();

	auto pPhysics = m_pGameInstance->Get_Physics();
	
	// scale 긁어오기
	_matrix OriginMatrix = m_OriginTransformMatrix;
	_float3 vScale = _float3();
	if (m_fOffsetSize != _float())
	{
		vScale = _float3(m_fOffsetSize, m_fOffsetSize, m_fOffsetSize);
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

	PxFilterData filterData;
	filterData.word0 = 1;  // 충돌 그룹 설정
	filterData.word1 = 1;  // 이 값과 충돌할 그룹을 설정
	//m_pShape->setSimulationFilterData(filterData);
	m_pShape->setSimulationFilterData(physx::PxFilterData{ 1, 1, 0, 0 });

	// 쿼리 필터 데이터 설정
	PxFilterData queryFilterData;
	queryFilterData.word0 = 1;  // 쿼리 그룹 설정
	queryFilterData.word1 = 1;  // 쿼리 그룹 설정
	//m_pShape->setQueryFilterData(queryFilterData);
	m_pShape->setQueryFilterData(physx::PxFilterData{ 1, 1, 0, 0 });

	// Transform 설정 (위치와 회전)
	PxMat44 pxMat = CUtils::To_Float4x4(OriginMatrix);
	PxTransform transform = CUtils::mat44ToTransform(pxMat);
	if (m_bDynamic)
	{
		m_pActor = pPhysics->createRigidDynamic(transform);
		SetUp_Actor();

		m_pActor->attachShape(*m_pShape);
		physx::PxRigidBodyExt::updateMassAndInertia(*m_pActor, m_fDensity);
	}
	else 
	{
		m_pStaticActor = pPhysics->createRigidStatic(transform);
		SetUp_Actor();
		m_pStaticActor->attachShape(*m_pShape);
	}
}

//정현아 여길 봐줘
/// <summary>
/// RigidBody를 어떻게 사용할 것인지 세팅 플래그 변경하는 함수
/// +) Create_Actor의 하단에 호출중
/// </summary>
void CRigidBody::SetUp_Actor()
{
	if (m_bTrigger)
	{
		if(true == m_bDynamic)
			m_pActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	}
	else
	{
		m_pActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, m_bKinematic);
	}

	if(nullptr != m_pActor)
		m_pActor->userData = this;

	if (m_bTrigger)
	{
		m_pShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		m_pShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		m_pShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	else
	{
		m_pShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		m_pShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		m_pShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
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
		if (m_pActor->getScene())
			m_pGameInstance->RemoveActor(*m_pActor);
	}
}

void CRigidBody::Add_Force(_float3 vForce)
{
	if (m_pActor == nullptr) return;

	if (false == m_bKinematic && false == m_bTrigger)
	{
		physx::PxVec3 PxForce = physx::PxVec3(vForce.x, vForce.y, vForce.z);
		m_pActor->addForce(PxForce, physx::PxForceMode::eFORCE);
	}
}

physx::PxTransform CRigidBody::Get_PxTransform()
{
	return physx::PxShapeExt::getGlobalPose(*m_pShape, *m_pActor);
}

// 현 actor의 physX에서의 행렬을 지정해준다.
void CRigidBody::Set_PxWorldMatrix(const _float4x4& _worldMatrix)
{
	m_pActor->setGlobalPose(physx::PxTransform{CUtils::To_Float4x4(_worldMatrix)});
	
}

// physX에서의 행렬을 DX에서의 행렬로 변환하여 가져온다.
_float4x4 CRigidBody::Get_PxWorldMatrix()
{
	physx::PxMat44 pos(physx::PxShapeExt::getGlobalPose(*m_pShape, *m_pActor));
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

 