#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CBladeKnight_Idle_State final : public CFSM_State
{
private:
	CBladeKnight_Idle_State();
	virtual ~CBladeKnight_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBladeKnight_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			MOVE STATE
//*********************************
class CBladeKnight_Move_State final : public CFSM_State
{
private:
	CBladeKnight_Move_State();
	virtual ~CBladeKnight_Move_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };

public:
	static	CBladeKnight_Move_State* Create();
	virtual void Free() override;
};


//*********************************
//			FIND STATE
//*********************************
class CBladeKnight_Find_State final : public CFSM_State
{
private:
	CBladeKnight_Find_State();
	virtual ~CBladeKnight_Find_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CBladeKnight_Find_State* Create();
	virtual void Free() override;
};


//*********************************
//			ATTACK STATE
//*********************************
class CBladeKnight_Attack_State final : public CFSM_State
{
private:
	CBladeKnight_Attack_State();
	virtual ~CBladeKnight_Attack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fX = { 0.f };
	_float m_fSpeed = { 0.f };
	_bool bStop = { false };

public:
	static	CBladeKnight_Attack_State* Create();
	virtual void Free() override;
};


//*********************************
//			RETREAT STATE
//*********************************
class CBladeKnight_Retreat_State final : public CFSM_State
{
private:
	CBladeKnight_Retreat_State();
	virtual ~CBladeKnight_Retreat_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fSpeed = { 0.f };
	_float m_fTimeDelta = { 0.f };

	_bool m_bRecoverSpeed = { false };

public:
	static	CBladeKnight_Retreat_State* Create();
	virtual void Free() override;
};


//*********************************
//			TORNADOATTACK STATE
//*********************************
class CBladeKnight_TornadoAttack_State final : public CFSM_State
{
private:
	CBladeKnight_TornadoAttack_State();
	virtual ~CBladeKnight_TornadoAttack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fSpeed = { 0.f };

public:
	static	CBladeKnight_TornadoAttack_State* Create();
	virtual void Free() override;
};


//*********************************
//			DAMAGE STATE
//*********************************
class CBladeKnight_Damage_State final : public CFSM_State
{
private:
	CBladeKnight_Damage_State();
	virtual ~CBladeKnight_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
	
private:
	_float m_fJumpVelocity = { 0.f };
	_vector m_vKirbyLook = {};

	_float		m_fFlyTime = { 0.f };
	_float		m_fDeadTime = { 0.f };
	_float		m_fDeadMaxTime = { 0.f };
	_float		m_fEffectTime = { 0.f };

public:
	static	CBladeKnight_Damage_State* Create();
	virtual void Free() override;
};

END

