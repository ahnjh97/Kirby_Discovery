#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CCappyBody_Idle_State final : public CFSM_State
{
private:
	CCappyBody_Idle_State();
	virtual ~CCappyBody_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CCappyBody_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			FIND STATE
//*********************************
class CCappyBody_Find_State final : public CFSM_State
{
private:
	CCappyBody_Find_State();
	virtual ~CCappyBody_Find_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CCappyBody_Find_State* Create();
	virtual void Free() override;
};


//*********************************
//			RUN STATE
//*********************************
class CCappyBody_Run_State final : public CFSM_State
{
private:
	CCappyBody_Run_State();
	virtual ~CCappyBody_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CCappyBody_Run_State* Create();
	virtual void Free() override;
};


//*********************************
//			HATLOSE STATE
//*********************************
class CCappyBody_HatLose_State final : public CFSM_State
{
private:
	CCappyBody_HatLose_State();
	virtual ~CCappyBody_HatLose_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	static	CCappyBody_HatLose_State* Create();
	virtual void Free() override;
};


//*********************************
//			DAMAGE STATE
//*********************************
class CCappyBody_Damage_State final : public CFSM_State
{
private:
	CCappyBody_Damage_State();
	virtual ~CCappyBody_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float		m_fFlyTime = { 0.f };
	_float		m_fDeadTime = { 0.f };
	_float		m_fDeadMaxTime = { 0.f };

public:
	static	CCappyBody_Damage_State* Create();
	virtual void Free() override;
};

END

