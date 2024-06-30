#include "stdafx.h"
#include "KirbyCar_State.h"
#include "Kirby_State_Function.h"
#include "ToppleableBridge.h"
#include "StarBlock.h"

#pragma region 차량 아이들 상태

CKirbyCar_Idle_State::CKirbyCar_Idle_State()
{
}

void CKirbyCar_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCar_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta);

	// 0.1초간 풀 감속 (최대 속도 8이라 가정)
	if (Kirbydesc->m_fMoveSpeed > 0.f)
		Kirbydesc->m_fMoveSpeed -= 25.f * fTimeDelta;
	if (Kirbydesc->m_fMoveSpeed < 0.f)
		Kirbydesc->m_fMoveSpeed = 0.f;

	// Z 회전 복구 (최대 회전 각도 10도)
	Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 4.f;
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

	if (DESC(m_bCarJump) == true)
	{
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
	}
	else
	{
		pController->FreeFall(pTransformCom, fTimeDelta);
	}

	// 차량을 땅에 버리는 로직이다.
	if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
	{
		DESC(m_fDumpAbilityTime) += fTimeDelta;

		if (DESC(m_fDumpAbilityTime) > 1.f)
		{
			DESC(m_fDumpAbilityTime) = 0.f;
			DESC(m_fJumpVelocity) = 15.f;
			pKirby->Change_State(CKirby::STATE_SPITDEFORM, 60.f, false, false, CKirby::BODY_VACUUM);

			DESC(m_bBooster) = false;
			pKirby->Delete_Effect("Come On Dash");
			return;
		}
	}
	else
	{
		if (DESC(m_fDumpAbilityTime) > 0.f)
			DESC(m_fDumpAbilityTime) -= fTimeDelta * 2.f;

		if (DESC(m_fDumpAbilityTime) < 0.f)
			DESC(m_fDumpAbilityTime) = 0.f;
	}



	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
	{
		DESC(m_fJumpVelocity) = 20.f;
		DESC(m_bCarJump) = true;
		pKirby->Change_State(CKirby::CARSTATE_JUMPSTART, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
		return;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBooster) == false)
	{
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
		pCamera->Make_Shake();
		GAMEINSTANCE Setting_RadialBlur(pTransformCom->Get_State(CTransform::STATE_POSITION), 20.f, 40.f);
		DESC(m_bBooster) = true;

		//부슽 이펙트
		ComeOn_Dash(pTransformCom);
		pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

		pKirby->Change_State(CKirby::CARSTATE_BOOST, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
		return;
	}

	if (JoyStick_On() == true)
	{
		pKirby->Change_State(CKirby::CARSTATE_MOVING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
		return;
	}

}

void CKirbyCar_Idle_State::OnStateExit()
{
}

CKirbyCar_Idle_State* CKirbyCar_Idle_State::Create()
{
	CKirbyCar_Idle_State* pInstance = new CKirbyCar_Idle_State();
	return pInstance;
}

void CKirbyCar_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region 차량 운전 상태

CKirbyCar_Run_State::CKirbyCar_Run_State()
{
}

void CKirbyCar_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCar_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (DESC(m_bCarJump) == true)
	{
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
	}
	else
	{
		pController->FreeFall(pTransformCom, fTimeDelta);
	}


	if (pController->Compute_Height() > 2.f)
	{
		pKirby->Change_State(CKirby::CARSTATE_FALL, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
		return;
	}


	if (JoyStick_controller(Kirbydesc, pCamera) == true)
	{
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 7.f);

		// 10도 안쪽으로 각도가 좁혀졌을 때,
		if ( ToDegree(acos(DESC(m_vMoveDir).Dot(DESC(m_vTargetDir)))) < 11.f)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 30.f;
			if (Kirbydesc->m_fMoveSpeed > 15.f)
				Kirbydesc->m_fMoveSpeed = 15.f;
		}
		else
		{
			Kirbydesc->m_fMoveSpeed += (10.f - Kirbydesc->m_fMoveSpeed) * fTimeDelta * 1.5f;
		}
		_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_bCarJump) = true;
			DESC(m_fJumpVelocity) = 20.f;
			pKirby->Change_State(CKirby::CARSTATE_JUMPSTART, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBooster) == false)
		{
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake();
			GAMEINSTANCE Setting_RadialBlur(pTransformCom->Get_State(CTransform::STATE_POSITION), 20.f, 40.f);
			DESC(m_bBooster) = true;

			//부슽 이펙트
			ComeOn_Dash(pTransformCom);
			pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

			pKirby->Change_State(CKirby::CARSTATE_BOOST, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
	else
	{
		pKirby->Change_State(CKirby::CARSTATE_IDLING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
		return;
	}


}

void CKirbyCar_Run_State::OnStateExit()
{
}

CKirbyCar_Run_State* CKirbyCar_Run_State::Create()
{
	CKirbyCar_Run_State* pInstance = new CKirbyCar_Run_State();
	return pInstance;
}

void CKirbyCar_Run_State::Free()
{
	__super::Free();

}

#pragma endregion

#pragma region 차량 점프 상태

CKirbyCar_Jump_State::CKirbyCar_Jump_State()
{
}

void CKirbyCar_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCar_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (JoyStick_controller(Kirbydesc, pCamera) == true)
	{
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 7.f);

		// 10도 안쪽으로 각도가 좁혀졌을 때,
		if (ToDegree(acos(DESC(m_vMoveDir).Dot(DESC(m_vTargetDir)))) < 11.f)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 30.f;
			if (Kirbydesc->m_fMoveSpeed > 15.f)
				Kirbydesc->m_fMoveSpeed = 15.f;
		}
		else
		{
			Kirbydesc->m_fMoveSpeed += (10.f - Kirbydesc->m_fMoveSpeed) * fTimeDelta * 1.5f;
		}
		_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
	}
	else
	{
		// 0.1초간 풀 감속 (최대 속도 8이라 가정)
		if (Kirbydesc->m_fMoveSpeed > 0.f)
			Kirbydesc->m_fMoveSpeed -= 25.f * fTimeDelta;
		if (Kirbydesc->m_fMoveSpeed < 0.f)
			Kirbydesc->m_fMoveSpeed = 0.f;

		// Z 회전 복구 (최대 회전 각도 10도)
		Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 4.f;
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
	}

	if (DESC(m_bCarJump) == true) 
	{
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
	}
	else
	{
		pController->FreeFall(pTransformCom, fTimeDelta);
	}


	if (DESC(m_bBooster) == true)
	{
		DESC(m_fBoosterTime) += fTimeDelta;
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
			DESC(m_fBoosterTime) = 0.f;

		if (DESC(m_fBoosterTime) > 3.f)
		{
			DESC(m_fBoosterTime) = 0.f;
			DESC(m_bBooster) = false;
			pKirby->Delete_Effect("Come On Dash");
		}
	}
	else if (DESC(m_bBooster) == false)
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
		{
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake();
			GAMEINSTANCE Setting_RadialBlur(pTransformCom->Get_State(CTransform::STATE_POSITION), 20.f, 40.f);
			DESC(m_bBooster) = true;

			//부슽 이펙트
			ComeOn_Dash(pTransformCom);
			pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

			pKirby->Change_State(CKirby::CARSTATE_BOOST, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}


	if (pKirby->Get_State() == CKirby::CARSTATE_JUMPSTART)
	{
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CARSTATE_JUMP, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CARSTATE_JUMP)
	{
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CARSTATE_FALL, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
		else if (pController->Is_Terrain())
		{
			DESC(m_bCarJump) = false;
			pKirby->Change_State(CKirby::CARSTATE_LANDING, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CARSTATE_FALL)
	{
		if (pController->Is_Terrain())
		{
			DESC(m_bCarJump) = false;
			pKirby->Change_State(CKirby::CARSTATE_LANDING, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CARSTATE_LANDING)
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_fJumpVelocity) = 20.f;
			DESC(m_bCarJump) = true;
			pKirby->Change_State(CKirby::CARSTATE_JUMPSTART, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}

		if (pKirby->isAnimFinish())
		{
			if ( DESC(m_bBooster) == true )
			{
				pKirby->Change_State(CKirby::CARSTATE_BOOST, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
				return;
			}
			else
			{
				pKirby->Change_State(CKirby::CARSTATE_IDLING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
				return;
			}
		}
	}
}

void CKirbyCar_Jump_State::OnStateExit()
{
}

CKirbyCar_Jump_State* CKirbyCar_Jump_State::Create()
{
	CKirbyCar_Jump_State* pInstance = new CKirbyCar_Jump_State();
	return pInstance;
}

void CKirbyCar_Jump_State::Free()
{
	__super::Free();

}

#pragma endregion

#pragma region 차량 데미지 상태

CKirbyCar_Damage_State::CKirbyCar_Damage_State()
{
}

void CKirbyCar_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCar_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (pKirby->Get_State() == CKirby::CARSTATE_DAMAGE)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		_float fDamageJumpPower = pKirby->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pKirby->Set_DamageJumpPower(fDamageJumpPower);

		if (pController->Is_Terrain() || pKirby->isAnimFinish())
		{
			DESC(m_bCarJump) = false;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::CARSTATE_IDLING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
}

void CKirbyCar_Damage_State::OnStateExit()
{
}

CKirbyCar_Damage_State* CKirbyCar_Damage_State::Create()
{
	CKirbyCar_Damage_State* pInstance = new CKirbyCar_Damage_State();
	return pInstance;
}

void CKirbyCar_Damage_State::Free()
{
	__super::Free();

}

#pragma endregion

#pragma region 차량 흡수 상태

CKirbyCar_Vacuum_State::CKirbyCar_Vacuum_State()
{
}

void CKirbyCar_Vacuum_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	if (_iAnimIndex == CKirby::CARVACUUMSTATE_DEFORM)
	{
		m_pGameInstance->Set_BlackBackGround(true);
		m_pGameInstance->Set_SecondTimerRatio(0.f);
		static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr())->Set_FOVY(20.f);
	}
}

void CKirbyCar_Vacuum_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

	if (pKirby->Get_State() == CKirby::CARVACUUMSTATE_DEFORM)
	{
		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CARSTATE_DEMOEND, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CARSTATE_DEMOEND)
	{
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

		if (pKirby->isAnimFinish())
		{
			m_pGameInstance->Set_BlackBackGround(false);
			m_pGameInstance->Set_SecondTimerRatio(1.f);

			CCamera_Main* pCamMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			if (pCamMain == nullptr)
				ALARM_FAIL("망했다 카메라 없다");

			pCamMain->Set_FOVY(30.f);
			pCamMain->Set_Target(pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FIRST, {0.f, 0.f, 2.f}, 5.f);
			pKirby->Change_State(CKirby::CARSTATE_IDLING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			return;
		}
	}
}

void CKirbyCar_Vacuum_State::OnStateExit()
{
}

CKirbyCar_Vacuum_State* CKirbyCar_Vacuum_State::Create()
{
	CKirbyCar_Vacuum_State* pInstance = new CKirbyCar_Vacuum_State();
	return pInstance;
}

void CKirbyCar_Vacuum_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region 차량 부스터 상태

CKirbyCar_Boost_State::CKirbyCar_Boost_State()
{
}

void CKirbyCar_Boost_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCar_Boost_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (DESC(m_bCarJump) == true)
	{
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
	}
	else
	{
		pController->FreeFall(pTransformCom, fTimeDelta);
	}

	DESC(m_eEyeState) = CKirby::EYE_ANGER;

	if (pKirby->Get_State() != CKirby::CARSTATE_CRASH && pController->Compute_Height() > 2.f)
	{
		pKirby->Change_State(CKirby::CARSTATE_FALL, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		return;
	}


	if (pKirby->Get_State() == CKirby::CARSTATE_BOOST)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 7.f);
			DESC(m_fMoveSpeed) += fTimeDelta * 20.f;
			if (Kirbydesc->m_fMoveSpeed > 21.f)
				Kirbydesc->m_fMoveSpeed = 21.f;
		}
		else
		{
			DESC(m_fMoveSpeed) += fTimeDelta * 20.f;
			if (Kirbydesc->m_fMoveSpeed > 16.f)
				Kirbydesc->m_fMoveSpeed = 16.f;
		}
		_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


		DESC(m_fBoosterTime) += fTimeDelta;

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
			DESC(m_fBoosterTime) = 0.f;

		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			DESC(m_bCarJump) = true;
			DESC(m_fJumpVelocity) = 20.f;
			pKirby->Change_State(CKirby::CARSTATE_JUMPSTART, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			return;
		}

		if (DESC(m_fBoosterTime) > 3.f)
		{
			DESC(m_fBoosterTime) = 0.f;
			DESC(m_bBooster) = false;
			pKirby->Delete_Effect("Come On Dash");
			if (JoyStick_On() == false)
			{
				pKirby->Change_State(CKirby::CARSTATE_BOOSTEND, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				return;
			}
			else
			{
				pKirby->Change_State(CKirby::CARSTATE_MOVING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				return;
			}
		}

		// 충돌하는지 지속적인 검사.
		_vector vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

		if (pController->Compute_Wall(vLook) < 3.f)
		{
			CGameObject* pObj = pKirby->FindToppleableBridge(pController->Get_MostRecentActor());
			if (nullptr != pObj) {
				CToppleableBridge* pToppleableBridge = static_cast<CToppleableBridge*>(pObj);
				pToppleableBridge->OnCollision();
				return;
			}

			pObj = pKirby->FindStarBox(pController->Get_MostRecentActor());
			if (nullptr != pObj) {
				CStarBlock* pStarBlock = static_cast<CStarBlock*>(pObj);
				pStarBlock->Break_From_Car();
				return;
			}

			pKirby->Change_State(CKirby::CARSTATE_CRASH, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			DESC(m_fBoosterTime) = 0.f;
			DESC(m_bBooster) = false;
			pKirby->Delete_Effect("Come On Dash");
			Kirbydesc->m_fMoveSpeed = 0.f;
			DESC(m_bCarJump) = true;
			DESC(m_fJumpVelocity) = 20.f;
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(1.6f, 0.5f);
			GAMEINSTANCE Setting_RadialBlur(pTransformCom->Get_State(CTransform::STATE_POSITION), 30.f, 150.f);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CARSTATE_BOOSTEND)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (JoyStick_On() == true)
		{
			pKirby->Change_State(CKirby::CARSTATE_MOVING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			return;
		}

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CARSTATE_IDLING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CARSTATE_CRASH)
	{
		_vector vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
		if (pController->Is_Terrain())
		{
			DESC(m_bCarJump) = false;
			pKirby->Change_State(CKirby::CARSTATE_LANDING, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			return;
		}
		pController->Move_Dir(pTransformCom, -14.f * vLook * fTimeDelta, fTimeDelta);
	}

}

void CKirbyCar_Boost_State::OnStateExit()
{
}

CKirbyCar_Boost_State* CKirbyCar_Boost_State::Create()
{
	CKirbyCar_Boost_State* pInstance = new CKirbyCar_Boost_State();
	return pInstance;
}

void CKirbyCar_Boost_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region 차량 컷씬 상태

CKirbyCar_Cut_State::CKirbyCar_Cut_State()
{
}

void CKirbyCar_Cut_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
	m_fSpeed = 21.f;
}

void CKirbyCar_Cut_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	m_fCutAnimTime += fTimeDelta;

	if (m_fCutAnimTime > 0.2f)
	{
		m_pGameInstance->Restore_FirstTimer();
		m_pGameInstance->Restore_SecondTimer();

	}

	if (pKirby->Get_State() == CKirby::CARSTATE_CUT1)
	{
		Cut1_EyeState(pGameObject);

		// 42프레임에 떨어짐.
		// 60프레임 기준 약 0.66초만에 떨어지는 것임.
		if (m_iTurnCount == 0) 
		{
			DESC(m_vTargetDir) = _float4(0.f, 0.f, -1.f, 0.f);
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
			pController->Move_Dir(pTransformCom, DESC(m_vMoveDir) * fTimeDelta * 21.f, fTimeDelta);
			pController->FreeFall(pTransformCom, fTimeDelta, 10.f);

			if (m_fCutAnimTime > 0.5f && pController->Is_Terrain())
			{
				m_iTurnCount = 1;
			}
		}
		else if (m_iTurnCount == 1) 
		{

			DESC(m_vTargetDir) = _float4(-0.5f, 0.f, -0.5f, 0.f);
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
			pController->Move_Dir(pTransformCom, DESC(m_vMoveDir) * fTimeDelta * m_fSpeed, fTimeDelta);
			pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

			m_fSpeed -= fTimeDelta * 13.f;
			DESC(m_fMoveSpeed) = m_fSpeed;

			if (m_fCutAnimTime > 1.2f)
			{
				m_iTurnCount = 2;
			}
		}
		else if (m_iTurnCount == 2) {
			DESC(m_vTargetDir) = _float4(1.f, 0.f, 0.f, 0.f);
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
			pController->Move_Dir(pTransformCom, DESC(m_vMoveDir) * fTimeDelta * m_fSpeed, fTimeDelta);
			pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

			m_fSpeed -= fTimeDelta * 13.f;
			if (m_fSpeed < 0.f)
				m_fSpeed = 0.f;
			DESC(m_fMoveSpeed) = m_fSpeed;


			if (pKirby->isAnimFinish())
			{
				DESC(m_fMoveSpeed) = 0.f;
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::CARSTATE_IDLING, 60.f, true, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
				return;
			}
		}
	}

	else if (pKirby->Get_State() == CKirby::CARSTATE_CUT2)
	{
		Cut2_EyeState(pGameObject);

		DESC(m_vTargetDir) = _float4(0.f, 0.f, 1.f, 0.f);
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
	}
}

void CKirbyCar_Cut_State::OnStateExit()
{
	m_iTurnCount = 0;
	m_fCutAnimTime = 0.f;
	m_fSpeed = 21.f;
}

void CKirbyCar_Cut_State::Cut1_EyeState(CGameObject* pGameObject)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	_float fKeyFrame = pKirby->Get_AnimTrackPosition();

	if (fKeyFrame < 170.f)
		DESC(m_eEyeState) = CKirby::EYE_ANGER;
	else if (fKeyFrame >= 170.f && fKeyFrame < 176.f)
		DESC(m_eEyeState) = CKirby::EYE_BLINK;
	else if (fKeyFrame >= 176.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
}

void CKirbyCar_Cut_State::Cut2_EyeState(CGameObject* pGameObject)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();

	_float fKeyFrame = pKirby->Get_AnimTrackPosition();

	if (fKeyFrame < 48.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	else if (fKeyFrame >= 48.f && fKeyFrame < 350.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 350.f && fKeyFrame < 377.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	else if (fKeyFrame >= 377.f && fKeyFrame < 426.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 426.f && fKeyFrame < 435.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	else if (fKeyFrame >= 435.f && fKeyFrame < 450.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 450.f && fKeyFrame < 455.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	else if (fKeyFrame >= 455.f && fKeyFrame < 570.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 570.f && fKeyFrame < 575.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	else if (fKeyFrame >= 575.f && fKeyFrame < 903.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 903.f && fKeyFrame < 910.f)
		DESC(m_eEyeState) = CKirby::EYE_BLINK;
	else if (fKeyFrame >= 910.f && fKeyFrame < 974.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 974.f && fKeyFrame < 1120.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	else if (fKeyFrame >= 1120.f && fKeyFrame < 1210.f)
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
	else if (fKeyFrame >= 1210.f)
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;

}

CKirbyCar_Cut_State* CKirbyCar_Cut_State::Create()
{
	CKirbyCar_Cut_State* pInstance = new CKirbyCar_Cut_State();
	return pInstance;
}

void CKirbyCar_Cut_State::Free()
{
	__super::Free();
}

#pragma endregion
