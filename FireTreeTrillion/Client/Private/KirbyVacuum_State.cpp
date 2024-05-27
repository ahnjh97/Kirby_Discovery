#include "stdafx.h"
#include "KirbyVacuum_State.h"
#include "Kirby_State_Function.h"

#pragma region SPIT STATE

CKirbyVacuum_Spit_State::CKirbyVacuum_Spit_State()
{
}

void CKirbyVacuum_Spit_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyVacuum_Spit_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// 뱉는 로직
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pKirby->DefaultIdle();

	// 뱉는다.
	if (pKirby->isAnimFinish())
	{
		// 이제 먹은 상태가 아니여.
		DESC(m_isEat) = false;

		// 애님 끝났는데 땅을 밟았을 경우
		if (pController->Is_Terrain() == true)
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
		}
		// 애님 끝났는데 땅을 밟지않고 공중에 있다고 판단 될 경우
		else
		{
			pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
		}
	}
}

void CKirbyVacuum_Spit_State::OnStateExit()
{

}

CKirbyVacuum_Spit_State* CKirbyVacuum_Spit_State::Create()
{
	CKirbyVacuum_Spit_State* pInstance = new CKirbyVacuum_Spit_State();
	return pInstance;
}

void CKirbyVacuum_Spit_State::Free()
{
	__super::Free();
}

#pragma endregion




#pragma region Vacuum STATE

CKirbyVacuum_Vacuum_State::CKirbyVacuum_Vacuum_State()
{
}

void CKirbyVacuum_Vacuum_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyVacuum_Vacuum_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pKirby->DefaultIdle();

	// Vacuum 시작. 이 애니메이션이 무조건 발생한다.
	if (pKirby->Get_State() == CKirby::STATE_INHALESTART)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_INHALE, 50.f, true, true, CKirby::BODY_VACUUM);
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// 빨아들이는 곳 X를 유지하여야 한다.
	else if (pKirby->Get_State() == CKirby::STATE_INHALE)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fVacuumTime) += fTimeDelta;

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			if (DESC(m_fVacuumTime) > 1.2f)
			{
				pKirby->Change_State(CKirby::STATE_SUPERINHALESTART, 60.f, false, true, CKirby::BODY_VACUUM);
			}
		}
		// 최소 0.2초간은 흡입한다.
		else if (DESC(m_fVacuumTime) > 0.2f)
		{
			// X키에서 손을 떼었고, 0.2초가 지나서 상황이 종료된다.
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
		}



		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			pKirby->Change_State(CKirby::STATE_INHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART)
	{
		DESC(m_eEyeState) = CKirby::EYE_IDLE;
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		// 조이스틱 만졌을땐 바로 슈퍼로 넘어감.
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			pKirby->Change_State(CKirby::STATE_SUPERINHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
		}

		// 애님이 자연스럽게 끝났으면 슈퍼빨기
		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			pKirby->Change_State(CKirby::STATE_SUPERINHALE, 50.f, true, true, CKirby::BODY_VACUUM);
		}


		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALE)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			pKirby->Change_State(CKirby::STATE_SUPERINHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_INHALEEND)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_INHALEFALL)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		// 바닥에 닿았다면
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_INHALELANDING, 50.f, false, true, CKirby::BODY_VACUUM);
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_INHALELANDING)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		//뽀잉 끝나면
		if (pKirby->isAnimFinish())
		{
			if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
				pKirby->Change_State(CKirby::STATE_INHALE, 50.f, true, true, CKirby::BODY_VACUUM);
			else
			{
				DESC(m_fVacuumTime) = 0.f;
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			}
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
}

void CKirbyVacuum_Vacuum_State::OnStateExit()
{
}

CKirbyVacuum_Vacuum_State* CKirbyVacuum_Vacuum_State::Create()
{
	CKirbyVacuum_Vacuum_State* pInstance = new CKirbyVacuum_Vacuum_State();
	return pInstance;
}

void CKirbyVacuum_Vacuum_State::Free()
{
	__super::Free();
}

#pragma endregion




#pragma region VacuumWalk STATE

CKirbyVacuum_VacuumWalk_State::CKirbyVacuum_VacuumWalk_State()
{
}

void CKirbyVacuum_VacuumWalk_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyVacuum_VacuumWalk_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
	pKirby->DefaultIdle();

	// 빨아들이면서 걷기
	if (pKirby->Get_State() == CKirby::STATE_INHALEWALK)
	{
		DESC(m_fVacuumTime) += fTimeDelta;
		Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false && DESC(m_fVacuumTime) > 0.2f)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
		}

		if (DESC(m_fVacuumTime) > 1.2f)
		{
			pKirby->Change_State(CKirby::STATE_SUPERINHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == false)
		{
			pKirby->Change_State(CKirby::STATE_INHALE, 50.f, true, true, CKirby::BODY_VACUUM);
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// 슈퍼 빨아들이면서 걷기
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALEWALK)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;

		Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == false)
		{
			pKirby->Change_State(CKirby::STATE_SUPERINHALESTART, 60.f, false, true, CKirby::BODY_VACUUM);
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
}

void CKirbyVacuum_VacuumWalk_State::OnStateExit()
{
}

CKirbyVacuum_VacuumWalk_State* CKirbyVacuum_VacuumWalk_State::Create()
{
	CKirbyVacuum_VacuumWalk_State* pInstance = new CKirbyVacuum_VacuumWalk_State();
	return pInstance;
}

void CKirbyVacuum_VacuumWalk_State::Free()
{
	__super::Free();
}

#pragma endregion
