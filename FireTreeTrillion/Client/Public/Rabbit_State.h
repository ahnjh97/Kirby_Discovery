#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CRabbit_Idle_State final : public CFSM_State
{
private:
	CRabbit_Idle_State();
	virtual ~CRabbit_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_bool	m_bFind = { false };

public:
	static	CRabbit_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			FIND STATE
//*********************************
class CRabbit_Find_State final : public CFSM_State
{
private:
	CRabbit_Find_State();
	virtual ~CRabbit_Find_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CRabbit_Find_State* Create();
	virtual void Free() override;
};


//*********************************
//			JUMP STATE
//*********************************
class CRabbit_Jump_State final : public CFSM_State
{
private:
	CRabbit_Jump_State();
	virtual ~CRabbit_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CRabbit_Jump_State* Create();
	virtual void Free() override;
};


//*********************************
//			JUMPLANDING STATE
//*********************************
class CRabbit_JumpLanding_State final : public CFSM_State
{
private:
	CRabbit_JumpLanding_State();
	virtual ~CRabbit_JumpLanding_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fJumpVelocity = { 0.f };
	_float	m_fTimeDelta = { 0.f };

public:
	static	CRabbit_JumpLanding_State* Create();
	virtual void Free() override;
};


//*********************************
//			DAMAGE STATE
//*********************************
class CRabbit_Damage_State final : public CFSM_State
{
private:
	CRabbit_Damage_State();
	virtual ~CRabbit_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float		m_fFlyTime = { 0.f };
	_float		m_fDeadTime = { 0.f };
	_float		m_fDeadMaxTime = { 0.f };

	static	CRabbit_Damage_State* Create();
	virtual void Free() override;
};

END

