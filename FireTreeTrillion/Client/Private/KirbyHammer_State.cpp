#include "stdafx.h"
#include "KirbyHammer_State.h"
#include "Kirby_State_Function.h"

#pragma region HAMMER STATE

CKirbyHammer_Attack_State::CKirbyHammer_Attack_State()
{
}

void CKirbyHammer_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyHammer_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// 뱉는 로직이다.
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	if (pKirby->Get_State() == CKirby::HAMMERSTATE_HAMMERATTACKFINALTOY)
	{
		m_fAttackJumpTime += fTimeDelta;

		if (m_fAttackJumpTime > 0.4f && m_bAttackJumpTrigger == true)
		{
			DESC(m_fJumpVelocity) = 22.f;
			m_bAttackJumpTrigger = false;
		}
		else if (m_bAttackJumpTrigger == false)
		{
			DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
			pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

			if (m_fAttackJumpTime >= 0.4f && m_fAttackJumpTime < 0.7f)
			{
				_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
				DESC(m_fJumpVelocity) = fStopVelocityPower;
			}
			else
			{

			}
		}

		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
		}
	}
}

void CKirbyHammer_Attack_State::OnStateExit()
{
	m_bAttackJumpTrigger = true;
	m_fAttackJumpTime = 0.f;
}

CKirbyHammer_Attack_State* CKirbyHammer_Attack_State::Create()
{
	CKirbyHammer_Attack_State* pInstance = new CKirbyHammer_Attack_State();
	return pInstance;
}

void CKirbyHammer_Attack_State::Free()
{
	__super::Free();

}

#pragma endregion
