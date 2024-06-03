#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)


// 커비의 Sword Idle 상태를 통제한다.

class CKirbySword_Idle_State final : public CFSM_State
{
private:
	CKirbySword_Idle_State();
	virtual ~CKirbySword_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	void Key_Z(class CGameObject* pGameObject, _float fTimeDelta);
	void Key_X(class CGameObject* pGameObject, _float fTimeDelta);
	void Key_C(class CGameObject* pGameObject, _float fTimeDelta);
	void Key_V(class CGameObject* pGameObject, _float fTimeDelta);
	virtual void OnStateExit()														override;

public:
	_float	m_fChargeTime = { 0.f };
	static	CKirbySword_Idle_State* Create();
	virtual void			   Free() override;
};





// 커비의 Sword RUN 상태를 통제한다.

class CKirbySword_Run_State final : public CFSM_State
{
private:
	CKirbySword_Run_State();
	virtual ~CKirbySword_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbySword_Run_State* Create();
	virtual void					 Free() override;
};




// 커비의 가드와 슬라이딩 상태를 제어한다.

class CKirbySword_Guard_State final : public CFSM_State
{
private:
	CKirbySword_Guard_State();
	virtual ~CKirbySword_Guard_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbySword_Guard_State* Create();
	virtual void					 Free() override;
};





// 커비의 평타 공격을 제어한다.

class CKirbySword_Attack_State final : public CFSM_State
{
private:
	CKirbySword_Attack_State();
	virtual ~CKirbySword_Attack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_bool	m_bPassNextAttackMotion = { false };
	_float	m_fAttackSpeed = { 0.f };
	_float	m_fAnimTime = { 0.f };
	_float	m_fLockTime = { 0.f };
	static	CKirbySword_Attack_State* Create();
	virtual void					 Free() override;
};



// 커비의 차징 스핀 공격을 제어한다.

class CKirbySword_ChargeSpin_State final : public CFSM_State
{
private:
	CKirbySword_ChargeSpin_State();
	virtual ~CKirbySword_ChargeSpin_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbySword_ChargeSpin_State* Create();
	virtual void					 Free() override;
};



// 커비의 공콤 및 공중제비를 제어한다.

class CKirbySword_JumpAttack_State final : public CFSM_State
{
private:
	CKirbySword_JumpAttack_State();
	virtual ~CKirbySword_JumpAttack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbySword_JumpAttack_State* Create();
	virtual void					 Free() override;
};



// 커비의 공중상태를 제어한다. (아마 한개일거임..

class CKirbySword_Fly_State final : public CFSM_State
{
private:
	CKirbySword_Fly_State();
	virtual ~CKirbySword_Fly_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbySword_Fly_State* Create();
	virtual void					 Free() override;
};




END
