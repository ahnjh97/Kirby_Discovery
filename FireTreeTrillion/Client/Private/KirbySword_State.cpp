#include "stdafx.h"
#include "KirbySword_State.h"
#include "Kirby_State_Function.h"

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

	//// 낙하 높이
	//if (pController->Compute_Height() > 2.f)
	//{
	//	DESC(m_eEyeState) = CKirby::EYE_IDLE;
	//	pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
	//	return;
	//}

	// Idle일 때, 방향키를 눌렀을 때 RUN 으로 간다.
	if (JoyStick_controller(Kirbydesc, pCamera))
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		pKirby->Change_State(CKirby::SWORDSTATE_RUN, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		return;
	}

	Key_Z(pGameObject, fTimeDelta);
	Key_X(pGameObject, fTimeDelta);
	Key_C(pGameObject, fTimeDelta);
	Key_V(pGameObject, fTimeDelta);

}

_bool CKirbySword_Idle_State::Key_Z(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);

	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
	{
		pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		return true;
	}

	return false;
}

_bool CKirbySword_Idle_State::Key_X(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// Idle일 때, X를 누르면 1타 공격을 시작한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBlockOtherVacuum) == false)
	{
		DESC(m_eEyeState) = CKirby::EYE_ANGER;

		// 조이스틱을 만지지 않았을 경우 현재 Dir 으로 공격 방향이 정해진다.
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == false)
			DESC(m_vAttackDir) = DESC(m_vMoveDir);

		if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_DECISIVESLASH)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SIDESLASH, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_SIDESLASH;
			return true;

		}
		else if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_SIDESLASH)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_MULITSWORDATTACK, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_MULITSWORDATTACK;
			return true;
		}
		else if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_MULITSWORDATTACK)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_DECISIVESLASH, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
			DESC(m_fMoveSpeed) = 0.f;
			return true;
		}
	}

	// Idle일 때, X를 차징하면 기를 모은다. 그러나 키를 누르지 않으면 차징 시간이 0이 된다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
	{
		DESC(m_fChargeTime) += fTimeDelta;

		if (DESC(m_fChargeTime) > 0.3f)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHCHARGE, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			// 다시 차징시간 0.f 부터 시작한다.
			DESC(m_fChargeTime) = 0.f;
			return true;
		}
	}
	else
		DESC(m_fChargeTime) = 0.f;

	return false;
}

_bool CKirbySword_Idle_State::Key_C(CGameObject* pGameObject, _float fTimeDelta)
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

		DESC(m_fChangeVelocityZeroTime) = 0.f;
		// 공중에서 체공하는 시간 0.15초
		DESC(m_fHoldAirTime) = 0.f;
		// 점프키를 누르는 시간
		DESC(m_fJumpHoldTime) = 0.f;

		// 재입력 블락기능 초기화
		DESC(m_bRePressBlock) = false;

		pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);
		return true;
	}

	return false;
}

_bool CKirbySword_Idle_State::Key_V(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	// 능력을 땅에 버리는 로직이다.
	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
	{
		DESC(m_bDumpAbilityPress) = true;
		DESC(m_fDumpAbilityTime) += fTimeDelta;

		if (DESC(m_fDumpAbilityTime) > 1.f)
		{
			DESC(m_fDumpAbilityTime) = 0.f;
			pKirby->Change_State(CKirby::STATE_ABILITYDUMP, 60.f, false, false, CKirby::BODY_DEFAULT);
			return true;
		}
	}
	else
	{
		DESC(m_bDumpAbilityPress) = false;
	}

	return false;
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
		pKirby->Change_State(DESC(m_eJumpState), 50.f, false, true, CKirby::BODY_DEFAULT);
		return;
	}

	// Run일 때, X를 누르면 1타 공격을 시작한다.
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBlockOtherVacuum) == false)
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

		return;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
	{
		pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
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
		pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		return;
	}

	if (pController->Compute_Height() > 2.f)
	{
		pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
		return;
	}

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
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));

	switch (_iAnimIndex)
	{
	case CKirby::SWORDSTATE_SWORDSLIDESTART:
	{
		SwordDash(pKirby->Get_TransformCom());
		pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	}
	break;
	default:
		break;
	}
}

void CKirbySword_Guard_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


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
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			if (JoyStick_controller(Kirbydesc, pCamera))
			{
				pKirby->Change_State(CKirby::STATE_DODGESTART, 50.f, false, false, CKirby::BODY_DEFAULT);
				return;
			}
			else
			{
				_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
				DESC(m_vDodgeDir) = vLook;
				pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDESTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
		}

		// Z키를 안누르고 있다면
		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS) == false)
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

	// 이 아래는 슬라이딩
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSLIDESTART)
	{
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDE, 60.f, true, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_fMoveSpeed) = 18.f;
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSLIDE)
	{
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vDodgeDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		DESC(m_fMoveSpeed) -= fTimeDelta * 30.f;

		if (DESC(m_fMoveSpeed) < 0.f)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDEEND, 120.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			DESC(m_fMoveSpeed) = 0.f;
			return;
		}

		// 슬라이드 중에 C를 누르면 공중 공격을 한다.
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_fJumpVelocity) = 22.f;
			pKirby->Change_State(CKirby::SWORDSTATE_UPWARDSLASH, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}

	}
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSLIDEEND)
	{
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (pKirby->isAnimFinish())
		{
			if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_PRESS))
			{
				pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
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


	
	switch (_iAnimIndex)
	{
	case CKirby::SWORDSTATE_SIDESLASH:
	{
		SwordSlash_One(pKirby->Get_TransformCom());
	}
	break;
	case CKirby::SWORDSTATE_MULITSWORDATTACK:
	{
		SwordSlash_Two(pKirby->Get_TransformCom());
	}
	break;
	case CKirby::SWORDSTATE_DECISIVESLASH:
	{
		SwordSlash_Final(pKirby->Get_TransformCom());
	}
	break;
	default:
		break;
	}
	
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


	// 공격중에도 차징시간이 계산된다. (모션이 끝나면 바로 충전을 시작함)
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
	{
		DESC(m_fChargeTime) += fTimeDelta;
	}
	else
		DESC(m_fChargeTime) = 0.f;


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
			if (DESC(m_fChargeTime) > 0.15f)
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				// 다시 차징시간 0.f 부터 시작한다.
				DESC(m_fChargeTime) = 0.f;
				pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHCHARGE, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}

			// 다음 공격으로 넘어간다는 시그널이 있을 경우
			if (m_bPassNextAttackMotion == true)
			{
				pKirby->Change_State(CKirby::SWORDSTATE_MULITSWORDATTACK, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_MULITSWORDATTACK;
				return;
			}
			else
			{
				pKirby->Change_State(CKirby::SWORDSTATE_SIDESLASHEND, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}


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
			return;
		}

		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			// 0.7초간 유예시간을 준다.
			DESC(m_fAttackTime) = 0.5f;
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
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
			if (DESC(m_fChargeTime) > 0.15f)
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				// 다시 차징시간 0.f 부터 시작한다.
				DESC(m_fChargeTime) = 0.f;
				pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHCHARGE, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}

			// 다음 공격으로 넘어간다는 시그널이 있을 경우
			if (m_bPassNextAttackMotion == true)
			{
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
				DESC(m_fMoveSpeed) = 0.f;
				pKirby->Change_State(CKirby::SWORDSTATE_DECISIVESLASH, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;

			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				// 0.7초간 유예시간을 준다.
				DESC(m_fAttackTime) = 0.5f;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;

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
			if (DESC(m_fChargeTime) > 0.15f)
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				// 다시 차징시간 0.f 부터 시작한다.
				DESC(m_fChargeTime) = 0.f;
				pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHCHARGE, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
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
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));

	switch (_iAnimIndex)
	{
	case CKirby::SWORDSTATE_SPINSLASHCHARGE:
	{
		SwordSpinCharge(pKirby->Get_TransformCom());
	}
	break;
	case CKirby::SWORDSTATE_GIGANTSPINSLASH:
	{
		SwordSpinSlash_Two(pKirby->Get_TransformCom());
	}
	break;
	case CKirby::SWORDSTATE_SUPERSPINSLASHCHARGE:
	{
		SwordSpinCharge(pKirby->Get_TransformCom());
	}
	break;
	case CKirby::SWORDSTATE_SUPERSPINSLASHLOOP:
	{
		SwordSpinSlash_One(pKirby->Get_TransformCom());
	}
	break;
	default:
		break;
	}
}

void CKirbySword_ChargeSpin_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	// 기본 차징 모션이다. 
	if (pKirby->Get_State() == CKirby::SWORDSTATE_SPINSLASHCHARGE)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_bWalkingCharge) = true;

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			DESC(m_fChargeTime) += fTimeDelta;

			if (JoyStick_On() == true)
			{
				// 내가 누른 방향에 따라, 이동개념이 달라진다.
				JoyStick_controller_Attack(Kirbydesc, pCamera);
				CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

				if (eDir == CKirby::DIR_FRONT || eDir == CKirby::DIR_BACK)
				{
					pKirby->Change_State(CKirby::SWORDSTATE_SHUFFIEFRONT, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
					return;
				}
				else
				{
					pKirby->Change_State(CKirby::SWORDSTATE_SHUFFIERIGHT, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
					return;
				}
			}

			// 1초간 차징시간이 더 추가되었을 때, 슈퍼 차지모드로 변경된다.
			if (DESC(m_fChargeTime) > 1.f)
			{
				pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHCHARGESTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
		}
		else
		{
			if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_SIDESLASH && DESC(m_fChargeTime) < 0.2f)
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				pKirby->Change_State(CKirby::SWORDSTATE_GIGANTSPINSLASH, 60.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
		}
	}

	// 슈퍼차징 시작애님.
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SUPERSPINSLASHCHARGESTART)
	{
		DESC(m_bWalkingCharge) = false;

		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == true)
		{
			DESC(m_fMoveSpeed) += fTimeDelta * 10.f;
			if (DESC(m_fMoveSpeed) > 3.f)
				DESC(m_fMoveSpeed) = 3.f;

			// 타겟기준
			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			DESC(m_fChargeTime) += fTimeDelta;
		}
		// 이때 X를 떼면 바로 슈퍼스핀.
		else
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHSTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}

		// 이게 핵심이다.
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHCHARGE, 60.f, true, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

	// 슈퍼차징 중
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SUPERSPINSLASHCHARGE)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			DESC(m_fChargeTime) += fTimeDelta;

			if (JoyStick_On() == true)
			{
				// 내가 누른 방향에 따라, 이동개념이 달라진다.
				JoyStick_controller_Attack(Kirbydesc, pCamera);
				CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

				if (eDir == CKirby::DIR_FRONT || eDir == CKirby::DIR_BACK)
				{
					pKirby->Change_State(CKirby::SWORDSTATE_SHUFFIEFRONT, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
					return;
				}
				else
				{
					pKirby->Change_State(CKirby::SWORDSTATE_SHUFFIERIGHT, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
					return;
				}
			}
		}
		// 이때 X를 떼면 바로 슈퍼스핀.
		else
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHSTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

	// 차징 모션중에 앞으로 가는 모션이다.
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SHUFFIEFRONT)
	{
		DESC(m_fMoveSpeed) += fTimeDelta * 10.f;
		if (DESC(m_fMoveSpeed) > 3.f)
			DESC(m_fMoveSpeed) = 3.f;

		// 타겟기준
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			DESC(m_fChargeTime) += fTimeDelta;

			if (JoyStick_On() == true)
			{
				// 내가 누른 방향에 따라, 이동개념이 달라진다.
				JoyStick_controller_Attack(Kirbydesc, pCamera);
				CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

				if (eDir == CKirby::DIR_LEFT || eDir == CKirby::DIR_RIGHT)
				{
					pKirby->Change_State(CKirby::SWORDSTATE_SHUFFIERIGHT, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
					return;
				}
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;

				// 차징 시간이 1초가 넘었을 경우엔
				if (DESC(m_fChargeTime) > 1.f)
					pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHCHARGE, 60.f, true, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				else
					pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHCHARGE, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);

				return;
			}

			if (DESC(m_bWalkingCharge) == true && DESC(m_fChargeTime) > 1.f)
			{
				pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHCHARGESTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
		}
		else
		{
			if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_SIDESLASH && DESC(m_fChargeTime) < 0.2f)
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				// 1초간 차징시간이 더 추가되었을 때, 슈퍼 차지모드로 변경된다.
				if (DESC(m_fChargeTime) > 1.f)
					pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHSTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				else
					pKirby->Change_State(CKirby::SWORDSTATE_GIGANTSPINSLASH, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
		}
	}

	// 차징 모션중에 옆으로 가는 모션이다.
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SHUFFIERIGHT)
	{
		DESC(m_fMoveSpeed) += fTimeDelta * 10.f;
		if (DESC(m_fMoveSpeed) > 3.f)
			DESC(m_fMoveSpeed) = 3.f;

		// 타겟기준
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			DESC(m_fChargeTime) += fTimeDelta;

			if (JoyStick_On() == true)
			{
				// 내가 누른 방향에 따라, 이동개념이 달라진다.
				JoyStick_controller_Attack(Kirbydesc, pCamera);
				CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

				if (eDir == CKirby::DIR_FRONT || eDir == CKirby::DIR_BACK)
				{
					pKirby->Change_State(CKirby::SWORDSTATE_SHUFFIEFRONT, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
					return;
				}
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				// 차징 시간이 1초가 넘었을 경우엔
				if (DESC(m_fChargeTime) > 1.f)
					pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHCHARGE, 60.f, true, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				else
					pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHCHARGE, 100.f, false, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);

				return;

			}

			if (DESC(m_bWalkingCharge) == true && DESC(m_fChargeTime) > 1.f)
			{
				pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHCHARGESTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}

		}
		else
		{
			if (DESC(m_ePreAttackState) == CKirby::SWORDSTATE_SIDESLASH && DESC(m_fChargeTime) < 0.2f)
			{
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
			else
			{
				DESC(m_eEyeState) = CKirby::EYE_ANGER;
				if (DESC(m_fChargeTime) > 1.f)
					pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHSTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				else
					pKirby->Change_State(CKirby::SWORDSTATE_GIGANTSPINSLASH, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
				return;
			}
		}
	}

	// 덜 차징 회전베기이다. 약하게 이동이 가능하다.
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_GIGANTSPINSLASH)
	{
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == true)
		{
			DESC(m_fMoveSpeed) += fTimeDelta * 10.f;
			if (DESC(m_fMoveSpeed) > 6.f)
				DESC(m_fMoveSpeed) = 6.f;

			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		}
		else
		{
			if (DESC(m_fMoveSpeed) > 0.f)
				DESC(m_fMoveSpeed) -= 120.f * fTimeDelta;
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;

			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}


		if (pKirby->isAnimFinish())
		{
			DESC(m_fChargeTime) += 0.f;

			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::SWORDSTATE_SPINSLASHEND, 100.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

	// 덜 차징 회전베기이다. 종료 모션이므로 빠르게 감속한다.
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SPINSLASHEND)
	{
		// 0.1초간 풀 감속 (최대 속도 8이라 가정)
		if (Kirbydesc->m_fMoveSpeed > 0.f)
			Kirbydesc->m_fMoveSpeed -= 90.f * fTimeDelta;
		if (Kirbydesc->m_fMoveSpeed < 0.f)
			Kirbydesc->m_fMoveSpeed = 0.f;

		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


		if (DESC(m_bSwordCharge1) == false)
		{
			CEffect::FX_DESC FXSDesc{};
			FXSDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXSDesc.vInitPos = _float3{ 0.f, .5f, 0.f };
			FXSDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW SwordSpin Fin"), &FXSDesc)))
				return;

			CParticle::PARTICLE_DESC FXPDesc{};
			FXPDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXPDesc.vInitPos = _float3{ 0.f, .2f, 0.f };
			FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Sword SpinFin Particle"), &FXPDesc)))
				return;

			DESC(m_bSwordCharge1) = true;
			DESC(m_bSwordCharge2) = true;
		}


		if (pKirby->isAnimFinish())
		{
			DESC(m_fChargeTime) = 0.f;
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}

	}

	// 슈퍼 스핀의 시작단계
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SUPERSPINSLASHSTART)
	{
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == true)
		{
			DESC(m_fMoveSpeed) += fTimeDelta * 30.f;
			if (DESC(m_fMoveSpeed) > 8.f)
				DESC(m_fMoveSpeed) = 8.f;

			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		}
		else
		{
			if (DESC(m_fMoveSpeed) > 0.f)
				DESC(m_fMoveSpeed) -= 60.f * fTimeDelta;
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;

			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}



		if (pKirby->isAnimFinish())
		{
			DESC(m_fChargeTime) = 0.f;
			pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHLOOP, 60.f, true, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

	// 슈퍼 스핀의 중간단계
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SUPERSPINSLASHLOOP)
	{
		DESC(m_fChargeTime) += fTimeDelta;

		if (JoyStick_controller_Attack(Kirbydesc, pCamera) == true)
		{
			DESC(m_fMoveSpeed) += fTimeDelta * 30.f;
			if (DESC(m_fMoveSpeed) > 8.f)
				DESC(m_fMoveSpeed) = 8.f;

			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		}
		else
		{
			if (DESC(m_fMoveSpeed) > 0.f)
				DESC(m_fMoveSpeed) -= 60.f * fTimeDelta;
			if (DESC(m_fMoveSpeed) < 0.f)
				DESC(m_fMoveSpeed) = 0.f;

			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}



		if (DESC(m_fChargeTime) > 1.1f)
		{
			pKirby->Change_State(CKirby::SWORDSTATE_SUPERSPINSLASHEND, 100.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

	// 슈퍼 스핀의 마지막 단계
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SUPERSPINSLASHEND)
	{
		if (DESC(m_bSwordCharge1) == false)
		{
			CEffect::FX_DESC FXSDesc{};
			FXSDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXSDesc.vInitPos = _float3{ 0.f, .5f, 0.f };
			FXSDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW SwordSpin Fin"), &FXSDesc)))
				return;

			CParticle::PARTICLE_DESC FXPDesc{};
			FXPDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXPDesc.vInitPos = _float3{ 0.f, .2f, 0.f };
			FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Sword SpinFin Particle"), &FXPDesc)))
				return;

				DESC(m_bSwordCharge1) = true;
				DESC(m_bSwordCharge2) = true;
		}



		// 0.1초간 풀 감속 (최대 속도 8이라 가정)
		if (Kirbydesc->m_fMoveSpeed > 0.f)
			Kirbydesc->m_fMoveSpeed -= 90.f * fTimeDelta;
		if (Kirbydesc->m_fMoveSpeed < 0.f)
			Kirbydesc->m_fMoveSpeed = 0.f;

		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


		if (pKirby->isAnimFinish())
		{
			DESC(m_fChargeTime) = 0.f;
			DESC(m_ePreAttackState) = CKirby::SWORDSTATE_DECISIVESLASH;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}

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
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));


	switch (_iAnimIndex)
	{
	case CKirby::SWORDSTATE_UPWARDSLASH:
	{
		pKirby->Delete_Effect("Sword Dash Test A");
		//pKirby->Delete_AllEffect();
		SwordSlash_Up(pKirby->Get_TransformCom());
	}
	break;
	case CKirby::SWORDSTATE_SWORDSPINSTART:
	{
		SwordSpin(pKirby->Get_TransformCom());
	}
	break;
	default:
		break;
	}

}

void CKirbySword_JumpAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (pKirby->Get_State() == CKirby::SWORDSTATE_UPWARDSLASH)
	{
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = DESC(m_vDodgeDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		DESC(m_fMoveSpeed) -= fTimeDelta * 30.f;
		if (DESC(m_fMoveSpeed) < 0.f)
			DESC(m_fMoveSpeed) = 0.f;

		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (/*DESC(m_fJumpVelocity) < -5.f*/pKirby->isAnimFinish())
		{
			DESC(m_bUpWardSlash) = true;
			pKirby->Change_State(CKirby::SWORDSTATE_SPINAFTER, 600.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}
	// 어퍼컷 공격 후 내려찍기
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDDIVE)
	{





		return;
	}
	// 공중제비 시작
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSPINSTART)
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
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSPIN, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}


	}
	// 공중제비
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SWORDSPIN)
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
			pKirby->Change_State(CKirby::SWORDSTATE_SPINAFTER, 600.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::SWORDSTATE_SPINAFTER)
	{
		Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 10.f;
			if (Kirbydesc->m_fMoveSpeed > 10.f)
				Kirbydesc->m_fMoveSpeed = 10.f;

			_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
		}
		else
		{
			Jump_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}

		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);


		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			DESC(m_fJumpVelocity) = 10.f;
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSPINSTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}

		if (pController->Is_Terrain())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			return;
		}
	}
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
			Kirbydesc->m_fJumpVelocity = 0.f;
			pController->Reset_FallVelocity();
			pKirby->Change_State(CKirby::STATE_FLIGHTLANDING, 70.f, false, false, CKirby::BODY_VACUUM);
			return;
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
		return;
	}
	// 끝나면 FALL로 돌아간다.
	if (pKirby->isAnimFinish())
	{
		pKirby->Change_State(CKirby::STATE_FLIGHTFALL, 60.f, true, true, CKirby::BODY_BALLOON);
		return;
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
