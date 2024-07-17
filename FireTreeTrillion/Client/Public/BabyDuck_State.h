#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CBabyDuck_Idle_State final : public CFSM_State
{
private:
	CBabyDuck_Idle_State();
	virtual ~CBabyDuck_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBabyDuck_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			RUN STATE
//*********************************
class CBabyDuck_Run_State final : public CFSM_State
{
private:
	CBabyDuck_Run_State();
	virtual ~CBabyDuck_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBabyDuck_Run_State* Create();
	virtual void Free() override;
};


//*********************************
//			ARRIVE STATE
//*********************************
class CBabyDuck_Arrive_State final : public CFSM_State
{
private:
	CBabyDuck_Arrive_State();
	virtual ~CBabyDuck_Arrive_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBabyDuck_Arrive_State* Create();
	virtual void Free() override;
};

END

