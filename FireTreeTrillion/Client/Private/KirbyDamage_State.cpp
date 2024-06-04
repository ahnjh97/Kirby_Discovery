#include "stdafx.h"
#include "KirbyDamage_State.h"
#include "Kirby_State_Function.h"

CKirbyDamage_State::CKirbyDamage_State()
{
}

void CKirbyDamage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyDamage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 임시
	//_float4 vDamageDir = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * -1.f;
	_float3 vDamageDir = pKirby->Get_DamegeDir();

	// 타겟기준
	_vector vMoveDelta = vDamageDir * fTimeDelta * 12.f;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

	if (pKirby->Get_State() == CKirby::STATE_DAMAGE)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() || pKirby->isAnimFinish())
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_EATDAMAGE)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() || pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_EATWAIT, 60.f, true, true, CKirby::BODY_BALLOON);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_FILGHTDAMAGE)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain() || pKirby->isAnimFinish())
		{
			Kirby_AbilityType_Assist(pKirby, CKirby::STATE_IDLE);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
		}

	}
}

void CKirbyDamage_State::OnStateExit()
{
}

CKirbyDamage_State* CKirbyDamage_State::Create()
{
	CKirbyDamage_State* pInstance = new CKirbyDamage_State();
	return pInstance;
}

void CKirbyDamage_State::Free()
{
	__super::Free();
}
