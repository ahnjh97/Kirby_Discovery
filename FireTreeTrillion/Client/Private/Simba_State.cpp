#include "stdafx.h"
#include "Simba_State.h"
#include "Simba.h"
#include "Kirby.h"

static _float s_fOffsetY = {};

// *********************** APPEAR1 ***********************
void CSimba_Appear1::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CSimba_Appear1::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	if (true == pSimba->IsAnimFinished())
	{
		switch (pSimba->Get_State())
		{
		case CSimba::Simba_DemoAppear1Cut2:
			break;
		}
		pSimba->Change_State(CSimba::Simba_DemoAppear1Cut2, 66.66f, false, true);
	}
}

// *********************** APPEAR2 ***********************
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
	if (true == pSimba->IsAnimFinished())
	{
		switch (pSimba->Get_State())
		{
		case CSimba::Simba_DemoAppear2Cut1:
			pSimba->Change_State(CSimba::Simba_DemoAppear2Cut2, 66.66f, false, true);
			break;
		case CSimba::Simba_DemoAppear2Cut2:
			if (true == pSimba->Get_RenderMant())
				pSimba->Set_RenderMant(false);
			m_fTime += fTimeDelta;
			if (m_fTime > 1.5f) {
				pSimba->Change_State(CSimba::Simba_Walk, 66.66f, true, false);

				_vector vPos = m_pTransform->Get_State(CTransform::STATE_POSITION);
				_vector vLook = XMVector3Normalize(m_pTransform->Get_State(CTransform::STATE_LOOK));
				_vector vNewPos = vPos - vLook * 16.f;
				m_pController->Set_Position(m_pTransform, vNewPos);
			}
			break;
		}
	}
}

// *********************** WALK ***********************
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
		if (0 == CUtils::Make_RandomInt(0, 1))
			pSimba->Change_State(CSimba::Simba_QuickClawStartL, 66.66f, false, true);
		else
			pSimba->Change_State(CSimba::Simba_QuickClawStartR, 66.66f, false, true);
	}
	else
	{
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 6.5f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);
	}
}

// *********************** QUICKCLAW ***********************
void CSimba_QuickClaw::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.58f;
}

void CSimba_QuickClaw::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_QuickClawStartL:
			pSimba->Change_State(CSimba::Simba_QuickClawChargeL, 66.66f, false, true);
			break;
		case CSimba::Simba_QuickClawStartR:
			pSimba->Change_State(CSimba::Simba_QuickClawChargeR, 66.66f, false, true);
			break;

		case CSimba::Simba_QuickClawChargeL:
			pSimba->Change_State(CSimba::Simba_QuickClawL, 66.66f, false, true);
			break;
		case CSimba::Simba_QuickClawChargeR:
			pSimba->Change_State(CSimba::Simba_QuickClawR, 66.66f, false, true);
			break;

		case CSimba::Simba_QuickClawL:
			pSimba->Change_State(CSimba::Simba_QuickClaw2R, 66.66f, false, true);
			break;
		case CSimba::Simba_QuickClawR:
			pSimba->Change_State(CSimba::Simba_QuickClaw2L, 66.66f, false, true);
			break;

		case CSimba::Simba_QuickClaw2L:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f)
				pSimba->Change_State(CSimba::Simba_QuickClawEndL, 66.66f, false, true);
			else
				pSimba->Change_State(CSimba::Simba_QuickClawLFromStart, 66.66f, false, true);
			break;
		case CSimba::Simba_QuickClaw2R:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f)
				pSimba->Change_State(CSimba::Simba_QuickClawEndR, 66.66f, false, true);
			else
				pSimba->Change_State(CSimba::Simba_QuickClawRFromStart, 66.66f, false, true);
			break;

		case CSimba::Simba_QuickClawEndL: case CSimba::Simba_QuickClawEndR:
			pSimba->Set_PreState(iState);
			pSimba->Change_State(CSimba::Simba_JumpStart, 50.f, false, true);
			break;

		case CSimba::Simba_QuickClawLFromStart: case CSimba::Simba_QuickClawRFromStart:
			pSimba->Change_State(CSimba::Simba_FinalCrusher, 66.66f, false, true);
			break;
		}
	}
}

// *********************** FinalCrusher ***********************
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
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_FinalCrusher:
			pSimba->Change_State(CSimba::Simba_FinalCrusherEnd, 66.66f, false, true);
			break;
		case CSimba::Simba_FinalCrusherEnd:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Set_PreState(iState);
				pSimba->Change_State(CSimba::Simba_JumpStart, 50.f, false, true);
			}
			else
				pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 66.66f, false, true);
			break;

		case CSimba::Simba_FinalCrusherStart: // After Jump
			pSimba->Change_State(CSimba::Simba_FinalCrusher, 66.66f, false, true);
			break;
		}
	}
}

// *********************** DoubleClaw ***********************
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
	if (true == pSimba->IsAnimFinished())
	{
		switch (iState)
		{
		case CSimba::Simba_DoubleClawChargeStart:
			pSimba->Change_State(CSimba::Simba_DoubleClawChargeWait, 50.f, false, true);
			break;
		case CSimba::Simba_DoubleClawChargeWait:
			pSimba->Change_State(CSimba::Simba_DoubleClawDashStart, 50.f, false, true);
			break;
		case CSimba::Simba_DoubleClawDashStart:
			pSimba->Change_State(CSimba::Simba_DoubleClawDash, 50.f, true, true);
			break;
		case CSimba::Simba_DoubleClaw:
			pSimba->Change_State(CSimba::Simba_DoubleClawEnd, 50.f, false, true);
			break;
		case CSimba::Simba_DoubleClawEnd:
			if (m_pGameInstance->Compute_Distance(m_pKirby, pSimba) > 15.f) {
				pSimba->Set_PreState(iState);
				pSimba->Change_State(CSimba::Simba_JumpStart, 50.f, false, true);
			}
			else
				; // 점프공격
				break;
		}
	}

	if (CSimba::Simba_DoubleClawDash == iState)
	{
		m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 0.8f);
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 10.f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);

		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pGameObject);
		if (fDis < 8.5f)
			pSimba->Change_State(CSimba::Simba_DoubleClaw, 66.66f, false, true);
	}
}

// *********************** Jump ***********************
void CSimba_Jump::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	s_fOffsetY = -0.7f;
	m_fJumpPower = 72.f;
}

void CSimba_Jump::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_uint iState = pSimba->Get_State();
	if (CSimba::Simba_JumpStart == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
		if (true == pSimba->IsAnimFinished())
			pSimba->Change_State(CSimba::Simba_Jump, 50.f, false, true);
	}
	else if (CSimba::Simba_Jump == iState)
	{
		m_pController->Jump(m_pTransform, m_fJumpPower, fTimeDelta);
		m_fJumpPower -= GRAVITY * fTimeDelta * 33.3f;
	
		_float fDis = m_pGameInstance->Compute_Distance(m_pKirby, pSimba);
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * fDis * 1.5f;
		m_pController->Move_Dir(m_pTransform, vLook, fTimeDelta, s_fOffsetY);

		if (pSimba->Get_AnimRatio() < 0.35f)
			m_pTransform->Look_At_Rotate(m_pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 7.f);

		if (true == pSimba->IsAnimFinished())
			pSimba->Change_State(CSimba::Simba_Landing, 50.f, false, true);
	}
	else if (CSimba::Simba_Landing == iState)
	{
		m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
		if (true == pSimba->IsAnimFinished())
		{
			if (m_pGameInstance->Compute_Distance(m_pKirby, pGameObject) > 15.f)
				pSimba->Change_State(CSimba::Simba_JumpStart, 50.f, false, true);
			else
			{
				_uint iPreState = pSimba->Get_PreState();
				if (CSimba::Simba_QuickClawEndL == iPreState || CSimba::Simba_QuickClawEndR == iPreState)
					pSimba->Change_State(CSimba::Simba_FinalCrusherStart, 66.66f, false, true);
				else if (CSimba::Simba_FinalCrusherEnd == iPreState)
					pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 50.f, false, true);
				else if (CSimba::Simba_DoubleClawEnd == iPreState)
					pSimba->Change_State(CSimba::Simba_DoubleClawChargeStart, 50.f, false, true); ; // 점프공격
			}	
		}
	}
}