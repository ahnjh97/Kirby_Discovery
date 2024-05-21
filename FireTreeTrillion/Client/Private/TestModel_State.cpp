#include "stdafx.h"
#include "TestModel_State.h"

#include "TestModel.h"
#include "Utils.h"


constexpr _float DefaultSpeed = 0.5f;

#pragma region ATTACK STATE
//*******************************************************
//						 ATTACK STATE
//*******************************************************
CTestModel_Attack_State::CTestModel_Attack_State()
{
}

void CTestModel_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);

}

void CTestModel_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CTestModel_Attack_State::OnStateExit()
{
}

CTestModel_Attack_State* CTestModel_Attack_State::Create()
{
	CTestModel_Attack_State* pInstance = new CTestModel_Attack_State();
	return pInstance;
}

void CTestModel_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CTestModel_Idle_State::CTestModel_Idle_State()
{
}

void CTestModel_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);


}

void CTestModel_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CTestModel_Idle_State::OnStateExit()
{
}


CTestModel_Idle_State* CTestModel_Idle_State::Create()
{
	CTestModel_Idle_State* pInstance = new CTestModel_Idle_State();
	return pInstance;
}

void CTestModel_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region RUN STATE
//*******************************************************
//						 RUN STATE
//*******************************************************
CTestModel_Run_State::CTestModel_Run_State()
{
}

void CTestModel_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);

}

void CTestModel_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CTestModel_Run_State::OnStateExit()
{
}

CTestModel_Run_State* CTestModel_Run_State::Create()
{
	return new CTestModel_Run_State();
}

void CTestModel_Run_State::Free()
{
	__super::Free();

}

#pragma endregion

