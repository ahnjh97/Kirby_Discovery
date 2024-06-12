#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

class CKirbyBoom_Fall_State final :
    public CFSM_State
{
private:
	CKirbyBoom_Fall_State();
	virtual ~CKirbyBoom_Fall_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

public:
	_float m_fAirThrowTime = { 0.f };
	static	CKirbyBoom_Fall_State* Create();
	virtual void			   Free() override;

};



class CKirbyBoom_Attack_State final :
	public CFSM_State
{
private:
	CKirbyBoom_Attack_State();
	virtual ~CKirbyBoom_Attack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

public:
	_float m_fThrowTime = { 0.f };
	static	CKirbyBoom_Attack_State* Create();
	virtual void			   Free() override;

};




class CKirbyBoom_ChargeAttack_State final :
	public CFSM_State
{
private:
	CKirbyBoom_ChargeAttack_State();
	virtual ~CKirbyBoom_ChargeAttack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

public:
	static	CKirbyBoom_ChargeAttack_State* Create();
	virtual void			   Free() override;

};

END