#include "stdafx.h"
#include "BrontoBurt_State.h"
#include "BrontoBurt.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CBrontoBurt_Idle_State::CBrontoBurt_Idle_State()
{
}

void CBrontoBurt_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CBrontoBurt_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBrontoBurt* pBrontoBurt = static_cast<CBrontoBurt*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
}

void CBrontoBurt_Idle_State::OnStateExit()
{
}

CBrontoBurt_Idle_State* CBrontoBurt_Idle_State::Create()
{
	CBrontoBurt_Idle_State* pInstance = new CBrontoBurt_Idle_State();
	return pInstance;
}

void CBrontoBurt_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CBrontoBurt_Damage_State::CBrontoBurt_Damage_State()
{
}

void CBrontoBurt_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CBrontoBurt_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBrontoBurt* pBrontoBurt = static_cast<CBrontoBurt*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pBrontoBurt->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pBrontoBurt->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 14.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pBrontoBurt->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pBrontoBurt->Set_DamageJumpPower(fDamageJumpPower);


		if (true == pBrontoBurt->IsAnimFinished())
			pBrontoBurt->Change_State(CBrontoBurt::BRONTOBURT_FLY, 40.f, true, true);
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pBrontoBurt->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pBrontoBurt->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;
		if (m_fFlyTime > 2.f)
		{
			pBrontoBurt->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pBrontoBurt->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pBrontoBurt->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pBrontoBurt->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pBrontoBurt->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > 0.7f)
			pBrontoBurt->Set_Dead();
	}
}

void CBrontoBurt_Damage_State::OnStateExit()
{
}

CBrontoBurt_Damage_State* CBrontoBurt_Damage_State::Create()
{
	CBrontoBurt_Damage_State* pInstance = new CBrontoBurt_Damage_State();
	return pInstance;
}

void CBrontoBurt_Damage_State::Free()
{
	__super::Free();
}
