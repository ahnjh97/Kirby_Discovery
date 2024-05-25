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
	// ¹ñ´Â ·ÎÁ÷


	// ¤·¤·
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

	// »¡¾ÆµéÀÌ±â ½ÃÀÛ
	if (pKirby->Get_State() == CKirby::STATE_INHALESTART)
	{
		if (Kirbydesc->m_isController == true)
		{
			Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// »¡¾ÆµéÀÌ´Â Áß
	else if (pKirby->Get_State() == CKirby::STATE_INHALE)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// ½´ÆÛ »¡¾ÆµéÀÌ±â ½ÃÀÛ
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART)
	{
		if (Kirbydesc->m_isController == true)
		{
			Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// ½´ÆÛ »¡¾ÆµéÀÌ±â
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALE)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// »¡¾ÆµéÀÌ±â Á¾·á
	else if (pKirby->Get_State() == CKirby::STATE_INHALEEND)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// »¡¾ÆµéÀÌ¸é¼­ ¶³¾îÁö±â
	else if (pKirby->Get_State() == CKirby::STATE_INHALEFALL)
	{
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// »¡¾ÆµéÀÌ¸é¼­ »ÇÀ×ÇÏ±â
	else if (pKirby->Get_State() == CKirby::STATE_INHALELANDING)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
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

	// »¡¾ÆµéÀÌ¸é¼­ °È±â
	if (pKirby->Get_State() == CKirby::STATE_INHALEWALK)
	{
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
	}
	// ½´ÆÛ »¡¾ÆµéÀÌ¸é¼­ °È±â
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALEWALK)
	{
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
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
