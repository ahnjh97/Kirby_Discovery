#include "stdafx.h"
#include "KirbySword_State.h"
#include "Kirby_State_Function.h"
#include "MultiEffect.h"

#pragma region SWORD IDLE STATE

CKirbySword_Idle_State::CKirbySword_Idle_State()
{
}

void CKirbySword_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbySword_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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
		pKirby->Change_State(CKirby::SWORDSTATE_RUN, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
	}

	Key_Z(pGameObject, fTimeDelta);
	Key_X(pGameObject, fTimeDelta);
	Key_C(pGameObject, fTimeDelta);
	Key_V(pGameObject, fTimeDelta);

}

void CKirbySword_Idle_State::Key_Z(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);

	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
	{
		pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
	}
}

void CKirbySword_Idle_State::Key_X(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// Idle일 때, X를 누르면 1타 공격을 시작한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_eEyeState) = CKirby::EYE_ANGER;

		// 조이스틱을 만지지 않았을 경우 현재 Dir 으로 공격 방향이 정해진다.
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == false)
			DESC(m_vAttackDir) = DESC(m_vMoveDir);

		if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_DECISIVESLASH)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SIDESLASH, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_SIDESLASH;
		}
		else if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_SIDESLASH)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_MULITSWORDATTACK, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_MULITSWORDATTACK;

		}
		else if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_MULITSWORDATTACK)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_DECISIVESLASH, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
			DESC(m_fMoveSpeed) = 0.f;

		}
	}

	// Idle일 때, X를 차징하면 기를 모은다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
	{

	}

}

void CKirbySword_Idle_State::Key_C(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

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

void CKirbySword_Idle_State::Key_V(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	// 능력을 땅에 버리는 로직이다.
	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
	{
		DESC(m_fDumpAbilityTime) += fTimeDelta;

		if (DESC(m_fDumpAbilityTime) > 1.f)
		{
			DESC(m_fDumpAbilityTime) = 0.f;
			pKirby->Change_State(CKirby::STATE_ABILITYDUMP, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	else
	{
		if (DESC(m_fDumpAbilityTime) > 0.f)
			DESC(m_fDumpAbilityTime) -= fTimeDelta * 2.f;

		if (DESC(m_fDumpAbilityTime) < 0.f)
			DESC(m_fDumpAbilityTime) = 0.f;
	}
}


void CKirbySword_Idle_State::OnStateExit()
{
	m_fChargeTime = 0.f;
}

CKirbySword_Idle_State* CKirbySword_Idle_State::Create()
{
	CKirbySword_Idle_State* pInstance = new CKirbySword_Idle_State();
	return pInstance;
}

void CKirbySword_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region SWORD RUN STATE

CKirbySword_Run_State::CKirbySword_Run_State()
{
}

void CKirbySword_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbySword_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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
		//vMyPos += pTransformCom->Get_State(CTransform::STATE_LOOK) * .4f;
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

	// 달리다가, C를 누르면 점프를 한다.
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

	// Run일 때, X를 누르면 1타 공격을 시작한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		DESC(m_eEyeState) = CKirby::EYE_ANGER;

		// 조이스틱을 만지지 않았을 경우 현재 Dir 으로 공격 방향이 정해진다.
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == false)
			DESC(m_vAttackDir) = DESC(m_vMoveDir);

		if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_DECISIVESLASH)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SIDESLASH, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_SIDESLASH;
		}
		else if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_SIDESLASH)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_MULITSWORDATTACK, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_MULITSWORDATTACK;

		}
		else if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_MULITSWORDATTACK)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_DECISIVESLASH, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_fMoveSpeed) = 0.f;
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;

		}
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
	{
		pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
	{
		DESC(m_fDumpAbilityTime) += fTimeDelta;

		if (DESC(m_fDumpAbilityTime) > 1.f)
		{
			DESC(m_fDumpAbilityTime) = 0.f;
			pKirby->Change_State(CKirby::STATE_ABILITYDUMP, 60.f, false, false, CKirby::BODY_DEFAULT);
		}
	}
	else
	{
		if (DESC(m_fDumpAbilityTime) > 0.f)
			DESC(m_fDumpAbilityTime) -= fTimeDelta * 2.f;

		if (DESC(m_fDumpAbilityTime) < 0.f)
			DESC(m_fDumpAbilityTime) = 0.f;
	}



	if (false == JoyStick_controller(Kirbydesc, pCamera))
		pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);

	if (pController->Compute_Height() > 2.f)
	{
		pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
	}

	Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	Turn_Z_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
}

void CKirbySword_Run_State::OnStateExit()
{


}

CKirbySword_Run_State* CKirbySword_Run_State::Create()
{
	CKirbySword_Run_State* pInstance = new CKirbySword_Run_State();
	return pInstance;
}

void CKirbySword_Run_State::Free()
{
	__super::Free();
}

#pragma endregion


// SILDE 포함!
#pragma region SWORD GUARD STATE

CKirbySword_Guard_State::CKirbySword_Guard_State()
{
}

void CKirbySword_Guard_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbySword_Guard_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	DESC(m_eEyeState) = CKirby::EYE_ANGER;

	if (pKirby->Get_State() == CKirby::SWORDSTATE_GUARD)
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
				pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDESTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			}
		}

		// Z키를 안누르고 있다면
		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS) == false)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		}
	}

	// 이 아래는 슬라이딩
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSLIDESTART)
	{
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDE, 60.f, true, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_fMoveSpeed) = 18.f;
		}
	}
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSLIDE)
	{
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vDodgeDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		DESC(m_fMoveSpeed) -= fTimeDelta * 30.f;

		if (DESC(m_fMoveSpeed) < 0.f)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDEEND, 120.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_fMoveSpeed) = 0.f;
		}

	}
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSLIDEEND)
	{
		if (pKirby->isAnimFinish())
		{
			if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
			{
				pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			}
		}
	}
}

void CKirbySword_Guard_State::OnStateExit()
{
}

CKirbySword_Guard_State* CKirbySword_Guard_State::Create()
{
	CKirbySword_Guard_State* pInstance = new CKirbySword_Guard_State();
	return pInstance;
}

void CKirbySword_Guard_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region SWORD ATTACK STATE

CKirbySword_Attack_State::CKirbySword_Attack_State()
{
}

void CKirbySword_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	m_fAttackSpeed = DESC(m_fMoveSpeed);
	m_fAnimTime = 0.f;
	m_fLockTime = 0.f;
}

void CKirbySword_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// 자유낙하 한다.
	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	if (pKirby->Get_State() == CKirby::SWORDSTATE_SIDESLASH)
	{
		m_fAnimTime += fTimeDelta;
		if (JoyStick_On() == true)
		{
			// 내가 누른 방향에 따라, 이동개념이 달라진다.
			JoyStick_controller_Attack(Kirbydesc, pCamera);
			CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

			DESC(m_fMoveSpeed) += fTimeDelta * 100.f;
			if (DESC(m_fMoveSpeed) > 20.f - (m_fAnimTime * 90.f))
				DESC(m_fMoveSpeed) = 20.f - (m_fAnimTime * 90.f);
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;

			if (eDir != CKirby::DIR_BACK)
			{
				_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
				pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
			}
		}
		else
		{
			if (DESC(m_fMoveSpeed) > 0.f)
				DESC(m_fMoveSpeed) -= 120.f * fTimeDelta;
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}


		// 한번이라도 애님도중에 눌렀을 경우, END또는 IDLE로 거치지 않고 바로 다음모션으로 넘어간다.
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			m_bPassNextAttackMotion = true;
		}

		if (pKirby->isAnimFinish())
		{
			// 다음 공격으로 넘어간다는 시그널이 있을 경우
			if (m_bPassNextAttackMotion == true)
			{
				pKirby->Change_State(CKirby::SWORDSTATE_MULITSWORDATTACK, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_MULITSWORDATTACK;
			}
			else
				pKirby->Change_State(CKirby::SWORDSTATE_SIDESLASHEND, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		}
	}


	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SIDESLASHEND)
	{
		// 0.1초간 풀 감속 (최대 속도 8이라 가정)
		if (DESC(m_fMoveSpeed) > 0.f)
			DESC(m_fMoveSpeed) -= 120.f * fTimeDelta;
		if (DESC(m_fMoveSpeed) < 0.f)
			DESC(m_fMoveSpeed) = 0.f;
		_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);



		// 엔드모션일 때, 뒤늦게 X키를 눌렀을 땐, 바로 다음 모션으로 보간되며 넘어간다.
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			pKirby->Change_State(CKirby::SWORDSTATE_MULITSWORDATTACK, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_MULITSWORDATTACK;
		}

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			// 0.7초간 유예시간을 준다.
			DESC(m_fAttackTime) = 0.5f;
		}
	}


	else if (pKirby->Get_State() == CKirby::SWORDSTATE_MULITSWORDATTACK)
	{
		m_fAnimTime += fTimeDelta;

		if (JoyStick_On() == true)
		{
			// 내가 누른 방향에 따라, 이동개념이 달라진다.
			JoyStick_controller_Attack(Kirbydesc, pCamera);
			CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

			DESC(m_fMoveSpeed) += fTimeDelta * 50.f;
			if (DESC(m_fMoveSpeed) > 6.f - (m_fAnimTime * 8.f))
				DESC(m_fMoveSpeed) = 6.f - (m_fAnimTime * 8.f);
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;

			if (eDir != CKirby::DIR_BACK)
			{
				_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
				pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
			}
		}
		else
		{
			if (DESC(m_fMoveSpeed) > 0.f)
				DESC(m_fMoveSpeed) -= 120.f * fTimeDelta;
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}



		// 한번이라도 애님도중에 눌렀을 경우, END또는 IDLE로 거치지 않고 바로 다음모션으로 넘어간다.
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			m_bPassNextAttackMotion = true;
		}


		if (pKirby->isAnimFinish())
		{
			// 다음 공격으로 넘어간다는 시그널이 있을 경우
			if (m_bPassNextAttackMotion == true)
			{
				pKirby->Change_State(CKirby::SWORDSTATE_DECISIVESLASH, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
				DESC(m_fMoveSpeed) = 0.f;
			}
			else
			{
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				// 0.7초간 유예시간을 준다.
				DESC(m_fAttackTime) = 0.5f;
			}
		}
	}


	// 마무리 모션
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_DECISIVESLASH)
	{
		m_fLockTime += fTimeDelta;

		if (m_fLockTime > 0.3f)
			m_fAnimTime += fTimeDelta;

		if (JoyStick_On() == true)
		{
			// 내가 누른 방향에 따라, 이동개념이 달라진다.
			JoyStick_controller_Attack(Kirbydesc, pCamera);
			CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

			DESC(m_fMoveSpeed) += fTimeDelta * 50.f;
			if (DESC(m_fMoveSpeed) > 20.f - (m_fAnimTime * 200.f))
				DESC(m_fMoveSpeed) = 20.f - (m_fAnimTime * 200.f);
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;

			if (eDir != CKirby::DIR_BACK)
			{
				_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
				pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
			}
		}
		else
		{
			if (DESC(m_fMoveSpeed) > 0.f)
				DESC(m_fMoveSpeed) -= 120.f * fTimeDelta;
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}



		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
		}
	}

}

void CKirbySword_Attack_State::OnStateExit()
{
	m_bPassNextAttackMotion = false;
}

CKirbySword_Attack_State* CKirbySword_Attack_State::Create()
{
	CKirbySword_Attack_State* pInstance = new CKirbySword_Attack_State();
	return pInstance;
}

void CKirbySword_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region SWORD CHARGESPIN STATE

CKirbySword_ChargeSpin_State::CKirbySword_ChargeSpin_State()
{
}

void CKirbySword_ChargeSpin_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbySword_ChargeSpin_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbySword_ChargeSpin_State::OnStateExit()
{
}

CKirbySword_ChargeSpin_State* CKirbySword_ChargeSpin_State::Create()
{
	CKirbySword_ChargeSpin_State* pInstance = new CKirbySword_ChargeSpin_State();
	return pInstance;
}

void CKirbySword_ChargeSpin_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region SWORD JUMPATTACK STATE

CKirbySword_JumpAttack_State::CKirbySword_JumpAttack_State()
{
}

void CKirbySword_JumpAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbySword_JumpAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbySword_JumpAttack_State::OnStateExit()
{
}

CKirbySword_JumpAttack_State* CKirbySword_JumpAttack_State::Create()
{
	CKirbySword_JumpAttack_State* pInstance = new CKirbySword_JumpAttack_State();
	return pInstance;
}

void CKirbySword_JumpAttack_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region SWORD FLY STATE

CKirbySword_Fly_State::CKirbySword_Fly_State()
{
}

void CKirbySword_Fly_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbySword_Fly_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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
		pKirby->Set_Animation((CKirby::STATE)(CKirby::SWORDSTATE_HAVESWORDWAITFLIGHT - CKirby::OFFSET_SWORD), 60.f, false, false);
	}
}

void CKirbySword_Fly_State::OnStateExit()
{
}

CKirbySword_Fly_State* CKirbySword_Fly_State::Create()
{
	CKirbySword_Fly_State* pInstance = new CKirbySword_Fly_State();
	return pInstance;
}

void CKirbySword_Fly_State::Free()
{
	__super::Free();
}

#pragma endregion
