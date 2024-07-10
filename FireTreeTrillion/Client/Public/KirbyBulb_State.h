#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

class CKirbyBulb_Idle_State final : public CFSM_State
{
private:
	CKirbyBulb_Idle_State();
	virtual ~CKirbyBulb_Idle_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	static	CKirbyBulb_Idle_State* Create();
	virtual void				   Free() override;
};



class CKirbyBulb_Run_State final : public CFSM_State
{
private:
	CKirbyBulb_Run_State();
	virtual ~CKirbyBulb_Run_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	static	CKirbyBulb_Run_State* Create();
	virtual void				   Free() override;
};



class CKirbyBulb_Jump_State final : public CFSM_State
{
private:
	CKirbyBulb_Jump_State();
	virtual ~CKirbyBulb_Jump_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	_float m_fFallTime = { 0.f };
	_float m_fChangeRunTime = { 0.f };
	static	CKirbyBulb_Jump_State* Create();
	virtual void				   Free() override;
};



class CKirbyBulb_Light_State final : public CFSM_State
{
private:
	CKirbyBulb_Light_State();
	virtual ~CKirbyBulb_Light_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	static	CKirbyBulb_Light_State* Create();
	virtual void				   Free() override;
};

class CKirbyBulb_Damage_State final : public CFSM_State
{
private:
	CKirbyBulb_Damage_State();
	virtual ~CKirbyBulb_Damage_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	static	CKirbyBulb_Damage_State* Create();
	virtual void				   Free() override;
};

class CKirbyBulb_Vacuum_State final : public CFSM_State
{
private:
	CKirbyBulb_Vacuum_State();
	virtual ~CKirbyBulb_Vacuum_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	_float m_fTime = { 0.f };
	_bool m_bLightReset = { true };
	static	CKirbyBulb_Vacuum_State* Create();
	virtual void				   Free() override;
};

class CKirbyBulb_Contents_State final : public CFSM_State
{
private:
	CKirbyBulb_Contents_State();
	virtual ~CKirbyBulb_Contents_State() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;
public:
	static	CKirbyBulb_Contents_State* Create();
	virtual void				   Free() override;
};


END