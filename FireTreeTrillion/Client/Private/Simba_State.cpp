#include "stdafx.h"
#include "Simba_State.h"
#include "Simba.h"
#include "Kirby.h"
#include "EventCenter.h"
#include "Camera_Main.h"

static _float s_fOffsetY = {};
static _uint s_iAttackCount = {};
static _float s_fJumpPower = {};
static _uint s_iWalkSmokeCount = {};
static _uint s_iBiteCount = {};

// *********************** Appear1 ***********************  // 사운드 완료
void CSimba_Appear1::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = 0.f;
	m_bPlaySound = false;
}

void CSimba_Appear1::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	if (CSimba::Simba_DemoAppear1Cut10 == iState && 0.24f < fAnimRatio && false == m_bPlaySound) {
		m_bPlaySound = true;
		m_pGameInstance->StopSound(CHANNEL_BOSSVOICE);
		m_pGameInstance->PlayMySound(L"SimbaLastDialog.wav", CHANNEL_BOSSVOICE, 0.3f);
	}

	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_DemoAppear1Cut2:
			pSimba->Change_State(CSimba::Simba_DemoAppear1Cut2Wait, 66.66f, true, false);
			break;
		case CSimba::Simba_DemoAppear1Cut3:
			pSimba->Change_State(CSimba::Simba_DemoAppear1Cut3Wait, 66.66f, true, false);
			break;
		case CSimba::Simba_DemoAppear1Cut4:
			pSimba->Change_State(CSimba::Simba_DemoAppear1Cut4Wait, 66.66f, true, false);
			break;
		case CSimba::Simba_DemoAppear1Cut9:
			pSimba->Change_State(CSimba::Simba_DemoAppear1Cut9Wait, 66.66f, true, false);
			break;
		case CSimba::Simba_DemoAppear1Cut10:
			pSimba->Change_State(CSimba::Simba_DemoAppear1Cut10Wait, 66.66f, true, false);
			break;
		}
	}
}

// *********************** Appear2 *********************** // 사운드 완료
void CSimba_Appear2::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fTime = 0.f; 
	m_bPlaySound = false;
	_uint iCurAnim = _pModel->Get_CurAnimIndex();
	if (CSimba::Simba_DemoAppear2Cut1 == iCurAnim)
		s_fOffsetY = -0.3f;
	else if(CSimba::Simba_DemoAppear2Cut2 == iCurAnim)
		s_fOffsetY = -0.5f;
}

void CSimba_Appear2::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();

	_float fAnimRatio = pSimba->Get_AnimRatio();
	if (CSimba::Simba_DemoAppear2Cut1 == iState)
	{
		if(fAnimRatio > 0.9f)
			s_fOffsetY = -0.5f;
		else if(fAnimRatio > 0.85f)
			s_fOffsetY = -0.45f;
		else if(fAnimRatio > 0.8f)
			s_fOffsetY = -0.4f;
		else if(fAnimRatio > 0.75f)
			s_fOffsetY = -0.35f;

		if (0.081f < fAnimRatio && false == m_bPlaySound)
		{
			m_bPlaySound = true;
			m_pGameInstance->StopSound(CHANNEL_BGM);
			m_pGameInstance->PlayBGM(L"SimbaBattleStart.wav", 0.3f);
		}
	}

	if (CSimba::Simba_DemoAppear2Cut2 == iState)
	{
		if (0.8f < fAnimRatio && false == m_bPlaySound) {
			m_bPlaySound = true;
			m_pGameInstance->StopSound(CHANNEL_BOSSVOICE);
			m_pGameInstance->PlayMySound(L"SimbaName.wav", CHANNEL_BOSSVOICE, 0.33f);
		}
	}
		
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_DemoAppear2Cut1:
			pSimba->Change_State(CSimba::Simba_DemoAppear2Cut2, 66.66f, false, false);
			break;
		case CSimba::Simba_DemoAppear2Cut2:
			if (true == pSimba->Get_RenderMant())
				pSimba->Set_RenderMant(false);
			m_fTime += fTimeDelta;

			if (m_fTime > 1.5f)
			{
				pSimba->Change_State(CSimba::Simba_Walk, 66.66f, true, false);
				//카메라에게 2번째 타겟으로 등록
				pSimba->SetUpSecondTarget();

				_vector vPos = m_pTransform->Get_State(CTransform::STATE_POSITION);
				_vector vLook = XMVector3Normalize(m_pTransform->Get_State(CTransform::STATE_LOOK));
				_vector vNewPos = vPos - vLook * 16.f;
				m_pController->Set_Position(m_pTransform, vNewPos);
				pSimba->CreateHpBar();
			}
			break;
		}
	}
}

// *********************** Walk *********************** // 사운드 완료
void CSimba_Walk::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.58f;
}

void CSimba_Walk::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_float fAnimRatio = pSimba->Get_AnimRatio();
	if (0.285f > fAnimRatio && 2 <= s_iWalkSmokeCount)
		s_iWalkSmokeCount = 0;

	if (0.285f < fAnimRatio && 0 == s_iWalkSmokeCount)
	{
		pSimba->WalkSmoke();
		m_pGameInstance->PlaySound_Free(L"SimbaWalk.wav", 0.23f);
		s_iWalkSmokeCount++;
	}
	else if (0.76f < fAnimRatio && 1 == s_iWalkSmokeCount) {
		pSimba->WalkSmoke();
		m_pGameInstance->PlaySound_Free(L"SimbaWalk.wav", 0.23f);
		s_iWalkSmokeCount++;
	}

	_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pGameObject);
	if (fDis < 6.5f)
	{
		//pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 60, false, true); // 디버깅용
		
		if (0 == CUtils::Make_RandomInt(0, 1))
			pSimba->Change_State(CSimba::Simba_QuickClawStartL, 55.f, false, true);
		else
			pSimba->Change_State(CSimba::Simba_QuickClawStartR, 55.f, false, true);
	}
	else
	{
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 7.2f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);
	}
}

// *********************** QuickClaw *********************** // 완료
void CSimba_QuickClaw::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.58f;
	if (CSimba::Simba_QuickClawL == _iAnimIndex || CSimba::Simba_QuickClawR == _iAnimIndex
		|| CSimba::Simba_QuickClaw2L == _iAnimIndex || CSimba::Simba_QuickClaw2R == _iAnimIndex) {
		m_bSlashEffect = false;
		m_bPlaySound = false;
	}
		
	else if (CSimba::Simba_QuickClawLFromStart == _iAnimIndex || CSimba::Simba_QuickClawRFromStart == _iAnimIndex)
		m_bChargeEffect = false;
	else if (CSimba::Simba_QuickClawStartL == _iAnimIndex || CSimba::Simba_QuickClawStartR == _iAnimIndex)
		m_bNailEffect = false;
}

void CSimba_QuickClaw::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_vector vLook = m_pTransform->Get_State(CTransform::STATE_LOOK);

	_uint iStarCount = pSimba->Get_StarCount();
	_uint iRockCount = pSimba->Get_RockCount();
	
	if ((CSimba::Simba_QuickClawStartL == iState || CSimba::Simba_QuickClawStartR == iState)) {
		if(0.3f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 6.f);

		if (0.82f < fAnimRatio && false == m_bNailEffect)
		{
			m_bNailEffect = true;
			pSimba->QuickClawNailFlash(iState);
			m_pGameInstance->PlaySound_Free(L"SimbaNail.wav", 0.4f);
		}
	}

	else if ((CSimba::Simba_QuickClawL == iState || CSimba::Simba_QuickClawR == iState))
	{
		if (0.25f < fAnimRatio && false == m_bPlaySound) {
			m_bPlaySound = true;
			m_pGameInstance->PlaySound_Free(L"SimbaQuickClawVoice1.wav", 0.32f);
		}
		
		if (0.4f < fAnimRatio && false == m_bSlashEffect) {
			m_bSlashEffect = true;
			pSimba->QuickClawSlash(iState);
			m_pGameInstance->PlaySound_Free(L"SimbaQuickClaw.wav", 0.32f);
		}
			
		_float fStart = 0.42f;
		_float fEnd = 0.64f;
		_float fSpeed = 6.2f;

		if(fStart - 0.02f < fAnimRatio && (fEnd + fStart) * 2.f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.f);
		if (fStart < fAnimRatio && fEnd > fAnimRatio)
		{
			_float fRatio = RATIO(fAnimRatio, fStart, fEnd);
			_vector vDir = vLook * (1 - EASE_IN_FAST(fRatio)) * fSpeed * fTimeDelta * m_pGameInstance->Compute_Distance(pSimba, m_pKirby);
			m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
		}

		if (0.46f < fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);

		if (0.4f < fAnimRatio && 0 == iRockCount) {
			pSimba->SpawnRocks(iState);
			pSimba->SpawnDebris(iState);
		}
		else if (0.5f < fAnimRatio && 1 == iRockCount)
			pSimba->SpawnRocks(iState);
		else if (0.6f < fAnimRatio && 2 == iRockCount)
			pSimba->SpawnRocks(iState);
	}
	
	else if ((CSimba::Simba_QuickClaw2L == iState || CSimba::Simba_QuickClaw2R == iState)) 
	{
		if (0.18f < fAnimRatio && false == m_bPlaySound)
		{
			m_bPlaySound = true;
			m_pGameInstance->PlaySound_Free(L"SimbaQuickClawVoice2.wav", 0.35f);
		}

		if (0.3f < fAnimRatio && false == m_bSlashEffect) {
			m_bSlashEffect = true;
			pSimba->QuickClawSlash(iState);
			m_pGameInstance->PlaySound_Free(L"SimbaQuickClaw.wav", 0.35f);
		}

		if(0.3f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);
		if (0.34f < fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);

		if (0.32f < fAnimRatio && 0 == iRockCount) {
			pSimba->SpawnRocks(iState);
			pSimba->SpawnDebris(iState);
		}
		else if (0.41f < fAnimRatio && 1 == iRockCount)
			pSimba->SpawnRocks(iState);
		else if (0.5f < fAnimRatio && 2 == iRockCount)
			pSimba->SpawnRocks(iState);

		_float fStart = 0.27f;
		_float fEnd = 0.3958f;
		_float fSpeed = 6.8f;
		if (fStart < fAnimRatio && fEnd > fAnimRatio)
		{
			_float fRatio = RATIO(fAnimRatio, fStart, fEnd);
			_vector vDir = vLook * (1 - EASE_IN_FAST(fRatio)) * fSpeed * fTimeDelta * m_pGameInstance->Compute_Distance(pSimba, m_pKirby);
			m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
		}
	}
		
	else if (CSimba::Simba_QuickClawLFromStart == iState || CSimba::Simba_QuickClawRFromStart == iState)
	{
		if(0.8f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 12.f);

		if (0.48f < fAnimRatio && false == m_bChargeEffect) {
			m_bChargeEffect = true;
			pSimba->FinalCrusherCharge();
		}
	}
		
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_QuickClawStartL:
			pSimba->Change_State(CSimba::Simba_QuickClawL, 55.f, false, false);
			break;
		case CSimba::Simba_QuickClawStartR:
			pSimba->Change_State(CSimba::Simba_QuickClawR, 55.f, false, false);
			break;

		//case CSimba::Simba_QuickClawChargeL:
		//	pSimba->Change_State(CSimba::Simba_QuickClawL, 66.66f, false, false);
		//	break;
		//case CSimba::Simba_QuickClawChargeR:
		//	pSimba->Change_State(CSimba::Simba_QuickClawR, 66.66f, false, false);
		//	break;

		case CSimba::Simba_QuickClawL:
			pSimba->Change_State(CSimba::Simba_QuickClaw2R, 55.f, false, false);
			break;
		case CSimba::Simba_QuickClawR:
			pSimba->Change_State(CSimba::Simba_QuickClaw2L, 55.f, false, false);
			break;

		case CSimba::Simba_QuickClaw2L:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 18.f)
				pSimba->Change_State(CSimba::Simba_QuickClawEndL, 55.f, false, false);
			else
				pSimba->Change_State(CSimba::Simba_QuickClawLFromStart, 55.f, false, false);
			break;
		case CSimba::Simba_QuickClaw2R:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 18.f)
				pSimba->Change_State(CSimba::Simba_QuickClawEndR, 55.f, false, false);
			else
				pSimba->Change_State(CSimba::Simba_QuickClawRFromStart, 55.f, false, false);
			break;

		case CSimba::Simba_QuickClawEndL: case CSimba::Simba_QuickClawEndR:
			pSimba->Set_PreState(iState);
			pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
			break;

		case CSimba::Simba_QuickClawLFromStart: case CSimba::Simba_QuickClawRFromStart:
			pSimba->Change_State(CSimba::Simba_FinalCrusher, 55.f, false, false);
			break;
		}
	}
}

// *********************** FinalCrusher *********************** // 완료
void CSimba_FinalCrusher::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
	m_bSmashEffect = false;
	m_bSwingEffect = false;
	if(CSimba::Simba_FinalCrusherStart == _iAnimIndex)
		m_bChargeEffect = false;
}

void CSimba_FinalCrusher::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_uint iStarCount = pSimba->Get_StarCount();
	_uint iRockCount = pSimba->Get_RockCount();
	_uint iDebrisCount = pSimba->Get_DebrisCount();

	if (CSimba::Simba_FinalCrusherStart == iState)
	{
		if(0.3f > fAnimRatio || 0.93f < fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);

		if (0.48f < fAnimRatio && false == m_bChargeEffect) {
			m_bChargeEffect = true;
			pSimba->FinalCrusherCharge();
		}
	}
		
	else if (CSimba::Simba_FinalCrusher == iState)
	{
		if(0.07f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 9.f);

		if (0.125f < fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);

		if (0.105f < fAnimRatio && 0 == iRockCount) {
			pSimba->SpawnRocks(iState);
			pSimba->SpawnDebris(iState);
			pSimba->FinalCrusherRing();
		}
		else if (0.12f < fAnimRatio && 1 == iRockCount)
			pSimba->SpawnRocks(iState);
		else if(0.135f < fAnimRatio && 2 == iRockCount)
			pSimba->SpawnRocks(iState);

		if (0.08f < fAnimRatio && false == m_bSwingEffect) {
			m_bSwingEffect = true;
			pSimba->FinalCrusherSwing();
		}
		//else if (0.105f < fAnimRatio && false == m_bSmashEffect)
		else if (0.12f < fAnimRatio && false == m_bSmashEffect)
		{
			m_bSmashEffect = true;
			pSimba->FinalCrusherSmash();
		}

		if(0.12f < fAnimRatio)
			pSimba->CheckFinalCrusherRingCollision(fTimeDelta);
	}
		
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_FinalCrusher:

			pSimba->Change_State(CSimba::Simba_FinalCrusherEnd, 60.f, false, false);
			break;
		case CSimba::Simba_FinalCrusherEnd:
			//pSimba->Change_State(CSimba::Simba_FinalCrusherStart, 60, false, true); // 디버깅용

			pSimba->Set_PreState(iState);

			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				
				if(0 == CUtils::Make_RandomInt(0, 1))
					pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
				else
					pSimba->Change_State(CSimba::Simba_AttackJumpPre, 50.f, false, true); // 점프공격
			}
			else
				pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 50.f, false, true);

			break;
		case CSimba::Simba_FinalCrusherStart: // After Jump
			pSimba->Change_State(CSimba::Simba_FinalCrusher, 50.f, false, false);
			break;
		}
	}
}

// *********************** DoubleClaw *********************** // 완료
void CSimba_DoubleClaw::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
}

void CSimba_DoubleClaw::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_uint iStarCount = pSimba->Get_StarCount();
	_uint iRockCount = pSimba->Get_RockCount();
	_uint iDebrisCount = pSimba->Get_DebrisCount();

	if (CSimba::Simba_DoubleClawChargeStart == iState && fAnimRatio < 0.18f) {
		_float fRatio = RATIO(fAnimRatio, 0, 0.18f);
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 7.f * (1 - EASE_IN_FAST(fRatio)));
	}
		
	else if (CSimba::Simba_DoubleClawDash == iState)
	{
		if (0.f <= fAnimRatio && 5 <= pSimba->Get_DebrisCount())
			pSimba->ResetDebrisCount();

		for (_uint i = 0; i < 3; i++) {
			if (i * 0.33f < fAnimRatio && i == iRockCount) {
				pSimba->SpawnDebris(iState);
				pSimba->DoubleClawDashGround();
			}
		}

		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.8f);
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 13.5f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);

		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pGameObject);
		if (fDis < 8.5f)
			pSimba->Change_State(CSimba::Simba_DoubleClaw, 35.f, false, false);
	}
	else if (CSimba::Simba_DoubleClaw == iState)
	{
		_uint iFireCount = pSimba->Get_FireCount();

		if (0.2f > fAnimRatio) {
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);

			for (_uint i = 0; i < 5; i++) {
				if (i * 0.04f < fAnimRatio && i == iDebrisCount)
					pSimba->SpawnDebris(iState);
			}
		}
			
		for (_uint i = 0; i < 45; i++)
		{
			if (i * 0.006f < fAnimRatio && i == iRockCount)
				pSimba->SpawnRocks(iState);
		}

		if(0.2f < fAnimRatio && 5 == iDebrisCount)
			pSimba->SpawnDebris(iState);

		if (0.224f < fAnimRatio && 0 == iStarCount) {
			pSimba->SpawnStar(iState);
			pSimba->DoubleClawSweep();
			pSimba->DoubleClawGround();
		}

		if (0.15f < fAnimRatio && 0.51f > fAnimRatio)
			pSimba->SpawnFire(iState);
	}
		
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_DoubleClawChargeStart:
			pSimba->Change_State(CSimba::Simba_DoubleClawChargeWait, 50.f, false, false);
			break;
		case CSimba::Simba_DoubleClawChargeWait:
			pSimba->Change_State(CSimba::Simba_DoubleClawDashStart, 50.f, false, false);
			break;
		case CSimba::Simba_DoubleClawDashStart:
			pSimba->Change_State(CSimba::Simba_DoubleClawDash, 50.f, true, false);
			break;
		case CSimba::Simba_DoubleClaw:
			pSimba->Change_State(CSimba::Simba_DoubleClawEnd, 50.f, false, false);
			break;
		case CSimba::Simba_DoubleClawEnd:
			//pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 60, false, true); // 디버깅용

			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Set_PreState(iState);
				pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true); // 점프공격
			}
			else
			{
				_uint iRandNum = CUtils::Make_RandomInt(0, 1);
				if(0 == iRandNum)
					pSimba->Change_State(CSimba::Simba_QuickClawStartL, 60.f, false, true);
				else
					pSimba->Change_State(CSimba::Simba_QuickClawStartR, 60.f, false, true);
			}
			break;
		}
	}
}

// *********************** Jump *********************** // 완료
void CSimba_Jump::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
	if(CSimba::Simba_JumpStart == _iAnimIndex)
		s_fJumpPower = 54.f;
	m_bSmokeEffect = false;
}

void CSimba_Jump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if (CSimba::Simba_Wait2 == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
		
		if (0.5f > fAnimRatio) {
			_float fRatio = RATIO(fAnimRatio, 0, 0.5f);
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 8.f * (1 - EASE_IN_FAST(fRatio)));
		}
			
		if (0.3f < fAnimRatio)
			pSimba->Change_State(CSimba::Simba_JumpStart, 40.f, false, true);
			
	}
	else if (CSimba::Simba_JumpStart == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	
		if (true == pSimba->IsAnimFinished()) {
			pSimba->Change_State(CSimba::Simba_Jump, 40.f, false, false);
			pSimba->JumpStartSmoke();
		}
			
	}
	else if (CSimba::Simba_Jump == iState)
	{
		_float fRatio = RATIO(s_fJumpPower, 0, 54.f);
		_float fFallVelocity = (1 - EASE_IN_FAST(fRatio)) * s_fJumpPower;
;		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 16.5f;
	
		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pSimba);
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * fDis * 1.7f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);

		if (0.5f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.8f);

		if (true == pSimba->IsAnimFinished())
			pSimba->Change_State(CSimba::Simba_Fall, 40.f, true, false);
	}
	else if (CSimba::Simba_Fall == iState)
	{
		_float fRatio = RATIO(s_fJumpPower, 0, 54.f);
		_float fFallVelocity = (1 - EASE_IN_FAST(fRatio)) * s_fJumpPower;
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 20.f;

		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pSimba);
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * fDis * 1.81f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);

		_float fChangeDis = 1.3f;
		if(fChangeDis > m_pController->RayCastToStaticActor(XMVectorSet(0, -1, 0, 0)))
			pSimba->Change_State(CSimba::Simba_Landing, 40.f, false, false);
	}
	else if (CSimba::Simba_Landing == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
		
		if (0.12f < fAnimRatio && false == m_bSmokeEffect) {
			m_bSmokeEffect = true;
			pSimba->LandingSmoke();
		}

		if (true == pSimba->IsAnimFinished())
		{
			//pSimba->Change_State(CSimba::Simba_JumpStart, 40.f, false, true); // 디버깅용

			_uint iPreState = pSimba->Get_PreState();

			if (CSimba::Simba_DoubleClawEnd == iPreState)
				pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true);				
			else
			{
				_int iRandNum = CUtils::Make_RandomInt(0, 2);
				if (m_pGameInstance->Compute_Distance(m_pKirby, pGameObject) > 15.f)
				{
					if(0 == iRandNum)
						pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
					else if(1 == iRandNum)
						pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true);
					else if (2 == iRandNum) {
						_uint iRandNum2 = CUtils::Make_RandomInt(0, 1);
						if(0 == iRandNum2)
							pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
						else
							pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);
					}
				}
				else
				{
					if (CSimba::Simba_QuickClawEndL == iPreState || CSimba::Simba_QuickClawEndR == iPreState)
						pSimba->Change_State(CSimba::Simba_FinalCrusherStart, 66.66f, false, true);
					else if (CSimba::Simba_FinalCrusherEnd == iPreState)
						pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 50.f, false, true);
					else if (CSimba::Simba_AttackJumpHit == iPreState)
					{
						if (0 == iRandNum)
							pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
						else
							pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);
					}
				}
			}
		}
	}
}

// *********************** BackStep *********************** // 버림
void CSimba_BackStep::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
}

void CSimba_BackStep::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if(CSimba::Simba_BackStepStart == iState && fAnimRatio < 0.5f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);

	if (CSimba::Simba_BackStep == iState)
		; // JumpParabola
	else
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_BackStepStart == iState)
			pSimba->Change_State(CSimba::Simba_BackStep, 70.f, false, false);
		else if (CSimba::Simba_BackStep == iState)
			pSimba->Change_State(CSimba::Simba_BackStepEnd, 70.f, false, false);
		else if (CSimba::Simba_BackStepEnd == iState)
			pSimba->Change_State(CSimba::Simba_BackStepLanding, 70.f, false, false);
		else if (CSimba::Simba_BackStepLanding == iState)
		{
			pSimba->Change_State(CSimba::Simba_BackStepStart, 70.f, false, true); // 점프공격
			//pSimba->Turn_RotationBoneMatrix(AttackJump);
			//pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true); // 점프공격
		}
	}
}

// *********************** AttackJump *********************** // 완료
void CSimba_AttackJump::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
	if (CSimba::Simba_AttackJump == _iAnimIndex)
		s_fJumpPower = 15.f;
	m_bWindEffect = false;
}

void CSimba_AttackJump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_uint iStarCount = pSimba->Get_StarCount();
	_uint iRockCount = pSimba->Get_RockCount();

	if (CSimba::Simba_AttackJumpPre == iState && fAnimRatio < 0.4f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.f);

	if (CSimba::Simba_AttackJump == iState) {

		if (0.22f < fAnimRatio && false == m_bWindEffect) {
			m_bWindEffect = true;
			pSimba->AttackJumpWind();
		}

		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 9.f;

		_vector vLook = m_pTransform->Get_State(CTransform::STATE_LOOK);
		_vector vSubtract = XMVector3Normalize(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION) - m_pTransform->Get_State_Vector(CTransform::STATE_POSITION));

		vLook = XMVector3Normalize(vLook + vSubtract * 1.5f);

		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pSimba);

		_float fSpeed = 2.2f;
		if (0.13f > fAnimRatio)
			fSpeed = 0.1f;
		if (0.6f < fAnimRatio)
			fSpeed = 1.5f;
		if (0.8f < fAnimRatio)
			fSpeed = 1.f;

		_vector vDir = vLook * (1 - EASE_IN(fAnimRatio)) * fSpeed * fTimeDelta * fDis;
		m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
	}
	else
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	if (CSimba::Simba_AttackJumpHit == iState)
	{
		if(0.12f < fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);

		if (0.06f < fAnimRatio && 0 == iRockCount) { // 왼손
			pSimba->AttackJumpHit();
			pSimba->SpawnRocks(iState);
			pSimba->SpawnDebris(iState);
		}
		else if (0.1f < fAnimRatio && 1 == iRockCount) { // 오른손
			pSimba->AttackJumpHit();
			pSimba->SpawnRocks(iState);
			pSimba->SpawnDebris(iState);
		}
	}
		
	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_AttackJumpPre == iState)
			pSimba->Change_State(CSimba::Simba_AttackJumpStart, 60.f, false, false);
		else if (CSimba::Simba_AttackJumpStart == iState)
			pSimba->Change_State(CSimba::Simba_AttackJump, 60.f, false, true);
			
		else if (CSimba::Simba_AttackJump == iState)
			pSimba->Change_State(CSimba::Simba_AttackJumpHit, 60.f, false, false);
		else if (CSimba::Simba_AttackJumpWait == iState)
			pSimba->Change_State(CSimba::Simba_AttackJumpHit, 60.f, false, false);
		else if (CSimba::Simba_AttackJumpHit == iState)
		{
			//pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, false); // 디버깅용

			_int iRandNum = CUtils::Make_RandomInt(0, 2);
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Set_PreState(iState);
				if (0 == iRandNum)
					pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
				else if(1 == iRandNum)
					pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, false);
				else {
					_uint iRandNum2 = CUtils::Make_RandomInt(0, 1);
					if (0 == iRandNum2)
						pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
					else
						pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);
				}
			}
			else
			{
				_uint iPreState = pSimba->Get_PreState();
				if (CSimba::Simba_QuickClawEndL == iPreState || CSimba::Simba_QuickClawEndR == iPreState)
					pSimba->Change_State(CSimba::Simba_FinalCrusherStart, 66.66f, false, true);
				else if (CSimba::Simba_FinalCrusherEnd == iPreState)
					pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 50.f, false, true);
				else if(CSimba::Simba_DoubleClaw == iPreState)
					pSimba->Change_State(CSimba::Simba_AttackJumpPre, 50.f, false, true);
				else if (CSimba::Simba_AttackJumpHit == iPreState)
				{
					if (0 == iRandNum)
						pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
					else
						pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);
				}
			}
		}
	}
}

// *********************** Damage *********************** // 완료
void CSimba_Damage::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
}

void CSimba_Damage::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	if (pSimba->IsAnimFinished()) {
		
		// HS 카메라 심바앞으로 가면서 심바 바라보기

		pSimba->Change_State(CSimba::Simba_Roar2, 50.f, false, false);
	}
}

// *********************** Roar *********************** // 완료
void CSimba_Roar::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
	m_bCamNotified = false;
	m_bEyeBloom = false;
	m_bElecParts = false;
}

void CSimba_Roar::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_float fRatio = RATIO(fAnimRatio, 0, 0.08f);
	if (0.06f > fAnimRatio)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1- EASE_IN(fRatio)) * 4.f);

	if (0.285f < fAnimRatio && false == m_bElecParts) // 열정적인 전기
	{
		m_bElecParts = true;
		pSimba->RoarElecParts(); 
	}
	
	if (0.54f < fAnimRatio && false == m_bEyeBloom) // 잔잔한 전기 + 눈 빛남
	{
		m_bEyeBloom = true;

		pSimba->Set_EyeBloom(true);
		m_pGameInstance->Update_RimLight(.5f, 3.f, { 1.f, .7f, 1.f });
		//효선아 여기야 카메라 찾아와

		pSimba->Set_SimbaEye(CSimba::SIMBAEYE_NONE);
	}

	if (0.895f < fAnimRatio && false == m_bCamNotified) 
	{
		m_bCamNotified = true;

		//HS 카메라 다시 심바,커비 타겟 두명으로
	}

	if (pSimba->IsAnimFinished())
	{
		pSimba->Set_PreState(CSimba::Simba_Roar2);
		if (true == pSimba->IsKirbyOnMyLeft())
			pSimba->Change_State(CSimba::Simba_BiteRushJumpStartL, 50.f, false, true);
		else
			pSimba->Change_State(CSimba::Simba_BiteRushJumpStartR, 50.f, false, true);
	}
}

// *********************** BiteRushJump *********************** // 완료
void CSimba_BiteRushJump::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	if (CSimba::Simba_BiteRushJumpL == _iAnimIndex || CSimba::Simba_BiteRushJumpR == _iAnimIndex)
		s_fJumpPower = 30.f;
}

void CSimba_BiteRushJump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	
	if (CSimba::Simba_BiteRushJumpL == iState || CSimba::Simba_BiteRushJumpR == iState)
	{
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 14.f;
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1 - EASE_IN(fAnimRatio)) * 6.5f);

		_float fSpeed = 28.f;
		_vector vRight = m_pTransform->Get_State_Vector(CTransform::STATE_RIGHT);
		_vector vDir = vRight * (1 - EASE_IN(fAnimRatio)) * fSpeed * fTimeDelta;
		
		if (CSimba::Simba_BiteRushJumpL == iState)
			vDir = -vDir;
		m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
	}
	else if (CSimba::Simba_BiteRushFallL == iState || CSimba::Simba_BiteRushFallR == iState) {
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 27.f;
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1 - EASE_IN(fAnimRatio)) * 6.5f);
		_float fSpeed = 3.f;
		_vector vRight = m_pTransform->Get_State_Vector(CTransform::STATE_RIGHT);
		_vector vDir = vRight * fSpeed * fTimeDelta;
		if (CSimba::Simba_BiteRushFallL == iState)
			vDir = -vDir;
		m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
	}
	else if (CSimba::Simba_BiteRushLandingL == iState || CSimba::Simba_BiteRushLandingR == iState) {
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 38.f;
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

		_uint iSmokeCount = pSimba->Get_SmokeCount();

		if (0.08f < fAnimRatio && 0 == iSmokeCount)
			pSimba->BiteRushJumpSmoke(iState);
		else if (0.115f < fAnimRatio && 1 == iSmokeCount)
			pSimba->BiteRushJumpSmoke(iState);
		else if (0.18f < fAnimRatio && 2 == iSmokeCount)
			pSimba->BiteRushJumpSmoke(iState);
	}
	else
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	_float fHeight = { FLT_MAX };
	if (CSimba::Simba_BiteRushFallL == iState || CSimba::Simba_BiteRushFallR == iState)
		fHeight = m_pController->RayCastToStaticActor(XMVectorSet(0, -1, 0, 0));

	if (1.f > fHeight)
	{
		if(CSimba::Simba_BiteRushFallL == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushLandingL, 60.f, false, false);
		else if (CSimba::Simba_BiteRushFallR == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushLandingR, 60.f, false, false);
	}
		
	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_BiteRushJumpStartL == iState) {
			pSimba->BiteRushJumpSmoke(iState);
			pSimba->Change_State(CSimba::Simba_BiteRushJumpL, 50.f, false, false);
		}
		else if (CSimba::Simba_BiteRushJumpStartR == iState) {
			pSimba->BiteRushJumpSmoke(iState);
			pSimba->Change_State(CSimba::Simba_BiteRushJumpR, 50.f, false, false);
		}
		else if (CSimba::Simba_BiteRushJumpL == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushFallL, 60.f, true, false);
		else if (CSimba::Simba_BiteRushJumpR == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushFallR, 60.f, true, false);

		else if (CSimba::Simba_BiteRushLandingL == iState || CSimba::Simba_BiteRushLandingR == iState) {
			//if (true == pSimba->IsKirbyOnMyLeft())
			//	pSimba->Change_State(CSimba::Simba_BiteRushJumpStartL, 50.f, false, false);// 디버깅용
			//else
			//	pSimba->Change_State(CSimba::Simba_BiteRushJumpStartR, 50.f, false, false);// 디버깅용

			_uint iPreState = pSimba->Get_PreState();

			if (CSimba::Simba_Roar2 == iPreState)
				pSimba->Change_State(CSimba::Simba_DimensionClawStart, 60.f, false, true);
			else if(CSimba::Simba_DimensionClawEnd == iPreState)
				pSimba->Change_State(CSimba::Simba_BiteRushStart, 50.f, false, true);
			else if (CSimba::Simba_DimensionLaserEnd == iPreState)
				pSimba->Change_State(CSimba::Simba_DimensionClawStart, 60.f, false, true);
		}
	}
}

// *********************** DimensionClaw *********************** // 완료
void CSimba_DimensionClaw::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	if (CSimba::Simba_DimensionClawStart == _iAnimIndex) {
		m_bClawFire = false;
		s_iAttackCount = 0;
	}	
	else if (CSimba::Simba_DimensionClaw == _iAnimIndex || CSimba::Simba_DimensionClawContinue == _iAnimIndex)
		m_bSetDimensionClawMatrix = false;
	else if(CSimba::Simba_DimensionClawStartContinue)
		m_bClawFire = false;
}

void CSimba_DimensionClaw::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_uint iStarCount = pSimba->Get_StarCount();

	if ((CSimba::Simba_DimensionClawStart == iState)) {
		if (0.87f < fAnimRatio) {
			_float fRatio = RATIO(fAnimRatio, 0.87f, 1);
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1 - EASE_IN(fRatio)) * 9.5f);
		}

		if (0.5f < fAnimRatio /*&& false == m_bClawFire*/) {
			//m_bClawFire = true;
			pSimba->DimensionClawFire();
		}
	}

	if (CSimba::Simba_DimensionClawStartContinue == iState) {
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta /** (1 - EASE_IN_FAST(fAnimRatio))*/ * 3.f);

		pSimba->DimensionClawFire();
	}
		
	if (CSimba::Simba_DimensionClaw == iState)
	{
		if (false == m_bSetDimensionClawMatrix)
		{
			m_bSetDimensionClawMatrix = true;
			pSimba->SetUpDimensionClawWorldMatrix();
		}

		if (0.32f < fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);
		if (0.38f < fAnimRatio && 1 == iStarCount)
			pSimba->SpawnStar(iState);
	}

	if (CSimba::Simba_DimensionClawContinue == iState)
	{
		if (false == m_bSetDimensionClawMatrix)
		{
			m_bSetDimensionClawMatrix = true;
			pSimba->SetUpDimensionClawWorldMatrix();
		}

		if (0.53f < fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);
		if (0.63f < fAnimRatio && 1 == iStarCount)
			pSimba->SpawnStar(iState);
	}

	if (CSimba::Simba_DimensionClaw == iState || CSimba::Simba_DimensionClawContinue == iState ||
		CSimba::Simba_DimensionClawEnd == iState || CSimba::Simba_DimensionClawStartContinue == iState)
		pSimba->Set_DimensionGateActivation(true);

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_DimensionClawStart == iState)
			pSimba->Change_State(CSimba::Simba_DimensionClaw, 60.f, false, false);
		
		else if (CSimba::Simba_DimensionClaw == iState) {
			pSimba->Change_State(CSimba::Simba_DimensionClawStartContinue, 60.f, false, false);
			s_iAttackCount++;
			pSimba->ChangeDimensionClawUpDown();
			//pSimba->HideDimensionClawActor();
		}

		else if (CSimba::Simba_DimensionClawStartContinue == iState)
			pSimba->Change_State(CSimba::Simba_DimensionClawContinue, 60.f, false, false);

		else if (CSimba::Simba_DimensionClawContinue == iState)
		{
			pSimba->ChangeDimensionClawUpDown();
			//pSimba->HideDimensionClawActor();

			if (2 > s_iAttackCount) {
				pSimba->Change_State(CSimba::Simba_DimensionClawStartContinue, 60.f, false, false);
				s_iAttackCount++;
			}
			else
				pSimba->Change_State(CSimba::Simba_DimensionClawEnd, 60.f, false, false);
		}

		else if (CSimba::Simba_DimensionClawEnd == iState)
		{
			//pSimba->Change_State(CSimba::Simba_DimensionClawStart, 50.f, false, true); // 디버깅용
			
			pSimba->Set_PreState(iState);
			if (true == pSimba->IsKirbyOnMyLeft())
				pSimba->Change_State(CSimba::Simba_BiteRushJumpStartL, 50.f, false, true);
			else
				pSimba->Change_State(CSimba::Simba_BiteRushJumpStartR, 50.f, false, true);
		}	
	}
}

// *********************** BiteRush *********************** // 완료
void CSimba_BiteRush::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	if (CSimba::Simba_BiteRush == _iAnimIndex)
		m_fTime = 0.f;
}

void CSimba_BiteRush::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_uint iStarCount = pSimba->Get_StarCount();

	if (CSimba::Simba_BiteRush == iState)
	{
		m_fTime += fTimeDelta;

		_vector vLook = m_pTransform->Get_State(CTransform::STATE_LOOK);
		_vector vLook2 = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), 30.f);
		_vector vLook3 = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), -30.f);

		_vector vDir = XMVector3Normalize(vLook) * fTimeDelta * 12.f;
		m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.6f);

		_float fDis = 8.f;
		if (fDis > m_pController->RayCastToStaticActor(vLook) || fDis > m_pController->RayCastToStaticActor(vLook2)
			|| fDis > m_pController->RayCastToStaticActor(vLook3) || 2.8f < m_fTime)
			pSimba->Change_State(CSimba::Simba_BiteRushTiredStart, 50.f, false, false);

		if (2 <= pSimba->Get_StarCount())
			pSimba->ResetStarCount();
		else if (0.15f < fAnimRatio && 0.7f > fAnimRatio && 0 == iStarCount) {
			pSimba->Set_StarPosToLeftHand();
			s_iBiteCount = 0;
			pSimba->TeethBite(s_iBiteCount);
			pSimba->SpawnStar(iState);
		}
		else if (0.65f < fAnimRatio && 0 == s_iBiteCount) {
			s_iBiteCount++;
			pSimba->TeethBite(s_iBiteCount);
		}
		else if (0.7f < fAnimRatio && 1 == iStarCount) {
			pSimba->Set_StarPosToRightHand();
			pSimba->SpawnStar(iState);
		}
	}

	if(CSimba::Simba_BiteRushTiredEnd == iState && 0.8f < fAnimRatio)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_BiteRushStart == iState || CSimba::Simba_BiteRushStartStraight == iState)
			pSimba->Change_State(CSimba::Simba_BiteRush, 50.f, true, false);
		else if (CSimba::Simba_BiteRushEnd == iState)
			//pSimba->Change_State(CSimba::Simba_BiteRushStart, 50.f, false, false); // 디버깅용
			pSimba->Change_State(CSimba::Simba_DimensionLaserStart, 50.f, false, false);

		else if (CSimba::Simba_BiteRushTiredStart == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushTiredEnd, 50.f, false, false);	
		else if (CSimba::Simba_BiteRushTiredEnd == iState)
			//pSimba->Change_State(CSimba::Simba_BiteRushStart, 50.f, false, false); // 디버깅용
			pSimba->Change_State(CSimba::Simba_DimensionLaserStart, 50.f, false, false);
	}
}

// *********************** DimensionLaser *********************** // 완료
void CSimba_DimensionLaser::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_bLaserActivated = false;
}

void CSimba_DimensionLaser::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_uint iStarCount = pSimba->Get_StarCount();

	if (CSimba::Simba_DimensionLaserStart == iState)
	{
		if(0.95f < fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.5f);

		pSimba->DimensionLaserVomit();
	}
		
	if (CSimba::Simba_DimensionLaser == iState)
	{
		if (0.1f < fAnimRatio && 0.55f > fAnimRatio) {
			pSimba->LaserAttack(fTimeDelta);

			if (false == m_bLaserActivated)
			{
				m_bLaserActivated = true;
				pSimba->DimensionLaser();
			}
		}
			

		_uint iDebrisCount = pSimba->Get_DebrisCount();
		for (_uint i = 0; i < 35; i++)
		{
			if (i * 0.006f + 0.1f < fAnimRatio && i == iDebrisCount)
				pSimba->SpawnDebris(iState);
		}

		if(0.05f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.f);
		else if(0.33f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.f);

		if (0.185f < fAnimRatio && 0.225f > fAnimRatio && 0 == iStarCount)
			pSimba->SpawnStar(iState);

		else if (0.225f < fAnimRatio && 0.265f > fAnimRatio && 1 == iStarCount)
			pSimba->SpawnStar(iState);
		else if (0.265f < fAnimRatio && 0.305f > fAnimRatio && 2 == iStarCount)
			pSimba->SpawnStar(iState);
		else if (0.305f < fAnimRatio && 3 == iStarCount)
			pSimba->SpawnStar(iState);
	}

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_DimensionLaserStart == iState)
			pSimba->Change_State(CSimba::Simba_DimensionLaser, 60.f, false, false);
		else if (CSimba::Simba_DimensionLaser == iState)
			pSimba->Change_State(CSimba::Simba_DimensionLaserEnd, 60.f, false, false);
		else if (CSimba::Simba_DimensionLaserEnd == iState)
		{
			//pSimba->Change_State(CSimba::Simba_DimensionLaserStart, 60.f, false, false); // 디버깅용

			pSimba->Set_PreState(iState);
			if (true == pSimba->IsKirbyOnMyLeft())
				pSimba->Change_State(CSimba::Simba_BiteRushJumpStartL, 50.f, false, true);
			else
				pSimba->Change_State(CSimba::Simba_BiteRushJumpStartR, 50.f, false, true);
		}
	}
}

// *********************** Death *********************** // 완료
void CSimba_Death::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_bCageNotified = false;
}

void CSimba_Death::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	if (CSimba::Simba_DemoDeadCut2 == iState)
	{
		if (0.185f < fAnimRatio)
			pSimba->Set_EyeBloom(false);
			
		if (0.83f < fAnimRatio && false == pSimba->Get_RenderEyeLid())
			pSimba->Set_RenderEyeLid(true);
	}

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_DemoDeadCut1 == iState)
			pSimba->Change_State(CSimba::Simba_DemoDeadCut2, 60.f, false, false);
		else if (CSimba::Simba_DemoDeadCut2 == iState && false == m_bCageNotified) {
			m_bCageNotified = true;
			CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_BOSSORIGIN);
		}
	}
}