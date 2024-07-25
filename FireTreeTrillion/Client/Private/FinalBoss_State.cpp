#include "stdafx.h"
#include "FinalBoss_State.h"
#include "FinalBoss.h"
#include "Kirby.h"
#include "Laser.h"
#include "DimensionGate.h"
#include "Meteor.h"
#include "BossClone.h"
#include "Ability.h"
#include "Camera_Main.h"
#include "Gully.h"
//#include "SpikeSpear.h"


//이펙트 생성 함수
void ThrustCharge(CFinalBoss* pBoss)
{
	CMultiEffect::MULTI_FX_DESC FXDesc{};
	FXDesc.pSocketMatrix = pBoss->Get_TransformCom()->Get_WorldFloat4x4_Ptr();
	FXDesc.vInitPos = { 0.f, 1.f, 6.f };
	FXDesc.vInitScale = { 5.f, 5.f, 5.f };

	FXDesc.fStartDelay = 1.f;

	pBoss->Add_Effect("HS_FB thrust ready", FXDesc, false);
}

void AirStep_Smoke(CFinalBoss* pBoss)
{

}

void DimensionGateLight(CFinalBoss* pBoss)
{
}

void LaserReady(CFinalBoss* pBoss)
{
	CMultiEffect::MULTI_FX_DESC MDesc{};
	MDesc.pSocketMatrix = pBoss->Get_TransformCom()->Get_WorldFloat4x4_Ptr();
	MDesc.vInitPos = { 0.f, 2.f, 0.f };
	MDesc.vInitScale = { 2.f, 2.f, 2.f };
	//pBoss->Add_Effect("HS_FB laser charge particle B", ParticleDesc, false);YW Laser Set Effects
	pBoss->Add_Effect("YW Laser Set Effects B", MDesc, true);

	/*CMultiEffect::MULTI_FX_DESC FXDesc{};
	FXDesc.pSocketMatrix = pBoss->Get_TransformCom()->Get_WorldFloat4x4_Ptr();
	FXDesc.vInitPos = { 0.f, 3.f, 2.f };
	FXDesc.vInitScale = { 2.f, 2.f, 2.f };

	FXDesc.fStartDelay = 1.f;

	pBoss->Add_Effect("HS_FB charge light", FXDesc);*/
}
#pragma region APPEAR STATE
//*********************************
//			 APPEAR STATE
//*********************************
CFinalBoss_Appear_State::CFinalBoss_Appear_State()
{
}

void CFinalBoss_Appear_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Appear_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	m_pGameInstance->Update_DofFocus(pTransformCom->Get_State(CTransform::STATE_POSITION));

	if (pFinalBoss->IsAnimFinished())
	{
		HRESULT hr;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_BossUI"), TEXT("Prototype_GameObject_HUD_BossHpBar"), pFinalBoss);
		CHECK_FAILED(hr);

		CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
		pKirby->Get_KirbyInfo()->m_bFinalBossCutStart = false;
		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
	}
}

void CFinalBoss_Appear_State::OnStateExit()
{
}

CFinalBoss_Appear_State* CFinalBoss_Appear_State::Create()
{
	CFinalBoss_Appear_State* pInstance = new CFinalBoss_Appear_State();
	return pInstance;
}

void CFinalBoss_Appear_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CFinalBoss_Idle_State::CFinalBoss_Idle_State()
{
}

void CFinalBoss_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fDelayTime = 0.f;
}

void CFinalBoss_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f, false);

	if (true == pFinalBoss->Get_Auto())
	{
		if (CFinalBoss::STATE_2PAZE == pFinalBoss->Get_BossState())
		{
			//pFinalBoss->Set_BossState(CFinalBoss::STATE_FLYING);
			if (CFinalBoss::FINALBOSS_WAITAIR == pFinalBoss->Get_State())
			{
				if (pFinalBoss->IsAnimFinished())
				{
					if (0 == m_iMeteor)
					{
						++m_iMeteor;
						pFinalBoss->Change_State(CFinalBoss::FINALBOSS_ROAR, 50.f, false, true);
					}
					else if (1 == m_iMeteor)
					{
						++m_iMeteor;
						pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
					}
					else
					{
						pFinalBoss->Set_Chain(true);
						pFinalBoss->Set_BossState(CFinalBoss::STATE_FLYING);
						m_iCnt = 18;
						pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
					}
				}
			}
		}
		else if (CFinalBoss::FINALBOSS_WAITAIR == pFinalBoss->Get_State())
		{
			if (0 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished()/*0.5f < pFinalBoss->Get_AnimRatio()*/)
				{
					++m_iCnt;
					m_vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;
					pFinalBoss->Set_Direction(m_vLook);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABREADY, 50.f, false, true);
				}
			}
			else if (5 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHREADY, 50.f, false, true);
				}
			}
			else if (8 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					// 좌우 활공 패턴
					_vector vBossPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
					_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

					pFinalBoss->Set_Direction(RotateGlide(vKirbyPos, vBossPos, 45.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNRIGHTAIRSTART, 50.f, false, true);
				}
			}
			else if (9 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					// 사운드
					m_pGameInstance->PlaySound_Free(L"BossChimera_Laser.wav", 0.5f);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONLASEREADY, 50.f, false, true);
				}
			}
			else if (10 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADYAIR, 50.f, false, true);
				}
			}
			else if (11 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					m_vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;
					pFinalBoss->Set_Direction(m_vLook);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABREADY, 50.f, false, true);
				}
			}
			else if (14 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHREADY, 50.f, false, true);
				}
			}
			else if (18 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					m_vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;
					pFinalBoss->Set_Direction(m_vLook);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABREADY, 50.f, false, true);
				}
			}
			else if (21 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished()/*0.5f < pFinalBoss->Get_AnimRatio()*/)
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
				}
			}
			else if (22 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					m_vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;
					pFinalBoss->Set_Direction(m_vLook);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABREADY, 50.f, false, true);
				}
			}
		}
		else
		{
			if (1 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADY, 50.f, false, true);
				}
			}
			else if (2 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADY, 50.f, false, true);
				}
			}
			else if (3 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGRIGHTSTART, 40.f, false, true);
				}
			}
			else if (4 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);
				}
			}
			else if (6 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGRIGHTSTART, 50.f, false, true);
				}
			}
			else if (7 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);
				}
			}
			else if (12 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					// 사운드
					m_pGameInstance->PlaySound_Free(L"BossChimera_ThrustReady.wav", 0.5f);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTREADY, 50.f, false, true);
					//효선아 Thrust
					ThrustCharge(pFinalBoss);
				}
			}
			else if (13 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);
				}
			}
			else if (15 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					// 사운드
					m_pGameInstance->PlaySound_Free(L"BossChimera_ThrustReady.wav", 0.5f);
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTREADY, 50.f, false, true);
					//효선아 Thrust
					ThrustCharge(pFinalBoss);
				}
			}
			else if (16 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGRIGHTSTART, 50.f, false, true);
				}
			}
			else if (17 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);
				}
			}
			else if (19 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADY, 50.f, false, true);
				}
			}
			else if (20 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished())
				{
					++m_iCnt;
					pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);
				}
			}
			else if (23 == m_iCnt)
			{
				if (pFinalBoss->IsAnimFinished()/*0.5f < pFinalBoss->Get_AnimRatio()*/)
				{
					m_iCnt = 6;
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_JUMPREADY, 50.f, false, true);
				}
			}
		}
	}
	else
	{
		if (CFinalBoss::STATE_2PAZE == pFinalBoss->Get_BossState())
		{
			//pFinalBoss->Set_BossState(CFinalBoss::STATE_FLYING);
			if (CFinalBoss::FINALBOSS_WAITAIR == pFinalBoss->Get_State())
			{
				if (pFinalBoss->IsAnimFinished())
				{
					if (0 == m_iCnt)
					{
						++m_iCnt;
						pFinalBoss->Change_State(CFinalBoss::FINALBOSS_ROAR, 50.f, false, true);
					}
					else if (1 == m_iCnt)
					{
						++m_iCnt;
						pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
					}
					else
					{
						pFinalBoss->Set_Chain(true);
						pFinalBoss->Set_BossState(CFinalBoss::STATE_FLYING);
						pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
					}
				}
			}
		}
		else if (CFinalBoss::FINALBOSS_WAITAIR == pFinalBoss->Get_State())
		{
			if (true == pFinalBoss->Get_Stab())
			{
				pFinalBoss->Set_Stab(false);
				// Stab 패턴
				m_vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;
				pFinalBoss->Set_Direction(m_vLook);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABREADY, 50.f, false, true);
			}
			else if (true == pFinalBoss->Get_Slash())
			{
				pFinalBoss->Set_Slash(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHREADY, 50.f, false, true);
			}
			else if (true == pFinalBoss->Get_Meteor())
			{
				pFinalBoss->Set_Meteor(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
			}
			else if (true == pFinalBoss->Get_Laser())
			{
				pFinalBoss->Set_Laser(false);
				// 사운드
				m_pGameInstance->PlaySound_Free(L"BossChimera_Laser.wav", 0.5f);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONLASEREADY, 50.f, false, true);
			}
			else if (true == pFinalBoss->Get_Side())
			{
				pFinalBoss->Set_Side(false);
				// 좌우 활공 패턴
				_vector vBossPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

				if (rand() % 2 == 0)
				{
					pFinalBoss->Set_Direction(RotateGlide(vKirbyPos, vBossPos, -45.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNLEFTAIRSTART, 50.f, false, true);
				}
				else
				{
					pFinalBoss->Set_Direction(RotateGlide(vKirbyPos, vBossPos, 45.f));
					pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNRIGHTAIRSTART, 50.f, false, true);
				}
			}
			else if (true == pFinalBoss->Get_AirArrow())
			{
				pFinalBoss->Set_AirArrow(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADYAIR, 50.f, false, true);
			}
		}
		else
		{
			if (true == pFinalBoss->Get_BackStep())
			{
				pFinalBoss->Set_BackStep(false);
				pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));

				CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
				MultiFXDesc.vInitPos = static_cast<_float3> (pTransformCom->Get_State(CTransform::STATE_POSITION));
				MultiFXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 360.f),0.f };
				MultiFXDesc.vInitScale = { 2.f, 2.f, 2.f };
				MultiFXDesc.fStartDelay = .5f;
				pFinalBoss->Add_Effect("HS_FB fly smoke", MultiFXDesc);

				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);
			}
			else if (true == pFinalBoss->Get_Swing())
			{
				pFinalBoss->Set_Swing(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGRIGHTSTART, 40.f, false, true);
			}
			else if (true == pFinalBoss->Get_GroundArrow())
			{
				pFinalBoss->Set_GroundArrow(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADY, 50.f, false, true);
			}
			else if (true == pFinalBoss->Get_Thrust())
			{
				// 사운드
				m_pGameInstance->PlaySound_Free(L"BossChimera_ThrustReady.wav", 0.5f);
				pFinalBoss->Set_Thrust(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTREADY, 50.f, false, true);
				//효선아 Thrust
				ThrustCharge(pFinalBoss);
			}
			else if (true == pFinalBoss->Get_Spike())
			{
				pFinalBoss->Set_Spike(false);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADY, 50.f, false, true);
			}
		}
	}
}

void CFinalBoss_Idle_State::OnStateExit()
{
}

_vector CFinalBoss_Idle_State::RotateGlide(_fvector vPivotPos, _fvector _vRotatePos, _float fAngle)
{
	// 원점 기준 상대 좌표 구하기
	_float4 vRotatePos = _vRotatePos - vPivotPos;

	_float rotatedX = vRotatePos.x * cos(ToRadian(fAngle)) - vRotatePos.z * sin(ToRadian(fAngle));
	_float rotatedZ = vRotatePos.x * sin(ToRadian(fAngle)) + vRotatePos.z * cos(ToRadian(fAngle));

	// 회전된 좌표를 다시 원점 기준으로 변환
	_float4 vRotateBossPos;
	vRotateBossPos.x = rotatedX + vPivotPos.m128_f32[0];
	vRotateBossPos.y = _vRotatePos.m128_f32[1];
	vRotateBossPos.z = rotatedZ + vPivotPos.m128_f32[2];
	vRotateBossPos.w = 1.f;

	return vRotateBossPos;
}

CFinalBoss_Idle_State* CFinalBoss_Idle_State::Create()
{
	CFinalBoss_Idle_State* pInstance = new CFinalBoss_Idle_State();
	return pInstance;
}

void CFinalBoss_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region STAB STATE
//*********************************
//			 STAB STATE
//*********************************
CFinalBoss_Stab_State::CFinalBoss_Stab_State()
{
}

void CFinalBoss_Stab_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fSpeed = 100.f;
	m_bSound = false;
}

void CFinalBoss_Stab_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	if (CFinalBoss::FINALBOSS_STABREADY == pFinalBoss->Get_State())
	{
		if (0.8f < pFinalBoss->Get_AnimRatio())
		{
			if(false == m_bSound)
			{
				m_bSound = true;
				// 사운드 처리
				m_pGameInstance->PlaySound_Free(L"BossChimera_StabStart.wav", 0.5f);
			}
		}
	}

	pTransformCom->Look_At_Axis(pFinalBoss->Get_Direction());

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_STABREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_STABSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABWAIT, 50.f, true, true);
			break;
		case CFinalBoss::FINALBOSS_STAB:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_STABEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SLASHCHAINSTABREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABSTART, 50.f, false, true);
			break;
		}
	}

	if (CFinalBoss::FINALBOSS_STABSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_STABWAIT == pFinalBoss->Get_State())
	{
		m_fSpeed += fTimeDelta * 30.f;
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * m_fSpeed, fTimeDelta);

		if (CFinalBoss::FINALBOSS_STABWAIT == pFinalBoss->Get_State())
		{
			if (pController->Is_Terrain())
			{
				_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
				_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

				CMultiEffect::MULTI_FX_DESC FXDesc{};
				FXDesc.vInitPos = (_float3)(vPos + vLook * 3.f);
				FXDesc.vInitScale = { 2.5f, 2.5f, 2.5f };
				pFinalBoss->Add_Effect("HS_FB arrow bomb", FXDesc, false);
				CParticle::PARTICLE_DESC FXPDesc{};
				FXPDesc.vInitPos = (_float3)(vPos + vLook * 3.f);
				FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
				pFinalBoss->Add_Effect("YW Final Boss Wiggle B", FXPDesc, false);

				// 사운드 처리
				m_pGameInstance->PlaySound_Free(L"BossChimera_Stab.wav", 0.8f);

				vLook.y = 0.f;
				HRESULT hr = S_OK;

				// 별 아이템 떨굼
				_uint iItemCnt = { 6 };
				for (_uint i = 0; i < iItemCnt; ++i)
				{
					CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
					if (i < iItemCnt / 2)
						AbilityItemDesc.fRotateDir = 1.f;																	// 별 회전 방향 오른쪽
					else
						AbilityItemDesc.fRotateDir = -1.f;																	// 별 회전 방향 왼쪽
					AbilityItemDesc.fAngle = 360.f / (_float)iItemCnt * i;													// 별의 진행 방향의 각도
					AbilityItemDesc.vDir = pTransformCom->Get_State(CTransform::STATE_LOOK);							// 별의 진행 방향
					AbilityItemDesc.vPosition = pTransformCom->Get_State(CTransform::STATE_POSITION) + vLook * 5.5f;	// 별의 생성 위치
					AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
					hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
					CHECK_FAILED(hr);
				}

				CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
				if (pCamera != nullptr)
					pCamera->Make_Shake(0.5f, 1.f);

				pFinalBoss->Activate_FrustumCollider(0.5f, 8.f, 120.f);
				pFinalBoss->Set_BossState(CFinalBoss::STATE_GROUND);
				pFinalBoss->Set_Particle(true);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STAB, 50.f, false, true);
			}
		}
	}
}

void CFinalBoss_Stab_State::OnStateExit()
{
}

CFinalBoss_Stab_State* CFinalBoss_Stab_State::Create()
{
	CFinalBoss_Stab_State* pInstance = new CFinalBoss_Stab_State();
	return pInstance;
}

void CFinalBoss_Stab_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region GLIDEBACK STATE
//*********************************
//			 GLIDEBACK STATE
//*********************************
CFinalBoss_GlideBack_State::CFinalBoss_GlideBack_State()
{
}

void CFinalBoss_GlideBack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fSpeed = 1.f;
	m_fTimeDelta = 0.f;
}

void CFinalBoss_GlideBack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	if (CFinalBoss::FINALBOSS_AWAYFASTSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_AWAYFAST == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_AWAYFASTENDAIR == pFinalBoss->Get_State())
	{
		if (CFinalBoss::FINALBOSS_AWAYFASTENDAIR == pFinalBoss->Get_State())
		{
			if (0.3f < pFinalBoss->Get_AnimRatio())
			{
				if (0.f < m_fSpeed)
					m_fSpeed -= fTimeDelta * 2.f;
				else
					m_fSpeed = 0.f;

				pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * 60.f * m_fSpeed, fTimeDelta);
			}
			else
				pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * 60.f, fTimeDelta);
		}
		else
			pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * 60.f, fTimeDelta);
	}

	if (CFinalBoss::FINALBOSS_AWAYFASTENDAIR == pFinalBoss->Get_State())
	{
		if (pFinalBoss->IsAnimFinished())
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
	}
	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_AWAYFASTREADY:
			m_pGameInstance->PlaySound_Free(L"BossChimera_BackStep.wav", 0.35f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_AWAYFASTSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFAST, 30.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_AWAYFAST:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTENDAIR, 50.f, false, true);
			break;
			//case CFinalBoss::FINALBOSS_AWAYFASTENDAIR:
			//	pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTENDAIR, 50.f, false, true);
			//	break;
		}
	}
}

void CFinalBoss_GlideBack_State::OnStateExit()
{
}

CFinalBoss_GlideBack_State* CFinalBoss_GlideBack_State::Create()
{
	CFinalBoss_GlideBack_State* pInstance = new CFinalBoss_GlideBack_State();
	return pInstance;
}

void CFinalBoss_GlideBack_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region GLIDE STATE
//*********************************
//			 GLIDE STATE
//*********************************
CFinalBoss_Glide_State::CFinalBoss_Glide_State()
{
}

void CFinalBoss_Glide_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Glide_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	pTransformCom->Look_At_Interpolate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta);

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_TURNLEFTAIRSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNLEFTAIR, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_TURNLEFTAIR:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNLEFTAIREND, 60.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_TURNLEFTAIREND:
			pFinalBoss->Set_Glide(false);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_TURNRIGHTAIRSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNRIGHTAIR, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_TURNRIGHTAIR:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNRIGHTAIREND, 60.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_TURNRIGHTAIREND:
			pFinalBoss->Set_Glide(false);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			break;
		}
	}

	if (CFinalBoss::FINALBOSS_TURNLEFTAIRSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_TURNRIGHTAIRSTART == pFinalBoss->Get_State())
	{
		if (0.3f < pFinalBoss->Get_AnimRatio())
			pFinalBoss->Set_Glide(true);
	}
}

void CFinalBoss_Glide_State::OnStateExit()
{
}

CFinalBoss_Glide_State* CFinalBoss_Glide_State::Create()
{
	CFinalBoss_Glide_State* pInstance = new CFinalBoss_Glide_State();
	return pInstance;
}

void CFinalBoss_Glide_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region SLASH STATE
//*********************************
//			 SLASH STATE
//*********************************
CFinalBoss_Slash_State::CFinalBoss_Slash_State()
{
}

void CFinalBoss_Slash_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fTimeDelta = 0.f;
	m_fSpeed = 70.f;
	m_bChain = false;
	m_bSound = false;
}

void CFinalBoss_Slash_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 현재 위치 벡터
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_SLASHREADY:
			m_pGameInstance->PlaySound_Free(L"BossChimera_GullyStart.wav", 0.5f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SLASHSTART:
			_vector vLook = vKirbyPos - vPos;
			vLook.m128_f32[1] = 0.f;
			pFinalBoss->Set_Direction(vLook);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASH, 60.f, true, true);
			break;
			//case CFinalBoss::FINALBOSS_SLASHEND:
			//	//pFinalBoss->Set_FlyHigh(false);
			//	pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			//	break;
			//case CFinalBoss::FINALBOSS_SLASHEND:
			//	pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHSTART, 50.f, false, true);
			//	break;
		}
	}

	if (CFinalBoss::FINALBOSS_SLASHSTART == pFinalBoss->Get_State())
	{
		pTransformCom->Look_At_Interpolate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta);

		m_fTimeDelta += fTimeDelta;

		if (1.f > m_fTimeDelta)
		{
			vPos.m128_f32[0] += m_fTimeDelta;
			vPos.m128_f32[1] += -tan(m_fTimeDelta) * 2.f;
		}

		if (5.f > vPos.m128_f32[1])
		{
			_vector vLook = vKirbyPos - vPos;
			vLook.m128_f32[1] = 0.f;
			pFinalBoss->Set_Direction(vLook);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASH, 60.f, true, true);
		}
		vPos.m128_f32[1] += 1.f;
		pController->Move(pTransformCom, vPos, fTimeDelta);
	}
	else if (CFinalBoss::FINALBOSS_SLASH == pFinalBoss->Get_State())
	{
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * m_fSpeed, fTimeDelta);

		if (5.f > pController->Compute_Height(XMVectorSet(0.f, -1.f, 0.f, 0.f)))
		{
			pFinalBoss->Set_Gully(true);
			//m_fSpeed = 10.f;

			if(false == m_bSound)
			{
				m_bSound = true;
				m_pGameInstance->PlaySound_Free(L"BossChimera_Gully.wav", 0.5f);
			}
		}
		else
		{
			pFinalBoss->Set_Gully(false);
			//m_fSpeed = 70.f;
		}

		m_fTimeDelta += fTimeDelta;
		if (3.f < m_fTimeDelta)
		{
			if (false == pFinalBoss->Get_Chain())
			{
				pFinalBoss->Set_Chain(true);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHCHAINREADY, 50.f, false, true);
			}
			else
			{
				pFinalBoss->Set_Chain(false);
				pFinalBoss->Set_Direction(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHCHAINSTABREADY, 50.f, false, true);
			}
			//if (true == pFinalBoss->Get_Chain())
			//	pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHCHAINREADY, 50.f, false, true);
			//else
			//{
			//	pFinalBoss->Set_Direction(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f);
			//	pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHCHAINSTABREADY, 50.f, false, true);
			//}
		}
	}
}

void CFinalBoss_Slash_State::OnStateExit()
{
}

CFinalBoss_Slash_State* CFinalBoss_Slash_State::Create()
{
	CFinalBoss_Slash_State* pInstance = new CFinalBoss_Slash_State();
	return pInstance;
}

void CFinalBoss_Slash_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region CHAIN STATE
//*********************************
//			 CHAIN STATE
//*********************************
CFinalBoss_Chain_State::CFinalBoss_Chain_State()
{
}

void CFinalBoss_Chain_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fTimeDelta = 0.f;
}

void CFinalBoss_Chain_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 현재 위치 벡터
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	m_fTimeDelta += fTimeDelta;

	if (1.f > m_fTimeDelta)
	{
		vPos.m128_f32[1] += m_fTimeDelta + 1.f;
		pController->Move(pTransformCom, vPos, fTimeDelta);
	}

	//pTransformCom->Look_At_Interpolate(vKirbyPos, fTimeDelta * 2.f);
	_vector vLook = XMVector3Normalize(pTransformCom->Get_State(CTransform::STATE_LOOK));
	vLook.m128_f32[1] = 0.f;
	_vector vTargetLook = XMVector3Normalize(vKirbyPos - vPos);
	vTargetLook.m128_f32[1] = 0.f;
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, vTargetLook));
	_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, vTargetLook));
	if (fY < 0)
		fAngle = -fAngle;
	if (abs(fAngle) >= XMConvertToRadians(3.f))
		pTransformCom->Turn(::XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * fAngle * 5.f);

	if (pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_SLASHCHAINREADY:
			m_pGameInstance->PlaySound_Free(L"BossChimera_GullyStart.wav", 0.5f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SLASHCHAINSTABREADY:
			pFinalBoss->Set_Direction(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABSTART, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Chain_State::OnStateExit()
{
}

CFinalBoss_Chain_State* CFinalBoss_Chain_State::Create()
{
	CFinalBoss_Chain_State* pInstance = new CFinalBoss_Chain_State();
	return pInstance;
}

void CFinalBoss_Chain_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region SWING STATE
//*********************************
//			 SWING STATE
//*********************************
CFinalBoss_Swing_State::CFinalBoss_Swing_State()
{
}

void CFinalBoss_Swing_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_bItem = false;
	m_fSpeed = 1.f;
}

void CFinalBoss_Swing_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	if (CFinalBoss::FINALBOSS_SWINGRIGHTSTART == pFinalBoss->Get_State())
	{
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 3.f);

	}
	else if (CFinalBoss::FINALBOSS_SWINGRIGHT == pFinalBoss->Get_State())
	{
		if (0.7f < pFinalBoss->Get_AnimRatio())
		{
			if (false == m_bItem)
			{
				m_bItem = true;

				HRESULT hr;

				// 별 아이템 떨굼
				CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
				AbilityItemDesc.fRotateDir = -1.f;																	// 별 회전 방향 오른쪽																// 별 회전 방향 왼쪽
				AbilityItemDesc.fAngle = 0.f;													// 별의 진행 방향의 각도
				AbilityItemDesc.vDir = XMVectorSet(0.f, 0.f, 0.f, 0.f);							// 별의 진행 방향
				AbilityItemDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) * 2.5f - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;	// 별의 생성 위치
				AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);
			}
		}

		// 브레이크 : 제곱 감속
		_float fDeceleration = m_fSpeed * m_fSpeed;

		if (0.f < m_fSpeed)
			m_fSpeed -= fTimeDelta * 2.f;
		else
			m_fSpeed = 0.f;
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * 20.f * fDeceleration, fTimeDelta);
	}
	else if (CFinalBoss::FINALBOSS_SWINGFINISHLEFT == pFinalBoss->Get_State())
	{
		if (0.45f < pFinalBoss->Get_AnimRatio())
		{
			if (false == m_bItem)
			{
				m_bItem = true;

				HRESULT hr;

				_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
				vLook.m128_f32[1] = 0.f;

				// 별 아이템 떨굼
				for (_uint i = 0; i < 4; ++i)
				{
					CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
					AbilityItemDesc.fRotateDir = 1.f;																	// 별 회전 방향 오른쪽
					AbilityItemDesc.fAngle = 90.f * i;
					AbilityItemDesc.vDir = vLook * 2.f;							// 별의 진행 방향
					AbilityItemDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);	// 별의 생성 위치
					AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
					hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
					CHECK_FAILED(hr);
				}
			}
		}
		else if (0.2f < pFinalBoss->Get_AnimRatio())
		{
			pFinalBoss->Set_TickPerSecond(65.f);

			// 브레이크 : 제곱 감속
			_float fDeceleration = m_fSpeed * m_fSpeed;

			if (0.f < m_fSpeed)
				m_fSpeed -= fTimeDelta * 2.f;
			else
				m_fSpeed = 0.f;
			pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * 30.f * fDeceleration, fTimeDelta);
		}
		else
			pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 3.f);
	}
	else if (CFinalBoss::FINALBOSS_SWINGLEFT == pFinalBoss->Get_State())
	{
		if (0.7f < pFinalBoss->Get_AnimRatio())
		{
			if (false == m_bItem)
			{
				m_bItem = true;

				HRESULT hr;

				// 별 아이템 떨굼
				CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
				AbilityItemDesc.fRotateDir = 1.f;																	// 별 회전 방향 오른쪽																// 별 회전 방향 왼쪽
				AbilityItemDesc.fAngle = 0.f;													// 별의 진행 방향의 각도
				AbilityItemDesc.vDir = XMVectorSet(0.f, 0.f, 0.f, 0.f);							// 별의 진행 방향
				AbilityItemDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) * 3.5f - pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;	// 별의 생성 위치
				AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);
			}
		}
		else if (0.3f < pFinalBoss->Get_AnimRatio())
		{
			pFinalBoss->Set_TickPerSecond(65.f);

			// 브레이크 : 제곱 감속
			_float fDeceleration = m_fSpeed * m_fSpeed;

			if (0.f < m_fSpeed)
				m_fSpeed -= fTimeDelta * 2.f;
			else
				m_fSpeed = 0.f;
			pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * 20.f * fDeceleration, fTimeDelta);
		}
	}

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_SWINGRIGHTSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGRIGHT, 60.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SWINGRIGHT:
			if (rand() % 2 == 0)
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGFINISHLEFT, 30.f, false, true);
			else
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGLEFT, 30.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SWINGFINISHLEFT:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGFINISHLEFTEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SWINGFINISHLEFTEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 60.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SWINGLEFT:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGLEFTEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SWINGLEFTEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Swing_State::OnStateExit()
{
}

CFinalBoss_Swing_State* CFinalBoss_Swing_State::Create()
{
	CFinalBoss_Swing_State* pInstance = new CFinalBoss_Swing_State();
	return pInstance;
}

void CFinalBoss_Swing_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region ARROW STATE
//*********************************
//			 ARROW STATE
//*********************************
CFinalBoss_Arrow_State::CFinalBoss_Arrow_State()
{
}

void CFinalBoss_Arrow_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Arrow_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_RAYARROWREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_RAYARROWSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_RAYARROWEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_RAYARROWREADYAIR:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWSTARTAIR, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_RAYARROWSTARTAIR:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWENDAIR, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_RAYARROWENDAIR:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Arrow_State::OnStateExit()
{
}

CFinalBoss_Arrow_State* CFinalBoss_Arrow_State::Create()
{
	CFinalBoss_Arrow_State* pInstance = new CFinalBoss_Arrow_State();
	return pInstance;
}

void CFinalBoss_Arrow_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region THRUST STATE
//*********************************
//			 THRUST STATE
//*********************************
CFinalBoss_Thrust_State::CFinalBoss_Thrust_State()
{
}

void CFinalBoss_Thrust_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_bItem = false;
	m_fSpeed = 40.f;
	m_fTimeDelta = 0.f;
}

void CFinalBoss_Thrust_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	if (CFinalBoss::FINALBOSS_FLASHTHRUSTREADY == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_FLASHTHRUSTSWINGFINISHLEFT == pFinalBoss->Get_State())
	{
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);
	}
	else if (CFinalBoss::FINALBOSS_FLASHTHRUSTSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_FLASHTHRUST == pFinalBoss->Get_State())
	{
		if (CFinalBoss::FINALBOSS_FLASHTHRUST == pFinalBoss->Get_State())
		{
			if (0.1f < pFinalBoss->Get_AnimRatio() && 0.5f > pFinalBoss->Get_AnimRatio())
			{
				m_fTimeDelta += fTimeDelta;

				if (0.2f > m_fTimeDelta)
				{
					HRESULT hr;

					// 별 아이템 떨굼
					CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
					AbilityItemDesc.fRotateDir = 1.f;												// 별 회전 방향 오른쪽
					AbilityItemDesc.fAngle = 0.f;													// 별의 진행 방향의 각도
					AbilityItemDesc.vDir = pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) * CUtils::Make_RandomFloat(0.2f, 0.7f);							// 별의 진행 방향
					AbilityItemDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);	// 별의 생성 위치
					AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
					hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
					CHECK_FAILED(hr);

					// 별 아이템 떨굼
					AbilityItemDesc.fRotateDir = -1.f;												// 별 회전 방향 오른쪽
					AbilityItemDesc.fAngle = 0.f;													// 별의 진행 방향의 각도
					AbilityItemDesc.vDir = -pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) * CUtils::Make_RandomFloat(0.2f, 0.7f);							// 별의 진행 방향
					AbilityItemDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);	// 별의 생성 위치
					AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
					hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
					CHECK_FAILED(hr);
				}
				//else
				//	m_fTimeDelta = 0.f;
			}
		}

		_float fHeight = pController->Compute_Height(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));

		if (2.f < fHeight)
			m_fSpeed = 0.f;
		else
			m_fSpeed = 40.f;

		pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * m_fSpeed, fTimeDelta);
	}

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_FLASHTHRUSTREADY:
		{
			// 사운드
			m_pGameInstance->PlaySound_Free(L"BossChimera_Jump.wav", 0.5f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTSTART, 50.f, false, true);

			CEffect::FX_DESC FXDesc{};
			FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			FXDesc.vInitScale = { 1.2f, 1.2f, 1.2f };
			//pFinalBoss->Add_Effect("HS_RayArrow test", FXDesc, true);

			CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
			MultiFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			MultiFXDesc.vInitPos = { .4f, 1.f, 11.f };
			MultiFXDesc.vInitScale = { 2.5f, 2.5f, 2.5f };

			pFinalBoss->Add_Effect("HS_FB thrust dash arrow", MultiFXDesc);

		}
		break;
		case CFinalBoss::FINALBOSS_FLASHTHRUSTSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUST, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_FLASHTHRUST:
			if (rand() % 2 == 1)
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTEND, 50.f, false, true);
			else
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTSWINGFINISHLEFT, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_FLASHTHRUSTSWINGFINISHLEFT:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGFINISHLEFT, 30.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_FLASHTHRUSTEND:
			if (rand() % 2 == 0)
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
			else
			{
				// 사운드
				m_pGameInstance->PlaySound_Free(L"BossChimera_ThrustReady.wav", 0.5f);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTREADY, 50.f, false, true);

				//효선아 Thrust
				ThrustCharge(pFinalBoss);
			}
			break;
		}
	}
}

void CFinalBoss_Thrust_State::OnStateExit()
{
}

CFinalBoss_Thrust_State* CFinalBoss_Thrust_State::Create()
{
	CFinalBoss_Thrust_State* pInstance = new CFinalBoss_Thrust_State();
	return pInstance;
}

void CFinalBoss_Thrust_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region LASER STATE
//*********************************
//			 LASER STATE
//*********************************
CFinalBoss_Laser_State::CFinalBoss_Laser_State()
{
}

void CFinalBoss_Laser_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fTimeDelta = 0.f;
}

void CFinalBoss_Laser_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	CLaser* pLaser = static_cast<CLaser*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Laser")));

	if (CFinalBoss::FINALBOSS_DIMENSIONLASERSTART == pFinalBoss->Get_State())
	{
		if (m_bEffectTrigger == true)
		{
			pFinalBoss->Delete_Effect("YW Laser Set Effects B");
			CEffect::FX_DESC FX_Desc{};
			FX_Desc.pSocketMatrix = pFinalBoss->Get_TransformCom()->Get_WorldFloat4x4_Ptr();
			FX_Desc.vInitPos = { 0.f, 2.f, 0.f };
			FX_Desc.vInitScale = { 1.f, 1.f, 1.f };
			pFinalBoss->Add_Effect("YW FB Laser Charge Complete Effect", FX_Desc, false);
			m_bEffectTrigger = false;
		}


		if (pFinalBoss->IsAnimFinished())
		{
			HRESULT hr;

			CEffect::FX_DESC FX_Desc{};
			FX_Desc.pSocketMatrix = pFinalBoss->Get_TransformCom()->Get_WorldFloat4x4_Ptr();
			FX_Desc.vInitPos = { 0.f, 2.f, 0.f };
			FX_Desc.vInitScale = { 1.f, 1.f, 1.f };
			pFinalBoss->Add_Effect("HS_FB general charge", FX_Desc, true);

			CLaser::LASER_DESC LaserDesc = {};
			LaserDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Laser"), TEXT("Prototype_GameObject_Laser"), &LaserDesc);
			CHECK_FAILED(hr);
		}

		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta);
	}
	else if (CFinalBoss::FINALBOSS_DIMENSIONLASER == pFinalBoss->Get_State())
	{
		m_fTimeDelta += fTimeDelta;
		if (7.f < m_fTimeDelta)
		{
			pLaser->Set_EndLaser(true);
			pFinalBoss->Delete_Effect("HS_FB general charge");
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONLASEREND, 50.f, false, true);
		}
	}

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_DIMENSIONLASEREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONLASERCHARGE, 50.f, false, true);

			//효선아 여기야
			LaserReady(pFinalBoss);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONLASERCHARGE:
			pFinalBoss->Delete_AllEffect();
			m_pGameInstance->PlayMySound(L"BossChimera_LaserCharge.wav", CHANNEL_SOUND12, 0.5f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONLASERSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONLASERSTART:
			// 사운드
			m_pGameInstance->StopSound(CHANNEL_SOUND12);
			m_pGameInstance->PlayMySound(L"BossChimera_LaserAttack.wav", CHANNEL_SOUND12, 0.5f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONLASER, 50.f, true, true);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONLASEREND:
			m_pGameInstance->StopSound(CHANNEL_SOUND12);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Laser_State::OnStateExit()
{
}

CFinalBoss_Laser_State* CFinalBoss_Laser_State::Create()
{
	CFinalBoss_Laser_State* pInstance = new CFinalBoss_Laser_State();
	return pInstance;
}

void CFinalBoss_Laser_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region SPIKE STATE
//*********************************
//			 SPIKE STATE
//*********************************
CFinalBoss_Spike_State::CFinalBoss_Spike_State()
{
}

void CFinalBoss_Spike_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_iCnt = 0;
	m_fTimeDelta = 0.f;
}

void CFinalBoss_Spike_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vRight = pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
	_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	if (CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADYWAIT == pFinalBoss->Get_State())
	{
		m_fTimeDelta += fTimeDelta;
		m_fLifeTime += fTimeDelta;
		if (m_iCnt < 5)
		{
			if (0.2f < m_fTimeDelta)
			{
				m_fTimeDelta = 0.f;

				HRESULT hr;

				_float3 vInitPos = _float3{};

				CDimensionGate::DIMENSIONGATE_DESC DimensionGateDesc = {};
				vInitPos = DimensionGateDesc.vPosition = vPos + (vLook * m_arrLook[m_iCnt] * 1.5f) + (vRight * m_arrRight[m_iCnt] * 1.5f);
				DimensionGateDesc.fScale = 0.05f;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Gate"), TEXT("Prototype_GameObject_DimensionGate"), &DimensionGateDesc);
				CHECK_FAILED(hr);

				++m_iCnt;

				//효선아 여기야

				CMultiEffect::MULTI_FX_DESC FXDesc{};
				vInitPos.y -= 1.f;
				FXDesc.vInitPos = vInitPos;
				FXDesc.vInitScale = { 3.f, 3.f, 3.f };
				FXDesc.vInitRot = { 0.f, 0.f, 0.f };
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"),
					TEXT("Prototype_GameObject_HS_FB downward light"), &FXDesc);
				CHECK_FAILED(hr);


			}
		}

		m_fLifeTime += fTimeDelta;
		if (3.f < m_fTimeDelta)
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKEEND, 50.f, false, true);
	}
	//else if (CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADYWAIT == pFinalBoss->Get_State())
	//{
	//	m_fTimeDelta += fTimeDelta;
	//	if(3.f < m_fTimeDelta)
	//		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKEEND, 50.f, true, true);
	//}
	//else

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADYWAIT, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONSPIKEREADYWAIT:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKESTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONSPIKESTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKE, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONSPIKE:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DIMENSIONSPIKEEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_DIMENSIONSPIKEEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Spike_State::OnStateExit()
{
}

CFinalBoss_Spike_State* CFinalBoss_Spike_State::Create()
{
	CFinalBoss_Spike_State* pInstance = new CFinalBoss_Spike_State();
	return pInstance;
}

void CFinalBoss_Spike_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region JUMP STATE
//*********************************
//			 JUMP STATE
//*********************************
CFinalBoss_Jump_State::CFinalBoss_Jump_State()
{
}

void CFinalBoss_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (CFinalBoss::FINALBOSS_JUMPREADY != pFinalBoss->Get_State())
	{
		_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_UP)) * fTimeDelta * 30.f, fTimeDelta);
	}

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_JUMPREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_JUMPSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_JUMPSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_JUMPEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_JUMPEND:
			if (CFinalBoss::STATE_2PAZE == pFinalBoss->Get_BossState())
			{
				pController->Set_Position(pTransformCom, pFinalBoss->Get_RallyPoint()[2]);
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			}
			else
			{
				pController->Set_Position(pTransformCom, XMVectorSet(-0.115f, 40.f, -0.218f, 1.f));
				pTransformCom->Look_At(XMVectorSet(0.f, 0.f, -1.f, 1.f));
				pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RECOVERYSTART, 50.f, false, true);
			}
			break;
		}
	}
}

void CFinalBoss_Jump_State::OnStateExit()
{
}

CFinalBoss_Jump_State* CFinalBoss_Jump_State::Create()
{
	CFinalBoss_Jump_State* pInstance = new CFinalBoss_Jump_State();
	return pInstance;
}

void CFinalBoss_Jump_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region METEOR STATE
//*********************************
//			 METEOR STATE
//*********************************
CFinalBoss_Meteor_State::CFinalBoss_Meteor_State()
{
}

void CFinalBoss_Meteor_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Meteor_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vRight = pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
	_vector vUp = pTransformCom->Get_State_Vector(CTransform::STATE_UP);
	_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	if (true == pFinalBoss->IsAnimFinished())
	{
		CMeteor::METEOR_DESC MeteorDesc = {};

		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_SUMMONSTART:
			HRESULT hr;
			if (true == pFinalBoss->Get_Chain())
			{
				pFinalBoss->Set_Chain(false);
				MeteorDesc.vPosition = vPos + (vLook * (_float)CUtils::Make_RandomInt(-80, -70))/* + (vRight * (_float)CUtils::Make_RandomInt(-5, 5))*/ + (vUp * (_float)CUtils::Make_RandomInt(30, 45));
				MeteorDesc.fDelayTime = 2.f;
				MeteorDesc.strTag = TEXT("Prototype_Component_Model_BigDebris");
				MeteorDesc.bBig = true;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Meteor"), TEXT("Prototype_GameObject_Meteor"), &MeteorDesc);
				CHECK_FAILED(hr);
			}
			else
			{
				MeteorDesc.vPosition = vPos + (vLook * (_float)CUtils::Make_RandomInt(-70, -50)) + (vRight * (_float)CUtils::Make_RandomInt(-25, -15)) + (vUp * (_float)CUtils::Make_RandomInt(25, 40));
				MeteorDesc.fDelayTime = 1.5f;
				MeteorDesc.strTag = TEXT("Prototype_Component_Model_SmallDebris");
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Meteor"), TEXT("Prototype_GameObject_Meteor"), &MeteorDesc);
				CHECK_FAILED(hr);
				MeteorDesc.vPosition = vPos + (vLook * (_float)CUtils::Make_RandomInt(-70, -60)) + (vRight * (_float)CUtils::Make_RandomInt(-5, 5)) + (vUp * (_float)CUtils::Make_RandomInt(30, 45));
				MeteorDesc.fDelayTime = 2.f;
				MeteorDesc.strTag = TEXT("Prototype_Component_Model_SmallDebris");
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Meteor"), TEXT("Prototype_GameObject_Meteor"), &MeteorDesc);
				CHECK_FAILED(hr);
				MeteorDesc.vPosition = vPos + (vLook * (_float)CUtils::Make_RandomInt(-70, -50)) + (vRight * (_float)CUtils::Make_RandomInt(15, 25)) + (vUp * (_float)CUtils::Make_RandomInt(20, 40));
				MeteorDesc.fDelayTime = 2.5f;
				MeteorDesc.strTag = TEXT("Prototype_Component_Model_SmallDebris");
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Meteor"), TEXT("Prototype_GameObject_Meteor"), &MeteorDesc);
				CHECK_FAILED(hr);
			}
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONWAIT, 30.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SUMMONWAIT:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMON, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SUMMON:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SUMMONEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Meteor_State::OnStateExit()
{
}

CFinalBoss_Meteor_State* CFinalBoss_Meteor_State::Create()
{
	CFinalBoss_Meteor_State* pInstance = new CFinalBoss_Meteor_State();
	return pInstance;
}

void CFinalBoss_Meteor_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region ROAR STATE
//*********************************
//			 ROAR STATE
//*********************************
CFinalBoss_Roar_State::CFinalBoss_Roar_State()
{
}

void CFinalBoss_Roar_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_bShake = false;
}

void CFinalBoss_Roar_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransform = pFinalBoss->Get_TransformCom();

	if (0.3f < pFinalBoss->Get_AnimRatio())
	{
		if (false == m_bShake)
		{
			m_bShake = true;

			// 사운드 처리
			m_pGameInstance->PlaySound_Free(L"BossChimera_Roar.wav", 0.5f);

			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			if (pCamera != nullptr)
				pCamera->Make_Shake(0.5f, 3.5f);
		}
	}

	if (true == pFinalBoss->IsAnimFinished())
	{
		HRESULT hr;
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_ROAR:
			CDimensionGate::DIMENSIONGATE_DESC DimensionGateDesc = {};
			_vector vPos = pTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vLook = pTransform->Get_State_Vector(CTransform::STATE_LOOK);
			DimensionGateDesc.vPosition = vPos - vLook * 80.f + XMVectorSet(0.f, 1.f, 0.f, 0.f) * 40.f;
			DimensionGateDesc.fScale = 0.4f;
			DimensionGateDesc.bSwitch = true;
			DimensionGateDesc.bCamera = true;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Gate"), TEXT("Prototype_GameObject_DimensionGate"), &DimensionGateDesc);
			CHECK_FAILED(hr);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Roar_State::OnStateExit()
{
}

CFinalBoss_Roar_State* CFinalBoss_Roar_State::Create()
{
	CFinalBoss_Roar_State* pInstance = new CFinalBoss_Roar_State();
	return pInstance;
}

void CFinalBoss_Roar_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CFinalBoss_Damage_State::CFinalBoss_Damage_State()
{
}

void CFinalBoss_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fSpeed = 5.f;
}

void CFinalBoss_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (0.f < m_fSpeed)
	{
		m_fSpeed -= fTimeDelta * 3.f;
		pController->Move_Dir(pTransformCom, -XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * m_fSpeed, fTimeDelta);
	}
	else
		m_fSpeed = 0.f;

	if (pFinalBoss->IsAnimFinished())
	{
		if (CFinalBoss::STATE_2PAZE == pFinalBoss->Get_BossState())
		{
			m_pGameInstance->PlaySound_Free(L"BossChimera_Jump.wav", 0.5f);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_JUMPREADY, 50.f, false, true);
		}
		else
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
	}
}

void CFinalBoss_Damage_State::OnStateExit()
{
}

CFinalBoss_Damage_State* CFinalBoss_Damage_State::Create()
{
	CFinalBoss_Damage_State* pInstance = new CFinalBoss_Damage_State();
	return pInstance;
}

void CFinalBoss_Damage_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region RECOVERY STATE
//*********************************
//			 RECOVERY STATE
//*********************************
CFinalBoss_Recovery_State::CFinalBoss_Recovery_State()
{
}

void CFinalBoss_Recovery_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_bMove = false;
	m_fSpeed = 1.f;
	m_fSecondSpeed = 1.f;
	m_fItemCycle = 0.f;
	m_fItemTime = 0.f;
}

void CFinalBoss_Recovery_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (CFinalBoss::FINALBOSS_RECOVERYSTART == pFinalBoss->Get_State())
	{
		_float fDeceleration = m_fSpeed * m_fSpeed;

		if (0.45f < pFinalBoss->Get_AnimRatio())
		{
			if (0.f < m_fSpeed)
				m_fSpeed -= fTimeDelta;
			else
				m_fSpeed = 0.f;
		}

		_vector vUp = pTransformCom->Get_State_Vector(CTransform::STATE_UP);
		pController->Move_Dir(pTransformCom, XMVector3Normalize(-vUp) * fTimeDelta * fDeceleration * 20.f, fTimeDelta);
	}
	else if (CFinalBoss::FINALBOSS_RECOVERYWAIT == pFinalBoss->Get_State())
	{
		if (true == m_bStart)
		{
			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vTargetPos = pFinalBoss->Get_RallyPoint()[5];

			_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vTargetPos)));
			if (false == m_bMove)
			{
				if (15.f < fDistance)
				{
					if (0.f < m_fSpeed)
						m_fSpeed -= fTimeDelta * 1.2f;
					else
						m_fSpeed = 0.f;

					vPos += XMVector3Normalize(vTargetPos - vPos) * fTimeDelta * 22.f * m_fSpeed;
					vPos.m128_f32[1] = pFinalBoss->Get_Position().m128_f32[1];
					pController->Move(pTransformCom, vPos, fTimeDelta);
				}
				else
				{
					m_fDelayTime += fTimeDelta;
					if (0.1f < m_fDelayTime)
						m_bMove = true;
				}
			}
			else if (true == m_bMove)
			{
				if (0.5f < fDistance)
				{
					if (0.f < m_fSecondSpeed)
						m_fSecondSpeed -= fTimeDelta * 0.8f;
					else
					{
						m_fSecondSpeed = 0.f;

						m_fItemCycle += fTimeDelta;

						if (3.f > m_fItemCycle)
						{
							m_fItemTime += fTimeDelta;
							if (1.f < m_fItemTime)
							{
								m_fItemTime = 0.f;
								_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
								vPos.m128_f32[1] += 1.f;
								for (_uint i = 0; i < 3; ++i)
								{
									HRESULT hr;
									// 별 아이템 떨굼
									CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
									AbilityItemDesc.fRotateDir = 1.f;																	// 별 회전 방향 오른쪽															// 별 회전 방향 왼쪽
									AbilityItemDesc.fAngle = 15.f * ((_float)i - 1.f);													// 별의 진행 방향의 각도
									AbilityItemDesc.vDir = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 2.f;							// 별의 진행 방향
									AbilityItemDesc.vPosition = vPos;	// 별의 생성 위치
									AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
									hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
									CHECK_FAILED(hr);
								}
							}
						}
						else
						{
							if (7.f < m_fItemCycle)
								m_fItemCycle = 0.f;
						}
					}

					vPos += XMVector3Normalize(vTargetPos - vPos) * fTimeDelta * 22.f * m_fSecondSpeed;
					vPos.m128_f32[1] += 1.f;
					pController->Move(pTransformCom, vPos, fTimeDelta);
				}
			}
		}
	}

	if (true == pFinalBoss->IsAnimFinished())
	{
		CBossClone::BOSSCLONE_DESC BossCloneDesc = {};

		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_RECOVERYSTART:
			pFinalBoss->Set_BeforeHp(pFinalBoss->Get_Hp());
			HRESULT hr;
			pFinalBoss->Set_Position(pTransformCom->Get_State(CTransform::STATE_POSITION));
			BossCloneDesc.vPosition = pTransformCom->Get_State(CTransform::STATE_POSITION);
			BossCloneDesc.vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			BossCloneDesc.vTargetPos = pFinalBoss->Get_RallyPoint()[3];
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_BossMonster"), TEXT("Prototype_GameObject_BossClone"), &BossCloneDesc);
			CHECK_FAILED(hr);
			BossCloneDesc.vTargetPos = pFinalBoss->Get_RallyPoint()[4];
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_BossMonster"), TEXT("Prototype_GameObject_BossClone"), &BossCloneDesc);
			CHECK_FAILED(hr);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RECOVERYWAIT, 50.f, true, true);
			break;
		case CFinalBoss::FINALBOSS_RECOVERYWAIT:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RECOVERYEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_RECOVERYEND:
			//pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SUMMONSTART, 50.f, false, true);
			break;
		}
	}
}

void CFinalBoss_Recovery_State::OnStateExit()
{
	m_bStart = true;
}

CFinalBoss_Recovery_State* CFinalBoss_Recovery_State::Create()
{
	CFinalBoss_Recovery_State* pInstance = new CFinalBoss_Recovery_State();
	return pInstance;
}

void CFinalBoss_Recovery_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region LASTDAMAGE STATE
//*********************************
//			 LASTDAMAGE STATE
//********************************* 
CFinalBoss_LastDamage_State::CFinalBoss_LastDamage_State()
{
}

void CFinalBoss_LastDamage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_LastDamage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (true == pFinalBoss->IsAnimFinished())
	{
		CBossClone::BOSSCLONE_DESC BossCloneDesc = {};
		CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());

		switch (pFinalBoss->Get_State())
		{
			//피 다 깎여서 엎드림
		case CFinalBoss::FINALBOSS_LASTDAMAGESTART:

			if (pCamera != nullptr)
				pCamera->Make_Sequence(CCamera_Main::SEQ_FINALBOSS_DEAD);

			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_LASTDAMAGEWAIT, 50.f, false, false);
			break;
			//일어서서 빤쓰
		case CFinalBoss::FINALBOSS_LASTDAMAGEWAIT:

			if (pCamera != nullptr)
				pCamera->Make_Sequence(CCamera_Main::SEQ_FINALBOSS_ENDING);

			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DEMODISAPPEARCUT2, 50.f, false, false);
			break;
			//커비 애니메이션 시작
		case CFinalBoss::FINALBOSS_DEMODISAPPEARCUT2:

			if (pCamera != nullptr)
				pCamera->Make_Sequence(CCamera_Main::SEQ_FINALBOSS_DUMP);

			CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
			pKirby->Get_KirbyInfo()->m_bFinalBossDead = true;
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_DEMODISAPPEARCUT3, 50.f, false, false);
			break;
		}
	}
}

void CFinalBoss_LastDamage_State::OnStateExit()
{
}

CFinalBoss_LastDamage_State* CFinalBoss_LastDamage_State::Create()
{
	CFinalBoss_LastDamage_State* pInstance = new CFinalBoss_LastDamage_State();
	return pInstance;
}

void CFinalBoss_LastDamage_State::Free()
{
	__super::Free();
}
#pragma endregion
