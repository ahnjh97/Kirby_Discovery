#include "stdafx.h"
#include "KirbyCar_State.h"
#include "Kirby_State_Function.h"

CKirbyCar_Idle_State::CKirbyCar_Idle_State()
{
}

void CKirbyCar_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCar_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbyCar_Idle_State::OnStateExit()
{
}

CKirbyCar_Idle_State* CKirbyCar_Idle_State::Create()
{
	CKirbyCar_Idle_State* pInstance = new CKirbyCar_Idle_State();
	return pInstance;
}

void CKirbyCar_Idle_State::Free()
{
	__super::Free();
}
