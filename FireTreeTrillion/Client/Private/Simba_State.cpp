#include "stdafx.h"
#include "Simba_State.h"
#include "Simba.h"
#include "Kirby.h"
#include "EventCenter.h"
#include "Camera_Main.h"

static _float s_fOffsetY = {};
static _uint s_iAttackCount = {};
static _float s_fJumpPower = {};

// *********************** Appear1 ***********************  // 완료
void CSimba_Appear1::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = 0.f;
}

void CSimba_Appear1::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	if (true == pSimba->IsAnimFinished())
	{
		switch (pSimba->Get_State())
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

// *********************** Appear2 *********************** // 완료
void CSimba_Appear2::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fTime = 0.f; 

	_uint iCurAnim = _pModel->Get_CurAnimIndex();
	if(CSimba::Simba_DemoAppear2Cut1 == iCurAnim)
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
				CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
				if(pCamera!= nullptr)
					pCamera->Set_Target(pSimba->Get_TransformCom(), CCamera::TARGET_SECOND, CCamera::FOCUS_BOTH);

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

// *********************** Walk *********************** // 완료
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
	_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pGameObject);
	if (fDis < 6.5f)
	{
		pSimba->Change_State(CSimba::Simba_DimensionClawStart, 50.f, false, true); // 디버깅용
		/*if (0 == CUtils::Make_RandomInt(0, 1))
			pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
		else
			pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);*/
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
	m_bStarSpawned = false;
}

void CSimba_QuickClaw::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_vector vLook = m_pTransform->Get_State(CTransform::STATE_LOOK);

	if ((CSimba::Simba_QuickClawStartL == iState || CSimba::Simba_QuickClawStartR == iState) && fAnimRatio < 0.35f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 15.f);

	else if ((CSimba::Simba_QuickClawL == iState || CSimba::Simba_QuickClawR == iState))
	{
		_float fStart = 0.42f;
		_float fEnd = 0.64f;
		_float fSpeed = 6.2f;

		if (fStart < fAnimRatio && fEnd > fAnimRatio)
		{
			_float fRatio = RATIO(fAnimRatio, fStart, fEnd);
			_vector vDir = vLook * (1 - EASE_IN_FAST(fRatio)) * fSpeed * fTimeDelta * m_pGameInstance->Compute_Distance(pSimba, m_pKirby);
			m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
		}

		if (0.8f < fAnimRatio && false == m_bStarSpawned)
		{
			m_bStarSpawned = true;
			pSimba->SpawnStar(iState);
		}
	}
	
	else if ((CSimba::Simba_QuickClaw2L == iState || CSimba::Simba_QuickClaw2R == iState)) {
		if(0.3f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);
		if (0.45f < fAnimRatio && false == m_bStarSpawned)
		{
			m_bStarSpawned = true;
			pSimba->SpawnStar(iState);
		}

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
		
	else if((CSimba::Simba_QuickClawLFromStart == iState || CSimba::Simba_QuickClawRFromStart == iState) && fAnimRatio < 0.8f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 12.f);
	
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_QuickClawStartL:
			pSimba->Change_State(CSimba::Simba_QuickClawChargeL, 66.66f, false, false);
			break;
		case CSimba::Simba_QuickClawStartR:
			pSimba->Change_State(CSimba::Simba_QuickClawChargeR, 66.66f, false, false);
			break;

		case CSimba::Simba_QuickClawChargeL:
			pSimba->Change_State(CSimba::Simba_QuickClawL, 66.66f, false, false);
			break;
		case CSimba::Simba_QuickClawChargeR:
			pSimba->Change_State(CSimba::Simba_QuickClawR, 66.66f, false, false);
			break;

		case CSimba::Simba_QuickClawL:
			pSimba->Change_State(CSimba::Simba_QuickClaw2R, 66.66f, false, false);
			break;
		case CSimba::Simba_QuickClawR:
			pSimba->Change_State(CSimba::Simba_QuickClaw2L, 66.66f, false, false);
			break;

		case CSimba::Simba_QuickClaw2L:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 18.f)
				pSimba->Change_State(CSimba::Simba_QuickClawEndL, 66.66f, false, false);
			else
				pSimba->Change_State(CSimba::Simba_QuickClawLFromStart, 66.66f, false, false);
			break;
		case CSimba::Simba_QuickClaw2R:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 18.f)
				pSimba->Change_State(CSimba::Simba_QuickClawEndR, 66.66f, false, false);
			else
				pSimba->Change_State(CSimba::Simba_QuickClawRFromStart, 66.66f, false, false);
			break;

		case CSimba::Simba_QuickClawEndL: case CSimba::Simba_QuickClawEndR:
			pSimba->Set_PreState(iState);
			pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
			break;

		case CSimba::Simba_QuickClawLFromStart: case CSimba::Simba_QuickClawRFromStart:
			pSimba->Change_State(CSimba::Simba_FinalCrusher, 66.66f, false, false);
			break;
		}
	}
}

// *********************** FinalCrusher *********************** // 이펙트 및 퍼지는 원 충돌로직 필요
void CSimba_FinalCrusher::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
}

void CSimba_FinalCrusher::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();

	_float fAnimRatio = pSimba->Get_AnimRatio();
	if(CSimba::Simba_FinalCrusherStart == iState && (0.3f > fAnimRatio || 0.93f < fAnimRatio ))
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);
	else if(CSimba::Simba_FinalCrusher == iState && 0.07f > fAnimRatio )
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 9.f);

	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_FinalCrusher:
			pSimba->Change_State(CSimba::Simba_FinalCrusherEnd, 66.66f, false, false);
			break;
		case CSimba::Simba_FinalCrusherEnd:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Set_PreState(iState);
				//if(0 == CUtils::Make_RandomInt(0, 1))
					pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
				//else
				//	pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true); // 점프공격
			}
			else
				pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 50.f, false, true);
			break;

		case CSimba::Simba_FinalCrusherStart: // After Jump
			pSimba->Change_State(CSimba::Simba_FinalCrusher, 66.66f, false, false);
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

	if (CSimba::Simba_DoubleClawChargeStart == iState && fAnimRatio < 0.2f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);
	else if (CSimba::Simba_DoubleClawDash == iState)
	{
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.8f);
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 13.5f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);

		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pGameObject);
		if (fDis < 8.5f)
			pSimba->Change_State(CSimba::Simba_DoubleClaw, 60.f, false, false);
	}
	else if(CSimba::Simba_DoubleClaw == iState && fAnimRatio < 0.2f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 10.f);

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
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Turn_RotationBoneMatrix(AttackJump);
				pSimba->Set_PreState(iState);
				pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true); // 점프공격
			}
			else
			{
				_uint iRandNum = CUtils::Make_RandomInt(0, 1);
				if(0 == iRandNum)
					pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.f, false, true);
				else
					pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.f, false, true);
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
}

void CSimba_Jump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if (CSimba::Simba_Wait2 == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.f);
		if (0.3f < fAnimRatio)
			pSimba->Change_State(CSimba::Simba_JumpStart, 40.f, false, true);
			
	}
	else if (CSimba::Simba_JumpStart == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	
		if (true == pSimba->IsAnimFinished())
			pSimba->Change_State(CSimba::Simba_Jump, 40.f, false, false);
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
		
		if (true == pSimba->IsAnimFinished())
		{
			//pSimba->Change_State(CSimba::Simba_JumpStart, 40.f, false, true); // 디버깅용

			_uint iPreState = pSimba->Get_PreState();

			if (CSimba::Simba_DoubleClawEnd == iPreState)
			{
				// 거리 멀면 점프공격
				pSimba->Turn_RotationBoneMatrix(AttackJump);
				pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true);
				// 아니면 백스탭
			}
				
			else
			{
				_int iRandNum = CUtils::Make_RandomInt(0, 1);
				if (m_pGameInstance->Compute_Distance(m_pKirby, pGameObject) > 15.f)
				{
					if(0 == iRandNum)
						pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
					else
					{
						pSimba->Turn_RotationBoneMatrix(AttackJump);
						pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, true);
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

// *********************** BackStep *********************** // 이동로직 구현 필요 // 후순위
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
}

void CSimba_AttackJump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if (CSimba::Simba_AttackJumpPre == iState && fAnimRatio < 0.4f)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.f);

	if (CSimba::Simba_AttackJump == iState) {
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 9.f;

		_vector vLook = m_pTransform->Get_State(CTransform::STATE_LOOK);
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

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_AttackJumpPre == iState)
			pSimba->Change_State(CSimba::Simba_AttackJumpStart, 60.f, false, false);
		else if (CSimba::Simba_AttackJumpStart == iState)
		{
			pSimba->Turn_RotationBoneMatrix(-2.f);
			pSimba->Change_State(CSimba::Simba_AttackJump, 60.f, false, true);
		}
			
		else if (CSimba::Simba_AttackJump == iState) {
			pSimba->Turn_RotationBoneMatrix(0.f);

			// 거리 조건문 (근데 안해도 될거같음)
			pSimba->Change_State(CSimba::Simba_AttackJumpHit, 60.f, false, false);
			//pSimba->Change_State(CSimba::Simba_AttackJumpWait, 60.f, false, false);
		}
		else if (CSimba::Simba_AttackJumpWait == iState)
			pSimba->Change_State(CSimba::Simba_AttackJumpHit, 60.f, false, false);
		else if (CSimba::Simba_AttackJumpHit == iState)
		{
			//pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, false); // 디버깅용

			_int iRandNum = CUtils::Make_RandomInt(0, 1);
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Set_PreState(iState);
				if (0 == iRandNum) {
					pSimba->Turn_RotationBoneMatrix(0.f);
					pSimba->Change_State(CSimba::Simba_Wait2, 40.f, false, true);
				}
				else
					pSimba->Change_State(CSimba::Simba_AttackJumpPre, 60.f, false, false);
			}
			else
			{
				pSimba->Turn_RotationBoneMatrix(0.f);
				if (0 == iRandNum)
					pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
				else
					pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);
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
		pSimba->Turn_RotationBoneMatrix(0.f);
		pSimba->Change_State(CSimba::Simba_Roar2, 50.f, false, false);
	}
}

// *********************** Roar *********************** // 완료
void CSimba_Roar::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
}

void CSimba_Roar::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_float fAnimRatio = pSimba->Get_AnimRatio();
	_float fRatio = RATIO(fAnimRatio, 0, 0.08f);
	if (0.06f > fAnimRatio)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1- EASE_IN(fRatio)) * 4.f);
	
	if (pSimba->IsAnimFinished())
	{
		pSimba->Set_PreState(CSimba::Simba_Roar2);
		pSimba->Turn_RotationBoneMatrix(BiteRushJump);
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
		s_fJumpPower = 23.5f;
}

void CSimba_BiteRushJump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();
	
	if ((CSimba::Simba_BiteRushJumpStartL == iState || CSimba::Simba_BiteRushJumpStartR == iState) && 0.27f > fAnimRatio)
	{
		_float fRatio = RATIO(fAnimRatio, 0, 0.27f);
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1 - EASE_IN_FAST(fRatio)) * 6.f);
	}

	if (CSimba::Simba_BiteRushJumpL == iState || CSimba::Simba_BiteRushJumpR == iState
		/*|| CSimba::Simba_BiteRushFallL == iState || CSimba::Simba_BiteRushFallR == iState*/)
	{
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 16.f;
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta /** (1 - EASE_IN_FAST(fRatio))*/ * 2.6f);

		_float fSpeed = 28.f;
		_vector vRight = m_pTransform->Get_State_Vector(CTransform::STATE_RIGHT);
		_vector vDir = vRight * (1 - EASE_IN(fAnimRatio)) * fSpeed * fTimeDelta;
		
		if (CSimba::Simba_BiteRushJumpL == iState)
			vDir = -vDir;
		m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
	}
	else if (CSimba::Simba_BiteRushLandingL == iState || CSimba::Simba_BiteRushLandingR == iState) {
		m_pController->Jump(m_pTransform, s_fJumpPower, fTimeDelta);
		s_fJumpPower -= GRAVITY * fTimeDelta * 20.f;
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	}
	else
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	/*if (CSimba::Simba_BiteRushFallL == iState && 0.2f < fAnimRatio) {
		
		pSimba->Change_State(CSimba::Simba_BiteRushLandingL, 50.f, false, true);
	}
		
	else if (CSimba::Simba_BiteRushFallR == iState && 0.2f < fAnimRatio)
	{
		pSimba->Change_State(CSimba::Simba_BiteRushLandingR, 50.f, false, true);
	}*/
		
	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_BiteRushJumpStartL == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushJumpL, 50.f, false, false);
		else if (CSimba::Simba_BiteRushJumpStartR == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushJumpR, 50.f, false, false);

		else if (CSimba::Simba_BiteRushJumpL == iState) {
			//pSimba->Turn_RotationBoneMatrix(-15.f);
			pSimba->Change_State(CSimba::Simba_BiteRushLandingL, 60.f, false, false);
		}
			
		else if (CSimba::Simba_BiteRushJumpR == iState) {
			//pSimba->Turn_RotationBoneMatrix(-15.f);
			pSimba->Change_State(CSimba::Simba_BiteRushLandingR, 60.f, false, false);
		}
			
		else if (CSimba::Simba_BiteRushLandingL == iState || CSimba::Simba_BiteRushLandingR == iState) {
			////pSimba->Turn_RotationBoneMatrix(BiteRushJump);
			//if (true == pSimba->IsKirbyOnMyLeft())
			//	pSimba->Change_State(CSimba::Simba_BiteRushJumpStartL, 50.f, false, false);// 디버깅용
			//else
			//	pSimba->Change_State(CSimba::Simba_BiteRushJumpStartR, 50.f, false, false);// 디버깅용

			_uint iPreState = pSimba->Get_PreState();
			pSimba->Turn_RotationBoneMatrix(0);

			if (CSimba::Simba_Roar2 == iPreState)
				pSimba->Change_State(CSimba::Simba_DimensionClawStart, 60.f, false, true);
			else if(CSimba::Simba_DimensionClawEnd == iPreState)
			{
				pSimba->Turn_RotationBoneMatrix(BiteRush);
				pSimba->Change_State(CSimba::Simba_BiteRushStart, 50.f, false, true);
				//pSimba->Change_State(CSimba::Simba_BiteRushStartStraight, 50.f, false, true);
			}
			else if (CSimba::Simba_DimensionLaserEnd == iPreState)
				pSimba->Change_State(CSimba::Simba_DimensionClawStart, 60.f, false, true);
		}
	}
}

// *********************** DimensionClaw *********************** // 이펙트, 충돌 필요
void CSimba_DimensionClaw::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	if (CSimba::Simba_DimensionClawStart == _iAnimIndex)
		s_iAttackCount = 0;
}

void CSimba_DimensionClaw::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if ((CSimba::Simba_DimensionClawStart == iState) && 0.87f < fAnimRatio) {
		_float fRatio = RATIO(fAnimRatio, 0.87f, 1);
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * (1 - EASE_IN(fRatio)) * 9.5f);
	}

	if (CSimba::Simba_DimensionClawStartContinue == iState)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta /** (1 - EASE_IN_FAST(fAnimRatio))*/ * 3.f);

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_DimensionClawStart == iState)
			pSimba->Change_State(CSimba::Simba_DimensionClaw, 60.f, false, false);
		
		else if (CSimba::Simba_DimensionClaw == iState) {
			pSimba->Change_State(CSimba::Simba_DimensionClawStartContinue, 60.f, false, false);
			s_iAttackCount++;
		}

		else if (CSimba::Simba_DimensionClawStartContinue == iState) {
			pSimba->Change_State(CSimba::Simba_DimensionClawContinue, 60.f, false, false);
			s_iAttackCount++;
		}

		else if (CSimba::Simba_DimensionClawContinue == iState)
		{
			if(3 > s_iAttackCount)
				pSimba->Change_State(CSimba::Simba_DimensionClawStartContinue, 60.f, false, false);
			else
				pSimba->Change_State(CSimba::Simba_DimensionClawEnd, 60.f, false, false);
		}

		else if (CSimba::Simba_DimensionClawEnd == iState)
		{
			pSimba->Change_State(CSimba::Simba_DimensionClawStart, 50.f, false, true);
			
			/*pSimba->Set_PreState(iState);
			pSimba->Turn_RotationBoneMatrix(BiteRushJump);
			if (true == pSimba->IsKirbyOnMyLeft())
				pSimba->Change_State(CSimba::Simba_BiteRushJumpStartL, 50.f, false, true);
			else
				pSimba->Change_State(CSimba::Simba_BiteRushJumpStartR, 50.f, false, true);*/
		}	
	}
}

// *********************** BiteRush *********************** // 종료조건 업글필요
void CSimba_BiteRush::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CSimba_BiteRush::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if (CSimba::Simba_BiteRush == iState)
	{
		_vector vLook = m_pTransform->Get_State(CTransform::STATE_LOOK);
		_vector vLook2 = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), 30.f);
		_vector vLook3 = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), -30.f);

		_vector vDir = XMVector3Normalize(vLook) * fTimeDelta * 12.f;
		m_pController->Move_Dir(m_pTransform, vDir, fTimeDelta, s_fOffsetY);
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.6f);

		_float fDis = 8.f;
		if (fDis > m_pController->RayCastToStaticActor(vLook) || fDis > m_pController->RayCastToStaticActor(vLook2)
			|| fDis > m_pController->RayCastToStaticActor(vLook3))
			pSimba->Change_State(CSimba::Simba_BiteRushTiredStart, 50.f, false, false);
	}

	if(CSimba::Simba_BiteRushTiredEnd == iState && 0.8f < fAnimRatio)
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_BiteRushStart == iState || CSimba::Simba_BiteRushStartStraight == iState)
			pSimba->Change_State(CSimba::Simba_BiteRush, 50.f, true, false);
		else if (CSimba::Simba_BiteRushEnd == iState)
			pSimba->Change_State(CSimba::Simba_DimensionLaserStart, 50.f, false, false);

		else if (CSimba::Simba_BiteRushTiredStart == iState)
			pSimba->Change_State(CSimba::Simba_BiteRushTiredEnd, 50.f, false, false);	
		else if (CSimba::Simba_BiteRushTiredEnd == iState)
			pSimba->Change_State(CSimba::Simba_DimensionLaserStart, 50.f, false, false);
	}
}

// *********************** DimensionLaser *********************** // 이펙트, 충돌 필요
void CSimba_DimensionLaser::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CSimba_DimensionLaser::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	_float fAnimRatio = pSimba->Get_AnimRatio();

	if (CSimba::Simba_DimensionLaserStart == iState)
	{
		_float fMin = 0.f;
		_float fMax  = 0.2f;
		if (fMax > fAnimRatio)
		{
			_float fRatio = RATIO(fAnimRatio, fMin, fMax);
			_float fStart = BiteRush;
			fRatio = EASE_OUT(fRatio);
			_float fAngle = LERP(fStart, 0, fRatio);
			pSimba->Turn_RotationBoneMatrix(fAngle);
		}
		else
			pSimba->Turn_RotationBoneMatrix(0);

		if(0.95f < fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.5f);
	}
		
	if (CSimba::Simba_DimensionLaser == iState)
	{
		if(0.05f > fAnimRatio)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 5.f);
		else
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 0.5f);
	}

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_DimensionLaserStart == iState)
			pSimba->Change_State(CSimba::Simba_DimensionLaser, 60.f, false, false);
		else if (CSimba::Simba_DimensionLaser == iState)
			pSimba->Change_State(CSimba::Simba_DimensionLaserEnd, 60.f, false, false);
		else if (CSimba::Simba_DimensionLaserEnd == iState)
		{
			pSimba->Set_PreState(iState);
			pSimba->Turn_RotationBoneMatrix(BiteRushJump);
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
}

void CSimba_Death::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	if (CSimba::Simba_DemoDeadCut2 == iState && 0.83f < pSimba->Get_AnimRatio())
	{
		if (false == pSimba->Get_RenderEyeLid())
			pSimba->Set_RenderEyeLid(true);
	}

	if (pSimba->IsAnimFinished())
	{
		if (CSimba::Simba_Death == iState) {
			pSimba->Change_State(CSimba::Simba_DemoDeadCut1, 50.f, false, true);
			CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_THRONEBREAK);
		}
		else if (CSimba::Simba_DemoDeadCut1 == iState)
			pSimba->Change_State(CSimba::Simba_DemoDeadCut2, 60.f, false, false);
		else if (CSimba::Simba_DemoDeadCut2 == iState && false == m_bCageNotified) {
			m_bCageNotified = true;
			CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_CAGEBREAK);
		}
	}
}