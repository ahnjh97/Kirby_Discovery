#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

class CDeeDeeDee_Idle_State final : public CFSM_State
{
private:
	CDeeDeeDee_Idle_State();
	virtual ~CDeeDeeDee_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float m_fIdleDelay = { 0.f };
	static	CDeeDeeDee_Idle_State* Create();
	virtual void Free() override;

};

class CDeeDeeDee_Run_State final : public CFSM_State
{
private:
	CDeeDeeDee_Run_State();
	virtual ~CDeeDeeDee_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CDeeDeeDee_Run_State* Create();
	virtual void Free() override;

};


class CDeeDeeDee_Shout_State final : public CFSM_State
{
private:
	CDeeDeeDee_Shout_State();
	virtual ~CDeeDeeDee_Shout_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float	m_fShoutTime = { 0.f };
	_bool	m_bShoutTrigger = { true };

	_bool	m_bTurnTrigger = { true };
	_bool	m_bSetTurn = { true };
	static	CDeeDeeDee_Shout_State* Create();
	virtual void Free() override;
};


class CDeeDeeDee_Slide_State final : public CFSM_State
{
private:
	CDeeDeeDee_Slide_State();
	virtual ~CDeeDeeDee_Slide_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float	m_fSlidePower = { 15.f };
	static	CDeeDeeDee_Slide_State* Create();
	virtual void Free() override;
};


class CDeeDeeDee_Initialize_State final : public CFSM_State
{
private:
	CDeeDeeDee_Initialize_State();
	virtual ~CDeeDeeDee_Initialize_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float m_fDelayTime = { 0.f };
	static	CDeeDeeDee_Initialize_State* Create();
	virtual void Free() override;
};


class CDeeDeeDee_Jump_State final : public CFSM_State
{
private:
	CDeeDeeDee_Jump_State();
	virtual ~CDeeDeeDee_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_bool	m_bTargetTrigger = { true };
	_bool	m_bTarget = { true };
	_float  m_fJumpDelayTime = { 0.f };

	static	CDeeDeeDee_Jump_State* Create();
	virtual void Free() override;
};


class CDeeDeeDee_SideAttack_State final : public CFSM_State
{
private:
	CDeeDeeDee_SideAttack_State();
	virtual ~CDeeDeeDee_SideAttack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CDeeDeeDee_SideAttack_State* Create();
	virtual void Free() override;
};


class CDeeDeeDee_HammerAttack_State final : public CFSM_State
{
private:
	CDeeDeeDee_HammerAttack_State();
	virtual ~CDeeDeeDee_HammerAttack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_bool	m_bTargetTrigger = { true };
	_bool	m_bTarget = { true };

	static	CDeeDeeDee_HammerAttack_State* Create();
	virtual void Free() override;
};


class CDeeDeeDee_Death_State final : public CFSM_State
{
private:
	CDeeDeeDee_Death_State();
	virtual ~CDeeDeeDee_Death_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CDeeDeeDee_Death_State* Create();
	virtual void Free() override;
};

END