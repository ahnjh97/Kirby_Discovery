#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)


//*********************************
//			IDLE STATE
//*********************************
class CPartTimerKirby_Idle_State final : public CFSM_State
{
private:
	CPartTimerKirby_Idle_State();
	virtual ~CPartTimerKirby_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	enum class PARTTIME_ITEM Get_CurrentFood(_float4 vPos);
	void RenderOff_Food(_uint uFoodIdx);

public:
	static	CPartTimerKirby_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			MOVE STATE
//*********************************
class CPartTimerKirby_Move_State final : public CFSM_State
{
private:
	CPartTimerKirby_Move_State();
	virtual ~CPartTimerKirby_Move_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	enum DIR { LEFT, RIGHT, DIR_END };
	_uint	m_uDir = LEFT;

public:
	static	CPartTimerKirby_Move_State* Create();
	virtual void Free() override;
};


//*********************************
//			GRAB STATE
//*********************************
class CPartTimerKirby_Grab_State final : public CFSM_State
{
private:
	CPartTimerKirby_Grab_State();
	virtual ~CPartTimerKirby_Grab_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
	void RenderOn_AllFood();

	enum class PARTTIME_ITEM Get_CurrentFood(_float4 vPos);

private:
	class CPartTimeFood*	m_pFood = nullptr;
	_float					m_fSpeed = _float();

public:
	static	CPartTimerKirby_Grab_State* Create();
	virtual void Free() override;
};


//*********************************
//			START-WIN STATE
//*********************************
class CPartTimerKirby_Win_State final : public CFSM_State
{
private:
	CPartTimerKirby_Win_State();
	virtual ~CPartTimerKirby_Win_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CPartTimerKirby_Win_State* Create();
	virtual void Free() override;
};



END

