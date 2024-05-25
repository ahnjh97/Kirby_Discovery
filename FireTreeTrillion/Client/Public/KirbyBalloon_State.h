#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

// 풍선 IDLE 상태를 통제한다.

class CKirbyBalloon_Idle_State final : public CFSM_State
{
private:
	CKirbyBalloon_Idle_State();
	virtual ~CKirbyBalloon_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyBalloon_Idle_State* Create();
	virtual void						Free() override;

};


// 풍선 달리기 상태를 통제한다.

class CKirbyBalloon_Run_State final : public CFSM_State
{
private:
	CKirbyBalloon_Run_State();
	virtual ~CKirbyBalloon_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyBalloon_Run_State* Create();
	virtual void						Free() override;

};


// 풍선의 점프 상태를 통제한다.

class CKirbyBalloon_Jump_State final : public CFSM_State
{
private:
	CKirbyBalloon_Jump_State();
	virtual ~CKirbyBalloon_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyBalloon_Jump_State* Create();
	virtual void						Free() override;

};


// 커비의 플라이 상태를 통제한다. ( 중요 )

class CKirbyBalloon_Fly_State final : public CFSM_State
{
private:
	CKirbyBalloon_Fly_State();
	virtual ~CKirbyBalloon_Fly_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyBalloon_Fly_State* Create();
	virtual void						Free() override;

};

END