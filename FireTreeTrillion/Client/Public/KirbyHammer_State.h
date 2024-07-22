#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"


BEGIN(Client)

class CKirbyHammer_Idle_State final : public CFSM_State
{
private:
	CKirbyHammer_Idle_State();
	virtual ~CKirbyHammer_Idle_State() = default;

public:

	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:

	static	CKirbyHammer_Idle_State* Create();
	virtual void						Free() override;
};


class CKirbyHammer_Attack_State final : public CFSM_State
{
private:
	CKirbyHammer_Attack_State();
	virtual ~CKirbyHammer_Attack_State() = default;

public:

	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float	m_fAttackJumpTime = { 0.f };
	_float	m_fPreAttackJumpTime = { 0.f };

	_bool	m_bAttackJumpTrigger = { true };

	_bool	m_bCountTrigger = { true };

	static	CKirbyHammer_Attack_State* Create();
	virtual void						Free() override;

};

class CKirbyHammer_Onigorosi_State final : public CFSM_State
{
private:
	CKirbyHammer_Onigorosi_State();
	virtual ~CKirbyHammer_Onigorosi_State() = default;

public:

	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float	m_fMoveTime = { 0.f };
	_float	m_fPreMoveTime = { 0.f };

	static	CKirbyHammer_Onigorosi_State* Create();
	virtual void						Free() override;

};


class CKirbyHammer_JumpAttack_State final : public CFSM_State
{
private:
	CKirbyHammer_JumpAttack_State();
	virtual ~CKirbyHammer_JumpAttack_State() = default;

public:

	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyHammer_JumpAttack_State* Create();
	virtual void						Free() override;
};


class CKirbyHammer_Jump_State final : public CFSM_State
{
private:
	CKirbyHammer_Jump_State();
	virtual ~CKirbyHammer_Jump_State() = default;

public:

	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	_bool Key_X(class CGameObject* pGameObject, _float fTimeDelta);
	_bool Key_C(class CGameObject* pGameObject, _float fTimeDelta);

	_float	m_fChangeRunTime = { 0.f };
	_bool	m_bJumpEffectTrigger = { true };

public:
	static	CKirbyHammer_Jump_State* Create();
	virtual void						Free() override;

};


END