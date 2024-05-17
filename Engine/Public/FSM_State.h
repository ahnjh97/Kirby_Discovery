#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CFSM_State abstract : public CBase
{
protected:
	CFSM_State();
	virtual ~CFSM_State() = default;

public:
	// 상태 진입했을 때 처음만 호출
	virtual void OnStateEnter() {}
	// 상태 진입되어 있는 상태에서 매 tick마다 호출
	virtual void OnStateUpdate(_float fTimeDelta) {}
	// 상태가 변경될 때 호출
	virtual void OnStateExit() {}

protected:
	class CGameInstance* m_pGameInstance = { nullptr };

public:
	virtual void Free() override;


};

END

