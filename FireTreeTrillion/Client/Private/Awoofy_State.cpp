#include "stdafx.h"
#include "Awoofy_State.h"
#include "Awoofy.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CAwoofy_Idle_State::CAwoofy_Idle_State()
{
}

void CAwoofy_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CAwoofy_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (m_pGameInstance->Get_KeyState(DIK_W, KEY_DOWN))
		pAwofy->Change_State(CAwoofy::AWOOFY_RUN, 60.f, true, true);

	if (m_pGameInstance->Get_KeyState(DIK_D, KEY_PRESS))
		pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);

	if (m_pGameInstance->Get_KeyState(DIK_A, KEY_PRESS))
		pTransformCom->Turn(XMVectorSet(0.f, -1.f, 0.f, 0.f), fTimeDelta);

	pController->FreeFall(pTransformCom, fTimeDelta, 0.5f);
}

void CAwoofy_Idle_State::OnStateExit()
{
}

CAwoofy_Idle_State* CAwoofy_Idle_State::Create()
{
	CAwoofy_Idle_State* pInstance = new CAwoofy_Idle_State();
	return pInstance;
}

void CAwoofy_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region RUN STATE
//*******************************************************
//						 RUN STATE
//*******************************************************
CAwoofy_Run_State::CAwoofy_Run_State()
{
}

void CAwoofy_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CAwoofy_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (m_pGameInstance->Get_KeyState(DIK_W, KEY_PRESS))
		pController->Move_Dir(pTransformCom, XMVector4Normalize(pTransformCom->Get_State(CTransform::STATE_LOOK)) * 0.1f, fTimeDelta);
	else
		pAwofy->Change_State(CAwoofy::AWOOFY_WAIT, 60.f, true, true);

	if (m_pGameInstance->Get_KeyState(DIK_D, KEY_PRESS))
		pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);

	if (m_pGameInstance->Get_KeyState(DIK_A, KEY_PRESS))
		pTransformCom->Turn(XMVectorSet(0.f, -1.f, 0.f, 0.f), fTimeDelta);

	pController->FreeFall(pTransformCom, fTimeDelta, 0.5f);
}

void CAwoofy_Run_State::OnStateExit()
{
}

CAwoofy_Run_State* CAwoofy_Run_State::Create()
{
	CAwoofy_Run_State* pInstance = new CAwoofy_Run_State();
	return pInstance;
}

void CAwoofy_Run_State::Free()
{
	__super::Free();
}

#pragma endregion