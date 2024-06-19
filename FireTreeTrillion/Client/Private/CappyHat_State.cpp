#include "stdafx.h"
#include "CappyHat_State.h"
#include "CappyHat.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************

CCappyHat_Idle_State::CCappyHat_Idle_State()
{
}

void CCappyHat_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CCappyHat_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CCappyHat_Idle_State::OnStateExit()
{
}

CCappyHat_Idle_State* CCappyHat_Idle_State::Create()
{
	CCappyHat_Idle_State* pInstance = new CCappyHat_Idle_State();
	return pInstance;
}

void CCappyHat_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CCappyHat_Damage_State::CCappyHat_Damage_State()
{
}

void CCappyHat_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);

}

void CCappyHat_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CCappyHat* pCappy = static_cast<CCappyHat*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pCappy->Get_PhyXState() == PO_NORMAL)
	{
		//// 일단 그 방향으로 바라보게만 한다.
		//_float3 vDamegeDir = pCappy->Get_DamegeDir();
		//if (vDamegeDir != XMVectorZero())
		//	pTransformCom->Look_At_Axis(-vDamegeDir);

		//// 이제 날아가는 것을 구현해보자.
		//pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		//// 점프되는 체공시간을 구현해보자.
		//_float fDamageJumpPower = pCappy->Get_DamageJumpPower();
		//pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		//fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		//pCappy->Set_DamageJumpPower(fDamageJumpPower);


		//if (true == pCappy->IsAnimFinished() || pController->Is_Terrain())
		//{
		//	if (pCappy->Get_Hp() <= 0.f)
		//		pCappy->Set_Dead();
		//	else
		//		pCappy->Change_State(CCappyHat::CAPPYHAT_FLY, 40.f, false, true);
		//}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pCappy->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pCappy->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pCappy->Set_PhyXState(PO_FLYDEADAWAY);
			pCappy->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pCappy->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pCappy->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pCappy->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pCappy->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pCappy->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
			pCappy->Set_Dead();

	}
}

void CCappyHat_Damage_State::OnStateExit()
{
}

CCappyHat_Damage_State* CCappyHat_Damage_State::Create()
{
	CCappyHat_Damage_State* pInstance = new CCappyHat_Damage_State();
	return pInstance;
}

void CCappyHat_Damage_State::Free()
{
	__super::Free();
}
#pragma endregion