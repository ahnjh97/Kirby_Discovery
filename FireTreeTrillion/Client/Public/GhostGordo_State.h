#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CGhostGordo_Idle_State final : public CFSM_State
{
private:
	CGhostGordo_Idle_State();
	virtual ~CGhostGordo_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CGhostGordo_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			MOVE STATE
//*********************************
class CGhostGordo_Move_State final : public CFSM_State
{
private:
	CGhostGordo_Move_State();
	virtual ~CGhostGordo_Move_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };
	_float	m_fTimeDelta = { 0.f };

public:
	static	CGhostGordo_Move_State* Create();
	virtual void Free() override;
};


//*********************************
//			LOOK STATE
//*********************************
class CGhostGordo_Look_State final : public CFSM_State
{
private:
	CGhostGordo_Look_State();
	virtual ~CGhostGordo_Look_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CGhostGordo_Look_State* Create();
	virtual void Free() override;
};

END

