#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CBomber_Idle_State final : public CFSM_State
{
private:
	CBomber_Idle_State();
	virtual ~CBomber_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBomber_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			WALK STATE
//*********************************
class CBomber_Walk_State final : public CFSM_State
{
private:
	CBomber_Walk_State();
	virtual ~CBomber_Walk_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CBomber_Walk_State* Create();
	virtual void Free() override;
};


//*********************************
//			BOMB STATE
//*********************************
class CBomber_Suicide_State final : public CFSM_State
{
private:
	CBomber_Suicide_State();
	virtual ~CBomber_Suicide_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBomber_Suicide_State* Create();
	virtual void Free() override;
};


//*********************************
//			EXPLOSION STATE
//*********************************
class CBomber_Explosion_State final : public CFSM_State
{
private:
	CBomber_Explosion_State();
	virtual ~CBomber_Explosion_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
private:
	_float		m_fFlyTime = { 0.f };
	_float		m_fDeadTime = { 0.f };
	_float		m_fDeadMaxTime = { 0.f };
	_float		m_fEffectTime = { 0.f };

public:
	static	CBomber_Explosion_State* Create();
	virtual void Free() override;
};

END

