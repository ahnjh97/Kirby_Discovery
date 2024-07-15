#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"

BEGIN(Client)

// *********************** APPEAR1 ***********************
class CSimba_Appear1 final : public CFSM_State
{
private:
	CSimba_Appear1(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Appear1() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController*	m_pController = { nullptr };
	CTransform*				m_pTransform = { nullptr };
	CGameObject*			m_pKirby = { nullptr };
	CTransform*				m_pKirbyTransform = { nullptr };

public:
	static	CSimba_Appear1* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{  return new CSimba_Appear1(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** APPEAR2 ***********************
class CSimba_Appear2 final : public CFSM_State
{
private:
	CSimba_Appear2(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Appear2() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

	_float m_fTime = {};

public:
	static	CSimba_Appear2* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform) 
	{ return new CSimba_Appear2(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** WALK ***********************
class CSimba_Walk final : public CFSM_State
{
private:
	CSimba_Walk(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Walk() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController*	m_pController = { nullptr };
	CTransform*				m_pTransform = { nullptr };
	CGameObject*			m_pKirby = { nullptr };
	CTransform*				m_pKirbyTransform = { nullptr };

public:
	static	CSimba_Walk* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{ return new CSimba_Walk(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** QUICKCLAW ***********************
class CSimba_QuickClaw final : public CFSM_State
{
private:
	CSimba_QuickClaw(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_QuickClaw() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_QuickClaw* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{ return new CSimba_QuickClaw(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** FinalCrusher ***********************
class CSimba_FinalCrusher final : public CFSM_State
{
private:
	CSimba_FinalCrusher(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_FinalCrusher() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_FinalCrusher* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{ return new CSimba_FinalCrusher(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** DoubleClaw ***********************
class CSimba_DoubleClaw final : public CFSM_State
{
private:
	CSimba_DoubleClaw(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_DoubleClaw() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_DoubleClaw* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{ return new CSimba_DoubleClaw(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** Jump ***********************
class CSimba_Jump final : public CFSM_State
{
private:
	CSimba_Jump(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Jump() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

	_float m_fJumpPower = {};

public:
	static	CSimba_Jump* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform) 
		{ return new CSimba_Jump(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** BackStep ***********************
class CSimba_BackStep final : public CFSM_State
{
private:
	CSimba_BackStep(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_BackStep() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

	_float m_fJumpPower = {};

public:
	static	CSimba_BackStep* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
				{ return new CSimba_BackStep(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** AttackJump ***********************
class CSimba_AttackJump final : public CFSM_State
{
private:
	CSimba_AttackJump(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_AttackJump() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

	_float m_fJumpPower = {};

public:
	static	CSimba_AttackJump* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
			{ return new CSimba_AttackJump(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** Damage ***********************
class CSimba_Damage final : public CFSM_State
{
private:
	CSimba_Damage(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Damage() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_Damage* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_Damage(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** Roar ***********************
class CSimba_Roar final : public CFSM_State
{
private:
	CSimba_Roar(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Roar() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_Roar* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_Roar(pController, pTransform, pKirby, pKirbyTransform);	}
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** BiteRushJump ***********************
class CSimba_BiteRushJump final : public CFSM_State
{
private:
	CSimba_BiteRushJump(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_BiteRushJump() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_BiteRushJump* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_BiteRushJump(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** DimensionClaw ***********************
class CSimba_DimensionClaw final : public CFSM_State
{
private:
	CSimba_DimensionClaw(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_DimensionClaw() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_DimensionClaw* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_DimensionClaw(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** BiteRush ***********************
class CSimba_BiteRush final : public CFSM_State
{
private:
	CSimba_BiteRush(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_BiteRush() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_BiteRush* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_BiteRush(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** DimensionLaser ***********************
class CSimba_DimensionLaser final : public CFSM_State
{
private:
	CSimba_DimensionLaser(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_DimensionLaser() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

public:
	static	CSimba_DimensionLaser* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_DimensionLaser(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

// *********************** Death ***********************
class CSimba_Death final : public CFSM_State
{
private:
	CSimba_Death(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
	{
		m_pController = pController; m_pTransform = pTransform; m_pKirby = pKirby; m_pKirbyTransform = pKirbyTransform;
		Safe_AddRef(m_pController); Safe_AddRef(m_pTransform); Safe_AddRef(m_pKirby); Safe_AddRef(m_pKirbyTransform);
	}
	virtual ~CSimba_Death() = default;

public:
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset) override;
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override {}

private:
	CCharacterController* m_pController = { nullptr };
	CTransform* m_pTransform = { nullptr };
	CGameObject* m_pKirby = { nullptr };
	CTransform* m_pKirbyTransform = { nullptr };

	_bool m_bCageNotified = { false };

public:
	static	CSimba_Death* Create(CCharacterController* pController, CTransform* pTransform, CGameObject* pKirby, CTransform* pKirbyTransform)
		{ return new CSimba_Death(pController, pTransform, pKirby, pKirbyTransform); }
	virtual void Free() override { __super::Free(); Safe_Release(m_pController); Safe_Release(m_pTransform); Safe_Release(m_pKirby); Safe_Release(m_pKirbyTransform); }
};

END