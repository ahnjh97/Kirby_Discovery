#pragma once
#include "Client_Defines.h"
#include "FSM_State.h"


BEGIN(Client)

class CKirbyHammer_Attack_State final : public CFSM_State
{
private:
	CKirbyHammer_Attack_State();
	virtual ~CKirbyHammer_Attack_State() = default;

	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet) override;
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta)	override;
	virtual void OnStateExit()														override;

public:
	_float	m_fAttackJumpTime = { 0.f };
	_bool	m_bAttackJumpTrigger = { true };

	static	CKirbyHammer_Attack_State* Create();
	virtual void						Free() override;

};

END