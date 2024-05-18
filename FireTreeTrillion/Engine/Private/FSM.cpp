#include "FSM.h"
#include "FSM_State.h"

string debug__[16] = { "ATTACK", "CLIMB", "IDLE", "INVENTORY", "JUMP", "JUMP_RUN"
						, "PLAY", "RUN", "TAKE"};

HRESULT CFSM::Initialize(void* pArg)
{
	FSM_INFO* FSM_Desc = (FSM_INFO*)pArg;
	m_iState = FSM_Desc->iState;
	m_pCurrent_State = Find_State(m_iState);
	return S_OK;
}


void CFSM::Update(_float fTimeDelta, _uint eState)
{
	if (m_pCurrent_State == nullptr)
		return;

	if (ChangeState(eState) == true)
		return;

	m_pCurrent_State->OnStateUpdate(fTimeDelta);
}


_bool CFSM::ChangeState(_uint iState)
{
	auto pNextState = Find_State(iState);
	CHECK_NULLPTR(pNextState);

	if (m_pCurrent_State == pNextState)
		return false;

	// ============ 상태가 변경되었다면 ============
	// 이전 상태 Terminate
	m_pCurrent_State->OnStateExit();
	pNextState->OnStateEnter();

	//printf("%s -> %s \n", debug__[m_iPreState].c_str(), debug__[iState].c_str());

	m_pCurrent_State = pNextState;
	m_iPreState = iState;
	return true;
}


void CFSM::Add_State(_uint iState, CFSM_State* pInitState)
{
	CHECK_NULLPTR(pInitState);
	m_mapFSM_State.emplace(iState, pInitState);
}


CFSM_State* CFSM::Find_State(_uint iState)
{
	auto iter = m_mapFSM_State.find(iState);
	if(iter == m_mapFSM_State.end()) return nullptr;
	return iter->second;
}


CFSM* CFSM::Create()
{
	return new CFSM();
}


void CFSM::Free()
{
	__super::Free();

	for (auto& pFSMstate : m_mapFSM_State)
		Safe_Release(pFSMstate.second);

	m_mapFSM_State.clear();
}

