#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

// *********************** APPEAR1 ***********************
class CSimba_Appear1 final : public CFSM_State
{
private:
	CSimba_Appear1(CCharacterController* pController, CTransform* pTransform) 
		{ m_pController = pController; m_pTransform = pTransform; Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); }
	virtual ~CSimba_Appear1() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController*	m_pController = { nullptr };
	CTransform*				m_pTransform = { nullptr };

public:
	static	CSimba_Appear1* Create(CCharacterController* pController, CTransform* pTransform) {  return new CSimba_Appear1(pController, pTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); }
};

// *********************** APPEAR2 ***********************
class CSimba_Appear2 final : public CFSM_State
{
private:
	CSimba_Appear2(CCharacterController* pController, CTransform* pTransform) 
		{ m_pController = pController; m_pTransform = pTransform; Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); }
	virtual ~CSimba_Appear2() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController*	m_pController = { nullptr };
	CTransform*				m_pTransform = { nullptr };

	_float m_fTime = {};

public:
	static	CSimba_Appear2* Create(CCharacterController* pController, CTransform* pTransform) { return new CSimba_Appear2(pController, pTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); }
};

// *********************** WALK ***********************
class CSimba_Walk final : public CFSM_State
{
private:
	CSimba_Walk(CCharacterController* pController, CTransform* pTransform) 
		{ m_pController = pController; m_pTransform = pTransform; Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); }
	virtual ~CSimba_Walk() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController*	m_pController = { nullptr };
	CTransform*				m_pTransform = { nullptr };

public:
	static	CSimba_Walk* Create(CCharacterController* pController, CTransform* pTransform) { return new CSimba_Walk(pController, pTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); }
};

// *********************** QUICKCLAW ***********************
class CSimba_QuickClaw final : public CFSM_State
{
private:
	CSimba_QuickClaw(CCharacterController* pController, CTransform* pTransform)
		{ m_pController = pController; m_pTransform = pTransform; Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); }
	virtual ~CSimba_QuickClaw() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };

public:
	static	CSimba_QuickClaw* Create(CCharacterController* pController, CTransform* pTransform) { return new CSimba_QuickClaw(pController, pTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); }
};
END