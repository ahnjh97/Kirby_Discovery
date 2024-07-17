#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"


BEGIN(Client)

// 커비의 IDLE 상태를 통제한다.

class CKirbyDefault_Idle_State final : public CFSM_State
{
private:
	CKirbyDefault_Idle_State();
	virtual ~CKirbyDefault_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	_bool Key_Z(class CGameObject* pGameObject, _float fTimeDelta);
	_bool Key_X(class CGameObject* pGameObject, _float fTimeDelta);
	_bool Key_C(class CGameObject* pGameObject, _float fTimeDelta);
	_bool Key_V(class CGameObject* pGameObject, _float fTimeDelta);
	_bool Key_Happy(class CGameObject* pGameObject, _float fTimeDelta);

	virtual void OnStateExit()														override;

public:
	static	CKirbyDefault_Idle_State*	Create();
	virtual void						Free() override;

	_float m_fIdleStreachTime = { 0.f };
	_uint m_iIdleChoose = { 0 };

};




// 커비의 RUN 상태를 통제한다.

class CKirbyDefault_Run_State final : public CFSM_State
{
private:
	CKirbyDefault_Run_State();
	virtual ~CKirbyDefault_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CKirbyDefault_Run_State* Create();
	virtual void					 Free() override;

	_float	m_fRunStartTime = { 0.f };


};





// 커비의 점프 상태를 통제한다.

class CKirbyDefault_Jump_State final : public CFSM_State
{
private:
	CKirbyDefault_Jump_State();
	virtual ~CKirbyDefault_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	_bool Key_X(class CGameObject* pGameObject, _float fTimeDelta);
	_bool Key_C(class CGameObject* pGameObject, _float fTimeDelta);


public:
	static	CKirbyDefault_Jump_State* Create();
	virtual void					  Free() override;

	_float	m_fChangeRunTime = { 0.f };
	_float	m_fFallTime = { 0.f };
};



// 커비의 가드 상태 및 파쿠르를 통제한다.

class CKirbyDefault_Guard_State final : public CFSM_State
{
private:
	CKirbyDefault_Guard_State();
	virtual ~CKirbyDefault_Guard_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

public:
	static	CKirbyDefault_Guard_State* Create();
	virtual void						Free() override;
};



// 커비의 슬라이딩 상태를 통제한다.

class CKirbyDefault_Slide_State final : public CFSM_State
{
private:
	CKirbyDefault_Slide_State();
	virtual ~CKirbyDefault_Slide_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

public:
	_float	m_fEffectTime = { 0.f };
	static	CKirbyDefault_Slide_State* Create();
	virtual void						Free() override;
};





// 커비의 감정표현 상태를 통제한다.

class CKirbyDefault_Happy_State final : public CFSM_State
{
private:
	CKirbyDefault_Happy_State();
	virtual ~CKirbyDefault_Happy_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

	_float	m_fHappyTime = { 0.f };

public:
	static	CKirbyDefault_Happy_State* Create();
	virtual void						Free() override;
};



// 커비의 사다리 상태를 통제한다.

class CKirbyDefault_Ladder_State final : public CFSM_State
{
private:
	CKirbyDefault_Ladder_State();
	virtual ~CKirbyDefault_Ladder_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;

	virtual void OnStateExit()														override;

public:
	_float4	vLadderDir = { 0.f, 1.f, 0.f, 0.f };
	static	CKirbyDefault_Ladder_State* Create();
	virtual void						Free() override;
};


END