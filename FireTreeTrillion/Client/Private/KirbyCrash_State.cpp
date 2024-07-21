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

		static_cast<CCamera_Main*>(pCamera)->Zoom(20.f);
		_float4 vMakingPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_fParticleTime += fTime;
		m_fRange += fTime * 6.f;
		m_fBoomTime += fTime;

		if (m_fParticleTime > 0.2f)
		{
			for (_int i = 0; i < 10; ++i)
			{
				_float4 vTemp = { m_fRange * 2.f, 0.f, 0.f, 0.f };
				_float4x4 RotMat = _float4x4::Identity;
				CUtils::Turn_OtherMatrix(RotMat, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
				vTemp = XMVectorSetW(XMVector3Transform(vTemp, RotMat), 0.f);
				CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
				Crashdesc.vPos = vMakingPos + vTemp;
				Crashdesc.vDir = vTemp;
				Crashdesc.vDir.Normalize();
				Crashdesc.vDir.y += 1.f;
				Crashdesc.vDir.Normalize();
				Crashdesc.fSpeed = CUtils::Make_RandomFloat(60.f, 150.f);
				Crashdesc.bGravity = true;
				Crashdesc.fScale = { CUtils::Make_RandomFloat(0.1f, 0.6f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
					return;
			}

			m_fParticleTime = 0.f;
		}

		if (m_fBoomTime > 0.4f)
		{
			if (m_iBoomCount < 4)
			{
				if (m_iBoomCount == 0)
				{
					CMultiEffect::MULTI_FX_DESC FXM2desc{};
					FXM2desc.vInitPos = _float3(vMakingPos) + _float3(0.f, 1.f, 0.f);
					FXM2desc.vInitScale = { 1.f, 1.f, 1.f };
					if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Lights"), &FXM2desc)))
						return;
				}


				CEffect::FX_DESC FXDesc{};
				FXDesc.vInitPos = _float3(vMakingPos);
				FXDesc.vInitRot = { 0.f, 0.f, 0.f };
				FXDesc.vInitScale = { 1.f + ((_float)m_iBoomCount * 0.2f), 1.f + ((_float)m_iBoomCount * 0.2f), 1.f + ((_float)m_iBoomCount * 0.2f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Smoke"), &FXDesc)))
					return;

				CMultiEffect::MULTI_FX_DESC FXMdesc{};
				FXMdesc.vInitPos = _float3(vMakingPos) + _float3(0.f, 1.f, 0.f);
				FXMdesc.vInitScale = { 1.f, 1.f, 1.f };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Booms"), &FXMdesc)))
					return;

				m_iBoomCount++;

			}
			m_fBoomTime = 0.f;
		}




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


		_float4 vMakingPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_fParticleTime += fTime;
		m_fRange += fTime * 12.f;
		m_fBoomTime += fTime;

		if (m_fParticleTime > 0.2f)
		{
			for (_int i = 0; i < 10; ++i)
			{
				_float4 vTemp = { m_fRange * 2.f, 0.f, 0.f, 0.f };
				_float4x4 RotMat = _float4x4::Identity;
				CUtils::Turn_OtherMatrix(RotMat, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
				vTemp = XMVectorSetW(XMVector3Transform(vTemp, RotMat), 0.f);
				CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
				Crashdesc.vPos = vMakingPos + vTemp;
				Crashdesc.vDir = vTemp;
				Crashdesc.vDir.Normalize();
				Crashdesc.vDir.y += 1.f;
				Crashdesc.vDir.Normalize();
				Crashdesc.fSpeed = CUtils::Make_RandomFloat(60.f, 150.f);
				Crashdesc.bGravity = true;
				Crashdesc.fScale = { CUtils::Make_RandomFloat(0.1f, 0.6f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
					return;
			}

			m_fParticleTime = 0.f;
		}

		if (m_fBoomTime > 0.4f)
		{
			if (m_iBoomCount < 4)
			{
				CEffect::FX_DESC FXDesc{};
				FXDesc.vInitPos = _float3(vMakingPos);
				FXDesc.vInitRot = { 0.f, 0.f, 0.f };
				FXDesc.vInitScale = { 1.f + ((_float)m_iBoomCount * 0.2f), 1.f + ((_float)m_iBoomCount * 0.2f), 1.f + ((_float)m_iBoomCount * 0.2f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Smoke"), &FXDesc)))
					return;

				CMultiEffect::MULTI_FX_DESC FXMdesc{};
				FXMdesc.vInitPos = _float3(vMakingPos) + _float3(0.f, 1.f, 0.f);
				FXMdesc.vInitScale = { 1.f, 1.f, 1.f };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Booms"), &FXMdesc)))
					return;

				m_iBoomCount++;

			}
			m_fBoomTime = 0.f;
		}

		m_pGameInstance->Get_DirectionLightAddress()->Interpolate_Light(_float4(0.001f, 0.001f, 0.001f, 0.001f), 1.f, 1.f);




		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::CRASHSTATE_ATTACKEND, 60.f, false, false, CKirby::BODY_CRASHDEFAULT, CKirby::OFFSET_CRASH);
			return;
		}

	}
	else if(pKirby->Get_State() == CKirby::CRASHSTATE_ATTACKEND)
	{
		if (m_iBoomCount == 0)
		{
			CEffect::FX_DESC FXDesc{};
			FXDesc.vInitPos = { 0.f, 0.f, 0.f };
			FXDesc.vInitRot = { 0.f, 0.f, 0.f };
			FXDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash FIn"), &FXDesc)))
				return;

			m_iBoomCount = 1;
		}

		_float fTime = m_pGameInstance->Get_OriginalTimer();
		DESC(m_fTimeRatio) += fTime * 0.3f;
		if (DESC(m_fTimeRatio) > 1.f)
			DESC(m_fTimeRatio) = 1.f;

		m_pGameInstance->Set_FirstTimerRatio(DESC(m_fTimeRatio));
		m_pGameInstance->Set_SecondTimerRatio(DESC(m_fTimeRatio) * 0.2f);


		if (pKirby->isAnimFinish())
		{
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			vPos.y += 5.f;
			CParticle::PARTICLE_DESC FXPDesc{};
			FXPDesc.vInitPos = (_float3)vPos;
			FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Fin Particle"), &FXPDesc)))
				return;

			m_pGameInstance->Set_Brown(0.5f, true);
			m_pGameInstance->Get_DirectionLightAddress()->Interpolate_Light(DESC(m_vPreDiffuseLight), 1.f, 2.f);
			static_cast<CCamera_Main*>(pCamera)->Zoom(0.f);

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

	m_bLightRangeInv = false;
	m_fLightRange = 0.f;

	m_fParticleTime = 0.f;
	m_fRange = 0.f;
	m_fBoomTime = 0.f;
	m_iBoomCount = 0;

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
		static_cast<CCamera_Main*>(pCamera)->Zoom(20.f - (pKirby->Get_AnimRatio() * 20.f));
		_float4 vMakingPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		vMakingPos.y += (-DESC(m_fCrashChargeTime) + 2.5f);

		_float fTime = m_pGameInstance->Get_OriginalTimer();

		m_fParticleTime += fTime;
		m_fRange += fTime * 6.f;
		m_fBoomTime += fTime;

		if (m_fParticleTime > 0.2f)
		{
			for (_int i = 0; i < 10; ++i)
			{
				_float4 vTemp = { m_fRange * 2.f, 0.f, 0.f, 0.f };
				_float4x4 RotMat = _float4x4::Identity;
				CUtils::Turn_OtherMatrix(RotMat, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
				vTemp = XMVectorSetW(XMVector3Transform(vTemp, RotMat), 0.f);
				CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
				Crashdesc.vPos = vMakingPos + vTemp;
				Crashdesc.vDir = vTemp;
				Crashdesc.vDir.Normalize();
				Crashdesc.vDir.y += 1.f;
				Crashdesc.vDir.Normalize();
				Crashdesc.fSpeed = CUtils::Make_RandomFloat(60.f, 150.f);
				Crashdesc.bGravity = true;
				Crashdesc.fScale = { CUtils::Make_RandomFloat(0.1f, 0.6f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
					return;
			}

			m_fParticleTime = 0.f;
		}

		if (m_fBoomTime > 0.4f)
		{
			if (m_iBoomCount < 4)
			{
				CEffect::FX_DESC FXDesc{};
				FXDesc.vInitPos = _float3(vMakingPos);
				FXDesc.vInitRot = { 0.f, 0.f, 0.f };
				FXDesc.vInitScale = { 1.f + ((_float)m_iBoomCount * 0.2f), 1.f + ((_float)m_iBoomCount * 0.2f), 1.f + ((_float)m_iBoomCount * 0.2f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Smoke"), &FXDesc)))
					return;

				CMultiEffect::MULTI_FX_DESC FXMdesc{};
				FXMdesc.vInitPos = _float3(vMakingPos) + _float3(0.f, 5.f, 0.f);
				FXMdesc.vInitScale = { 1.f, 1.f, 1.f };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Booms"), &FXMdesc)))
					return;

				if (m_iBoomCount == 3)
				{
					FXDesc.vInitPos = { 0.f, 0.f, 0.f };
					FXDesc.vInitRot = { 0.f, 0.f, 0.f };
					FXDesc.vInitScale = { 1.f, 1.f, 1.f };
					if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash FIn"), &FXDesc)))
						return;
				}


				if (m_iBoomCount == 0)
				{
					CMultiEffect::MULTI_FX_DESC FXM2desc{};
					FXM2desc.vInitPos = _float3(vMakingPos) + _float3(0.f, 5.f, 0.f);
					FXM2desc.vInitScale = { 1.f, 1.f, 1.f };
					if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Lights"), &FXM2desc)))
						return;
				}

				m_iBoomCount++;

			}
			m_fBoomTime = 0.f;
		}


		DESC(m_fTimeRatio) += fTime * 0.3f;
		if (DESC(m_fTimeRatio) > 1.f)
			DESC(m_fTimeRatio) = 1.f;

		m_pGameInstance->Get_DirectionLightAddress()->Interpolate_Light(_float4(0.001f, 0.001f, 0.001f, 0.001f), 1.f, 1.f);
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

		_float fLargeLight = 5.f + (m_fLightRange * 7.f);
		if (fLargeLight < 5.f)
			fLargeLight = 5.f;

		pKirby->Large_Light(_float4(CUtils::Make_RandomFloat(0.1f, 1.f), CUtils::Make_RandomFloat(0.1f, 1.f), CUtils::Make_RandomFloat(0.1f, 1.f), 1.f), fLargeLight, 0.001f);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset), fOffset);

		if (pKirby->isAnimFinish())
		{
			if (m_bTerrainOn == false)
			{
				_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
				vPos.y += 5.f;
				CParticle::PARTICLE_DESC FXPDesc{};
				FXPDesc.vInitPos = (_float3)vPos;
				FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Boom Fin Particle"), &FXPDesc)))
					return;
			}

			m_pGameInstance->Set_Brown(0.5f, true);
			m_pGameInstance->Get_DirectionLightAddress()->Interpolate_Light(DESC(m_vPreDiffuseLight), 1.f, 2.f);
			static_cast<CCamera_Main*>(pCamera)->Zoom(0.f);
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

	m_fParticleTime = 0.f;
	m_fRange = 0.f;
	m_fBoomTime = 0.f;
	m_iBoomCount = 0;
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

	pKirby->Large_Light(_float4(0.9f, 1.f, 1.f, 1.f), 9.f, 0.001f);

	if (m_bNextState == true)
	{
		// 1틱 발동
		if (m_bTrigger == true)
		{
			CParticle::PARTICLE_DESC FXPDesc{};
			FXPDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXPDesc.vInitPos = _float3{ 0.f, 2.f, 0.f };
			FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Start Particle2"), &FXPDesc)))
				return;

			m_pGameInstance->Set_FirstTimerRatio(0.f);
			m_pGameInstance->Set_SecondTimerRatio(0.f);
			DESC(m_vPreDiffuseLight) = m_pGameInstance->Get_DirectionLightAddress()->Get_LightDesc()->vDiffuse;
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(0.3f, 0.5f);
			pKirby->Delete_Effect("YW Crash Charge");
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
		if (m_bEffectTrigger == true)
		{
			CMultiEffect::MULTI_FX_DESC MFXDesc{};
			MFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			MFXDesc.vInitPos = _float3{ 0.f, 1.5f, 0.f };
			MFXDesc.vInitScale = { 1.f, 1.f, 1.f };

			pKirby->Add_Effect("YW Crash Charge", MFXDesc, true);

			//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Charge"), &MFXDesc)))
			//	return;
			//pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
			m_bEffectTrigger = false;
		}

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
	m_bEffectTrigger = true;
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
			CParticle::PARTICLE_DESC FXPDesc{};
			FXPDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXPDesc.vInitPos = _float3{ 0.f, 2.f, 0.f };
			FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_YW Crash Start Particle2"), &FXPDesc)))
				return;

			m_pGameInstance->Set_FirstTimerRatio(0.f);
			m_pGameInstance->Set_SecondTimerRatio(0.f);
			DESC(m_vPreDiffuseLight) = m_pGameInstance->Get_DirectionLightAddress()->Get_LightDesc()->vDiffuse;
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(0.3f, 0.5f);

			pKirby->Delete_Effect("YW Crash Charge");

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

	pKirby->Large_Light(_float4(0.9f, 1.f, 1.f, 1.f), 12.f, 0.001f);


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
