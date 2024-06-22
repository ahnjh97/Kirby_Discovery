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

	void Setup_BaseInfo(BASE_INFO& _baseInfo, CGameObject* pGameObject);

public:
	virtual void Free() override;
};


//*********************************
//			IDLE STATE
//*********************************
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

//*********************************
//			MOVE STATE
//*********************************
class CDee_Move_State final : public CDee_State
{
private:
	CDee_Move_State();
	virtual ~CDee_Move_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Move_State* Create();
	virtual void Free() override;
};

//*********************************
//			EMOTION STATE
//*********************************
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

//*********************************
//			HUNGRY STATE
//*********************************
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


public:
	static	CDee_Hungry_State* Create();
	virtual void Free() override;
};

//*********************************
//			STUN STATE
//*********************************
class CDee_Stun_State final : public CDee_State
{
private:
	CDee_Stun_State();
	virtual ~CDee_Stun_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;


public:
	static	CDee_Stun_State* Create();
	virtual void Free() override;
};


END
