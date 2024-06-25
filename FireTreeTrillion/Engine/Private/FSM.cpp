#include "FSM.h"
#include "FSM_State.h"
#include "Model.h"

HRESULT CFSM::Initialize(void* pArg)
{
	FSM_INFO* FSM_Desc = (FSM_INFO*)pArg;
	m_iState = FSM_Desc->iState;
	m_uNumModel = FSM_Desc->uNumModel;
	CModel** pModel = FSM_Desc->pModel;

	for (_uint i = 0; i < m_uNumModel; ++i) 
	{
		m_vecModels.push_back(pModel[i]);
		Safe_AddRef(pModel[i]);
	}

	m_pCurrent_State = Find_State(m_iState);

	if (m_pCurrent_State == nullptr)
		return E_FAIL;

	return S_OK;
}

void CFSM::Update(class CGameObject* pGameObject, _float fTimeDelta)
{
	if (m_pCurrent_State == nullptr)
		return;

	m_pCurrent_State->OnStateUpdate(pGameObject, fTimeDelta);
}

/// <summary>
/// 상태를 변경해주는 함수
/// </summary>
/// <param name="iState"> 변경하기 위한 상태값(enum값) </param>
/// <param name="_fAnimSpeed"> 변경하는 상태의 애니메이션 속도 </param>
/// <param name="_bLoop"> 루프 유무 </param>
/// <param name="_bInterpolation">보간 유무 </param>
/// <returns = "false"> 바꾸고자 하는 상태가 현 상태일 경우 </returns>
_bool CFSM::ChangeState(_uint iState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _uIndex, _uint _uOffSet)
{
	auto pNextState = Find_State(iState);
	CHECK_NULLPTR(pNextState);

	if (m_pCurrent_State == pNextState)
		return false;

	// ============ 상태가 변경되었다면 ============
	// 이전 상태 Terminate
	m_pCurrent_State->OnStateExit();

	pNextState->OnStateEnter(m_vecModels[_uIndex], iState, _fAnimSpeed, _bLoop, _bInterpolation, _uOffSet);

	m_iState = iState;
	m_pCurrent_State = pNextState;
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

	for (auto& pModel : m_vecModels)
		Safe_Release(pModel);
}
