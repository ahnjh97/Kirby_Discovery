#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			ATTACK STATE
//*********************************
class CTestModel_Attack_State final : public CFSM_State
{
private:
	CTestModel_Attack_State();
	virtual ~CTestModel_Attack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CTestModel_Attack_State* Create();
	virtual void					 Free() override;

};

//*********************************
//			 IDLE STATE
//*********************************
class CTestModel_Idle_State final : public CFSM_State
{
private:
	CTestModel_Idle_State();
	virtual ~CTestModel_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CTestModel_Idle_State*	Create();
	virtual void					Free() override;

};


//*********************************
//			RUN STATE
//*********************************
class CTestModel_Run_State final : public CFSM_State
{
private:
	CTestModel_Run_State();
	virtual ~CTestModel_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CTestModel_Run_State*	Create();
	virtual void					Free() override;

};

END

