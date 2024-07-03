#include "stdafx.h"
#include "CKirbyDump_State.h"
#include "Kirby_State_Function.h"


#pragma region 덤프 아이들 상태

CKirbyDump_Idle_State::CKirbyDump_Idle_State()
{
}

void CKirbyDump_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyDump_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbyDump_Idle_State::OnStateExit()
{
}

CKirbyDump_Idle_State* CKirbyDump_Idle_State::Create()
{
    CKirbyDump_Idle_State* pInstance = new CKirbyDump_Idle_State();
    return pInstance;
}

void CKirbyDump_Idle_State::Free()
{
    __super::Free();
}

#pragma endregion

#pragma region 차량 운전 상태


CKirbyDump_Run_State::CKirbyDump_Run_State()
{
}

void CKirbyDump_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyDump_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKirbyDump_Run_State::OnStateExit()
{
}

CKirbyDump_Run_State* CKirbyDump_Run_State::Create()
{
    CKirbyDump_Run_State* pInstance = new CKirbyDump_Run_State();
    return pInstance;
}

void CKirbyDump_Run_State::Free()
{
    __super::Free();
}

#pragma endregion


#pragma region 차량 점프 상태

CKirbyDump_Jump_State::CKirbyDump_Jump_State()
{
}

void CKirbyDump_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

}

void CKirbyDump_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{


}

void CKirbyDump_Jump_State::OnStateExit()
{
}

CKirbyDump_Jump_State* CKirbyDump_Jump_State::Create()
{
    CKirbyDump_Jump_State* pInstance = new CKirbyDump_Jump_State();
    return pInstance;
}

void CKirbyDump_Jump_State::Free()
{
    __super::Free();
}

#pragma endregion
