#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

static _uint m_iCnt = { 0 };

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

private:
	_bool	m_bSound1 = { false };
	_bool	m_bSound2 = { false };

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
	_vector m_vReturnPos = {};

	_float	m_fDelayTime = { 0.f };

	_uint	m_iMeteor = { 0 };
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

private:
	_float	m_fSpeed = { 0.f };
	_bool	m_bSound = { false };

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
	_float	m_fSpeed = { 0.f };
	_bool	m_bChain = { false };
	_bool	m_bSound = { false };

public:
	static	CFinalBoss_Slash_State* Create();
	virtual void Free() override;
};


//*********************************
//			Chain STATE
//*********************************
class CFinalBoss_Chain_State final : public CFSM_State
{
private:
	CFinalBoss_Chain_State();
	virtual ~CFinalBoss_Chain_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };

public:
	static	CFinalBoss_Chain_State* Create();
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
	_bool	m_bItem = { false };
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

private:
	_float	m_fTimeDelta = { 0.f };
	_float	m_fSpeed = { 0.f };
	_bool	m_bItem = { false };

public:
	static	CFinalBoss_Thrust_State* Create();
	virtual void Free() override;
};


//*********************************
//			LASER STATE
//*********************************
class CFinalBoss_Laser_State final : public CFSM_State
{
private:
	CFinalBoss_Laser_State();
	virtual ~CFinalBoss_Laser_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };
	_bool	m_bEffectTrigger = { true };

public:
	static	CFinalBoss_Laser_State* Create();
	virtual void Free() override;
};


//*********************************
//			SPIKE STATE
//*********************************
class CFinalBoss_Spike_State final : public CFSM_State
{
private:
	CFinalBoss_Spike_State();
	virtual ~CFinalBoss_Spike_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fTimeDelta = { 0.f };
	_float	m_fLifeTime = { 0.f };
	_float	m_arrRight[5] = { 6.f, 6.f, 0.f, -6.f, -6.f };
	_float	m_arrLook[5] = { 6.f, -2.f, -6.f, -2.f, 6.f };
	_uint	m_iCnt = { 0 };

public:
	static	CFinalBoss_Spike_State* Create();
	virtual void Free() override;
};


//*********************************
//			JUMP STATE
//*********************************
class CFinalBoss_Jump_State final : public CFSM_State
{
private:
	CFinalBoss_Jump_State();
	virtual ~CFinalBoss_Jump_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Jump_State* Create();
	virtual void Free() override;
};


//*********************************
//			METEOR STATE
//*********************************
class CFinalBoss_Meteor_State final : public CFSM_State
{
private:
	CFinalBoss_Meteor_State();
	virtual ~CFinalBoss_Meteor_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CFinalBoss_Meteor_State* Create();
	virtual void Free() override;
};


//*********************************
//			ROAR STATE
//*********************************
class CFinalBoss_Roar_State final : public CFSM_State
{
private:
	CFinalBoss_Roar_State();
	virtual ~CFinalBoss_Roar_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_bool	m_bShake = { false };

public:
	static	CFinalBoss_Roar_State* Create();
	virtual void Free() override;
};


//*********************************
//			DAMAGE STATE
//*********************************
class CFinalBoss_Damage_State final : public CFSM_State
{
private:
	CFinalBoss_Damage_State();
	virtual ~CFinalBoss_Damage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };

public:
	static	CFinalBoss_Damage_State* Create();
	virtual void Free() override;
};


//*********************************
//			RECOVERY STATE
//*********************************
class CFinalBoss_Recovery_State final : public CFSM_State
{
private:
	CFinalBoss_Recovery_State();
	virtual ~CFinalBoss_Recovery_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float	m_fSpeed = { 0.f };
	_float	m_fSecondSpeed = { 0.f };
	_float	m_fDelayTime = { 0.f };
	_float	m_fItemCycle = { 0.f };
	_float	m_fItemTime = { 0.f };

	_bool	m_bEffect = { false };
	_bool	m_bMove = { false };
	_bool	m_bStart = { false };

public:
	static	CFinalBoss_Recovery_State* Create();
	virtual void Free() override;
};


//*********************************
//			LASTDAMAGE STATE
//*********************************
class CFinalBoss_LastDamage_State final : public CFSM_State
{
private:
	CFinalBoss_LastDamage_State();
	virtual ~CFinalBoss_LastDamage_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_bool	m_bSound = { false };

public:
	static	CFinalBoss_LastDamage_State* Create();
	virtual void Free() override;
};

END

