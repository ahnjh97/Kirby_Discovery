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

	//// 일반 충돌 상태 (Get_Vacuuming()이 true라면 빨아들이는 상태이다.
	//if (pBrontoBurt->Get_Vacuuming() == false)
	//{
	//	// 일단 그 방향으로 바라보게만 한다.
	//	_float3 vDamegeDir = pBrontoBurt->Get_DamegeDir();
	//	//vDamegeDir.y = 0.f;
	//	pTransformCom->Look_At_Axis(-vDamegeDir);

	//	// 이제 날아가는 것을 구현해보자.
	//	pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 14.f, fTimeDelta);

	//	// 점프되는 체공시간을 구현해보자.
	//	_float fDamageJumpPower = pBrontoBurt->Get_DamageJumpPower();
	//	pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
	//	fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
	//	pBrontoBurt->Set_DamageJumpPower(fDamageJumpPower);


	//	if (true == pBrontoBurt->IsAnimFinished())
	//		pBrontoBurt->Change_State(CBrontoBurt::BRONTOBURT_FLY, 40.f, true, true);
	//}
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
