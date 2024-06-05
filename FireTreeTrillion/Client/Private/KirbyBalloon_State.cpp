#include "stdafx.h"
#include "KirbyBalloon_State.h"
#include "Kirby_State_Function.h"

#pragma region BALLOON IDLE STATE

CKirbyBalloon_Idle_State::CKirbyBalloon_Idle_State()
{
}

void CKirbyBalloon_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBalloon_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
	// 자유낙하
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
	// 낙하 높이
	if (pController->Compute_Height() > 2.f && DESC(m_eTemporaryEatType) == ABILITY_DEFAULT)
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		DESC(m_isEatFall) = true;
		pKirby->Change_State(CKirby::STATE_EATJUMP, 50.f, false, true, CKirby::BODY_BALLOON);
	}

	// 먹는 순간의 로직
	if (pKirby->Get_State() == CKirby::STATE_EAT)
	{
		if (pKirby->isAnimFinish())
		{
			if (DESC(m_eTemporaryEatType) == ABILITY_DEFAULT)
			{
				pKirby->Change_State(CKirby::STATE_EATWAIT, 60.f, true, true, CKirby::BODY_BALLOON);
			}
			else
			{
				DESC(m_isEat) = false;
				pKirby->Change_State(CKirby::STATE_SWALLOWSTART, 60.f, false, false, CKirby::BODY_BALLOON);
			}
		}
	}
	// 먹었을 때의 아이들 상태
	else if (pKirby->Get_State() == CKirby::STATE_EATWAIT)
	{
		// Idle일 때, 방향키를 눌렀을 때 RUN 으로 간다.
		if (JoyStick_controller(Kirbydesc, pCamera))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_EATRUN, 100.f, true, true, CKirby::BODY_BALLOON);
		}

		// Idle일 때, C를 누르면 점프를 한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_EATJUMP, 50.f, false, true, CKirby::BODY_BALLOON);

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;
			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;
		}

		// Eat 상태일 때 뱉는 모션을 취한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_SPIT, 70.f, false, false, CKirby::BODY_VACUUM);
		}
	}

}

void CKirbyBalloon_Idle_State::OnStateExit()
{
}

CKirbyBalloon_Idle_State* CKirbyBalloon_Idle_State::Create()
{
	CKirbyBalloon_Idle_State* pInstance = new CKirbyBalloon_Idle_State();
	return pInstance;
}

void CKirbyBalloon_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion





#pragma region RUN STATE

CKirbyBalloon_Run_State::CKirbyBalloon_Run_State()
{
}

void CKirbyBalloon_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBalloon_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// Idle일 때, C를 누르면 점프를 한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
	{
		// 점프의 초기 파워
		DESC(m_fJumpVelocity) = 22.f;
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_EATJUMP, 50.f, false, true, CKirby::BODY_BALLOON);

		DESC(m_fChangeVelocityZeroTime) = 0.f;
		// 공중에서 체공하는 시간 0.15초
		DESC(m_fHoldAirTime) = 0.f;
		// 점프키를 누르는 시간
		DESC(m_fJumpHoldTime) = 0.f;
		// 재입력 블락기능 초기화
		DESC(m_bRePressBlock) = false;
	}

	if (false == JoyStick_controller(Kirbydesc, pCamera))
		pKirby->Change_State(CKirby::STATE_EATWAIT, 60.f, true, true, CKirby::BODY_BALLOON);

	if (pController->Compute_Height() > 2.f)
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		DESC(m_isEatFall) = true;
		pKirby->Change_State(CKirby::STATE_EATJUMP, 50.f, false, true, CKirby::BODY_BALLOON);
	}

	// Eat 상태일 때 뱉는 모션을 취한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_SPIT, 70.f, false, false, CKirby::BODY_VACUUM);
	}

	Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	Turn_Z_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

}

void CKirbyBalloon_Run_State::OnStateExit()
{
}

CKirbyBalloon_Run_State* CKirbyBalloon_Run_State::Create()
{
	CKirbyBalloon_Run_State* pInstance = new CKirbyBalloon_Run_State();
	return pInstance;
}

void CKirbyBalloon_Run_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region JUMP STATE

CKirbyBalloon_Jump_State::CKirbyBalloon_Jump_State()
{

}

void CKirbyBalloon_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBalloon_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_UP))
	{
		// 때는 순간 트루가 되고, 점프 가능 시점에 다시 누를 때 까지 C에대한 누적 등 반응하지 않는다.
		DESC(m_bRePressBlock) = true;
	}

	if (pKirby->Get_State() == CKirby::STATE_EATLANDING)
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	else
		Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);


	if (JoyStick_controller(Kirbydesc, pCamera))
	{
		Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}
	else
	{
		if (pKirby->Get_State() == CKirby::STATE_EATLANDING)
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		else
			Jump_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}

	// Eat 상태일 때 뱉는 모션을 취한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_SPIT, 70.f, false, false, CKirby::BODY_VACUUM);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) && (pKirby->Get_State() == CKirby::STATE_EATJUMP))
	{
		if (pController->Compute_Height() < 3.f)
		{
			DESC(m_bReserveJumpKey) = true;
		}
	}

	// 떨어지는 것 (EAT상태는 점프와 FALL을 공유한다.)
	if (DESC(m_isEatFall) == true && pKirby->Get_State() == CKirby::STATE_EATJUMP)
	{
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (pController->Is_Terrain())
		{
			DESC(m_isEatFall) = false;
			pKirby->Change_State(CKirby::STATE_EATLANDING, 30.f, false, false, CKirby::BODY_BALLOON);

			if (CUtils::Make_RandomInt(0, 1) > 0)
				DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		}
	}
	// 점프
	else if (pKirby->Get_State() == CKirby::STATE_EATJUMP)
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

		// 만약, 땅에 안전하게 착지했을 경우, 홀딩 시간에 따라 뽀잉 애니메이션이 분기된다.
		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_EATLANDING, 50.f, false, false, CKirby::BODY_BALLOON);
			if (CUtils::Make_RandomInt(0, 1) > 0)
				DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		}


	}
	// 뽀잉
	else if (pKirby->Get_State() == CKirby::STATE_EATLANDING)
	{
		if (DESC(m_bReserveJumpKey) == true)
		{
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_EATJUMP, 50.f, false, true, CKirby::BODY_BALLOON);

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;
			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;
			// 예약 초기화
			DESC(m_bReserveJumpKey) = false;
			return;
		}


		// 최소 애니메이션이 재생되는 시간이다. ( 방향키를 누르면 0.2초 후 바로 Run 상태가 됨 )
		_float fChangeRunTime = 0.15f;

		m_fChangeRunTime += fTimeDelta;

		// Idle일 때, C를 누르면 점프를 한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;
			pKirby->Change_State(CKirby::STATE_EATJUMP, 50.f, false, true, CKirby::BODY_BALLOON);

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;
			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;

		}

		// 바로 방향키를 갈겼다면
		if (m_fChangeRunTime > fChangeRunTime && JoyStick_controller(Kirbydesc, pCamera))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_EATRUN, 100.f, true, true, CKirby::BODY_BALLOON);
		}
		// 자연스럽게 끝났다면
		else if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_EATWAIT, 60.f, true, true, CKirby::BODY_BALLOON);
		}
	}
}

void CKirbyBalloon_Jump_State::OnStateExit()
{
}

CKirbyBalloon_Jump_State* CKirbyBalloon_Jump_State::Create()
{
	CKirbyBalloon_Jump_State* pInstance = new CKirbyBalloon_Jump_State();
	return pInstance;
}

void CKirbyBalloon_Jump_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region FLY STATE

CKirbyBalloon_Fly_State::CKirbyBalloon_Fly_State()
{
}

void CKirbyBalloon_Fly_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBalloon_Fly_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// 최대 5.5초 비행가능하다.
	const _float fFlyTime = 5.5f;

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	if (JoyStick_controller(Kirbydesc, pCamera) == true)
		Fly_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	else
		Fly_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

	Kirbydesc->m_eMouthState = CKirby::MOUTH_ANGER;
	pKirby->DefaultIdle();

	if (pKirby->Get_State() != CKirby::STATE_FLIGHTLANDING)
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLANDING, 70.f, false, false, CKirby::BODY_VACUUM);
		}
	}


	if (pKirby->Get_State() == CKirby::STATE_FLIGHTSTART)
	{
		Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
		DESC(m_fFlyTime) += fTimeDelta;
		Kirbydesc->m_fJumpVelocity -= GRAVITY * fTimeDelta;
		if (Kirbydesc->m_fJumpVelocity < -2.f)
			Kirbydesc->m_fJumpVelocity = -2.f;
		pController->Jump(pTransformCom, Kirbydesc->m_fJumpVelocity, fTimeDelta);


		// 추가로 누르면 더 올라감!
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			if (pController->Compute_Height() > 5.f && pController->Compute_Height() < 19.f)
				DESC(m_fJumpVelocity) = 0.f;
			else
				DESC(m_fJumpVelocity) = 4.f;

			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_FLIGHT);
		}
		// 비행 시작 애니메이션이 끝났을 경우
		else if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTFALL, 60.f, true, true, CKirby::BODY_BALLOON);
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_FLIGHT)
	{

		Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
		DESC(m_fFlyTime) += fTimeDelta;
		Kirbydesc->m_fJumpVelocity -= GRAVITY * fTimeDelta;
		if (Kirbydesc->m_fJumpVelocity < -2.f)
			Kirbydesc->m_fJumpVelocity = -2.f;
		pController->Jump(pTransformCom, Kirbydesc->m_fJumpVelocity, fTimeDelta);
		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLANDING, 70.f, false, false, CKirby::BODY_VACUUM);
		}
		// 끝나면 FALL로 돌아간다.
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTFALL, 60.f, true, true, CKirby::BODY_BALLOON);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			if (pController->Compute_Height() > 5.f && pController->Compute_Height() < 19.f)
				DESC(m_fJumpVelocity) = 0.f;
			else
				DESC(m_fJumpVelocity) = 4.f;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_FLIGHT);
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_FLIGHTFALL)
	{
		Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
		DESC(m_fFlyTime) += fTimeDelta;
		Kirbydesc->m_fJumpVelocity -= GRAVITY * fTimeDelta;
		if (Kirbydesc->m_fJumpVelocity < -2.f)
			Kirbydesc->m_fJumpVelocity = -2.f;
		pController->Jump(pTransformCom, Kirbydesc->m_fJumpVelocity, fTimeDelta);
		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLANDING, 70.f, false, false, CKirby::BODY_VACUUM);
		}


		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS))
		{
			// 날면서 뽀잉
			if (pController->Compute_Height() > 5.f && pController->Compute_Height() < 19.f)
				DESC(m_fJumpVelocity) = 0.f;
			else
				DESC(m_fJumpVelocity) = 4.f;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_FLIGHT);
		}
		// 힘들경우 LIMITFALL 로 간다.
		if (DESC(m_fFlyTime) > fFlyTime)
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLIMITFALL, 60.f, false, false, CKirby::BODY_BALLOON);
		}


	}
	else if (pKirby->Get_State() == CKirby::STATE_FLIGHTLIMIT)
	{
		Kirbydesc->m_eEyeState = CKirby::EYE_CLOSE;
		DESC(m_fFlyTime) += fTimeDelta;
		Kirbydesc->m_fJumpVelocity -= GRAVITY * fTimeDelta;
		if (Kirbydesc->m_fJumpVelocity < -2.f)
			Kirbydesc->m_fJumpVelocity = -2.f;
		pController->Jump(pTransformCom, Kirbydesc->m_fJumpVelocity, fTimeDelta);
		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLANDING, 70.f, false, false, CKirby::BODY_VACUUM);
		}

		// 끝나면 FALL로 돌아간다.
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLIMITFALL, 60.f, true, true, CKirby::BODY_BALLOON);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
			pKirby->Set_Animation(CKirby::STATE_FLIGHTLIMIT, 60.f, false, false);


	}
	else if (pKirby->Get_State() == CKirby::STATE_FLIGHTLIMITFALL)
	{

		Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
		DESC(m_fFlyTime) += fTimeDelta;
		Kirbydesc->m_fJumpVelocity -= GRAVITY * fTimeDelta;
		if (Kirbydesc->m_fJumpVelocity < -2.f)
			Kirbydesc->m_fJumpVelocity = -2.f;
		pController->Jump(pTransformCom, Kirbydesc->m_fJumpVelocity, fTimeDelta);
		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_FLIGHTLANDING, 70.f, false, false, CKirby::BODY_VACUUM);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS))
		{
			// 힘들어 하기만 한다.
			pKirby->Change_State(CKirby::STATE_FLIGHTLIMIT, 60.f, false, false, CKirby::BODY_BALLOON);
		}


	}
	else if (pKirby->Get_State() == CKirby::STATE_FLIGHTLANDING)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
			Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		pController->FreeFall(pTransformCom, fTimeDelta, 1.5f);

		if (pKirby->isAnimFinish())
		{
			pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
			// 땅에 있을 때
			if (pController->Is_Terrain())
			{
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
				Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
				Kirbydesc->m_eMouthState = CKirby::MOUTH_IDLE;
				DESC(m_fFlyTime) = 0.f;
			}
			// 땅에 없을 때
			else
			{
				pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
				Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
				Kirbydesc->m_eMouthState = CKirby::MOUTH_IDLE;
				DESC(m_fFlyTime) = 0.f;
			}
		}
	}
}

void CKirbyBalloon_Fly_State::OnStateExit()
{
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	Kirbydesc->m_eEyeState = CKirby::EYE_IDLE;
	Kirbydesc->m_eMouthState = CKirby::MOUTH_IDLE;
}

CKirbyBalloon_Fly_State* CKirbyBalloon_Fly_State::Create()
{
	CKirbyBalloon_Fly_State* pInstance = new CKirbyBalloon_Fly_State();
	return pInstance;
}

void CKirbyBalloon_Fly_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region SWALLOW STATE


CKirbyBalloon_Swallow_State::CKirbyBalloon_Swallow_State()
{
}

void CKirbyBalloon_Swallow_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBalloon_Swallow_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	CTransform* pCameraTransform = pCamera->Get_TransformCom();
	_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
	DESC(m_vTargetDir) = vCamLook * -1.f;

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	m_pGameInstance->Set_SecondTimerRatio(0.f);
	m_pGameInstance->Set_BlackBackGround(true);


	if (pKirby->Get_State() == CKirby::STATE_SWALLOWSTART)
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		DESC(m_eMouthState) = CKirby::MOUTH_ANGER;

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_SWALLOWEND, 70.f, false, false, CKirby::BODY_BALLOON);
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_SWALLOWEND)
	{

		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		DESC(m_eMouthState) = CKirby::MOUTH_ANGER;

		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_IDLE;

			// 파츠 실시간 교체는 밖에서 해준다.
			pKirby->Set_AbilityType(DESC(m_eTemporaryEatType));
			// 이제 먹었으니까. 입에 머금고 있는 능력을 삭제한다.
			DESC(m_eTemporaryEatType) = ABILITY_END;

			pKirby->Change_State(CKirby::STATE_GETABILITY, 70.f, false, false, CKirby::BODY_DEFAULT);
		}
	}

}

void CKirbyBalloon_Swallow_State::OnStateExit()
{
}

CKirbyBalloon_Swallow_State* CKirbyBalloon_Swallow_State::Create()
{
	CKirbyBalloon_Swallow_State* pInstance = new CKirbyBalloon_Swallow_State();
	return pInstance;
}

void CKirbyBalloon_Swallow_State::Free()
{
	__super::Free();
}

#pragma endregion
