#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"


BEGIN(Client)

class CKirbyVacuum_Spit_State final : public CFSM_State
{
private:
	CKirbyVacuum_Spit_State();
	virtual ~CKirbyVacuum_Spit_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyVacuum_Spit_State* Create();
	virtual void						Free() override;
};



class CKirbyVacuum_Vacuum_State final : public CFSM_State
{
private:
	CKirbyVacuum_Vacuum_State();
	virtual ~CKirbyVacuum_Vacuum_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyVacuum_Vacuum_State* Create();
	virtual void						Free() override;
};



class CKirbyVacuum_VacuumWalk_State final : public CFSM_State
{
private:
	CKirbyVacuum_VacuumWalk_State();
	virtual ~CKirbyVacuum_VacuumWalk_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyVacuum_VacuumWalk_State* Create();
	virtual void						Free() override;
};



class CKirbyVacuum_Vacuuming_State final : public CFSM_State
{
private:
	CKirbyVacuum_Vacuuming_State();
	virtual ~CKirbyVacuum_Vacuuming_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()		override;

	_float fVacuumObjectSpeed = { 2.f };
public:
	static	CKirbyVacuum_Vacuuming_State* Create();
	virtual void						Free() override;
};



END