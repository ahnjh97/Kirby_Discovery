#include "FSM_State.h"
#include "GameInstance.h"

CFSM_State::CFSM_State()
	: m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);

}

void CFSM_State::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}
