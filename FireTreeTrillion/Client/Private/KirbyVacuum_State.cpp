#include "stdafx.h"
#include "KirbyVacuum_State.h"
#include "Kirby_State_Function.h"

#pragma region SPIT STATE

CKirbyVacuum_Spit_State::CKirbyVacuum_Spit_State()
{
}

void CKirbyVacuum_Spit_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyVacuum_Spit_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// ¹ñ´Â ·ÎÁ÷


	// ¤·¤·
}

void CKirbyVacuum_Spit_State::OnStateExit()
{

}

CKirbyVacuum_Spit_State* CKirbyVacuum_Spit_State::Create()
{
	CKirbyVacuum_Spit_State* pInstance = new CKirbyVacuum_Spit_State();
	return pInstance;
}

void CKirbyVacuum_Spit_State::Free()
{
	__super::Free();
}
