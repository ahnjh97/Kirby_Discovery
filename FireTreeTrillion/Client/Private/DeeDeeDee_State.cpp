#include "stdafx.h"
#include "DeeDeeDee_State.h"
#include "DeeDeeDee.h"
#include "Kirby.h"

#pragma region IDLE STATE

CDeeDeeDee_Idle_State::CDeeDeeDee_Idle_State()
{
}

void CDeeDeeDee_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{


}

void CDeeDeeDee_Idle_State::OnStateExit()
{

}

CDeeDeeDee_Idle_State* CDeeDeeDee_Idle_State::Create()
{
	CDeeDeeDee_Idle_State* pInstance = new CDeeDeeDee_Idle_State();
	return pInstance;
}

void CDeeDeeDee_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion