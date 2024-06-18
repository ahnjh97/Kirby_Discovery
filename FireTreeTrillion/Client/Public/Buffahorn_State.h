#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CBuffahorn_Idle_State final : public CFSM_State
{
private:
	CBuffahorn_Idle_State();
	virtual ~CBuffahorn_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBuffahorn_Idle_State* Create();
	virtual void Free() override;
};

//*********************************
//			FIND STATE
//*********************************
class CBuffahorn_Find_State final : public CFSM_State
{
private:
	CBuffahorn_Find_State();
	virtual ~CBuffahorn_Find_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBuffahorn_Find_State* Create();
	virtual void Free() override;
};

//*********************************
//			WAIT STATE
//*********************************
class CBuffahorn_Wait_State final : public CFSM_State
{
private:
	CBuffahorn_Wait_State();
	virtual ~CBuffahorn_Wait_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBuffahorn_Wait_State* Create();
	virtual void Free() override;
};


//*********************************
//			RUN STATE
//*********************************
class CBuffahorn_Run_State final : public CFSM_State
{
private:
	CBuffahorn_Run_State();
	virtual ~CBuffahorn_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fTimeDelta = { 0.f };

public:
	static	CBuffahorn_Run_State* Create();
	virtual void Free() override;
};


//*********************************
//			BRAKE STATE
//*********************************
class CBuffahorn_Brake_State final : public CFSM_State
{
private:
	CBuffahorn_Brake_State();
	virtual ~CBuffahorn_Brake_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fSpeed = { 0.f };

public:
	static	CBuffahorn_Brake_State* Create();
	virtual void Free() override;
};


//*********************************
//			JUMP STATE
//*********************************
class CBuffahorn_Jump_State final : public CFSM_State
{
private:
	CBuffahorn_Jump_State();
	virtual ~CBuffahorn_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };

public:
	static	CBuffahorn_Jump_State* Create();
	virtual void Free() override;
};


//*********************************
//			DAMAGE STATE
//*********************************
class CBuffahorn_Damage_State final : public CFSM_State
{
private:
	CBuffahorn_Damage_State();
	virtual ~CBuffahorn_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float		m_fFlyTime = { 0.f };
	_float		m_fDeadTime = { 0.f };
	_float		m_fDeadMaxTime = { 0.f };


public:
	static	CBuffahorn_Damage_State* Create();
	virtual void Free() override;
};


//*********************************
//			BounceToTurn STATE
//*********************************
class CBuffahorn_BounceToTurn_State final : public CFSM_State
{
private:
	CBuffahorn_BounceToTurn_State();
	virtual ~CBuffahorn_BounceToTurn_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CBuffahorn_BounceToTurn_State* Create();
	virtual void Free() override;
};


END

