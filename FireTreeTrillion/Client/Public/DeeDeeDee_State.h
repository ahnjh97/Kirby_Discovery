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


END