#include "stdafx.h"
#include "KirbyDefault_State.h"
#include "Kirby_State_Function.h"
#include "MultiEffect.h"

#pragma region IDLE STATE

CKirbyDefault_Idle_State::CKirbyDefault_Idle_State()
{
}

void CKirbyDefault_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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
	if (pController->Compute_Height() > 2.f)
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
	}

	// Idle일 때, 방향키를 눌렀을 때 RUN 으로 간다.
	if (JoyStick_controller(Kirbydesc, pCamera))
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);
	}

	Key_Z(pGameObject, fTimeDelta);
	Key_X(pGameObject, fTimeDelta);
	Key_C(pGameObject, fTimeDelta);
	Key_V(pGameObject, fTimeDelta);
	Key_Happy(pGameObject, fTimeDelta);

	// IDLE상태에서 통제될 것들.
	if (pKirby->Get_State() == CKirby::STATE_IDLE)
	{
		m_fIdleStreachTime += fTimeDelta;
		// 멍 때리면 다른 Idle 모션을 연출한다.
		if (m_fIdleStreachTime > 5.f)
		{
			m_iIdleChoose == 0 ?
				pKirby->Change_State(CKirby::STATE_IDLESTREACH, 60.f, false, true, CKirby::BODY_DEFAULT) :
				pKirby->Change_State(CKirby::STATE_IDLELOOKAROUND, 60.f, false, true, CKirby::BODY_DEFAULT);

			m_iIdleChoose == 0 ?
				m_iIdleChoose++ : m_iIdleChoose--;
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_IDLESTREACH || pKirby->Get_State() == CKirby::STATE_IDLELOOKAROUND)
	{
		if (pKirby->Get_State() == CKirby::STATE_IDLESTREACH)
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
		}
	}
}

void CKirbyDefault_Idle_State::Key_Z(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);

	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
	{
		pKirby->Change_State(CKirby::STATE_GUARD, 60.f, true, true, CKirby::BODY_DEFAULT);
	}
}

void CKirbyDefault_Idle_State::Key_X(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// Idle일 때, X를 누르면 흡수를 시작한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_fVacuumTime) = 0.f;
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_INHALESTART, 60.f, false, false, CKirby::BODY_VACUUM);

		CMultiEffect::MULTI_FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, .6f, .4f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_VacuumC"), &FXDesc)))
			return;
		pKirby->Add_KirbyEffect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

	}
}

void CKirbyDefault_Idle_State::Key_C(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// Idle일 때, C를 누르면 점프를 한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
	{
		// 점프의 초기 파워
		DESC(m_fJumpVelocity) = 22.f;
		DESC(m_eEyeState) = CKirby::EYE_IDLE;

		DESC(m_eJumpState) == DESC(m_eJumpState) ? CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;
		pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);

		DESC(m_fChangeVelocityZeroTime) = 0.f;
		// 공중에서 체공하는 시간 0.15초
		DESC(m_fHoldAirTime) = 0.f;
		// 점프키를 누르는 시간
		DESC(m_fJumpHoldTime) = 0.f;

		// 재입력 블락기능 초기화
		DESC(m_bRePressBlock) = false;
	}

}

void CKirbyDefault_Idle_State::Key_V(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbyDefault_Idle_State::Key_Happy(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_1, KEY_DOWN))
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_eMouthState) = CKirby::MOUTH_SMILE;
			pKirby->Change_State(CKirby::STATE_EMOTEWAVEHAND, 60.f, false, true, CKirby::BODY_DEFAULT);
		}
		else if (m_pGameInstance->Get_DIKeyState(DIK_2, KEY_DOWN))
		{
			pKirby->Change_State(CKirby::STATE_WAITYAY, 60.f, false, true, CKirby::BODY_DEFAULT);

		}
		else if (m_pGameInstance->Get_DIKeyState(DIK_3, KEY_DOWN))
		{
			pKirby->Change_State(CKirby::STATE_WAITSIT, 60.f, false, true, CKirby::BODY_DEFAULT);

		}
	}
}

void CKirbyDefault_Idle_State::OnStateExit()
{
	m_fIdleStreachTime = 0.f;
}

CKirbyDefault_Idle_State* CKirbyDefault_Idle_State::Create()
{
	CKirbyDefault_Idle_State* pInstance = new CKirbyDefault_Idle_State();
	return pInstance;
}

void CKirbyDefault_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region RUN STATE

CKirbyDefault_Run_State::CKirbyDefault_Run_State()
{
}

void CKirbyDefault_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


#pragma region 가라 이펙트 세팅
	static _float fBbongTime{ 0.f };
	fBbongTime += fTimeDelta;
	if (.2f < fBbongTime)
	{
		CMultiEffect::MULTI_FX_DESC FXDesc{};
		_float4 vMyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		vMyPos += pTransformCom->Get_State(CTransform::STATE_LOOK) * .4f;
		FXDesc.vInitPos = { vMyPos.x, vMyPos.y + .3f, vMyPos.z };
		FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };

		_float3 vDir = -pTransformCom->Get_State(CTransform::STATE_LOOK);
		vDir.Normalize();
		_float3 vLook = { 0.f, 0.f, 1.f };

		_float fAngleLook = atan2f(vLook.z, vLook.x);
		_float fAngleDiff = fAngleLook - atan2f(vDir.z, vDir.x);
		fAngleDiff = ToDegree(fAngleDiff);

		_float3 vAngle = { 0.f, fAngleDiff, 0.f };
		FXDesc.vInitRot = vAngle;

		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
			return;

		fBbongTime = 0.f;

	}

#pragma endregion



	// Idle일 때, C를 누르면 점프를 한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
	{
		// 점프의 초기 파워
		DESC(m_fJumpVelocity) = 22.f;

		DESC(m_eJumpState) == CKirby::STATE_JUMPL ? DESC(m_eJumpState) = CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;
		pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);

		DESC(m_fChangeVelocityZeroTime) = 0.f;
		// 공중에서 체공하는 시간 0.15초
		DESC(m_fHoldAirTime) = 0.f;
		// 점프키를 누르는 시간
		DESC(m_fJumpHoldTime) = 0.f;
		// 재입력 블락기능 초기화
		DESC(m_bRePressBlock) = false;
	}

	// X를 누르면 흡수를 시작한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_fVacuumTime) = 0.f;
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_INHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);

		CMultiEffect::MULTI_FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, .65f, .4f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_VacuumC"), &FXDesc)))
			return;
		pKirby->Add_KirbyEffect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
	{
		pKirby->Change_State(CKirby::STATE_GUARD, 60.f, true, true, CKirby::BODY_DEFAULT);
	}

	if (false == JoyStick_controller(Kirbydesc, pCamera))
		pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);

	// 큰 회전이였을 경우
	if (Is_BigTurn(Kirbydesc) == true)
		pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);

	if (pController->Compute_Height() > 2.f)
	{
		pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
	}

	if (pKirby->Get_State() == CKirby::STATE_RUNSTART)
	{
		m_fRunStartTime += fTimeDelta;

		if (m_fRunStartTime > 0.8f)
			pKirby->Change_State(CKirby::STATE_RUN, 100.f, true, false, CKirby::BODY_DEFAULT);
	}

	Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	Turn_Z_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
}
void CKirbyDefault_Run_State::OnStateExit()
{
	m_fRunStartTime = 0.f;
}

CKirbyDefault_Run_State* CKirbyDefault_Run_State::Create()
{
	CKirbyDefault_Run_State* pInstance = new CKirbyDefault_Run_State();
	return pInstance;
}

void CKirbyDefault_Run_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region JUMP STATE

CKirbyDefault_Jump_State::CKirbyDefault_Jump_State()
{
}

void CKirbyDefault_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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


	if (pKirby->Get_State() == CKirby::STATE_LANDINGSMALL || pKirby->Get_State() == CKirby::STATE_LANDINGEND)
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	else
		Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);


	if (JoyStick_controller(Kirbydesc, pCamera))
	{
		Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}
	else
	{
		if (pKirby->Get_State() == CKirby::STATE_LANDINGSMALL || pKirby->Get_State() == CKirby::STATE_LANDINGEND)
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		else
			Jump_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}


	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_fVacuumTime) = 0.f;
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::STATE_INHALEFALL, 50.f, true, true, CKirby::BODY_VACUUM);

		CMultiEffect::MULTI_FX_DESC FXDesc{};
		FXDesc.vInitPos = { 0.f, .65f, .4f };
		FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
		if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_VacuumC"), &FXDesc)))
			return;
		pKirby->Add_KirbyEffect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

	}

	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) &&
		(pKirby->Get_State() == CKirby::STATE_LANDINGSMALL || pKirby->Get_State() == CKirby::STATE_LANDINGEND) == false)
	{
		if (pController->Compute_Height() < 3.f &&
			(pKirby->Get_State() == CKirby::STATE_JUMPL || pKirby->Get_State() == CKirby::STATE_JUMPR ||
				pKirby->Get_State() == CKirby::STATE_JUMPEND))
		{
			DESC(m_bReserveJumpKey) = true;
		}
		else
		{
			DESC(m_fFlyTime) = 0.f;
			DESC(m_fJumpVelocity) = 4.f;
			pKirby->Change_State(CKirby::STATE_FLIGHTSTART, 60.f, false, false, CKirby::BODY_BALLOON);
		}
	}




	// 떨어지는 것
	if (pKirby->Get_State() == CKirby::STATE_FALL)
	{
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
		m_fFallTime += fTimeDelta;

		if (pController->Is_Terrain())
		{
			if (m_fFallTime > 1.f)
			{
				pKirby->Change_State(CKirby::STATE_LANDINGEND, 30.f, false, false, CKirby::BODY_DEFAULT);
				if (CUtils::Make_RandomInt(0, 1) > 0)
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;
				m_fFallTime = 0.f;
			}
			else
			{
				pKirby->Change_State(CKirby::STATE_LANDINGSMALL, 50.f, false, false, CKirby::BODY_DEFAULT);
				if (CUtils::Make_RandomInt(0, 1) > 0)
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;

				m_fFallTime = 0.f;
			}
		}
	}
	// 점프
	else if (pKirby->Get_State() == CKirby::STATE_JUMPL || pKirby->Get_State() == CKirby::STATE_JUMPR
		|| pKirby->Get_State() == CKirby::STATE_JUMPEND)
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
			pKirby->Change_State(CKirby::STATE_JUMPEND, 60.f, false, true, CKirby::BODY_DEFAULT);
		}

		// 만약, 땅에 안전하게 착지했을 경우, 홀딩 시간에 따라 뽀잉 애니메이션이 분기된다.
		if (pController->Is_Terrain())
		{
			if (DESC(m_fJumpHoldTime) > 0.2f)
			{
				pKirby->Change_State(CKirby::STATE_LANDINGEND, 30.f, false, false, CKirby::BODY_DEFAULT);
				if (CUtils::Make_RandomInt(0, 1) > 0)
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;


#pragma region 가라 이펙트 세팅
				CMultiEffect::MULTI_FX_DESC FXDesc{};
				_float4 vMyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
				//vMyPos += pTransformCom->Get_State(CTransform::STATE_LOOK) * 1.2f;
				FXDesc.vInitPos = { vMyPos.x, vMyPos.y + .4f, vMyPos.z };
				FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };

				//_float3 vDir = -pTransformCom->Get_State(CTransform::STATE_LOOK);
				//vDir.Normalize();
				//_float3 vLook = { 0.f, 0.f, 1.f };

				//_float fAngleLook = atan2f(vLook.z, vLook.x);
				//_float fAngleDiff = fAngleLook - atan2f(vDir.z, vDir.x);
				//fAngleDiff = ToDegree(fAngleDiff);

				//_float3 vAngle = { 0.f, fAngleDiff, 0.f };

				FXDesc.vInitRot = {0.f, CUtils::Make_RandomFloat(0.f, 360.f), 0.f};

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 360.f), 0.f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 360.f), 0.f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 360.f), 0.f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 360.f), 0.f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;

				FXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 360.f), 0.f };

				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
					return;
#pragma endregion

			}
			else
			{
				pKirby->Change_State(CKirby::STATE_LANDINGSMALL, 50.f, false, false, CKirby::BODY_DEFAULT);
				if (CUtils::Make_RandomInt(0, 1) > 0)
					DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			}
		}


	}
	// 뽀잉
	else if (pKirby->Get_State() == CKirby::STATE_LANDINGEND || pKirby->Get_State() == CKirby::STATE_LANDINGSMALL)
	{
		if (DESC(m_bReserveJumpKey) == true)
		{
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;

			DESC(m_eJumpState) == CKirby::STATE_JUMPL ? DESC(m_eJumpState) = CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;
			pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);

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
		_float fChangeRunTime = 0.08f;

		m_fChangeRunTime += fTimeDelta;

		// Idle일 때, C를 누르면 점프를 한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			// 점프의 초기 파워
			DESC(m_fJumpVelocity) = 22.f;

			DESC(m_eJumpState) == DESC(m_eJumpState) ? CKirby::STATE_JUMPR : DESC(m_eJumpState) = CKirby::STATE_JUMPL;
			pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);

			DESC(m_fChangeVelocityZeroTime) = 0.f;
			// 공중에서 체공하는 시간 0.15초
			DESC(m_fHoldAirTime) = 0.f;
			// 점프키를 누르는 시간
			DESC(m_fJumpHoldTime) = 0.f;
			// 재입력 블락기능 초기화
			DESC(m_bRePressBlock) = false;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
		{
			pKirby->Change_State(CKirby::STATE_GUARD, 60.f, true, true, CKirby::BODY_DEFAULT);
		}

		// 바로 방향키를 갈겼다면
		if (m_fChangeRunTime > fChangeRunTime && JoyStick_controller(Kirbydesc, pCamera))
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);
		}
		// 자연스럽게 끝났다면
		else if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
		}
	}
}

void CKirbyDefault_Jump_State::OnStateExit()
{
	m_fChangeRunTime = 0.f;
}

CKirbyDefault_Jump_State* CKirbyDefault_Jump_State::Create()
{
	CKirbyDefault_Jump_State* pInstance = new CKirbyDefault_Jump_State();
	return pInstance;
}

void CKirbyDefault_Jump_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region GUARD STATE

CKirbyDefault_Guard_State::CKirbyDefault_Guard_State()
{
}

void CKirbyDefault_Guard_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Guard_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	DESC(m_eEyeState) = CKirby::EYE_ANGER;

	if (pKirby->Get_State() == CKirby::STATE_GUARD)
	{

		Guard_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_DOWN) ||
			m_pGameInstance->Get_DIKeyState(DIK_DOWN, KEY_DOWN) ||
			m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_DOWN) ||
			m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_DOWN))
		{
			pKirby->Change_State(CKirby::STATE_DODGESTART, 50.f, false, false, CKirby::BODY_DEFAULT);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			if (JoyStick_controller(Kirbydesc, pCamera))
				pKirby->Change_State(CKirby::STATE_DODGESTART, 50.f, false, false, CKirby::BODY_DEFAULT);
			else
			{
				_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
				DESC(m_vDodgeDir) = vLook;
				pKirby->Change_State(CKirby::STATE_SLIDESTART, 60.f, false, false, CKirby::BODY_DEFAULT);
			}
		}

		// Z키를 안누르고 있다면
		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS) == false)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
		}
	}
	// 닷지 시작
	else if (pKirby->Get_State() == CKirby::STATE_DODGESTART)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (GAMEINSTANCE Get_DIKeyState(DIK_UP, KEY_PRESS))
		{
			if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
				DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_LF, pCamera);
			else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
				DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_RF, pCamera);
			else
				DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_FRONT, pCamera);
		}
		else if (GAMEINSTANCE Get_DIKeyState(DIK_DOWN, KEY_PRESS))
		{
			if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
				DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_LB, pCamera);
			else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
				DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_RB, pCamera);
			else
				DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_BACK, pCamera);
		}
		else if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
		{
			DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_LEFT, pCamera);
		}
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
		{
			DESC(m_vDodgeDir) = Make_TargetDir(CKirby::DIR_RIGHT, pCamera);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS))
		{
			DESC(m_vMoveDir) = DESC(m_vDodgeDir);
			pKirby->Change_State(CKirby::STATE_SLIDESTART, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
		// 끝나면 바로 본격적인 닷지 스타트 ( 여기서 애니메이션을 분기한다.)
		if (pKirby->isAnimFinish() == true)
		{
			CKirby::DIR eDir = Kirby_Standard_Angle(Kirbydesc);
			DESC(m_fJumpVelocity) = 12.f;

			if (eDir == CKirby::DIR_FRONT)
				pKirby->Change_State(CKirby::STATE_DODGEFRONT1, 50.f, false, false, CKirby::BODY_DEFAULT);
			else if (eDir == CKirby::DIR_BACK)
				pKirby->Change_State(CKirby::STATE_DODGEBACK1, 50.f, false, false, CKirby::BODY_DEFAULT);
			else if (eDir == CKirby::DIR_LEFT)
				pKirby->Change_State(CKirby::STATE_DODGELEFT1, 50.f, false, false, CKirby::BODY_DEFAULT);
			else if (eDir == CKirby::DIR_RIGHT)
				pKirby->Change_State(CKirby::STATE_DODGERIGHT1, 50.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	// BACK
	else if (pKirby->Get_State() == CKirby::STATE_DODGEBACK1)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			DESC(m_fJumpVelocity) = 9.f;
			pKirby->Change_State(CKirby::STATE_DODGEBACK2, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_DODGEBACK2)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_fMoveSpeed) = 0.f;
		}
	}

	// FRONT
	else if (pKirby->Get_State() == CKirby::STATE_DODGEFRONT1)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			DESC(m_fJumpVelocity) = 9.f;
			pKirby->Change_State(CKirby::STATE_DODGEFRONT2, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_DODGEFRONT2)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_fMoveSpeed) = 0.f;

		}
	}

	// LEFT
	else if (pKirby->Get_State() == CKirby::STATE_DODGELEFT1)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			DESC(m_fJumpVelocity) = 9.f;
			pKirby->Change_State(CKirby::STATE_DODGELEFT2, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_DODGELEFT2)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_fMoveSpeed) = 0.f;
		}
	}

	// RIGHT
	else if (pKirby->Get_State() == CKirby::STATE_DODGERIGHT1)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			DESC(m_fJumpVelocity) = 9.f;
			pKirby->Change_State(CKirby::STATE_DODGERIGHT2, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_DODGERIGHT2)
	{
		Dodge_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() == true)
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_fMoveSpeed) = 0.f;
		}
	}

}

void CKirbyDefault_Guard_State::OnStateExit()
{
}

CKirbyDefault_Guard_State* CKirbyDefault_Guard_State::Create()
{
	CKirbyDefault_Guard_State* pInstance = new CKirbyDefault_Guard_State();
	return pInstance;
}

void CKirbyDefault_Guard_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region SILDE STATE

CKirbyDefault_Slide_State::CKirbyDefault_Slide_State()
{
}

void CKirbyDefault_Slide_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Slide_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	if (pKirby->Get_State() == CKirby::STATE_SLIDESTART)
	{
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_SLIDE, 60.f, true, false, CKirby::BODY_DEFAULT);
			DESC(m_fMoveSpeed) = 18.f;
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_SLIDE)
	{
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vDodgeDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		DESC(m_fMoveSpeed) -= fTimeDelta * 30.f;

		if (DESC(m_fMoveSpeed) < 0.f)
		{
			pKirby->Change_State(CKirby::STATE_SLIDEEND, 120.f, false, false, CKirby::BODY_DEFAULT);
			DESC(m_fMoveSpeed) = 0.f;
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_SLIDEEND)
	{
		if (pKirby->isAnimFinish())
		{
			if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
			{
				pKirby->Change_State(CKirby::STATE_GUARD, 60.f, true, true, CKirby::BODY_DEFAULT);
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, false, CKirby::BODY_DEFAULT);
			}
		}
	}
}

void CKirbyDefault_Slide_State::OnStateExit()
{
}

CKirbyDefault_Slide_State* CKirbyDefault_Slide_State::Create()
{
	CKirbyDefault_Slide_State* pInstance = new CKirbyDefault_Slide_State();
	return pInstance;
}

void CKirbyDefault_Slide_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region HAPPY STATE

CKirbyDefault_Happy_State::CKirbyDefault_Happy_State()
{

}

void CKirbyDefault_Happy_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Happy_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	m_fHappyTime += fTimeDelta;

	if (pKirby->Get_State() == CKirby::STATE_WAITYAY)
	{
		if (m_fHappyTime > 0.3f && m_fHappyTime < 0.45f)
		{
			DESC(m_eEyeState) = CKirby::EYE_BLINK;
		}
		else if (m_fHappyTime > 0.45f && m_fHappyTime < 1.f)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_SMILE;
		}
		else if (m_fHappyTime > 1.f)
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			DESC(m_eMouthState) = CKirby::MOUTH_HAPPY;
		}


	}
	else if (pKirby->Get_State() == CKirby::STATE_WAITSIT)
	{
		if (m_fHappyTime > 0.3f)
		{
			DESC(m_eMouthState) = CKirby::MOUTH_SMILE;
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_EMOTEWAVEHAND)
	{
		if (m_fHappyTime > 0.35f && m_fHappyTime < 1.35f)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_HAPPY;
		}
		else if (m_fHappyTime > 1.35f)
		{
			DESC(m_eEyeState) = CKirby::EYE_BLINK;
			DESC(m_eMouthState) = CKirby::MOUTH_SMILE;
		}
	}



	if (JoyStick_controller(Kirbydesc, pCamera))
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		DESC(m_eMouthState) = CKirby::MOUTH_IDLE;
		pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);
	}
	if (pKirby->isAnimFinish())
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		DESC(m_eMouthState) = CKirby::MOUTH_IDLE;
		pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
	}

}

void CKirbyDefault_Happy_State::OnStateExit()
{
	m_fHappyTime = 0.f;
}

CKirbyDefault_Happy_State* CKirbyDefault_Happy_State::Create()
{
	CKirbyDefault_Happy_State* pInstance = new CKirbyDefault_Happy_State();
	return pInstance;
}

void CKirbyDefault_Happy_State::Free()
{
	__super::Free();
}

#pragma endregion
