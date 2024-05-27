#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			IDLE STATE
//*********************************
class CAwoofy_Idle_State final : public CFSM_State
{
//public:
//	enum AWOOFY_STATE {
//		AWOOFY_BRAKE, AWOOFY_DAMAGE, AWOOFY_FALL, AWOOFY_FIND, AWOOFY_FINDSUB, AWOOFY_FINDWAIT, AWOOFY_GROOMING,
//		AWOOFY_HOWLING, AWOOFY_JUMP, AWOOFY_JUMPEND, AWOOFY_LANDING, AWOOFY_LOOKAROUND, AWOOFY_LOOKAROUNDAFTERBRAKE, AWOOFY_RUN,
//		AWOOFY_SLEEP, AWOOFY_SLEEPFALL, AWOOFY_WAIT, AWOOFY_WAKEUP, AWOOFY_WALK, AWOOFY_END
//	};

private:
	CAwoofy_Idle_State();
	virtual ~CAwoofy_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CAwoofy_Idle_State* Create();
	virtual void Free() override;
};

//*********************************
//			RUN STATE
//*********************************
class CAwoofy_Run_State final : public CFSM_State
{
private:
	CAwoofy_Run_State();
	virtual ~CAwoofy_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };
	_float	m_fAngle = { 0.f };

	_vector m_vAxisY = {};

public:
	static	CAwoofy_Run_State* Create();
	virtual void Free() override;
};

//*********************************
//			FIND STATE
//*********************************
class CAwoofy_Find_State final : public CFSM_State
{
private:
	CAwoofy_Find_State();
	virtual ~CAwoofy_Find_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fAngle = { 0.f };

public:
	static	CAwoofy_Find_State* Create();
	virtual void Free() override;
};


//*********************************
//			BRAKE STATE
//*********************************
class CAwoofy_Brake_State final : public CFSM_State
{
private:
	CAwoofy_Brake_State();
	virtual ~CAwoofy_Brake_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float m_fSpeed = { 1.f };

public:
	static	CAwoofy_Brake_State* Create();
	virtual void Free() override;
};

//*********************************
//			LOOKAROUNDAFTERBRAKE STATE
//*********************************
class CAwoofy_LookAroundAfterBrake_State final : public CFSM_State
{
private:
	CAwoofy_LookAroundAfterBrake_State();
	virtual ~CAwoofy_LookAroundAfterBrake_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CAwoofy_LookAroundAfterBrake_State* Create();
	virtual void Free() override;
};

//*********************************
//			DAMAGE STATE
//*********************************
class CAwoofy_Damage_State final : public CFSM_State
{
private:
	CAwoofy_Damage_State();
	virtual ~CAwoofy_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CAwoofy_Damage_State* Create();
	virtual void Free() override;
};

END

