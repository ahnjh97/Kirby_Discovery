#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"


BEGIN(Client)

// 커비의 IDLE 상태를 통제한다.

class CKirbyDefault_Idle_State final : public CFSM_State
{
private:
	CKirbyDefault_Idle_State();
	virtual ~CKirbyDefault_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDefault_Idle_State*	Create();
	virtual void						Free() override;

};




// 커비의 RUN 상태를 통제한다.

class CKirbyDefault_Run_State final : public CFSM_State
{
private:
	CKirbyDefault_Run_State();
	virtual ~CKirbyDefault_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDefault_Run_State* Create();
	virtual void					 Free() override;


};





// 커비의 점프 상태를 통제한다.

class CKirbyDefault_Jump_State final : public CFSM_State
{
private:
	CKirbyDefault_Jump_State();
	virtual ~CKirbyDefault_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDefault_Jump_State* Create();
	virtual void					  Free() override;

};



END