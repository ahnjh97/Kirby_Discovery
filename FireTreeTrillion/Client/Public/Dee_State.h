#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)


class CDee_State abstract : public CFSM_State
{
protected:
	CDee_State();
	virtual ~CDee_State() = default;

	typedef struct
	{
		class CWaddleDee* pDee = {nullptr};
		class CTransform* pTransformCom = { nullptr };
		class CCharacterController* pController = { nullptr };

		class CKirby* pKirby = { nullptr };
		class CTransform* pKirbyTransformCom = { nullptr };

		_float3 vMyPos;
		_float3 vKirbyPos;

		_float fDistance;

	}BASE_INFO;

	_float	m_fInteractActionTime = { 0.f };
	_float	m_fDuration = { 0.f };

	void Setup_BaseInfo(BASE_INFO& _baseInfo, CGameObject* pGameObject);
	void System_Tick(_float fTimeDelta);

public:
	virtual void Free() override;
};


class CDee_Idle_State final : public CDee_State
{
private:
	CDee_Idle_State();
	virtual ~CDee_Idle_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CDee_Idle_State* Create();
	virtual void Free() override;
};

class CDee_Sit_State final : public CDee_State
{
private:
	CDee_Sit_State();
	virtual ~CDee_Sit_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CDee_Sit_State* Create();
	virtual void Free() override;
};

class CDee_Walk_State final : public CDee_State
{
private:
	CDee_Walk_State();
	virtual ~CDee_Walk_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Walk_State* Create();
	virtual void Free() override;
};

class CDee_Run_State final : public CDee_State
{
private:
	CDee_Run_State();
	virtual ~CDee_Run_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Run_State* Create();
	virtual void Free() override;
};

class CDee_NPC_State final : public CDee_State
{
private:
	CDee_NPC_State();
	virtual ~CDee_NPC_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CDee_NPC_State* Create();
	virtual void Free() override;
};

class CDee_Emotion_State final : public CDee_State
{
private:
	CDee_Emotion_State();
	virtual ~CDee_Emotion_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Emotion_State* Create();
	virtual void Free() override;
};

class CDee_Interact_State final : public CDee_State
{
private:
	CDee_Interact_State();
	virtual ~CDee_Interact_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Interact_State* Create();
	virtual void Free() override;
};

class CDee_Hungry_State final : public CDee_State
{
private:
	CDee_Hungry_State();
	virtual ~CDee_Hungry_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	_float	m_fStartMoveDelay = { 0.f };

public:
	static	CDee_Hungry_State* Create();
	virtual void Free() override;
};

class CDee_FlyStun_State final : public CDee_State
{
private:
	CDee_FlyStun_State();
	virtual ~CDee_FlyStun_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

	_uint m_iBounceCnt = { 1 };
	_float4 m_vRandomAxis = { 0.f, 0.f, 0.f, 0.f };

public:
	static	CDee_FlyStun_State* Create();
	virtual void Free() override;
};

class CDee_Panic_State final : public CDee_State
{
private:
	CDee_Panic_State();
	virtual ~CDee_Panic_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	_float		m_fSwitchDirTime = { 0.f };
	_float3		m_vDir = { 0.f, 0.f, 0.f };

public:
	static	CDee_Panic_State* Create();
	virtual void Free() override;
};

class CBattleDee_NearDeeDeeDee_State final : public CDee_State
{
private:
	CBattleDee_NearDeeDeeDee_State();
	virtual ~CBattleDee_NearDeeDeeDee_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	//_float3		m_vDir = { 0.f, 0.f, 0.f };

public:
	static	CBattleDee_NearDeeDeeDee_State* Create();
	virtual void Free() override;
};

class CDee_Sleep_State final : public CDee_State
{
private:
	CDee_Sleep_State();
	virtual ~CDee_Sleep_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Sleep_State* Create();
	virtual void Free() override;
};

END
