#include "stdafx.h"
#include "PartTimerKirby_State.h"
#include "PartTimerKirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CPartTimerKirby_Idle_State::CPartTimerKirby_Idle_State()
{
}

void CPartTimerKirby_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CPartTimerKirby_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pPoorKirby = static_cast<CPartTimerKirby*>(pGameObject);

	if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_DOWN))
		pPoorKirby->Change_State(CPartTimerKirby::FOODSHOP_MOVEL, 50.f, false, true);
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_DOWN))
		pPoorKirby->Change_State(CPartTimerKirby::FOODSHOP_MOVER, 50.f, false, true);
	else if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		pPoorKirby->Change_State(CPartTimerKirby::FOODSHOP_CORRECT, 50.f, false, true);
}

void CPartTimerKirby_Idle_State::OnStateExit()
{
}

CPartTimerKirby_Idle_State* CPartTimerKirby_Idle_State::Create()
{
	CPartTimerKirby_Idle_State* pInstance = new CPartTimerKirby_Idle_State();
	return pInstance;
}

void CPartTimerKirby_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region MOVE STATE
//*********************************
//			 MOVE STATE
//*********************************
CPartTimerKirby_Move_State::CPartTimerKirby_Move_State()
{
}

void CPartTimerKirby_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CPartTimerKirby_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pPoorKirby = static_cast<CPartTimerKirby*>(pGameObject);
	if (pPoorKirby->IsAnimFinished())
		pPoorKirby->Change_State(CPartTimerKirby::FOODSHOP_SELECT, 50.f, true, true);
}

void CPartTimerKirby_Move_State::OnStateExit()
{
}

CPartTimerKirby_Move_State* CPartTimerKirby_Move_State::Create()
{
	CPartTimerKirby_Move_State* pInstance = new CPartTimerKirby_Move_State();
	return pInstance;
}

void CPartTimerKirby_Move_State::Free()
{
	__super::Free();

}

#pragma endregion

#pragma region GRAB STATE
//*********************************
//			GRAB STATE
//*********************************
CPartTimerKirby_Grab_State::CPartTimerKirby_Grab_State()
{
}

void CPartTimerKirby_Grab_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CPartTimerKirby_Grab_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pPoorKirby = static_cast<CPartTimerKirby*>(pGameObject);
	if (pPoorKirby->IsAnimFinished())
		pPoorKirby->Change_State(CPartTimerKirby::FOODSHOP_SELECT, 50.f, true, true);
}

void CPartTimerKirby_Grab_State::OnStateExit()
{
}

CPartTimerKirby_Grab_State* CPartTimerKirby_Grab_State::Create()
{
	CPartTimerKirby_Grab_State* pInstance = new CPartTimerKirby_Grab_State();
	return pInstance;
}

void CPartTimerKirby_Grab_State::Free()
{
	__super::Free();
}

#pragma endregion

