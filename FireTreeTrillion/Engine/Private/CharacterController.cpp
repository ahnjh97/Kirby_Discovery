#include "CharacterController.h"

#include "GameObject.h"
#include "GameInstance.h"
#include "PhysX.h"

CCharacterController::CCharacterController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice, pContext)
{
}

CCharacterController::CCharacterController(const CCharacterController& rhs)
	: CComponent(rhs)

{
}

HRESULT CCharacterController::Initialize(void* pArg)
{
	CONTROLLER_DESC* pDes = (CONTROLLER_DESC*)pArg;
	_float4 vInitialPos = pDes->vInitialPos;
	m_eCollisionType = (COLLISION_TYPE)pDes->uCollisionType;
	__super::Initialize(pArg);

	PxMaterial* material = m_pGameInstance->Get_Physics()->createMaterial(0.5f, 0.5f, 0.5f);
	m_tControllerDesc.material = material;
	m_tControllerDesc.upDirection = { 0.f, 1.f, 0.f };
	m_tControllerDesc.density = 100.f;
	m_tControllerDesc.position = PxExtendedVec3(vInitialPos.x, vInitialPos.y, vInitialPos.z);
	m_tControllerDesc.userData = this;//m_pObject;
	m_ControllerFilters.mFilterData = &m_tFilterDesc;

	Set_DefaultValue();

	Create_Controller();

	return S_OK;
}

void CCharacterController::Start_Tick()
{
	m_pGameInstance->AddActor(*m_pController->getActor());
	if (m_pObject != nullptr)
		Set_FootPosition(m_pObject->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION));
}

void CCharacterController::Render_IMGUI()
{
	__super::Render_IMGUI();

	ImGui::Text("FallVelocity : %.2f", (_float)m_fFallVelocity);

	ImGui::InputFloat("Radius", &m_tControllerDesc.radius);
	ImGui::InputFloat("height", &m_tControllerDesc.height);
	ImGui::InputFloat("contactOffset", &m_tControllerDesc.contactOffset);
	ImGui::InputFloat("density", &m_tControllerDesc.density);
	ImGui::InputFloat("slopeLimit", &m_fSlopeLimitDegree);
	m_tControllerDesc.slopeLimit = cosf(XMConvertToRadians(m_fSlopeLimitDegree));
	ImGui::InputFloat("stepOffset", &m_tControllerDesc.stepOffset);
	ImGui::InputFloat("maxJumpHeight", &m_tControllerDesc.maxJumpHeight);


	//ReCreate this Controller (for change shape or scale)
	if (ImGui::Button("Update Changes"))
	{
		Create_Controller();
		if (!Is_Activated())
			m_pGameInstance->AddActor(*m_pController->getActor());
	}
}

void CCharacterController::Set_Position(const _float4& vPos)
{
	m_pController->setPosition({(_double)vPos.x, (_double)vPos.y, (_double)vPos.z});
}

void CCharacterController::Set_FootPosition(const _float4& vPos)
{
	m_pController->setFootPosition({(_double)vPos.x, (_double)vPos.y, (_double)vPos.z});
}

_float4 CCharacterController::Get_Position()
{
	const auto vPos = m_pController->getPosition();
	return _float4{(_float)vPos.x, (_float)vPos.y, (_float)vPos.z, 1.f};
}

_float4 CCharacterController::Get_FootPosition()
{
	const auto vPos = m_pController->getFootPosition();
	return _float4{(_float)vPos.x, (_float)vPos.y, (_float)vPos.z, 1.f};
}

/// <summary> 객체의 Look방향으로 '이동'하는 함수 </summary>
/// <param name="pTransform"> 객체의 Transform </param>
/// <param name="fSpeed"> 이동 속도 </param>
void CCharacterController::Move(CTransform* pTransform, _float fSpeed, _float fTimeDelta)
{
	PxVec3 movement(0.f);
	_vector vLook = pTransform->Get_State_Vector(CTransform::STATE_LOOK);
	movement += CUtils::To_PxVec3(XMVector3Normalize(vLook)) * fSpeed * fTimeDelta;

	PxControllerFilters filter;
	PxControllerCollisionFlags collisionFlags = m_pController->move(movement, 0.001f, fTimeDelta, filter);

	PxExtendedVec3 pxPos = m_pController->getPosition();
	PxVec3 pos ((_float)pxPos.x,(_float)pxPos.y,(_float)pxPos.z);
	_vector xmPos = XMVectorSet(pos.x, pos.y - m_fOffset, pos.z, 0.f);

	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
}

void CCharacterController::Move_Dir(CTransform* pTransform, _fvector fDelta, _float fTimeDelta)
{
	PxVec3 movement(0.f);
	movement += CUtils::To_PxVec3(fDelta);

	PxControllerFilters filter;
	PxControllerCollisionFlags collisionFlags = m_pController->move(movement, 0.001f, fTimeDelta, filter);

	PxExtendedVec3 pxPos = m_pController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

	_vector xmPos = XMVectorSet(pos.x, pos.y - m_fOffset, pos.z, 0.f);

	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
}

/// <summary> 짬푸 </summary>
/// <param name="pTransform"> 객체의 Transform </param>
/// <param name="fFallVelocity"> 떨어지는 속도 </param>
/// <returns> 점프 상태 유무값 </returns>
_bool CCharacterController::Jump(CTransform* pTransform, _float fFallVelocity, _float fTimeDelta)
{
	// 이동
	PxVec3 moveVector = PxVec3(0.f, fFallVelocity, 0.f) * fTimeDelta;
	PxControllerCollisionFlags collisionFlags = m_pController->move(moveVector, 0.001f, fTimeDelta, PxControllerFilters());

	// 객체의 충돌 상태 받아오기
	PxControllerState m_pPxState;
	m_pController->getState(m_pPxState);

	// 지면 판정, 천장 판정 처리
	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN || m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
	{
		fFallVelocity = 0.f;
		return false;
	}

	// 객체의 위치 받아오기
	PxExtendedVec3 pxPos = m_pController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

	// 객체 FOOT POSITION 조정 using OFFSET
	_vector xmPos = XMVectorSet(pos.x, pos.y - m_fOffset, pos.z, 0.f);

	// 객체 위치 지정
	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));

	return true;
}

/// <summary> 자 유 낙 하 </summary>
void CCharacterController::FreeFall(CTransform* pTransform, _float fTimeDelta, _float fOffset)
{
	// 자유낙하용 velocity
	m_fFallVelocity -= GRAVITY * fTimeDelta * fOffset;

	PxVec3 moveVector = PxVec3(0.f, m_fFallVelocity, 0.f) * fTimeDelta;

	PxControllerCollisionFlags collisionFlags = m_pController->move(moveVector, 0.001f, fTimeDelta, PxControllerFilters());

	PxControllerState m_pPxState;
	m_pController->getState(m_pPxState);

	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN || m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
	{
		m_fFallVelocity = 0.f;
		return;
	}

	PxExtendedVec3 pxPos = m_pController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

	_vector xmPos = XMVectorSet(pos.x, pos.y - m_fOffset, pos.z, 0.f);

	pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
}

/// <summary>
/// 해당 character가 서있는 지면의 노말벡터의 평균값을 구하여 뱉는다.
/// </summary>
/// <param name="pTransform"> 해당 character의 트랜스폼 </param>
/// <returns> 지면의 노말벡터 </returns>
PxVec3 CCharacterController::Compute_Slope(CTransform* pTransform)
{
	PxExtendedVec3 position = m_pController->getPosition();
	PxVec3 rayOrigin = PxVec3((_float)position.x, (_float)position.y, (_float)position.z);

	_vector vRight = pTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	vRight = XMVector3Normalize(vRight);
	_vector vLook = pTransform->Get_State_Vector(CTransform::STATE_LOOK);
	vLook = XMVector3Normalize(vLook);

	PxVec3 right = CUtils::To_PxVec3(vRight * 0.5f);
	PxVec3 look = CUtils::To_PxVec3(vLook * 0.5f);

	// 객체 중심 위치에서 동서남북방향으로 살짝 움직인 position을 지정
	PxVec3 rayOriginRight = rayOrigin + right;	// 오른쪽, 왼쪽 레이캐스트
	PxVec3 rayOriginLeft  = rayOrigin - right;
	PxVec3 rayOriginFront = rayOrigin + look;	// 앞, 뒤 레이캐스트
	PxVec3 rayOriginBack  = rayOrigin - look;

	PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
	_float fMaxDistance = 1.f;
	
	PxVec3	normal(0.f);
	normal += TerrainRayCast_Collision(rayOriginRight, rayDirection, fMaxDistance);
	normal += TerrainRayCast_Collision(rayOriginLeft,  rayDirection, fMaxDistance);
	normal += TerrainRayCast_Collision(rayOriginFront, rayDirection, fMaxDistance);
	normal += TerrainRayCast_Collision(rayOriginBack,  rayDirection, fMaxDistance);
	
	normal.normalize();
	return normal;
}

/// <summary>
/// 지면으로 부터의 높이를 계산
/// </summary>
/// <param name="pTransform"></param>
/// <returns></returns>
_float CCharacterController::Compute_Height()
{
	PxExtendedVec3 position = m_pController->getPosition();
	PxVec3 rayOrigin = PxVec3((_float)position.x, (_float)position.y, (_float)position.z);

	PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
	_float fMaxDistance = 10.f;

	_float fHeight = { 0.f };
	PxRaycastHit hit;
	PxRaycastBuffer hitBuffer;
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

	_bool isRayCast = m_pGameInstance->Get_Scene()->raycast(rayOrigin, rayDirection, fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);

	if (isRayCast)
	{
		// 첫 번째 히트 결과
		hit = hitBuffer.block;
		fHeight = rayOrigin.y - hit.position.y;
	}
	else
		return 10.f;

	return fHeight;
}

PxVec3 CCharacterController::Compute_TerrainPosition()
{
	PxExtendedVec3 position = m_pController->getPosition();
	PxVec3 rayOrigin = PxVec3((_float)position.x, (_float)position.y, (_float)position.z);

	PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
	_float fMaxDistance = 10.f;

	PxRaycastHit hit;
	PxRaycastBuffer hitBuffer;
	PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

	_bool isRayCast = m_pGameInstance->Get_Scene()->raycast(rayOrigin, rayDirection, fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
	// 충돌이 발생한 경우 법선 벡터 반환
	if (isRayCast && hitBuffer.hasBlock)
	{
		hit = hitBuffer.block;
		return hit.position;
	}
	else
		return PxVec3(0.0f, 0.0f, 0.0f);
}

/// <summary>
/// 1. 'Character의 특정 위치'에서 지면으로부터 rayCast를 실행한다.
/// 2. rayCast로 'Terrain의 노말벡터'를 뽑는다.
/// </summary>
/// <param name="_rayOrigin"> Character의 특정 위치 </param>
/// <param name="_rayDirection"> raycast 방향벡터 </param>
/// <param name="_fMaxDistance"> raycast가 실행되는 최대 길이 </param>
/// <returns> Terrain의 노말벡터 </returns>
PxVec3 CCharacterController::TerrainRayCast_Collision(PxVec3 _rayOrigin, PxVec3 _rayDirection, _float _fMaxDistance)
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

// NOT YET
//PxControllerCollisionFlags CCharacterController::Move(_float3 vVelocity, _float fTimeDelta, _float minDist)
//{
//	// Disp == direction * speed * delta(delta 시간 동안의 이동량)
//	vVelocity *= fTimeDelta;
//
//	const physx::PxVec3 vDisp{vVelocity.x, vVelocity.y, vVelocity.z};
//	return m_pController->move(vDisp, minDist, fTimeDelta, m_ControllerFilters);
//}
 
// NOT YET
//PxControllerCollisionFlags CCharacterController::MoveDisp(_float3 vPosDelta, _float fTimeDelta, _float minDist)
//{
//	const physx::PxVec3 vDisp{vPosDelta.x, vPosDelta.y, vPosDelta.z};
//	return m_pController->move(vDisp, minDist, fTimeDelta, m_ControllerFilters);
//}

_bool CCharacterController::Is_Activated()
{
	return m_pController != nullptr && m_pController->getActor()->getScene() != nullptr;
}

void CCharacterController::Activate(_bool _bActive)
{
	if (_bActive)
	{
		if (Is_Activated() == false)
			m_pGameInstance->AddActor(*m_pController->getActor());
	}
	else
	{
		if (Is_Activated())
			m_pGameInstance->RemoveActor(*m_pController->getActor());
	}
}

void CCharacterController::Get_ShapeInfo(physx::PxCapsuleGeometry& CapsuleGeo, physx::PxTransform& pxTransform)
{
	if (m_pController)
	{
		PxShape* shape;
		m_pController->getActor()->getShapes(&shape, 1);
		PxGeometryHolder geomHolder = shape->getGeometry();
		//CapsuleGeo = shape->getGeometry().getType();
		if (geomHolder.getType() == PxGeometryType::eCAPSULE)
		{
			const PxCapsuleGeometry& capsuleGeometry = geomHolder.capsule();
			CapsuleGeo = capsuleGeometry;
		}
		pxTransform = physx::PxShapeExt::getGlobalPose(*shape, *m_pController->getActor());
	}
}

void CCharacterController::Create_Controller()
{
	Release_Controller();

	m_pControllerCallBack = new CControllerBehaviorCallback();
	m_tControllerDesc.behaviorCallback = m_pControllerCallBack;
	m_pControllerHitReport = new CUserControllerHitReport();
	m_tControllerDesc.reportCallback = m_pControllerHitReport;

	m_pController = m_pGameInstance->Get_ControllerManager()->createController(m_tControllerDesc);

	PxShape* shape;
	m_pController->getActor()->getShapes(&shape, 1);
	shape->setSimulationFilterData(physx::PxFilterData{ static_cast<physx::PxU32>(COLLISION_TYPE(m_eCollisionType)), 0, 0, 0 });
	//shape->setQueryFilterData(physx::PxFilterData{static_cast<physx::PxU32>(1), 0, 0, 0});

	//m_pGameInstance->RemoveActor(*m_pController->getActor());

	if (m_pObject != nullptr)
		Set_FootPosition(m_pObject->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION));
}

void CCharacterController::Release_Controller()
{
	//if (nullptr == m_pGameInstance->Get_Scene()) return;

	if (nullptr != m_pController)
	{
		Safe_Delete(m_pControllerCallBack);
		Safe_Delete(m_pControllerHitReport);

		if (m_pController->getActor()->getScene())
			m_pGameInstance->RemoveActor(*m_pController->getActor());
		m_pController->release();
	}
}

void CCharacterController::Set_DefaultValue()
{
	m_tControllerDesc.stepOffset = 0.f;
	m_tControllerDesc.volumeGrowth = 1.0f;
	m_tControllerDesc.upDirection = PxVec3(0, 1, 0);
	PxMaterial* material = m_pGameInstance->Get_Material();
	material = m_pGameInstance->Get_Physics()->createMaterial(0.5f, 0.5f, 0.5f);
	m_tControllerDesc.material = material;

	m_tControllerDesc.radius = 0.5f; // 반지름
	m_tControllerDesc.height = 1.f;	 // 높이
	m_tControllerDesc.contactOffset = 0.1f;
	m_fSlopeLimitDegree = 45.f;     
	m_tControllerDesc.slopeLimit = cosf(XMConvertToRadians(m_fSlopeLimitDegree));
	m_tControllerDesc.stepOffset = 0.1f;
	m_tControllerDesc.maxJumpHeight = 3.f;
}

CCharacterController* CCharacterController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCharacterController*		pInstance = new CCharacterController(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(L"Failed to Created : CCharacterController");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CCharacterController::Clone(void* pArg)
{
	CCharacterController*		pInstance = new CCharacterController(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(L"Failed to Cloned : CCharacterController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCharacterController::Free()
{
	__super::Free();

	Release_Controller();
}

