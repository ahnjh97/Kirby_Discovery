#pragma once

#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

class CKirbyCrash_Attack_State final : public CFSM_State
{
private:
	CKirbyCrash_Attack_State();
	virtual ~CKirbyCrash_Attack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_bool	m_bTimeCrashTrigger = { true };
	_float	m_fTime = { 0.f };

	static	CKirbyCrash_Attack_State*		Create();
	virtual void								Free() override;
};


class CKirbyCrash_BigAttack_State final : public CFSM_State
{
private:
	CKirbyCrash_BigAttack_State();
	virtual ~CKirbyCrash_BigAttack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_bool m_bTimeCrashTrigger = { true };
	_float m_fTime = { 0.f };
	_float m_fGravity = { 0.f };
	_bool m_bTerrainOn = { false };
	_float m_fTerrainTime = { 0.f };
	_float m_fLightRange = { 0.f };
	_bool  m_bLightRangeInv = { false };

	static	CKirbyCrash_BigAttack_State* Create();
	virtual void								Free() override;
};


class CKirbyCrash_Charge_State final : public CFSM_State
{
private:
	CKirbyCrash_Charge_State();
	virtual ~CKirbyCrash_Charge_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float	m_fTime = { 0.f };
	_float	m_fChargeTime = { 0.f };
	static	CKirbyCrash_Charge_State* Create();
	virtual void								Free() override;
};

class CKirbyCrash_BigCharge_State final : public CFSM_State
{
private:
	CKirbyCrash_BigCharge_State();
	virtual ~CKirbyCrash_BigCharge_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	
public:
	_float	m_fTime = { 0.f };
	static	CKirbyCrash_BigCharge_State* Create();
	virtual void								Free() override;
};
END