#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

//*********************************
//			WAIT STATE
//*********************************
class CSurprisedBoard_Wait_State final : public CFSM_State
{
private:
	CSurprisedBoard_Wait_State();
	virtual ~CSurprisedBoard_Wait_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

private:
	enum DIR { LEFT, RIGHT, DIR_END };
	_uint	m_uDir = LEFT;

public:
	static	CSurprisedBoard_Wait_State* Create();
	virtual void Free() override;
};


//*********************************
//			POPOUT STATE
//*********************************
class CSurprisedBoard_Popout_State final : public CFSM_State
{
private:
	CSurprisedBoard_Popout_State();
	virtual ~CSurprisedBoard_Popout_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CSurprisedBoard_Popout_State* Create();
	virtual void Free() override;
};


//*********************************
//			RETURN STATE
//*********************************
class CSurprisedBoard_Return_State final : public CFSM_State
{
private:
	CSurprisedBoard_Return_State();
	virtual ~CSurprisedBoard_Return_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	static	CSurprisedBoard_Return_State* Create();
	virtual void Free() override;
};

END

