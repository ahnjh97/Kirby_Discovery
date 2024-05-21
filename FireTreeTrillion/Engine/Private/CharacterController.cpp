#include "CharacterController.h"

#include "GameObject.h"
#include "GameInstance.h"

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
	__super::Initialize(pArg);

	PxMaterial* material = m_pGameInstance->Get_Physics()->createMaterial(0.5f, 0.5f, 0.5f);
	m_tControllerDesc.material = material;
	m_tControllerDesc.upDirection = { 0.f, 1.f, 0.f };
	m_tControllerDesc.density = 100.f;

	m_ControllerFilters.mFilterData = &m_tFilterDesc;
	if (pArg == nullptr)
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

	ImGui::InputFloat("Radius", &m_tControllerDesc.radius);
	ImGui::InputFloat("height", &m_tControllerDesc.height);
	ImGui::InputFloat("contactOffset", &m_tControllerDesc.contactOffset);
	ImGui::InputFloat("density", &m_tControllerDesc.density);
	ImGui::InputFloat("slopeLimit", &m_fSlopeLimitDegree);
	m_tControllerDesc.slopeLimit = cosf(XMConvertToRadians(m_fSlopeLimitDegree));
	ImGui::InputFloat("stepOffset", &m_tControllerDesc.stepOffset);
	ImGui::InputFloat("maxJumpHeight", &m_tControllerDesc.maxJumpHeight);

	//ReCreateController (for change shape or scale)
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

PxControllerCollisionFlags CCharacterController::Move(_float4 vVelocity, _float fTimeDelta, _float minDist)
{
	// Disp == direction * speed * delta(delta 시간 동안의 이동량)
	vVelocity *= fTimeDelta;

	const physx::PxVec3 vDisp{vVelocity.x, vVelocity.y, vVelocity.z};
	return m_pController->move(vDisp, minDist, fTimeDelta, m_ControllerFilters);
}

PxControllerCollisionFlags CCharacterController::MoveDisp(_float4 vPosDelta, _float fTimeDelta, _float minDist)
{
	const physx::PxVec3 vDisp{vPosDelta.x, vPosDelta.y, vPosDelta.z};
	return m_pController->move(vDisp, minDist, fTimeDelta, m_ControllerFilters);
}

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

	m_pController = m_pGameInstance->Get_ControllerManager()->createController(m_tControllerDesc);

	PxShape* shape;
	m_pController->getActor()->getShapes(&shape, 1);
	shape->setSimulationFilterData(physx::PxFilterData{ static_cast<physx::PxU32>(0/*ColliderType*/), 0, 0, 0 });
	shape->setQueryFilterData(physx::PxFilterData{static_cast<physx::PxU32>(1), 0, 0, 0});
	m_pController->getActor()->userData = this;

	m_pGameInstance->RemoveActor(*m_pController->getActor());

	if (m_pObject != nullptr)
		Set_FootPosition(m_pObject->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION));
}

void CCharacterController::Release_Controller()
{
	if (m_pController)
	{
		if (m_pController->getActor()->getScene())
			m_pGameInstance->RemoveActor(*m_pController->getActor());
		m_pController->release();
	}
}

void CCharacterController::Set_DefaultValue()
{
	m_tControllerDesc.radius = 0.5f;
	m_tControllerDesc.height = 1.f;
	m_tControllerDesc.contactOffset = 0.1f;
	m_tControllerDesc.density = 100.f;
	m_fSlopeLimitDegree = 45.f;     
	m_tControllerDesc.slopeLimit = cosf(XMConvertToRadians(m_fSlopeLimitDegree));
	m_tControllerDesc.stepOffset = 0.1f;
	m_tControllerDesc.maxJumpHeight = 3.f;
}

void CCharacterController::Free()
{
	__super::Free();

	Release_Controller();
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

