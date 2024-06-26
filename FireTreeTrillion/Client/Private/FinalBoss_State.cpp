#include "stdafx.h"
#include "FinalBoss_State.h"
#include "FinalBoss.h"
#include "Kirby.h"

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

	if(pFinalBoss->IsAnimFinished())
		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, false, true);
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
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 3.f, false);

	if (CFinalBoss::FINALBOSS_WAITAIR == pFinalBoss->Get_State())
	{
		// ㄹㅇ랜덤으로 가면 딱일듯
		if (/*pFinalBoss->IsAnimFinished()*/0.2f < pFinalBoss->Get_AnimRatio())
		{
			//pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHREADY, 50.f, false, true);
		//	if (rand() % 4 == 0)
		//	{
		//		// Stab 패턴
		//		m_vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		//		pFinalBoss->Set_Direction(m_vLook);
		//		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_STABREADY, 50.f, false, true);
		//	}
		//	else if (rand() % 4 == 1)
		//	{
		//		// Slash 패턴
		//		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHREADY, 50.f, false, true);
		//	}
		//	else if (rand() % 4 == 2)
		//	{
		//		// 화살 패턴
		//		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADYAIR, 50.f, false, true);
		//	}
		//	else
		//	{
		//		// 좌우 활공 패턴
		//		_vector vBossPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		//		_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

		//		if (rand() % 2 == 0)
		//		{
		//			pFinalBoss->Set_Direction(RotateGlide(vKirbyPos, vBossPos, -45.f));
		//			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNLEFTAIRSTART, 50.f, false, true);
		//		}
		//		else
		//		{
		//			pFinalBoss->Set_Direction(RotateGlide(vKirbyPos, vBossPos, 45.f));
		//			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_TURNRIGHTAIRSTART, 50.f, false, true);
		//		}
		//	}
		}
	}
	else
	{
		// 백스텝 활공 패턴
		//pFinalBoss->Set_Direction(-pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) + XMVectorSet(0.f, 0.3f, 0.f, 0.f));
		//pFinalBoss->Change_State(CFinalBoss::FINALBOSS_AWAYFASTREADY, 50.f, false, true);

		// 스윙 패턴
		//pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SWINGRIGHTSTART, 40.f, false, true);

		// 화살 패턴
		//if(pFinalBoss->IsAnimFinished())
		//	pFinalBoss->Change_State(CFinalBoss::FINALBOSS_RAYARROWREADY, 50.f, false, true);

		// 땅에서 찌르기 패턴
		if(pFinalBoss->IsAnimFinished())
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTREADY, 50.f, false, true);
	}
}

void CFinalBoss_Idle_State::OnStateExit()
{
}

//void CFinalBoss_Idle_State::GlideToRallyPoint(CFinalBoss* pFinalBoss, CTransform* pTransformCom)
//{
//	_float shortestDistance = FLT_MAX;
//	_bool bFindMonster = { false };
//
//	for (size_t i = 0; i < pFinalBoss->Get_RallyPoint().size(); i++)
//	{
		//_vector vBossPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		//_vector vRallyPoint = pFinalBoss->Get_RallyPoint()[i];

		//_vector vDistance = XMVectorSubtract(vBossPos, vRallyPoint);

		//_float fDistance = XMVectorGetX(XMVector3Length(vDistance));
//
//		// 가장 짧은 거리 업데이트
//		if (fDistance < shortestDistance)
//		{
//			shortestDistance = fDistance;
//			m_ShortestPos = vRallyPoint;
//			bFindMonster = true;
//		}
//	}
//
//	// 가장 가까운 몬스터를 찾았다면, 플레이어가 그 몬스터를 바라보도록 함
//	if (bFindMonster)
//	{
//		m_vLook = m_ShortestPos - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
//		pFinalBoss->Set_Look(m_vLook);
//	}
//}

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
}

void CFinalBoss_Stab_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

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
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * 90.f, fTimeDelta);

		if(CFinalBoss::FINALBOSS_STABWAIT == pFinalBoss->Get_State())
		{
			if(pController->Is_Terrain())
			{
				pFinalBoss->Set_BossState(CFinalBoss::STATE_GROUND);
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

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_AWAYFASTREADY:
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

	if (CFinalBoss::FINALBOSS_AWAYFASTSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_AWAYFAST == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_AWAYFASTENDAIR == pFinalBoss->Get_State())
	{
		if(CFinalBoss::FINALBOSS_AWAYFASTENDAIR == pFinalBoss->Get_State())
		{
			if(0.3f < pFinalBoss->Get_AnimRatio())
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

	if(CFinalBoss::FINALBOSS_TURNLEFTAIRSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_TURNRIGHTAIRSTART == pFinalBoss->Get_State())
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
			pFinalBoss->Set_Chain(true);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_SLASHCHAINREADY:
			pFinalBoss->Set_Chain(false);
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

		if(1.f > m_fTimeDelta)
		{
			vPos.m128_f32[0] += m_fTimeDelta;
			vPos.m128_f32[1] += 0.3f * -tan(m_fTimeDelta);
		}

		if(5.f > vPos.m128_f32[1])
		{
			_vector vLook = vKirbyPos - vPos;
			vLook.m128_f32[1] = 0.f;
			pFinalBoss->Set_Direction(vLook);
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASH, 60.f, true, true);
		}

		pController->Move(pTransformCom, vPos, fTimeDelta);
	}
	else if (CFinalBoss::FINALBOSS_SLASH == pFinalBoss->Get_State())
	{
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pFinalBoss->Get_Direction()) * fTimeDelta * 60.f, fTimeDelta);

		m_fTimeDelta += fTimeDelta;
		if (4.f < m_fTimeDelta)
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHEND, 50.f, false, true);
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


#pragma region SLASHEND STATE
//*********************************
//			 SLASHEND STATE
//*********************************
CFinalBoss_SlashEnd_State::CFinalBoss_SlashEnd_State()
{
}

void CFinalBoss_SlashEnd_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fTimeDelta = 0.f;
}

void CFinalBoss_SlashEnd_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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
		vPos.m128_f32[1] += m_fTimeDelta * 2.f;
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

	if(pFinalBoss->IsAnimFinished())
	{
		if(true == pFinalBoss->Get_Chain())
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHCHAINREADY, 50.f, false, true);
		else
		{
			pFinalBoss->Set_Direction(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASHCHAINSTABREADY, 50.f, false, true);
		}
	}
	//if (5.f > vPos.m128_f32[1])
	//{
	//	_vector vLook = vKirbyPos - vPos;
	//	vLook.m128_f32[1] = 0.f;
	//	pFinalBoss->Set_Direction(vLook);
	//	//pFinalBoss->Change_State(CFinalBoss::FINALBOSS_SLASH, 60.f, true, true);
	//}
}

void CFinalBoss_SlashEnd_State::OnStateExit()
{
}

CFinalBoss_SlashEnd_State* CFinalBoss_SlashEnd_State::Create()
{
	CFinalBoss_SlashEnd_State* pInstance = new CFinalBoss_SlashEnd_State();
	return pInstance;
}

void CFinalBoss_SlashEnd_State::Free()
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
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

	}
	else if (CFinalBoss::FINALBOSS_SWINGRIGHT == pFinalBoss->Get_State())
	{
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
		if (0.2f < pFinalBoss->Get_AnimRatio())
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
			pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);
	}
	else if (CFinalBoss::FINALBOSS_SWINGLEFT == pFinalBoss->Get_State())
	{
		if (0.3f < pFinalBoss->Get_AnimRatio())
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
			if(rand() % 2 == 0)
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
}

void CFinalBoss_Thrust_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (true == pFinalBoss->IsAnimFinished())
	{
		switch (pFinalBoss->Get_State())
		{
		case CFinalBoss::FINALBOSS_FLASHTHRUSTREADY:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTSTART, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_FLASHTHRUSTSTART:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUST, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_FLASHTHRUST:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_FLASHTHRUSTEND, 50.f, false, true);
			break;
		case CFinalBoss::FINALBOSS_FLASHTHRUSTEND:
			pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAIT, 50.f, false, true);
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
