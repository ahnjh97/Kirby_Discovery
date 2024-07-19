#include "stdafx.h"
#include "KirbyCrash_State.h"
#include "Kirby_State_Function.h"

#include "CrashParticle.h"

void CamShake(_float& fTime, const _float& fTimeDelta, const _float& fPower)
{
	fTime += fTimeDelta;

	if (fTime > 0.2f)
	{
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
		pCamera->Make_Shake(fPower, 1.f);
		fTime = 0.f;
	}
}

#pragma region Attack STATE

CKirbyCrash_Attack_State::CKirbyCrash_Attack_State()
{
}

void CKirbyCrash_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCrash_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	if (pKirby->Get_State() == CKirby::CRASHSTATE_ATTACKSTART)
	{
		_float fTime = m_pGameInstance->Get_OriginalTimer();

		DESC(m_fTimeRatio) += fTime * 0.3f;
		if (DESC(m_fTimeRatio) > 1.f)
			DESC(m_fTimeRatio) = 1.f;

		m_pGameInstance->Set_FirstTimerRatio(DESC(m_fTimeRatio));
		m_pGameInstance->Set_SecondTimerRatio(DESC(m_fTimeRatio) * 0.2f);

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACK, 60.f, false, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::CRASHSTATE_ATTACK)
	{
		_float fTime = m_pGameInstance->Get_OriginalTimer();
		DESC(m_fTimeRatio) += fTime * 0.3f;
		if (DESC(m_fTimeRatio) > 1.f)
			DESC(m_fTimeRatio) = 1.f;

		m_pGameInstance->Set_FirstTimerRatio(DESC(m_fTimeRatio));
		m_pGameInstance->Set_SecondTimerRatio(DESC(m_fTimeRatio) * 0.2f);

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACKEND, 60.f, false, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}

	}
	else if(pKirby->Get_State() == CKirby::CRASHSTATE_ATTACKEND)
	{
		_float fTime = m_pGameInstance->Get_OriginalTimer();
		DESC(m_fTimeRatio) += fTime * 0.3f;
		if (DESC(m_fTimeRatio) > 1.f)
			DESC(m_fTimeRatio) = 1.f;

		m_pGameInstance->Set_FirstTimerRatio(DESC(m_fTimeRatio));
		m_pGameInstance->Set_SecondTimerRatio(DESC(m_fTimeRatio) * 0.2f);

		if (pKirby->isAnimFinish())
		{
			DESC(m_iCrashTimeSlow) = 1;
			DESC(m_fTimeRatio) = 0.f;
			m_pGameInstance->Restore_FirstTimer();
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);			
			return;
		}
	}

}

void CKirbyCrash_Attack_State::OnStateExit()
{
	m_bTimeCrashTrigger = true;
	m_fTime = 0.f;
}

CKirbyCrash_Attack_State* CKirbyCrash_Attack_State::Create()
{
	CKirbyCrash_Attack_State* pInstance = new CKirbyCrash_Attack_State();
	return pInstance;
}

void CKirbyCrash_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region Big Attack STATE

CKirbyCrash_BigAttack_State::CKirbyCrash_BigAttack_State()
{
}

void CKirbyCrash_BigAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCrash_BigAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	if (pKirby->Get_State() == CKirby::CRASHSTATE_BIGATTACKFIRE)
	{

		_float fTime = m_pGameInstance->Get_OriginalTimer();

		DESC(m_fTimeRatio) += fTime * 0.3f;
		if (DESC(m_fTimeRatio) > 1.f)
			DESC(m_fTimeRatio) = 1.f;

		m_pGameInstance->Get_DirectionLightAddress()->Interpolate_Light(_float4(0.02f, 0.02f, 0.02f, 0.02f), 1.f, 1.f);
		m_pGameInstance->Set_ObjectBlack(0.1f, 1.f);

		m_pGameInstance->Set_FirstTimerRatio(DESC(m_fTimeRatio));
		m_pGameInstance->Set_SecondTimerRatio(DESC(m_fTimeRatio) * 0.2f);


		m_fGravity += fTimeDelta;
		_float fOffset = -DESC(m_fCrashChargeTime) + 2.5f;
		DESC(m_fCrashChargeTime) += fTimeDelta * 20.f - (0.35f * m_fGravity * fTimeDelta * 50.f);



		if (fOffset > 0.f)
			fOffset = 0.f;

		if (m_bLightRangeInv == false)
		{
			m_fLightRange += fTimeDelta * 20.f;
			if (m_fLightRange > 15.f)
			{
				m_bLightRangeInv = true;
			}
		}
		else
		{
			m_fLightRange -= fTimeDelta * 20.f;
			if (m_fLightRange < 0.f)
				m_fLightRange = 0.f;

		}

		pKirby->Large_Light(_float4(0.6f, 1.f, 1.f, 1.f), 5.f + (m_fLightRange * 5.f), 0.001f);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset), fOffset);

		if (pKirby->isAnimFinish())
		{
			m_pGameInstance->Get_DirectionLightAddress()->Interpolate_Light(DESC(m_vPreDiffuseLight), 1.f, 2.f);
			m_pGameInstance->Set_ObjectBlack(1.f, 2.f);
			m_bTerrainOn = true;
		}

		if (m_bTerrainOn == true)
		{
			m_fTerrainTime += fTimeDelta;
			if (m_fTerrainTime > 0.7f)
			{
				DESC(m_fTimeRatio) = 0.f;
				// 여기에서나 초기화 해준다.
				DESC(m_fCrashChargeTime) = 0.f;
				DESC(m_iCrashTimeSlow) = 2;
				m_pGameInstance->Restore_FirstTimer();
				Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
				pKirby->Large_Light(_float4(0.6f, 1.f, 1.f, 1.f), 5.f, 0.001f);
				return;
			}
		}
	}
}

void CKirbyCrash_BigAttack_State::OnStateExit()
{
	m_bTimeCrashTrigger = true;
	m_fTime = 0.f;
	m_fGravity = 0.f;

	m_bTerrainOn = false;
	m_fTerrainTime = 0.f;

	m_bLightRangeInv = false;
	m_fLightRange = 0.f;
}

CKirbyCrash_BigAttack_State* CKirbyCrash_BigAttack_State::Create()
{
	CKirbyCrash_BigAttack_State* pInstance = new CKirbyCrash_BigAttack_State();
	return pInstance;
}

void CKirbyCrash_BigAttack_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region Charge STATE

CKirbyCrash_Charge_State::CKirbyCrash_Charge_State()
{
}

void CKirbyCrash_Charge_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
	m_fTime = 0.2f;
}

void CKirbyCrash_Charge_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	m_fRockCreate += fTimeDelta;

	if (m_fRockCreate > 0.2f)
	{
		CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
		Crashdesc.vPos = pTransformCom->Get_State(CTransform::STATE_POSITION) + _float3(CUtils::Make_RandomFloat(-8.f, 8.f), -1.f, CUtils::Make_RandomFloat(-8.f, 8.f));
		Crashdesc.vDir = XMVectorSetW(CUtils::Make_RandomAngle_Vector(10.f, _float4(0.f, 1.f, 0.f, 0.f)), 0.f);
		Crashdesc.fSpeed = CUtils::Make_RandomFloat(20.f, 30.f);
		Crashdesc.bGravity = false;
		Crashdesc.fScale = { CUtils::Make_RandomFloat(0.2f, 0.25f) };
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
			return;

		m_fRockCreate = 0.f;
	}


	if (m_bNextState == true)
	{
		// 1틱 발동
		if (m_bTrigger == true)
		{
			m_pGameInstance->Set_FirstTimerRatio(0.f);
			m_pGameInstance->Set_SecondTimerRatio(0.f);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(0.3f, 0.5f);
			m_bTrigger = false;
		}

		m_fNextStateTime += m_pGameInstance->Get_OriginalTimer();
		if (m_fNextStateTime > 1.f)
		{
			DESC(m_fCrashChargeTime) = 0.f;
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACKSTART, 60.f, false, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}
		return;
	}

	// 차지의 시작이다.
	if (pKirby->Get_State() == CKirby::CRASHSTATE_ATTACKCHARGESTART)
	{
		// 지속적인 쉐이킹
		CamShake(m_fTime, fTimeDelta, 0.3f);
		DESC(m_fCrashChargeTime) += fTimeDelta;

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
			if (Kirbydesc->m_fMoveSpeed > 2.5f)
				Kirbydesc->m_fMoveSpeed = 2.5f;

			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}

		// 폭발을 
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			m_bNextState = true;
		}

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACKCHARGE, 60.f, true, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
		}
	}
	// 차지 중이다.
	else if (pKirby->Get_State() == CKirby::CRASHSTATE_ATTACKCHARGE)
	{
		// 지속적인 쉐이킹
		CamShake(m_fTime, fTimeDelta, 0.3f);
		DESC(m_fCrashChargeTime) += fTimeDelta;

		if (DESC(m_fCrashChargeTime) > 2.5f)
		{
			pKirby->Change_State(CKirby::CRASHSTATE_BIGATTACKCHARGESTART, 60.f, false, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}


		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			m_bNextState = true;
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACKCHARGEMOVE, 60.f, true, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}
	}
	// 차지 중이 아니다.
	else if (pKirby->Get_State() == CKirby::CRASHSTATE_ATTACKCHARGEMOVE)
	{
		// 지속적인 쉐이킹
		CamShake(m_fTime, fTimeDelta, 0.3f);
		DESC(m_fCrashChargeTime) += fTimeDelta;

		if (DESC(m_fCrashChargeTime) > 2.5f)
		{
			pKirby->Change_State(CKirby::CRASHSTATE_BIGATTACKCHARGESTART, 60.f, false, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}

		Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
		if (Kirbydesc->m_fMoveSpeed > 2.5f)
			Kirbydesc->m_fMoveSpeed = 2.5f;

		// 타겟기준
		_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			m_bNextState = true;
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == false)
		{
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACKCHARGE, 60.f, true, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}
	}
}

void CKirbyCrash_Charge_State::OnStateExit()
{
	m_bNextState = false;
	m_fNextStateTime = 0.f;
	m_bTrigger = true;
}

CKirbyCrash_Charge_State* CKirbyCrash_Charge_State::Create()
{
	CKirbyCrash_Charge_State* pInstance = new CKirbyCrash_Charge_State();
	return pInstance;
}

void CKirbyCrash_Charge_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region Big Charge STATE

CKirbyCrash_BigCharge_State::CKirbyCrash_BigCharge_State()
{
}

void CKirbyCrash_BigCharge_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
	m_fTime = 0.1f;
}

void CKirbyCrash_BigCharge_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	m_fRockCreate += fTimeDelta;

	if (m_fRockCreate > 0.2f)
	{
		CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
		Crashdesc.vPos = pTransformCom->Get_State(CTransform::STATE_POSITION) + _float3(CUtils::Make_RandomFloat(-8.f, 8.f), -1.f, CUtils::Make_RandomFloat(-8.f, 8.f));
		Crashdesc.vDir = XMVectorSetW(CUtils::Make_RandomAngle_Vector(10.f, _float4(0.f, 1.f, 0.f, 0.f)), 0.f);
		Crashdesc.fSpeed = CUtils::Make_RandomFloat(20.f, 30.f);
		Crashdesc.bGravity = false;
		Crashdesc.fScale = { CUtils::Make_RandomFloat(0.2f, 0.25f) };
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
			return;

		m_fRockCreate = 0.f;
	}

	if (m_bNextState == true)
	{
		// 1틱 발동
		if (m_bTrigger == true)
		{
			m_pGameInstance->Set_FirstTimerRatio(0.f);
			m_pGameInstance->Set_SecondTimerRatio(0.f);
			DESC(m_vPreDiffuseLight) = m_pGameInstance->Get_DirectionLightAddress()->Get_LightDesc()->vDiffuse;

			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(0.3f, 0.5f);
			m_bTrigger = false;
		}

		m_fNextStateTime += m_pGameInstance->Get_OriginalTimer();
		if (m_fNextStateTime > 1.f)
		{
			pKirby->Change_State(CKirby::CRASHSTATE_BIGATTACKFIRE, 60.f, false, true, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}
		return;
	}



	if (pKirby->Get_State() == CKirby::CRASHSTATE_BIGATTACKCHARGESTART)
	{
		// 지속적인 쉐이킹
		CamShake(m_fTime, fTimeDelta, 0.5f);
		DESC(m_fCrashChargeTime) += fTimeDelta;
		if (DESC(m_fCrashChargeTime) > 5.f)
			DESC(m_fCrashChargeTime) = 5.f;

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
			if (Kirbydesc->m_fMoveSpeed > 2.5f)
				Kirbydesc->m_fMoveSpeed = 2.5f;

			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}

		// 폭발을 
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			m_bNextState = true;
		}

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CRASHSTATE_BIGATTACKCHARGE, 60.f, true, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
		}

	}
	else if (pKirby->Get_State() == CKirby::CRASHSTATE_BIGATTACKCHARGE)
	{
		// 지속적인 쉐이킹
		CamShake(m_fTime, fTimeDelta, 0.5f);
		DESC(m_fCrashChargeTime) += fTimeDelta;
		if (DESC(m_fCrashChargeTime) > 5.f)
			DESC(m_fCrashChargeTime) = 5.f;

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
			if (Kirbydesc->m_fMoveSpeed > 2.5f)
				Kirbydesc->m_fMoveSpeed = 2.5f;

			// 타겟기준
			_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
			pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
			Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta, 3.f);
		}

		// 폭발을 
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			m_bNextState = true;
		}
	}

	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset), - DESC(m_fCrashChargeTime) + 2.5f);

}

void CKirbyCrash_BigCharge_State::OnStateExit()
{
	m_bNextState = false;
	m_fNextStateTime = 0.f;
	m_bTrigger = true;
}

CKirbyCrash_BigCharge_State* CKirbyCrash_BigCharge_State::Create()
{
	CKirbyCrash_BigCharge_State* pInstance = new CKirbyCrash_BigCharge_State();
	return pInstance;
}

void CKirbyCrash_BigCharge_State::Free()
{
	__super::Free();
}

#pragma endregion
