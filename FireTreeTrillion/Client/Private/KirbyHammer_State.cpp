#include "stdafx.h"
#include "KirbyHammer_State.h"
#include "Kirby_State_Function.h"

void MakeFireHammer(CTransform* pTransformCom, _float Scale)
{
	_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vRight = pTransformCom->Get_State(CTransform::STATE_RIGHT);
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);

	CFire::FIREDESC Firedesc = {};
	Firedesc.vFirePos = (vPos - (vRight * 0.6f) - (vLook * 0.5f) + (vUp * 1.7f) + (_float4)CUtils::Make_Random_Vector(1.2f));
	Firedesc.fUpRange = { 2.5f };
	Firedesc.vFirstColor = { 1.f, 0.3f, 0.4f, 0.5f };
	Firedesc.vTargetColor = { 1.f, 1.f, 1.f, 1.f };
	Firedesc.fScale = { Scale * CUtils::Make_RandomFloat(0.8f, 1.2f)};
	Firedesc.fTimeRatio = 0.5f;
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Fire"), TEXT("Prototype_GameObject_Fire"), &Firedesc)))
		return;
}

#pragma region IDLE STATE

CKirbyHammer_Idle_State::CKirbyHammer_Idle_State()
{
}

void CKirbyHammer_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyHammer_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	if (pKirby->Get_State() == CKirby::HAMMERSTATE_IDLE)
	{
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		// 자유낙하
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		// Idle일 때, 방향키를 눌렀을 때 RUN 으로 간다.
		if (JoyStick_controller(Kirbydesc, pCamera))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::HAMMERSTATE_RUN, 120.f, true, true, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_GUARD);
			return;
		}
		else if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBlockOtherVacuum) == false)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERSTART, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			return;
		}
		else if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;

			DESC(m_eJumpState) == DESC(m_eJumpState) ? CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;

			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;

			Kirby_AbilityType_Assist(pKirby, DESC(m_eJumpState));
			return;
		}
		
		if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
		{
			DESC(m_bDumpAbilityPress) = true;
			DESC(m_fDumpAbilityTime) += fTimeDelta;

			if (DESC(m_fDumpAbilityTime) > 1.f)
			{
				DESC(m_fDumpAbilityTime) = 0.f;
				pKirby->Change_State(CKirby::STATE_ABILITYDUMP, 60.f, false, false, CKirby::BODY_DEFAULT);
				return;
			}
		}
		else
		{
			DESC(m_bDumpAbilityPress) = false;
		}
	}

	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_RUN)
	{
		Bbong_FX(fTimeDelta, pTransformCom);
		Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Turn_Z_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (Kirby_Ladder_Logic(pKirby, Kirbydesc, pTransformCom))
		{
			pController->Set_Position(pTransformCom, DESC(m_vLadderPoint));
			DESC(m_vMoveDir) = DESC(m_vTargetDir) = DESC(m_vLadderLook);
			pKirby->Change_State(CKirby::STATE_LADDERWAITSTART, 200.f, false, false, CKirby::BODY_DEFAULT);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBlockOtherVacuum) == false)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERSTART, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			return;
		}

		// 달리다가, C를 누르면 점프를 한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;

			DESC(m_eJumpState) == CKirby::STATE_JUMPL ? DESC(m_eJumpState) = CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;
			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;
			Kirby_AbilityType_Assist(pKirby, DESC(m_eJumpState));
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_GUARD);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
		{
			DESC(m_bDumpAbilityPress) = true;
			DESC(m_fDumpAbilityTime) += fTimeDelta;

			if (DESC(m_fDumpAbilityTime) > 1.f)
			{
				DESC(m_fDumpAbilityTime) = 0.f;
				pKirby->Change_State(CKirby::STATE_ABILITYDUMP, 60.f, false, false, CKirby::BODY_DEFAULT);
				return;
			}
		}
		else
		{
			DESC(m_bDumpAbilityPress) = false;
		}

		if (false == JoyStick_controller(Kirbydesc, pCamera))
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			return;
		}

		if (pController->Compute_Height() > 2.f)
		{
			pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
			return;
		}
	}
}

void CKirbyHammer_Idle_State::OnStateExit()
{
}

CKirbyHammer_Idle_State* CKirbyHammer_Idle_State::Create()
{
	CKirbyHammer_Idle_State* pInstance = new CKirbyHammer_Idle_State();
	return pInstance;
}

void CKirbyHammer_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region HAMMERATTACK STATE

CKirbyHammer_Attack_State::CKirbyHammer_Attack_State()
{
}

void CKirbyHammer_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	CGameObject* pObject = m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"));
	CTransform* pTransformCom = pObject->Get_TransformCom();
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
	if (_iAnimIndex == CKirby::HAMMERSTATE_HAMMERATTACKTOY)
	{
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, -.5f, -1.f };
		FXDesc.vInitRot = { 0.f, 0.f, 0.f };
		FXDesc.fStartDelay = 0.1f;
		FXDesc.vInitScale = { 2.f, 2.f, 2.f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

		static_cast<CPhysXObject*>(pObject)->Add_Effect("YW KirbyHammerTrail", FXDesc, true);

		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW KirbyHammerTrail"), &FXDesc)))
		//	return;
		//static_cast<CPhysXObject*>(pObject)->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}
	else if (_iAnimIndex == CKirby::HAMMERSTATE_HAMMERATTACKFINALTOY)
	{
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, -.5f, -1.f };
		FXDesc.vInitRot = { 0.f, 0.f, 0.f };
		FXDesc.fStartDelay = 0.5f;
		FXDesc.vInitScale = { 2.f, 2.f, 2.f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

		static_cast<CPhysXObject*>(pObject)->Add_Effect("YW KirbyHammerTrail", FXDesc, true);

		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW KirbyHammerTrail"), &FXDesc)))
		//	return;
		//static_cast<CPhysXObject*>(pObject)->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}
}

void CKirbyHammer_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// 뱉는 로직이다.
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// 시작 2틱
	if (pKirby->Get_State() == CKirby::HAMMERSTATE_HAMMERATTACKSTARTTOY)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 30.f;
			if (Kirbydesc->m_fMoveSpeed > 4.f)
				Kirbydesc->m_fMoveSpeed = 4.f;
			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);


		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			return;
		}
	}
	// 중간
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_HAMMERATTACKTOY)
	{
		m_fAttackJumpTime += fTimeDelta;
		if (m_bCountTrigger == true) 
		{
			DESC(m_iHammerHit)++;
			m_bCountTrigger = false;
		}

		if (m_fAttackJumpTime > 0.05f && m_bAttackJumpTrigger == true)
		{
			DESC(m_fJumpVelocity) = 15.f;
			m_bAttackJumpTrigger = false;
		}
		else if (m_bAttackJumpTrigger == false)
		{
			DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
			pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

			if (m_fAttackJumpTime >= 0.15f)
			{
				_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
				DESC(m_fJumpVelocity) = fStopVelocityPower;
			}
		}


		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 30.f;
			if (Kirbydesc->m_fMoveSpeed > 4.f)
				Kirbydesc->m_fMoveSpeed = 4.f;
			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);


		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKHITTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake();

			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);
			_float4 vHammerHitPos = vPos + (vLook * 1.8f) + (vUp * 0.5f);
			PxVec3 rayOrigin = PxVec3((_float)vHammerHitPos.x, (_float)vHammerHitPos.y, (_float)vHammerHitPos.z);
			PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
			_float fMaxDistance = 5.f;
			PxRaycastHit hit;
			PxRaycastBuffer hitBuffer;
			PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
			_bool isRayCast = m_pGameInstance->Get_Scene()->raycast(rayOrigin, rayDirection, fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
			hit = hitBuffer.block;
			vHammerHitPos = XMVectorSetW(CUtils::To_Vector(hit.position), 1.f);
			vHammerHitPos.y += 0.1f;
			_float4 vTerrainNormal = XMVectorSetW(CUtils::To_Vector(hit.normal), 0.f);
			_float3 vRot = CUtils::Make_Degree_FromDir(-1.f * vTerrainNormal);
			CEffect::FX_DESC FXDesc{};
			FXDesc.vInitPos = (_float3)vHammerHitPos;
			FXDesc.vInitRot = vRot;
			FXDesc.vInitScale = { 1.8f, 1.8f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerHit"), &FXDesc)))
				return;
			FXDesc.vInitRot = vRot;
			FXDesc.vInitScale = { 3.f, 3.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerHit2"), &FXDesc)))
				return;
			CParticle::PARTICLE_DESC ParticleDesc{};
			ParticleDesc.vInitPos = (_float3)vHammerHitPos;
			ParticleDesc.vInitRot = vRot;
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_start particle test A"), &ParticleDesc)))
				return;

			pKirby->Set_WeaponAnim(6);
			return;
		}
	}
	// 타격
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_HAMMERATTACKHITTOY)
	{
		m_fAttackJumpTime += fTimeDelta;

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 30.f;
			if (Kirbydesc->m_fMoveSpeed > 4.f)
				Kirbydesc->m_fMoveSpeed = 4.f;
			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_fAttackJumpTime > 0.1f)
		{
			pController->FreeFall(pTransformCom, fTimeDelta);

			if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
			{
				pController->FreeFall(pTransformCom, fTimeDelta, 100.f);

				if (DESC(m_iHammerHit) < 4)
				{
					pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;
					return;
				}
				else if (DESC(m_iHammerHit) >= 4)
				{
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;
					pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKFINALTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
					pKirby->Set_WeaponAnim(5);
					return;
				}
			}

		}

		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			DESC(m_iHammerHit) = 0;
		}
	}
	// 막타 통 애님
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_HAMMERATTACKFINALTOY)
	{
		m_fAttackJumpTime += fTimeDelta;

		// 0.7초 이하일때, 이동이 자유롭다.
		if (m_fAttackJumpTime < 0.5f)
		{
			pController->FreeFall(pTransformCom, fTimeDelta);
			_float fDelta = EASE_INOUT(m_fAttackJumpTime * 2.f) - EASE_INOUT(m_fPreAttackJumpTime * 2.f);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			pController->Move_Dir(pTransformCom, vLook * fDelta * 2.2f, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_fAttackJumpTime > 0.45f && m_bAttackJumpTrigger == true)
		{
			if (m_bCountTrigger == true)
			{
				CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
				pCamera->Make_Shake();

				_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
				_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
				_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);
				_float4 vHammerHitPos = vPos + (vLook * 1.8f) + (vUp * 0.5f);
				PxVec3 rayOrigin = PxVec3((_float)vHammerHitPos.x, (_float)vHammerHitPos.y, (_float)vHammerHitPos.z);
				PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
				_float fMaxDistance = 5.f;
				PxRaycastHit hit;
				PxRaycastBuffer hitBuffer;
				PxQueryFilterData filterData(PxQueryFlag::eSTATIC);
				_bool isRayCast = m_pGameInstance->Get_Scene()->raycast(rayOrigin, rayDirection, fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);
				hit = hitBuffer.block;
				vHammerHitPos = XMVectorSetW(CUtils::To_Vector(hit.position), 1.f);
				vHammerHitPos.y += 0.1f;
				_float4 vTerrainNormal = XMVectorSetW(CUtils::To_Vector(hit.normal), 0.f);
				_float3 vRot = CUtils::Make_Degree_FromDir(-1.f * vTerrainNormal);
				CEffect::FX_DESC FXDesc{};
				FXDesc.vInitPos = (_float3)vHammerHitPos;
				FXDesc.vInitRot = vRot;
				FXDesc.vInitScale = { 3.f, 3.f, 1.f };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerHit"), &FXDesc)))
					return;
				FXDesc.vInitRot = vRot;
				FXDesc.vInitScale = { 6.f, 6.f, 1.f };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerHit2"), &FXDesc)))
					return;
				CParticle::PARTICLE_DESC ParticleDesc{};
				ParticleDesc.vInitPos = (_float3)vHammerHitPos;
				ParticleDesc.vInitRot = vRot;
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_start particle test A"), &ParticleDesc)))
					return;

				m_bCountTrigger = false;
			}
			DESC(m_fJumpVelocity) = 30.f;
			m_bAttackJumpTrigger = false;
		}
		else if (m_bAttackJumpTrigger == false)
		{
			DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
			pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

			if (m_fAttackJumpTime >= 0.4f && m_fAttackJumpTime < 0.7f)
			{
				_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
				DESC(m_fJumpVelocity) = fStopVelocityPower;
			}
		}

		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			DESC(m_iHammerHit) = 0;
		}


		m_fPreAttackJumpTime = m_fAttackJumpTime;
	}
}

void CKirbyHammer_Attack_State::OnStateExit()
{
	m_bAttackJumpTrigger = true;
	m_fAttackJumpTime = 0.f;
	m_fPreAttackJumpTime = 0.f;
	m_bCountTrigger = true;
}

CKirbyHammer_Attack_State* CKirbyHammer_Attack_State::Create()
{
	CKirbyHammer_Attack_State* pInstance = new CKirbyHammer_Attack_State();
	return pInstance;
}

void CKirbyHammer_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region ONIGOROSI STATE

CKirbyHammer_Onigorosi_State::CKirbyHammer_Onigorosi_State()
{
}

void CKirbyHammer_Onigorosi_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	CGameObject* pObject = m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"));
	CTransform* pTransformCom = pObject->Get_TransformCom();
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);

	// 덜 차징
	if (_iAnimIndex == CKirby::HAMMERSTATE_ONIGOROSIHAMMERFIRST)
	{
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, 0.5f, -.5f };
		FXDesc.vInitRot = { 0.f, 20.f, 0.f };
		FXDesc.fStartDelay = 0.16f;
		FXDesc.vInitScale = { 1.8f, 1.8f, 1.8f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

		static_cast<CPhysXObject*>(pObject)->Add_Effect("YW KirbyHammerTrail2", FXDesc, true);

		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW KirbyHammerTrail2"), &FXDesc)))
		//	return;
		//static_cast<CPhysXObject*>(pObject)->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}
	// 강화 차징
	else if (_iAnimIndex == CKirby::HAMMERSTATE_ONIGOROSIHAMMEREND)
	{
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, 0.5f, -.5f };
		FXDesc.vInitRot = { 0.f, 0.f, 0.f };
		FXDesc.fStartDelay = 0.44f;
		FXDesc.vInitScale = { 2.5f, 2.5f, 2.5f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

		static_cast<CPhysXObject*>(pObject)->Add_Effect("YW KirbyHammerTrail2", FXDesc, true);


		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW KirbyHammerTrail2"), &FXDesc)))
		//	return;
		//static_cast<CPhysXObject*>(pObject)->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}

}

void CKirbyHammer_Onigorosi_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta);

	// 모든 공격의 시작이라고 볼 수 있다.
	if (pKirby->Get_State() == CKirby::HAMMERSTATE_ONIGOROSIHAMMERSTART)
	{
		// 기를 모으는 시간.
		DESC(m_fHammerChargeTime) += fTimeDelta;

		if (JoyStick_On() == true)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERMOVE, 60.f, true, true, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			pKirby->Set_WeaponAnim(9);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKSTARTTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_fHammerChargeTime) = 0.f;
			DESC(m_bFirstChargeEffectTrigger) = true;
			DESC(m_bSecondChargeEffectTrigger) = true;
			return;
		}
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERCHARGE, 60.f, true, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			pKirby->Set_WeaponAnim(9);
			return;
		}
	}
	// 차지 모션
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_ONIGOROSIHAMMERCHARGE)
	{
		DESC(m_fHammerChargeTime) += fTimeDelta;

		if (DESC(m_bSecondChargeEffectTrigger) == true)
			MakeFireHammer(pTransformCom, 0.8f);
		else
			MakeFireHammer(pTransformCom, 5.f);

		if (DESC(m_fHammerChargeTime) > 0.5f && DESC(m_bFirstChargeEffectTrigger) == true)
		{
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);
			_float4 vRight = pTransformCom->Get_State(CTransform::STATE_RIGHT);

			CMultiEffect::MULTI_FX_DESC MulFXDesc{};
			MulFXDesc.vInitPos = { -0.4f, 1.7f, -0.7f };
			MulFXDesc.vInitScale = { 2.f, 2.f, 2.f };
			MulFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

			pKirby->Add_Effect("YW Light Cluster", MulFXDesc, true);

			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Light Cluster"), &MulFXDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
			DESC(m_bFirstChargeEffectTrigger) = false;


			CParticle::PARTICLE_DESC PartDesc{};
			PartDesc.vInitPos = { -0.4f, 1.7f, -0.7f };
			PartDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

			pKirby->Add_Effect("YW HammerChargeParticle", PartDesc, true);


			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerChargeParticle"), &PartDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
		}
		else if (DESC(m_fHammerChargeTime) > 2.f && DESC(m_bSecondChargeEffectTrigger) == true)
		{
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);
			_float4 vRight = pTransformCom->Get_State(CTransform::STATE_RIGHT);

			CMultiEffect::MULTI_FX_DESC MulFXDesc{};
			MulFXDesc.vInitPos = { -0.4f, 1.7f, -0.7f };
			MulFXDesc.vInitScale = { 3.f, 3.f, 3.f };
			MulFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

			pKirby->Add_Effect("YW Light ClusterBig", MulFXDesc, true);


			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Light ClusterBig"), &MulFXDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
			DESC(m_bSecondChargeEffectTrigger) = false;
		}



		if (JoyStick_On() == true)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERMOVE, 60.f, true, true, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			pKirby->Set_WeaponAnim(9);
			return;
		}


		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			if (DESC(m_fHammerChargeTime) < 2.f && DESC(m_fHammerChargeTime) > 0.5f)
			{
				pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERFIRST, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				DESC(m_fHammerChargeTime) = 0.f;
				DESC(m_bFirstChargeEffectTrigger) = true;
				DESC(m_bSecondChargeEffectTrigger) = true;
				pKirby->Delete_Effect("YW HammerChargeParticle");

				return;
			}
			else if (DESC(m_fHammerChargeTime) >= 2.f)
			{
				CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
				pCamera->Zoom(-5.f);
				pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMEREND, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				DESC(m_fHammerChargeTime) = 0.f;
				DESC(m_bFirstChargeEffectTrigger) = true;
				DESC(m_bSecondChargeEffectTrigger) = true;
				pKirby->Delete_Effect("YW HammerChargeParticle");

				pKirby->Set_WeaponAnim(10);
				return;
			}
			else
			{
				pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKSTARTTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
				DESC(m_eEyeState) = CKirby::EYE_CLOSE;
				DESC(m_bFirstChargeEffectTrigger) = true;
				DESC(m_bSecondChargeEffectTrigger) = true;
				DESC(m_fHammerChargeTime) = 0.f;
				return;
			}
		}
	}
	// 차지 중 이동하는 모션
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_ONIGOROSIHAMMERMOVE)
	{
		if (DESC(m_bSecondChargeEffectTrigger) == true)
			MakeFireHammer(pTransformCom, 0.8f);
		else
			MakeFireHammer(pTransformCom, 5.f);

		DESC(m_fHammerChargeTime) += fTimeDelta;

		if (DESC(m_fHammerChargeTime) > 0.5f && DESC(m_bFirstChargeEffectTrigger) == true)
		{
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);
			_float4 vRight = pTransformCom->Get_State(CTransform::STATE_RIGHT);

			CMultiEffect::MULTI_FX_DESC MulFXDesc{};
			MulFXDesc.vInitPos = { -0.4f, 1.7f, -0.7f };
			MulFXDesc.vInitScale = { 2.f, 2.f, 2.f };
			MulFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

			pKirby->Add_Effect("YW Light Cluster", MulFXDesc, true);

			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Light Cluster"), &MulFXDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

			CParticle::PARTICLE_DESC PartDesc{};
			PartDesc.vInitPos = { -0.4f, 1.7f, -0.7f };
			PartDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

			pKirby->Add_Effect("YW HammerChargeParticle", MulFXDesc, true);

			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerChargeParticle"), &PartDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

			DESC(m_bFirstChargeEffectTrigger) = false;
		}
		else if (DESC(m_fHammerChargeTime) > 2.f && DESC(m_bSecondChargeEffectTrigger) == true)
		{
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			_float4 vUp = pTransformCom->Get_State(CTransform::STATE_UP);
			_float4 vRight = pTransformCom->Get_State(CTransform::STATE_RIGHT);

			CMultiEffect::MULTI_FX_DESC MulFXDesc{};
			MulFXDesc.vInitPos = { -0.4f, 1.7f, -0.7f };
			MulFXDesc.vInitScale = { 3.f, 3.f, 3.f };
			MulFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

			pKirby->Add_Effect("YW Light ClusterBig", MulFXDesc, true);

			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Light ClusterBig"), &MulFXDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
			DESC(m_bSecondChargeEffectTrigger) = false;
		}


		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 50.f;
			if (Kirbydesc->m_fMoveSpeed > 3.f)
				Kirbydesc->m_fMoveSpeed = 3.f;
			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERCHARGE, 60.f, true, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			pKirby->Set_WeaponAnim(9);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			if (DESC(m_fHammerChargeTime) < 2.f && DESC(m_fHammerChargeTime) > 0.5f)
			{
				pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMERFIRST, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				DESC(m_fHammerChargeTime) = 0.f;
				DESC(m_bFirstChargeEffectTrigger) = true;
				DESC(m_bSecondChargeEffectTrigger) = true;
				pKirby->Delete_Effect("YW HammerChargeParticle");
				return;
			}
			else if (DESC(m_fHammerChargeTime) >= 2.f)
			{
				CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
				pCamera->Zoom(-10.f);
				pKirby->Change_State(CKirby::HAMMERSTATE_ONIGOROSIHAMMEREND, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				pKirby->Set_WeaponAnim(10);
				DESC(m_fHammerChargeTime) = 0.f;
				DESC(m_bFirstChargeEffectTrigger) = true;
				DESC(m_bSecondChargeEffectTrigger) = true;
				pKirby->Delete_Effect("YW HammerChargeParticle");
				return;
			}
			else
			{
				pKirby->Change_State(CKirby::HAMMERSTATE_HAMMERATTACKSTARTTOY, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
				DESC(m_eEyeState) = CKirby::EYE_CLOSE;
				DESC(m_fHammerChargeTime) = 0.f;
				DESC(m_bFirstChargeEffectTrigger) = true;
				DESC(m_bSecondChargeEffectTrigger) = true;

				return;
			}
		}
	}
	// 덜 차징 애니메이션
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_ONIGOROSIHAMMERFIRST)
	{
		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			pKirby->Set_WeaponAnim(3);
			return;
		}
	}
	// 강한 차징 애니메이션
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_ONIGOROSIHAMMEREND)
	{
		m_fMoveTime += fTimeDelta;

		if (0.4f < m_fMoveTime && m_fMoveTime < 0.6f)
		{
			_float fDelta = EASE_INOUT((m_fMoveTime - 0.4f) * 5.f) - EASE_INOUT((m_fPreMoveTime - 0.4f) * 5.f);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			pController->Move_Dir(pTransformCom, vLook * fDelta * 3.f, fTimeDelta);
		}

		if (pKirby->isAnimFinish())
		{
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Zoom(0.f);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			pKirby->Set_WeaponAnim(3);
			return;
		}

		m_fPreMoveTime = m_fMoveTime;
	}
}

void CKirbyHammer_Onigorosi_State::OnStateExit()
{
	m_fMoveTime = 0.f;
	m_fPreMoveTime = 0.f;
}

CKirbyHammer_Onigorosi_State* CKirbyHammer_Onigorosi_State::Create()
{
	CKirbyHammer_Onigorosi_State* pInstance = new CKirbyHammer_Onigorosi_State();
	return pInstance;
}

void CKirbyHammer_Onigorosi_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region JUMPATTACK STATE

CKirbyHammer_JumpAttack_State::CKirbyHammer_JumpAttack_State()
{
}

void CKirbyHammer_JumpAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	CGameObject* pObject = m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"));
	CTransform* pTransformCom = pObject->Get_TransformCom();
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);

	if (_iAnimIndex == CKirby::HAMMERSTATE_WHEELHAMMER)
	{
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, 0.f, 0.f };
		FXDesc.vInitRot = { 0.f, 0.f, 0.f };
		FXDesc.fStartDelay = 0.05f;
		FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();

		static_cast<CPhysXObject*>(pObject)->Add_Effect("YW HammerWheel", FXDesc, true);

		//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW HammerWheel"), &FXDesc)))
		//	return;
		//static_cast<CPhysXObject*>(pObject)->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

		
	}
}

void CKirbyHammer_JumpAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// 공중에서 공격하는 애니메이션이다.
	if (pKirby->Get_State() == CKirby::HAMMERSTATE_WHEELHAMMER)
	{
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 40.f;
			if (Kirbydesc->m_fMoveSpeed > 10.f)
				Kirbydesc->m_fMoveSpeed = 10.f;

			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}
		else
		{
			// 0.1초간 풀 감속 (최대 속도 8이라 가정)
			if (Kirbydesc->m_fMoveSpeed > 0.f)
				Kirbydesc->m_fMoveSpeed -= 100.f * fTimeDelta;
			if (Kirbydesc->m_fMoveSpeed < 0.f)
				Kirbydesc->m_fMoveSpeed = 0.f;

			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}

		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::HAMMERSTATE_WHEELHAMMEREND, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			return;
		}
	}
	// 공중에서 공격하는 애니메이션의 끝이라고 볼 수 있다.
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_WHEELHAMMEREND)
	{
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 40.f;
			if (Kirbydesc->m_fMoveSpeed > 10.f)
				Kirbydesc->m_fMoveSpeed = 10.f;

			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}
		else
		{
			// 0.1초간 풀 감속 (최대 속도 8이라 가정)
			if (Kirbydesc->m_fMoveSpeed > 0.f)
				Kirbydesc->m_fMoveSpeed -= 100.f * fTimeDelta;
			if (Kirbydesc->m_fMoveSpeed < 0.f)
				Kirbydesc->m_fMoveSpeed = 0.f;

			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			DESC(m_fJumpVelocity) = 10.f;
			pKirby->Change_State(CKirby::HAMMERSTATE_WHEELHAMMER, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
			return;
		}
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		if (pController->Is_Terrain())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_LANDINGSMALL);
			return;
		}
	}
}

void CKirbyHammer_JumpAttack_State::OnStateExit()
{
}

CKirbyHammer_JumpAttack_State* CKirbyHammer_JumpAttack_State::Create()
{
	CKirbyHammer_JumpAttack_State* pInstance = new CKirbyHammer_JumpAttack_State();
	return pInstance;
}

void CKirbyHammer_JumpAttack_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region JUMP STATE

CKirbyHammer_Jump_State::CKirbyHammer_Jump_State()
{
}

void CKirbyHammer_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyHammer_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	if (pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGSMALL || pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGEND)
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	else
		Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);


	if (JoyStick_controller(Kirbydesc, pCamera))
	{
		Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}
	else
	{
		if (pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGSMALL || pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGEND)
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		else
			Jump_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}

	if (Key_X(pGameObject, fTimeDelta) == true)
		return;
	Key_C(pGameObject, fTimeDelta);

	// 처음에 범위에 바로 들어갔을 때, 사다리에 스냅한다. 단, block이 켜져있을 땐, 절대 못붙음
	if (Kirby_Ladder_Logic(pKirby, Kirbydesc, pTransformCom))
	{
		pController->Set_Position(pTransformCom, DESC(m_vLadderPoint));
		DESC(m_vMoveDir) = DESC(m_vTargetDir) = DESC(m_vLadderLook);
		pKirby->Change_State(CKirby::STATE_LADDERWAITSTART, 200.f, false, false, CKirby::BODY_DEFAULT);
		return;
	}
	// 위에 block이 켜져있을 때 아마 여기로 들어왔을 것이다.
	else if (DESC(m_bCanLadder) == true)
	{
		// 낙하중이면서, 내 방향이 사다리 방향일 때 붙게 한다.
		if (Kirby_JoyStickLadder_Logic(pKirby, Kirbydesc, pTransformCom, pCamera) == true
			&& DESC(m_fJumpVelocity) < 0.f)
		{
			pController->Set_Position(pTransformCom, DESC(m_vLadderPoint));
			DESC(m_vMoveDir) = DESC(m_vTargetDir) = DESC(m_vLadderLook);
			pKirby->Change_State(CKirby::STATE_LADDERWAITSTART, 200.f, false, false, CKirby::BODY_DEFAULT);
			return;
		}
	}



	// 점프
	if (pKirby->Get_State() == CKirby::HAMMERSTATE_JUMPL || pKirby->Get_State() == CKirby::HAMMERSTATE_JUMPR
		|| pKirby->Get_State() == CKirby::HAMMERSTATE_JUMPEND)
	{
		// 0.3초 동안만 누적이 된다.
		if (DESC(m_bRePressBlock) == false && m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS) && DESC(m_fJumpHoldTime) < 0.3f)
		{
			DESC(m_fJumpHoldTime) += fTimeDelta;
			DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
			pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		}
		else
		{
			// 최소 점프 제한 키 유지 시간
			_float fJumpHoldTime = 0.1f;
			// 공중 체공 시간
			_float fHoldAirTime = 0.22f;
			// 최소단위 공중 올라가는 시간
			_float fChangeVelocityZeroTime = 0.09f;

			// 최소 점프시간 이하로 눌렀을 때
			if (DESC(m_fJumpHoldTime) < fJumpHoldTime)
			{
				DESC(m_fChangeVelocityZeroTime) += fTimeDelta;

				// 만약, 최소 점프 제한키 이하로 눌렀을 때, 최소 단위 공중 올라가는 시간이 지났다면
				if (DESC(m_fChangeVelocityZeroTime) > fChangeVelocityZeroTime)
				{
					// 공중 체공시간동안 중력값은 보정된다.
					DESC(m_fHoldAirTime) += fTimeDelta;
					if (DESC(m_fHoldAirTime) < fHoldAirTime)
					{
						// 중력값 보정식
						_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
						DESC(m_fJumpVelocity) = fStopVelocityPower + (fabs(DESC(m_fHoldAirTime) - fHoldAirTime * 0.5f) * (-1.f / (fHoldAirTime * 0.5f)));
					}
				}
			}
			// 만약, 0.1초 이상 눌렀을 경우
			else
			{
				// 바로 공중 체공시간이 가산되어 중력값이 보정된다.
				DESC(m_fHoldAirTime) += fTimeDelta;
				if (DESC(m_fHoldAirTime) < fHoldAirTime)
				{
					_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
					DESC(m_fJumpVelocity) = fStopVelocityPower + (fabs(DESC(m_fHoldAirTime) - fHoldAirTime * 0.5f) * (-1.f / (fHoldAirTime * 0.5f)));
				}

			}

			DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
			pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		}

		// 0.3초 이상 눌렀을 경우 JUMPEND (공중에서 한 바퀴 돌기) 행위를 한다.
		if (DESC(m_fJumpHoldTime) > 0.3f)
		{
			Kirby_AbilityType_Assist(pKirby ,CKirby::STATE_JUMPEND);
			//pKirby->Change_State(CKirby::STATE_JUMPEND, 60.f, false, true, CKirby::BODY_DEFAULT);
			//DESC(m_fJumpHoldTime) = 0.f;
		}


		// 만약, 땅에 안전하게 착지했을 경우, 홀딩 시간에 따라 뽀잉 애니메이션이 분기된다.
		if (pController->Is_Terrain())
		{
			if (DESC(m_fJumpHoldTime) > 0.2f)
			{
				if (CUtils::Make_RandomInt(0, 1) > 0)
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;

				CMultiEffect::MULTI_FX_DESC FXDesc{};
				_float4 vKirbyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
				_float4 vKirbyLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

				FXDesc.vInitPos = { vKirbyPos.x, vKirbyPos.y + .4f, vKirbyPos.z };
				FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y + 20.f, 0.f };
				FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Smoke Fast"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y - 20.f, 0.f };
				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Smoke Fast"), &FXDesc)))
					return;
				Kirby_AbilityType_Assist(pKirby, CKirby::STATE_LANDINGEND);
				//pKirby->Change_State(CKirby::STATE_LANDINGEND, 30.f, false, false, CKirby::BODY_DEFAULT);
			}
			else
			{
				if (CUtils::Make_RandomInt(0, 1) > 0)
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;

				CMultiEffect::MULTI_FX_DESC FXDesc{};
				_float4 vKirbyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
				_float4 vKirbyLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

				FXDesc.vInitPos = { vKirbyPos.x, vKirbyPos.y + .4f, vKirbyPos.z };
				FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y + 30.f, 0.f };
				FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y - -30.f, 0.f };
				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;
				Kirby_AbilityType_Assist(pKirby, CKirby::STATE_LANDINGSMALL);
				//pKirby->Change_State(CKirby::STATE_LANDINGSMALL, 50.f, false, false, CKirby::BODY_DEFAULT);
			}
		}
	}
	// 뽀잉
	else if (pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGEND || pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGSMALL)
	{
		if (DESC(m_bReserveJumpKey) == true)
		{
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;

			DESC(m_eJumpState) == CKirby::STATE_JUMPL ? DESC(m_eJumpState) = CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;

			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;
			// 예약 초기화
			DESC(m_bReserveJumpKey) = false;
			Kirby_AbilityType_Assist(pKirby, DESC(m_eJumpState));
			//pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);
		}

		// 최소 애니메이션이 재생되는 시간이다. ( 방향키를 누르면 0.2초 후 바로 Run 상태가 됨 )
		_float fChangeRunTime = 0.08f;

		m_fChangeRunTime += fTimeDelta;

		// Idle일 때, C를 누르면 점프를 한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;

			DESC(m_eJumpState) == DESC(m_eJumpState) ? CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;
			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;
			Kirby_AbilityType_Assist(pKirby, DESC(m_eJumpState));
			//pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_GUARD);
		}

		// 바로 방향키를 갈겼다면
		if (m_fChangeRunTime > fChangeRunTime && JoyStick_controller(Kirbydesc, pCamera))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;

			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_RUNSTART);

		}
		// 자연스럽게 끝났다면
		else if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;

			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);

		}

		//// Idle일 때, X를 누르면 1타 공격을 시작한다.
		//if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		//{


		//}

	}

}

void CKirbyHammer_Jump_State::OnStateExit()
{
	m_fChangeRunTime = 0.f;
}

_bool CKirbyHammer_Jump_State::Key_X(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_fJumpVelocity) = 10.f;
		pKirby->Change_State(CKirby::HAMMERSTATE_WHEELHAMMER, 60.f, false, false, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);
		return true;
	}
	return false;
}

_bool CKirbyHammer_Jump_State::Key_C(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_UP))
	{
		// 때는 순간 트루가 되고, 점프 가능 시점에 다시 누를 때 까지 C에대한 누적 등 반응하지 않는다.
		DESC(m_bRePressBlock) = true;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) &&
		(pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGSMALL || pKirby->Get_State() == CKirby::HAMMERSTATE_LANDINGEND) == false)
	{
		// 점프 예약을 한다.
		if (pController->Compute_Height() < 2.f &&
			(pKirby->Get_State() == CKirby::HAMMERSTATE_JUMPL || pKirby->Get_State() == CKirby::HAMMERSTATE_JUMPR ||
				pKirby->Get_State() == CKirby::HAMMERSTATE_JUMPEND))
		{
			DESC(m_bReserveJumpKey) = true;
		}
		else
		{
			DESC(m_fFlyTime) = 0.f;
			DESC(m_fJumpVelocity) = 4.f;
			pKirby->Change_State(CKirby::STATE_FLIGHTSTART, 60.f, false, false, CKirby::BODY_BALLOON);
			return true;
		}
	}
	return false;
}

CKirbyHammer_Jump_State* CKirbyHammer_Jump_State::Create()
{
	CKirbyHammer_Jump_State* pInstance = new CKirbyHammer_Jump_State();
	return pInstance;
}

void CKirbyHammer_Jump_State::Free()
{
	__super::Free();
}


#pragma endregion
