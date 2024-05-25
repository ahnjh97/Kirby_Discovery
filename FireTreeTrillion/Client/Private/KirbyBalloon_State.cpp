#include "stdafx.h"
#include "KirbyBalloon_State.h"
#include "Kirby_State_Function.h"

#pragma region BALLOON IDLE STATE

CKirbyBalloon_Idle_State::CKirbyBalloon_Idle_State()
{
}

void CKirbyBalloon_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyBalloon_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// 이건 알아서 모델이 바뀔 것이기 때문에, 딱히 조이스틱의 통제가 필요없다.

	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

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

void CKirbyBalloon_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyBalloon_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	Turn_Z_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

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

void CKirbyBalloon_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyBalloon_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 방향은 항상 보간한다.
	if (pKirby->Get_State() == CKirby::STATE_EATLANDING)
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
	else
		Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	// 컨트롤러 손 대고 있을 떄
	if (Kirbydesc->m_isController == true)
	{
		Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}
	// 컨트롤러 손 안 대고 있을 때
	else if (Kirbydesc->m_isController == false)
	{
		if (pKirby->Get_State() == CKirby::STATE_EATLANDING)
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		else
			Jump_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}


	if (pKirby->Get_State() == CKirby::STATE_EATJUMP)
	{
		Kirbydesc->m_fJumpVelocity -= GRAVITY * fTimeDelta * Kirbydesc->m_fGravityOffset;
		Kirbydesc->m_isJump = pController->Jump(pTransformCom, Kirbydesc->m_fJumpVelocity, fTimeDelta);

		// 착지를 했다면,
		if (Kirbydesc->m_isJump == false)
		{
			// 착지상태를 ON 한다. 그리고 아마 여기 들어올 일은 없을 것이다.
			Kirbydesc->m_isLanding = true;

			// 표정 디테일
			if (CUtils::Make_RandomInt(0, 1) == 1)
			{
				Kirbydesc->m_eEyeState = CKirby::EYE_CLOSE;
			}
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

void CKirbyBalloon_Fly_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyBalloon_Fly_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbyBalloon_Fly_State::OnStateExit()
{
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