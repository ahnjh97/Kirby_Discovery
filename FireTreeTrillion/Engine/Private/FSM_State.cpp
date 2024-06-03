#include "FSM_State.h"
#include "GameInstance.h"

CFSM_State::CFSM_State()
	: m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);

}

void CFSM_State::OnStateEnter(class CModel* _pModel, _uint iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	_pModel->Set_Animation(iAnimIndex - _iOffSet, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CFSM_State::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}

