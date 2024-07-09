#include "stdafx.h"
#include "Simba_State.h"

void CSimba_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
}

void CSimba_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CSimba_Idle_State::OnStateExit()
{
}

CSimba_Idle_State* CSimba_Idle_State::Create()
{
	return nullptr;
}

void CSimba_Idle_State::Free()
{
}
