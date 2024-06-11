#include "stdafx.h"
#include "KirbyBoom_State.h"
#include "Kirby_State_Function.h"

#pragma region BOOM JUMP STATE

CKirbyBoom_Fall_State::CKirbyBoom_Fall_State()
{
}

void CKirbyBoom_Fall_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBoom_Fall_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	pKirby->DefaultIdle();
	if (JoyStick_On() == true)
	{
		// 내가 누른 방향에 따라, 이동개념이 달라진다.
		JoyStick_controller_Attack(Kirbydesc, pCamera);
		DESC(m_fMoveSpeed) += fTimeDelta * 50.f;
		if (DESC(m_fMoveSpeed) > 4.f)
			DESC(m_fMoveSpeed) = 4.f;

		_vector vMoveDelta = DESC(m_vAttackDir) * fTimeDelta * DESC(m_fMoveSpeed);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
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


	if (pKirby->Get_State() == CKirby::BOOMSTATE_BOOMFALL)
	{

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_eEyeState) = CKirby::EYE_ANGER;
			pKirby->Change_State(CKirby::BOOMSTATE_THROWAIR, 60.f, false, false, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
		}
		// X를 꾹 유지할 경우
		else
		{
			if (JoyStick_controller(Kirbydesc, pCamera) == true)
			{
				Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
			}

			pController->FreeFall(pTransformCom, fTimeDelta, 1.2f);
			if (pController->Is_Terrain())
			{
				pKirby->Change_State(CKirby::BOOMSTATE_THROWCHARGE, 60.f, true, true, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
			}
		}
	}
	else if (pKirby->Get_State() == CKirby::BOOMSTATE_THROWAIR)
	{

		m_fAirThrowTime += fTimeDelta;

		if (m_fAirThrowTime < 0.25f)
			DESC(m_fJumpVelocity) = 1.f;
		else
		{
			DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		}

		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		// 임시..
		if (pController->Is_Terrain())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_LANDINGEND, 30.f, false, false, CKirby::BODY_DEFAULT);
		}
	}


}

void CKirbyBoom_Fall_State::OnStateExit()
{
	m_fAirThrowTime = 0.f;
}

CKirbyBoom_Fall_State* CKirbyBoom_Fall_State::Create()
{
	CKirbyBoom_Fall_State* pInstance = new CKirbyBoom_Fall_State();
	return pInstance;
}

void CKirbyBoom_Fall_State::Free()
{
	__super::Free();

}

#pragma endregion



#pragma region BOOM ATTACK STATE


CKirbyBoom_Attack_State::CKirbyBoom_Attack_State()
{
}

void CKirbyBoom_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBoom_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	if (pKirby->Get_State() == CKirby::BOOMSTATE_BOOMSHOOT)
	{
		// 폭탄을 던지면서 이동 로직이 필요하다.
		m_fThrowTime += fTimeDelta;

		if (JoyStick_On() == true)
		{
			// 내가 누른 방향에 따라, 이동개념이 달라진다.
			JoyStick_controller_Attack(Kirbydesc, pCamera);
			CKirby::DIR eDir = Kirby_Standard_Angle(DESC(m_vMoveDir), DESC(m_vAttackDir));

			DESC(m_fMoveSpeed) += fTimeDelta * 50.f;
			if (DESC(m_fMoveSpeed) > 6.f - (m_fThrowTime * 8.f))
				DESC(m_fMoveSpeed) = 6.f - (m_fThrowTime * 8.f);
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


		// 폭탄을 던지는 로직이 필요하다.


		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
		}
	}
	// 정지 샷
	else if (pKirby->Get_State() == CKirby::BOOMSTATE_THROW)
	{
		m_fThrowTime += fTimeDelta;
		if (m_fThrowTime < 0.05f)
		{
			if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
			{
				pKirby->Change_State(CKirby::BOOMSTATE_THROWCHARGE, 60.f, true, false, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
			}
		}
		// 폭탄을 던진다!

		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
		}
	}

}

void CKirbyBoom_Attack_State::OnStateExit()
{
	m_fThrowTime = 0.f;
}

CKirbyBoom_Attack_State* CKirbyBoom_Attack_State::Create()
{
	CKirbyBoom_Attack_State* pInstance = new CKirbyBoom_Attack_State();
	return pInstance;
}

void CKirbyBoom_Attack_State::Free()
{
	__super::Free();

}

#pragma endregion




#pragma region BOOM CHARGEATTACK STATE

CKirbyBoom_ChargeAttack_State::CKirbyBoom_ChargeAttack_State()
{
}

void CKirbyBoom_ChargeAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

}

void CKirbyBoom_ChargeAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));



	if (pKirby->Get_State() == CKirby::BOOMSTATE_THROWCHARGE)
	{
		// 방향키로 에임을 조절하는 순간 ROTATE로 넘어간다.
		if (JoyStick_On() == true)
		{
			pKirby->Change_State(CKirby::BOOMSTATE_THROWROTATE, 60.f, true, false, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
		}

		// X 를 땠을 경우
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			pKirby->Change_State(CKirby::BOOMSTATE_THROW, 60.f, false, false, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
		}
	}
	else if (pKirby->Get_State() == CKirby::BOOMSTATE_THROWROTATE)
	{
		// 방향키로 에임을 조절할 수 있다.
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
		{
			pKirby->Change_State(CKirby::BOOMSTATE_THROWCHARGE, 60.f, true, false, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
		}


		// X 를 땠을 경우
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			pKirby->Change_State(CKirby::BOOMSTATE_THROW, 60.f, false, false, CKirby::BODY_BOOMDEFAULT, CKirby::OFFSET_BOOM);
		}
	}

}

void CKirbyBoom_ChargeAttack_State::OnStateExit()
{
}

CKirbyBoom_ChargeAttack_State* CKirbyBoom_ChargeAttack_State::Create()
{
	CKirbyBoom_ChargeAttack_State* pInstance = new CKirbyBoom_ChargeAttack_State();
	return pInstance;
}

void CKirbyBoom_ChargeAttack_State::Free()
{
	__super::Free();

}

#pragma endregion
