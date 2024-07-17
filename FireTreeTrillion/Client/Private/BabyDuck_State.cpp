#include "stdafx.h"
#include "BabyDuck_State.h"
#include "BabyDuck.h"


#pragma region IDLE STATE
//*********************************
//			IDLE STATE
//*********************************
CBabyDuck_Idle_State::CBabyDuck_Idle_State()
{
}

void CBabyDuck_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBabyDuck_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	/*CSurprisedBoard* pSurprisedBoard = static_cast<CSurprisedBoard*>(pGameObject);
	CTransform* pTransform = pSurprisedBoard->Get_TransformCom();*/
}

void CBabyDuck_Idle_State::OnStateExit()
{
}

CBabyDuck_Idle_State* CBabyDuck_Idle_State::Create()
{
	CBabyDuck_Idle_State* pInstance = new CBabyDuck_Idle_State();
	return pInstance;
}

void CBabyDuck_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion



#pragma region RUN STATE
//*********************************
//			RUN STATE
//*********************************
CBabyDuck_Run_State::CBabyDuck_Run_State()
{
}

void CBabyDuck_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBabyDuck_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	/*CSurprisedBoard* pSurprisedBoard = static_cast<CSurprisedBoard*>(pGameObject);
	CTransform* pTransform = pSurprisedBoard->Get_TransformCom();*/
}

void CBabyDuck_Run_State::OnStateExit()
{
}

CBabyDuck_Run_State* CBabyDuck_Run_State::Create()
{
	CBabyDuck_Run_State* pInstance = new CBabyDuck_Run_State();
	return pInstance;
}

void CBabyDuck_Run_State::Free()
{
	__super::Free();
}
#pragma endregion



#pragma region ARRIVE STATE
//*********************************
//			ARRIVE STATE
//*********************************
CBabyDuck_Arrive_State::CBabyDuck_Arrive_State()
{
}

void CBabyDuck_Arrive_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBabyDuck_Arrive_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	/*CSurprisedBoard* pSurprisedBoard = static_cast<CSurprisedBoard*>(pGameObject);
	CTransform* pTransform = pSurprisedBoard->Get_TransformCom();*/
}

void CBabyDuck_Arrive_State::OnStateExit()
{
}

CBabyDuck_Arrive_State* CBabyDuck_Arrive_State::Create()
{
	CBabyDuck_Arrive_State* pInstance = new CBabyDuck_Arrive_State();
	return pInstance;
}

void CBabyDuck_Arrive_State::Free()
{
	__super::Free();
}

#pragma endregion
