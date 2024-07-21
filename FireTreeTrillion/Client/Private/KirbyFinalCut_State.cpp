#include "stdafx.h"
#include "KirbyFinalCut_State.h"
#include "Kirby_State_Function.h"

#pragma region CUT STATE

CKirbyFinalCut_State::CKirbyFinalCut_State()
{
}

void CKirbyFinalCut_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyFinalCut_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// ¹ñ´Â ·ÎÁ÷ÀÌ´Ù.
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta);

	if (pKirby->Get_State() == CKirby::STATE_WALK)
	{
		m_fTime += fTimeDelta;

		if (m_bCutStartPos == true)
		{
			pController->Set_Position(pTransformCom, _float4(0.f, 0.f, -20.f, 1.f));
			DESC(m_vTargetDir) = DESC(m_vMoveDir) = _float4(0.f, 0.f, 1.f, 0.f);
			m_bCutStartPos = false;
		}
		Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
		if (Kirbydesc->m_fMoveSpeed > 2.5f)
			Kirbydesc->m_fMoveSpeed = 2.5f;

		// Å¸°Ù±âÁØ
		_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
		pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

		if (m_fTime >= 1.6f)
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			return;
		}
	}
	else if (pKirby->Get_State() == CKirby::FINALCUTSTATE_CUT1)
	{





		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::FINALCUTSTATE_CUT2, 60.f, true, true, CKirby::BODY_FINALCUT, CKirby::OFFSET_FINALCUT);
			return;
		}

	}
	else if (pKirby->Get_State() == CKirby::FINALCUTSTATE_CUT2)
	{





	}
}

void CKirbyFinalCut_State::OnStateExit()
{
	m_bCutStartPos = true;
	m_fTime = 0.f;
}

CKirbyFinalCut_State* CKirbyFinalCut_State::Create()
{
	CKirbyFinalCut_State* pInstance = new CKirbyFinalCut_State();
	return pInstance;
}

void CKirbyFinalCut_State::Free()
{
	__super::Free();
}

#pragma endregion
