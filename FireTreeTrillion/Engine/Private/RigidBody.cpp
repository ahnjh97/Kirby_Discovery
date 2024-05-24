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
	//Create_Actor();
	return S_OK;
}

// start_tick은 한번만 실행
// rigidBody를 추가하는 순간 physX에서 작동하는 Actor가 추가된다.
void CRigidBody::Start_Tick()
{
	//__super::Start_Tick();
	CHECK_NULLPTR(m_pObject);
	m_pActor->setGlobalPose(physx::PxTransform{CUtils::To_Float4x4(m_pObject->Get_TransformCom()->Get_WorldMatrix())});
	m_pGameInstance->Get_Scene()->addActor(*m_pActor);
}

void CRigidBody::Update(CTransform* pTransform)
{
	if (m_bTrigger)
	{
		Set_PxWorldMatrix(pTransform->Get_WorldFloat4x4());
	}
	else if (m_bKinematic && Is_Activated())
	{
		m_pActor->setKinematicTarget(physx::PxTransform{CUtils::To_Float4x4(pTransform->Get_WorldFloat4x4())});
	}
}

void CRigidBody::Update(_fmatrix matrix)
{
	if (m_bTrigger)
	{
		Set_PxWorldMatrix(matrix);
	}
	else if (m_bKinematic && Is_Activated())
	{
		m_pActor->setKinematicTarget(physx::PxTransform{CUtils::To_Float4x4(matrix)});
	}
}

void CRigidBody::Update_PhysX(CTransform* pTransform)
{
	if (Is_Activated() == false) return;

	if (false == m_bKinematic && false == m_bTrigger)
	{
		pTransform->Set_WorldMatrix(Get_PxWorldMatrix());
	}
}

void CRigidBody::Render_IMGUI()
{
	__super::Render_IMGUI();

	//ReCreateActor(for change shape or scale)
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

void CRigidBody::Create_Actor()
{
	Release_Actor();

	auto pPhysics = m_pGameInstance->Get_Physics();
	auto pxMat = CUtils::To_Float4x4(_float4x4::Identity);
	if (m_pObject != nullptr)
		pxMat = CUtils::To_Float4x4(m_pObject->Get_TransformCom()->Get_WorldMatrix());
	auto pMtrl = m_pGameInstance->Get_Material();
	m_pActor = pPhysics->createRigidDynamic(physx::PxTransform{pxMat});

	_matrix OriginMatrix = m_OriginTransformMatrix;
	_float3 vScale = _float3(XMVectorGetX(XMVector3Length(OriginMatrix.r[0])),
							 XMVectorGetX(XMVector3Length(OriginMatrix.r[1])),
							 XMVectorGetX(XMVector3Length(OriginMatrix.r[2])));

	switch (m_eShapeType)
	{
	case TYPE_BOX:
		m_pShape = pPhysics->createShape(physx::PxBoxGeometry(0.5f * vScale.x, 0.5f * vScale.y, 0.5f * vScale.z), *pMtrl);
		break;
	case TYPE_SPHERE:
		m_pShape = pPhysics->createShape(physx::PxSphereGeometry(0.5f * vScale.x), *pMtrl);
		break;
	case TYPE_CAPSULE:
		m_pShape = pPhysics->createShape(physx::PxCapsuleGeometry(0.5f * vScale.x, 1.f * vScale.y), *pMtrl);
		break;
	case TYPE_END:
		[[fallthrough]];
	default:
		NODEFAULT;
	}

	SetUp_Actor();

	m_pActor->attachShape(*m_pShape);
	physx::PxRigidBodyExt::updateMassAndInertia(*m_pActor, m_fDensity);
}

void CRigidBody::SetUp_Actor()
{
	if (m_bTrigger)
	{
		m_pActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	}
	else
	{
		m_pActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, m_bKinematic);
	}
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

	m_pShape->setSimulationFilterData(physx::PxFilterData{ static_cast<physx::PxU32>(0/*ColliderType*/), 0, 0, 0 });
	m_pShape->setQueryFilterData(physx::PxFilterData{static_cast<physx::PxU32>(1), 0, 0, 0});

	_matrix RemoveScaleOriginMatrix = m_OriginTransformMatrix;
	RemoveScaleOriginMatrix.r[0] = XMVector3Normalize(RemoveScaleOriginMatrix.r[0]);
	RemoveScaleOriginMatrix.r[1] = XMVector3Normalize(RemoveScaleOriginMatrix.r[1]);
	RemoveScaleOriginMatrix.r[2] = XMVector3Normalize(RemoveScaleOriginMatrix.r[2]);

	physx::PxTransform relativePose(CUtils::To_Float4x4(RemoveScaleOriginMatrix));
	m_pShape->setLocalPose(relativePose);
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
		if (m_pActor->getScene() == nullptr)
			m_pGameInstance->AddActor(*m_pActor);
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

	// Trigger는 어떻게 사용할 지 추후 의논 예정
	if (false == (m_bKinematic))
	{
		physx::PxVec3 PxForce = physx::PxVec3(vForce.x, vForce.y, vForce.z);
		m_pActor->addForce(PxForce, physx::PxForceMode::eFORCE);
	}
}

void CRigidBody::PlayerController(_float3 vPos)
{
	PxCapsuleControllerDesc capsuleDesc;
	capsuleDesc.position = PxExtendedVec3(vPos.x, vPos.y, vPos.z);
	capsuleDesc.radius = 0.5f; // 반지름
	capsuleDesc.height = 0.1f; // 높이
	capsuleDesc.stepOffset = 0.f;
	capsuleDesc.volumeGrowth = 1.0f;
	capsuleDesc.slopeLimit = cosf(XMConvertToRadians(15.f)); // 15 degrees
	capsuleDesc.upDirection = PxVec3(0, 1, 0);
	capsuleDesc.contactOffset = 0.001f;
	PxMaterial* material = m_pGameInstance->Get_Material();
	material = m_pGameInstance->Get_Physics()->createMaterial(0.5f, 0.5f, 0.5f);
	capsuleDesc.material = material;

	// 캡슐 컨트롤러 생성
	m_pCapsuleController = m_pGameInstance->Get_ControllerManager()->createController(capsuleDesc);
	if (nullptr == m_pCapsuleController)
		MSG_BOX(TEXT("Failed to Controller"));
}
   
void CRigidBody::Go_Straight(CTransform* pTransform, _float fSpeed, _float fTimeDelta)
{
	PxVec3 movement(0.f);
	_vector vLook = pTransform->Get_State_Vector(CTransform::STATE_LOOK);
	movement += CUtils::To_PxVec3(XMVector3Normalize(vLook)) * fSpeed * fTimeDelta;
                
	PxControllerFilters filter;
	PxControllerCollisionFlags collisionFlags = m_pCapsuleController->move(movement, 0.001f, fTimeDelta, filter);

	PxExtendedVec3 pxPos = m_pCapsuleController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

	_vector xmPos = XMVectorSet(pos.x, pos.y - 0.5f, pos.z, 0.f);

	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
}

//[JS]
_bool CRigidBody::Jump(CTransform* pTransform, _float fFallVelocity, _float fTimeDelta)
{
	//fFallVelocity -= 9.81f * fTimeDelta;

	if (m_fFallVelocity < -1000.f)
		m_fFallVelocity = -10.f;

	PxVec3 moveVector = PxVec3(0.f, fFallVelocity, 0.f) * fTimeDelta;
	PxControllerCollisionFlags collisionFlags = m_pCapsuleController->move(moveVector, 0.001f, fTimeDelta, PxControllerFilters());
	//Go_Straight(pTransform, 5.f, fTimeDelta);

	PxControllerState m_pPxState;

	m_pCapsuleController->getState(m_pPxState);

	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN || m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
	{
		fFallVelocity = 0.f;
		return false;
	}

	PxExtendedVec3 pxPos = m_pCapsuleController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);
	_vector xmPos = XMVectorSet(pos.x, pos.y - 0.5f, pos.z, 0.f);

	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));

	return true;
}

void CRigidBody::FreeFall(CTransform* pTransform, _float fTimeDelta)
{
	m_fFallVelocity -= 9.81f * fTimeDelta;

	if (m_fFallVelocity < -1000.f)
		m_fFallVelocity = -10.f;

	PxVec3 moveVector = PxVec3(0.f, m_fFallVelocity, 0.f) * fTimeDelta;
	PxControllerCollisionFlags collisionFlags = m_pCapsuleController->move(moveVector, 0.001f, fTimeDelta, PxControllerFilters());
	//Go_Straight(pTransform, 5.f, fTimeDelta);

	PxControllerState m_pPxState;

	m_pCapsuleController->getState(m_pPxState);

	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN || m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
	{
		m_fFallVelocity = 0.f;
	}

	PxExtendedVec3 pxPos = m_pCapsuleController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

	_vector xmPos = XMVectorSet(pos.x, pos.y - 0.5f, pos.z, 0.f);

	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
}

PxVec3 CRigidBody::Compute_Slope(CTransform* pTransform)
{
	PxExtendedVec3 position = m_pCapsuleController->getPosition();
	PxVec3 rayOrigin = PxVec3((_float)position.x, (_float)position.y, (_float)position.z);

	_vector vRight = pTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	vRight = XMVector3Normalize(vRight);
	_vector vLook = pTransform->Get_State_Vector(CTransform::STATE_LOOK);
	vLook = XMVector3Normalize(vLook);

	PxVec3 right = CUtils::To_PxVec3(vRight * 0.5f);
	PxVec3 look = CUtils::To_PxVec3(vLook * 0.5f);

	// 오른쪽, 왼쪽 레이캐스트
	PxVec3 rayOriginRight = rayOrigin + right;
	PxVec3 rayOriginLeft = rayOrigin - right;

	// 앞, 뒤 레이캐스트
	PxVec3 rayOriginFront = rayOrigin + look;
	PxVec3 rayOriginBack = rayOrigin - look;

	PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
	_float maxDistance = 1.f;

	PxVec3	normal = { 0.f, 0.f, 0.f };

	normal += TerrainRayCast_Collision(rayOriginRight, rayDirection, maxDistance);

	normal += TerrainRayCast_Collision(rayOriginLeft, rayDirection, maxDistance);

	normal += TerrainRayCast_Collision(rayOriginFront, rayDirection, maxDistance);

	normal += TerrainRayCast_Collision(rayOriginBack, rayDirection, maxDistance);
	 
	normal.normalize();
	return normal;

	//_bool isRayCastLeft = m_pGameInstance->Get_Scene()->raycast(rayOriginLeft, rayDirection, maxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
	//// 충돌이 발생한 경우 법선 벡터 반환
	//if (isRayCastLeft && hitBuffer.hasBlock)
	//{
	//	hit = hitBuffer.block;

	//	normal += hit.normal;
	//}

	//_bool isRayCastFront = m_pGameInstance->Get_Scene()->raycast(rayOriginFront, rayDirection, maxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
	//// 충돌이 발생한 경우 법선 벡터 반환
	//if (isRayCastLeft && hitBuffer.hasBlock)
	//{
	//	hit = hitBuffer.block;

	//	normal += hit.normal;
	//}

	//_bool isRayCastBack = m_pGameInstance->Get_Scene()->raycast(rayOriginBack, rayDirection, maxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
	//// 충돌이 발생한 경우 법선 벡터 반환
	//if (isRayCastLeft && hitBuffer.hasBlock)
	//{
	//	hit = hitBuffer.block;

	//	normal += hit.normal;
	//}


	//if (false == isRayCastRight && false == isRayCastLeft)
	//	return PxVec3(0.0f, 1.0f, 0.0f);
	//else
	//{
	//	normal.normalize();
	//	return normal;
	//}
}

PxVec3 CRigidBody::TerrainRayCast_Collision(PxVec3 _rayOrigin, PxVec3 _rayDirection, _float _fMaxDistance)
{
	PxRaycastHit hit;
	PxRaycastBuffer hitBuffer;

	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

	_bool isRayCast = m_pGameInstance->Get_Scene()->raycast(_rayOrigin, _rayDirection, _fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
	// 충돌이 발생한 경우 법선 벡터 반환
	if (isRayCast && hitBuffer.hasBlock)
	{
		hit = hitBuffer.block;

		return hit.normal;
	}
	else
		return PxVec3(0.0f, 1.0f, 0.0f);
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
		MSG_BOX(TEXT("Failed To Created : CRigidBody"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CRigidBody::Clone(void * pArg)
{
	CRigidBody*		pInstance = new CRigidBody(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CRigidBody"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRigidBody::Free()
{
	__super::Free();

	Release_Actor();

	if(nullptr != m_pCapsuleController)
		m_pCapsuleController->release();
}

 