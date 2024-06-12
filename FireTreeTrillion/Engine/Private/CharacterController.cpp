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
	m_tControllerDesc.position = PxExtendedVec3(vInitialPos.x, vInitialPos.y, vInitialPos.z);
	m_eCollisionType = (COLLISION_TYPE)pDes->uCollisionType;
	__super::Initialize(pArg);

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

#ifdef _DEBUG
/// <summary> 속성값을 변화시키고 다시 컨트롤러를 생성한다. </summary>
void CCharacterController::Render_IMGUI()
{
	__super::Render_IMGUI();

	ImGui::Text("FallVelocity : %.2f", (_float)m_fFallVelocity); ImGui::NewLine();

	if (ImGui::Button("Update Changes"))
	{
		Create_Controller();
		if (!Is_Activated())
			m_pGameInstance->AddActor(*m_pController->getActor());
	}

	// 캡슐(컨트롤러)의 질량
	ImGui::InputFloat("density", &m_tControllerDesc.density);
	// 캐릭터가 올라갈 수 있는 최대 계단 높이
	ImGui::InputFloat("stepOffset", &m_tControllerDesc.stepOffset);
	// Material
	ImGui::InputFloat3("Material", (_float*)&m_vMaterialOptions);
	// 캡슐의 반지름
	ImGui::InputFloat("Radius", &m_tControllerDesc.radius);
	// 캡슐의 높이
	ImGui::InputFloat("height", &m_tControllerDesc.height);
	// 경사도
	ImGui::InputFloat("slopeLimit", &m_fSlopeLimitDegree);
	m_tControllerDesc.slopeLimit = cosf(XMConvertToRadians(m_fSlopeLimitDegree));
}
#endif


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
void CCharacterController::Move(CTransform* pTransform, _fvector vPosition, _float fTimeDelta)
{
	PxExtendedVec3 pxCurrentPos = m_pController->getPosition();
	PxVec3 moveVector((_float)pxCurrentPos.x, (_float)pxCurrentPos.y, (_float)pxCurrentPos.z);
	//// 이동
	//PxControllerCollisionFlags collisionFlags = m_pController->move(moveVector, 0.001f, fTimeDelta, PxControllerFilters());

	PxVec3 displacement = CUtils::To_PxVec3(vPosition) - moveVector;

	PxControllerFilters filter;
	PxControllerCollisionFlags collisionFlags = m_pController->move(displacement, 0.001f, fTimeDelta, filter);

	PxExtendedVec3 pxPos = m_pController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

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

_bool CCharacterController::Jump_Parabola(CTransform* pTransform, _fvector vGoPos, _float fTimeDelta)
{
	PxExtendedVec3 pxCurrentPos = m_pController->getPosition();
	PxVec3 moveVector((_float)pxCurrentPos.x, (_float)pxCurrentPos.y, (_float)pxCurrentPos.z);
	//// 이동
	//PxControllerCollisionFlags collisionFlags = m_pController->move(moveVector, 0.001f, fTimeDelta, PxControllerFilters());

	PxVec3 displacement = CUtils::To_PxVec3(vGoPos) - moveVector;

	if (vGoPos.m128_f32[1] > 0.0f)
	{
		PxControllerFilters filters;
		m_pController->move(displacement, 0.0f, fTimeDelta, filters);

		// 객체의 충돌 상태 받아오기
		PxControllerState m_pPxState;

		m_pController->getState(m_pPxState);

		// 지면 판정, 천장 판정 처리
		if (m_pPxState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN || m_pPxState.collisionFlags & PxControllerCollisionFlag::eCOLLISION_UP)
			return false;

		pxCurrentPos = m_pController->getPosition();
		PxVec3 pos((_float)pxCurrentPos.x, (_float)pxCurrentPos.y, (_float)pxCurrentPos.z);

		_vector xmPos = XMVectorSet(pos.x, pos.y - 0.5f, pos.z, 0.f);

		pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
	}
	// 객체의 위치 받아오기
	//PxExtendedVec3 pxCurPos = m_pController->getPosition();     
	//PxVec3 pos((_float)pxCurrentPos.x, (_float)pxCurrentPos.y, (_float)pxCurrentPos.z);

	//_vector xmPos = XMVectorSet(pos.x, pos.y - 0.5f, pos.z, 0.f);

	//pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));

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

	PxExtendedVec3 pxPos = m_pController->getPosition();
	PxVec3 pos((_float)pxPos.x, (_float)pxPos.y, (_float)pxPos.z);

	_vector xmPos = XMVectorSet(pos.x, pos.y - m_fOffset, pos.z, 0.f);

	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN || m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
	{
		m_fFallVelocity = 0.f;
		pTransform->Set_State(CTransform::STATE_POSITION, XMVectorSetW(xmPos, 1.f));
		return;
	}

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
	_float fMaxDistance = 3.f;
	
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
_float CCharacterController::Compute_Height(_fvector vAxis)
{
	PxExtendedVec3 position = m_pController->getPosition();
	PxVec3 rayOrigin = PxVec3((_float)position.x, (_float)position.y, (_float)position.z) + CUtils::To_PxVec3(XMVector3Normalize(vAxis));

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
		return 20.f;

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

_vector CCharacterController::Compute_TerrainPosition_Vector()
{
	return XMVectorSetW(CUtils::To_Vector(Compute_TerrainPosition()), 1.f);
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

void CCharacterController::RegisterAsPlayer()
{
	m_pGameInstance->Register_Player(m_pController->getActor());
}

void CCharacterController::Create_Controller()
{
	Release_Controller();

	m_pControllerCallBack = new CControllerBehaviorCallback();
	m_tControllerDesc.behaviorCallback = m_pControllerCallBack;
	m_pControllerHitReport = new CUserControllerHitReport();
	m_tControllerDesc.reportCallback = m_pControllerHitReport;
	m_ControllerMaterial = m_pGameInstance->Get_Physics()->createMaterial(m_vMaterialOptions.x, m_vMaterialOptions.y, m_vMaterialOptions.z);
	m_tControllerDesc.material = m_ControllerMaterial;
	m_pController = m_pGameInstance->Get_ControllerManager()->createController(m_tControllerDesc);

	PxShape* shape;
	m_pController->getActor()->getShapes(&shape, 1);
	shape->setSimulationFilterData(physx::PxFilterData{ static_cast<physx::PxU32>(m_eCollisionType), 0, 0, 0 });
	//shape->setQueryFilterData(physx::PxFilterData{static_cast<physx::PxU32>(1), 0, 0, 0});
	//m_pGameInstance->RemoveActor(*m_pController->getActor());

	if (m_pObject != nullptr)
		Set_FootPosition(m_pObject->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION));
}

void CCharacterController::Release_Controller()
{
	if (nullptr == m_pGameInstance->Get_Scene()) return;

	if (nullptr != m_pController)
	{
		Safe_Delete(m_pControllerCallBack);
		Safe_Delete(m_pControllerHitReport);

		if (m_pController->getActor()->getScene())
			m_pGameInstance->RemoveActor(*m_pController->getActor());
		
		m_pController->release();
		m_ControllerMaterial->release();
	}
}

/// <summary> 캐릭터 컨트롤러를 생성하기 위해 필요한 구조체를 세팅한다. </summary>
void CCharacterController::Set_DefaultValue()
{
	#pragma region 변하지 않을 값들 (건드릴 경우 피쌤과 논의 요망)
	// 컨트롤러 볼륨 크기 
	m_tControllerDesc.volumeGrowth = 1.0f;
	// 컨트롤러의 UP-VECTOR
	m_tControllerDesc.upDirection = PxVec3(0, 1, 0);
	// 충돌판정 거리
	m_tControllerDesc.contactOffset = 0.01f;
	// 사용자 정의 데이터
	m_tControllerDesc.userData = this;
	// ControllerFilters
	m_ControllerFilters.mFilterData = &m_tFilterDesc;
	#pragma endregion

	// 컨트롤러의 질량(밀도)
	m_tControllerDesc.density = 100.f;

	// 캐릭터가 올라갈 수 있는 최대 계단의 높이
	m_tControllerDesc.stepOffset = 0.f;

	// 캐릭터와 환경 간의 물리적 상호작용을 위해 사용되는 물질
	//PxMaterial* material = m_pGameInstance->Get_Material();
	m_ControllerMaterial = m_pGameInstance->Get_Physics()->createMaterial(0.5f, 0.5f, 0.5f);
	m_tControllerDesc.material = m_ControllerMaterial;

	// 캐릭터컨트롤러(캡슐)의 가로 반지름
	m_tControllerDesc.radius = 0.5f;

	// 캐릭터컨트롤러(캡슐)의 높이
	m_tControllerDesc.height = 1.f;
	
	// 캐릭터가 오를 수 있는 최대 경사도
	m_fSlopeLimitDegree = 45.f;     
	m_tControllerDesc.slopeLimit = cosf(XMConvertToRadians(m_fSlopeLimitDegree));
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

	Safe_Release(m_pObject);
	Release_Controller();
}

