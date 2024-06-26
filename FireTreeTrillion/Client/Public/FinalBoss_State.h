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

private:
	_vector m_vLook = {};
	_float m_fDelayTime = {};
	//_vector	m_ShortestPos = {};

public:
	_vector RotateGlide(_fvector vPivotPos, _fvector _vRotatePos, _float fAngle);
//	void GlideToRallyPoint(class CFinalBoss* pFinalBoss, class CTransform* pTransformCom);

public:
	static	CFinalBoss_Idle_State* Create();
	virtual void Free() override;
};


//*********************************
//			STAB STATE
//*********************************
class CFinalBoss_Stab_State final : public CFSM_State
{
private:
	CFinalBoss_Stab_State();
	virtual ~CFinalBoss_Stab_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Stab_State* Create();
	virtual void Free() override;
};


//*********************************
//			GLIDEBACK STATE
//*********************************
class CFinalBoss_GlideBack_State final : public CFSM_State
{
private:
	CFinalBoss_GlideBack_State();
	virtual ~CFinalBoss_GlideBack_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };
	_float	m_fTimeDelta = { 0.f };

public:
	static	CFinalBoss_GlideBack_State* Create();
	virtual void Free() override;
};


//*********************************
//			GLIDE STATE
//*********************************
class CFinalBoss_Glide_State final : public CFSM_State
{
private:
	CFinalBoss_Glide_State();
	virtual ~CFinalBoss_Glide_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CFinalBoss_Glide_State* Create();
	virtual void Free() override;
};


//*********************************
//			SLASH STATE
//*********************************
class CFinalBoss_Slash_State final : public CFSM_State
{
private:
	CFinalBoss_Slash_State();
	virtual ~CFinalBoss_Slash_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CFinalBoss_Slash_State* Create();
	virtual void Free() override;
};


//*********************************
//			SLASHEND STATE
//*********************************
class CFinalBoss_SlashEnd_State final : public CFSM_State
{
private:
	CFinalBoss_SlashEnd_State();
	virtual ~CFinalBoss_SlashEnd_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CFinalBoss_SlashEnd_State* Create();
	virtual void Free() override;
};


//*********************************
//			SWING STATE
//*********************************
class CFinalBoss_Swing_State final : public CFSM_State
{
private:
	CFinalBoss_Swing_State();
	virtual ~CFinalBoss_Swing_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };
	_float	m_fTimeDelta = { 0.f };

public:
	static	CFinalBoss_Swing_State* Create();
	virtual void Free() override;
};


//*********************************
//			ARROW STATE
//*********************************
class CFinalBoss_Arrow_State final : public CFSM_State
{
private:
	CFinalBoss_Arrow_State();
	virtual ~CFinalBoss_Arrow_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Arrow_State* Create();
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

