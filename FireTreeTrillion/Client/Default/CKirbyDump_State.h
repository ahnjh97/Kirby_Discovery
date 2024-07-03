#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

class CKirbyDump_Idle_State : public CFSM_State
{
private:
	CKirbyDump_Idle_State();
	virtual ~CKirbyDump_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDump_Idle_State* Create();
	virtual void				  Free() override;
};


class CKirbyDump_Run_State : public CFSM_State
{
private:
	CKirbyDump_Run_State();
	virtual ~CKirbyDump_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDump_Run_State* Create();
	virtual void				  Free() override;
};

class CKirbyDump_Jump_State : public CFSM_State
{
private:
	CKirbyDump_Jump_State();
	virtual ~CKirbyDump_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDump_Jump_State* Create();
	virtual void				  Free() override;
};


END