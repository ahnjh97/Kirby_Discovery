#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

// APPEAR1
class CSimba_Appear1 final : public CFSM_State
{
private:
	CSimba_Appear1(CCharacterController* pController) {  m_pController = pController; Safe_AddRef(m_pController); }
	virtual ~CSimba_Appear1() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override
			{ __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset); }
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };

public:
	static	CSimba_Appear1* Create(CCharacterController* pController) {  return new CSimba_Appear1(pController); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); }
};

// APPEAR2
class CSimba_Appear2 final : public CFSM_State
{
private:
	CSimba_Appear2(CCharacterController* pController) { m_pController = pController; Safe_AddRef(m_pController); }
	virtual ~CSimba_Appear2() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override
			{ __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset); m_fTime = 0.f; }
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };

	_float m_fTime = {};

public:
	static	CSimba_Appear2* Create(CCharacterController* pController) { return new CSimba_Appear2(pController); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); }
};

// IDLE 
class CSimba_Walk final : public CFSM_State
{
private:
	CSimba_Walk(CCharacterController* pController) { m_pController = pController; Safe_AddRef(m_pController); }
	virtual ~CSimba_Walk() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override
			{ __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset); }
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };

public:
	static	CSimba_Walk* Create(CCharacterController* pController) { return new CSimba_Walk(pController); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); }
};

END



