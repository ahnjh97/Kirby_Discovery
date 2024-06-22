#include "stdafx.h"
#include "FinalBoss_State.h"
#include "FinalBoss.h"

#pragma region APPEAR STATE
//*********************************
//			 APPEAR STATE
//*********************************
CFinalBoss_Appear_State::CFinalBoss_Appear_State()
{
}

void CFinalBoss_Appear_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Appear_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if(pFinalBoss->IsAnimFinished())
		pFinalBoss->Change_State(CFinalBoss::FINALBOSS_WAITAIR, 50.f, true, true);
}

void CFinalBoss_Appear_State::OnStateExit()
{
}

CFinalBoss_Appear_State* CFinalBoss_Appear_State::Create()
{
	CFinalBoss_Appear_State* pInstance = new CFinalBoss_Appear_State();
	return pInstance;
}

void CFinalBoss_Appear_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CFinalBoss_Idle_State::CFinalBoss_Idle_State()
{
}

void CFinalBoss_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinalBoss_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
}

void CFinalBoss_Idle_State::OnStateExit()
{
}

CFinalBoss_Idle_State* CFinalBoss_Idle_State::Create()
{
	CFinalBoss_Idle_State* pInstance = new CFinalBoss_Idle_State();
	return pInstance;
}

void CFinalBoss_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion

CFinalBoss_Thrust_State::CFinalBoss_Thrust_State()
{
}

void CFinalBoss_Thrust_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
}

void CFinalBoss_Thrust_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CFinalBoss_Thrust_State::OnStateExit()
{
}

CFinalBoss_Thrust_State* CFinalBoss_Thrust_State::Create()
{
	return nullptr;
}

void CFinalBoss_Thrust_State::Free()
{
}
