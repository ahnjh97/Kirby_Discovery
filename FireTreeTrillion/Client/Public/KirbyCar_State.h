#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

class CKirbyCar_Idle_State : public CFSM_State
{
private:
	CKirbyCar_Idle_State();
	virtual ~CKirbyCar_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyCar_Idle_State* Create();
	virtual void				  Free() override;
};




class CKirbyCar_Run_State : public CFSM_State
{
private:
	CKirbyCar_Run_State();
	virtual ~CKirbyCar_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyCar_Run_State* Create();
	virtual void				  Free() override;
};


class CKirbyCar_Boost_State : public CFSM_State
{
private:
	CKirbyCar_Boost_State();
	virtual ~CKirbyCar_Boost_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyCar_Boost_State* Create();
	virtual void				  Free() override;
};




class CKirbyCar_Jump_State : public CFSM_State
{
private:
	CKirbyCar_Jump_State();
	virtual ~CKirbyCar_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyCar_Jump_State* Create();
	virtual void				  Free() override;
};




class CKirbyCar_Damage_State : public CFSM_State
{
private:
	CKirbyCar_Damage_State();
	virtual ~CKirbyCar_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyCar_Damage_State* Create();
	virtual void				  Free() override;
};





class CKirbyCar_Vacuum_State : public CFSM_State
{
private:
	CKirbyCar_Vacuum_State();
	virtual ~CKirbyCar_Vacuum_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyCar_Vacuum_State* Create();
	virtual void				  Free() override;
};





class CKirbyCar_Cut_State : public CFSM_State
{
private:
	CKirbyCar_Cut_State();
	virtual ~CKirbyCar_Cut_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_uint	m_iTurnCount = { 0 };
	_float	m_fCutAnimTime = { 0.f };
	_float	m_fSpeed = { 0.f };
	static	CKirbyCar_Cut_State* Create();
	virtual void				  Free() override;
};


END