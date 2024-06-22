#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			APPEAR STATE
//*********************************
class CFinalBoss_Appear_State final : public CFSM_State
{
private:
	CFinalBoss_Appear_State();
	virtual ~CFinalBoss_Appear_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Appear_State* Create();
	virtual void Free() override;
};


//*********************************
//			IDLE STATE
//*********************************
class CFinalBoss_Idle_State final : public CFSM_State
{
private:
	CFinalBoss_Idle_State();
	virtual ~CFinalBoss_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			THRUST STATE
//*********************************
class CFinalBoss_Thrust_State final : public CFSM_State
{
private:
	CFinalBoss_Thrust_State();
	virtual ~CFinalBoss_Thrust_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Thrust_State* Create();
	virtual void Free() override;
};
////*********************************
////			DAMAGE STATE
////*********************************
//class CKabu_Damage_State final : public CFSM_State
//{
//private:
//	CKabu_Damage_State();
//	virtual ~CKabu_Damage_State() = default;
//
//public:
//	// 상태 진입했을 때 처음만 호출
//	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
//	// 상태 진입되어 있는 상태에서 매 tick마다 호출
//	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
//	virtual void OnStateExit()														override;
//
//public:
//	_float		m_fFlyTime = { 0.f };
//	_float		m_fDeadTime = { 0.f };
//	_float		m_fDeadMaxTime = { 0.f };
//
//
//	static	CKabu_Damage_State* Create();
//	virtual void Free() override;
//};

END

