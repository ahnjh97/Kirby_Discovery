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
		pSimba->Change_State(CSimba::Simba_DemoAppear1Cut2, 50.f, false, true);
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
			pSimba->Change_State(CSimba::Simba_DemoAppear2Cut2, 50.f, false, true);
			break;
		case CSimba::Simba_DemoAppear2Cut2:
			m_fTime += fTimeDelta;
			if (m_fTime > 1.5f) {
				pSimba->Change_State(CSimba::Simba_Walk, 50.f, true, false);

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
	CGameObject* pKirby = m_pGameInstance->Get_GameObject(LEVEL_SIMBA, TEXT("Layer_Player"));
	CTransform* pKirbyTransform = pKirby->Get_TransformCom();

	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	m_pTransform->Look_At_Rotate(pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	_float fDis = m_pGameInstance->Compute_Distance(pKirby, pGameObject);
	if (fDis < 6.5f)
	{
		if (0 == CUtils::Make_RandomInt(0, 1))
			pSimba->Change_State(CSimba::Simba_QuickClawStartL, 50.f, false, true);
		else
			pSimba->Change_State(CSimba::Simba_QuickClawStartR, 50.f, false, true);
	}
	else
	{
		_vector vLook = m_pTransform->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 5.2f;
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
	CGameObject* pKirby = m_pGameInstance->Get_GameObject(LEVEL_SIMBA, TEXT("Layer_Player"));
	CTransform* pKirbyTransform = pKirby->Get_TransformCom();

	m_pController->FreeFall(m_pTransform, fTimeDelta, 6.f, s_fOffsetY);
	m_pTransform->Look_At_Rotate(pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	if (true == pSimba->IsAnimFinished())
	{
		switch (pSimba->Get_State())
		{
		case CSimba::Simba_QuickClawStartL:
			pSimba->Change_State(CSimba::Simba_QuickClawChargeL, 50.f, false, true);
			break;
		case CSimba::Simba_QuickClawStartR:
			pSimba->Change_State(CSimba::Simba_QuickClawChargeR, 50.f, false, true);
			break;

		case CSimba::Simba_QuickClawChargeL:
			pSimba->Change_State(CSimba::Simba_QuickClawL, 50.f, false, true);
			break;
		case CSimba::Simba_QuickClawChargeR:
			pSimba->Change_State(CSimba::Simba_QuickClawR, 50.f, false, true);
			break;

		case CSimba::Simba_QuickClawL:
			pSimba->Change_State(CSimba::Simba_QuickClaw2R, 50.f, false, true);
			break;
		case CSimba::Simba_QuickClawR:
			pSimba->Change_State(CSimba::Simba_QuickClaw2L, 50.f, false, true);
			break;

		case CSimba::Simba_QuickClaw2L:
			pSimba->Change_State(CSimba::Simba_QuickClawEndL, 50.f, false, true);
			break;
		case CSimba::Simba_QuickClaw2R:
			pSimba->Change_State(CSimba::Simba_QuickClawEndR, 50.f, false, true);
			break;
		
		default:
			break;
		}
	}
}