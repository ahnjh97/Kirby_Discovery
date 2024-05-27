#include "stdafx.h"
#include "Awoofy_State.h"
#include "Awoofy.h"
#include "Kirby.h"

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
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));


	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	if(6.f > fDistance)
	{
		_vector vOrginLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
		_vector vTargetLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

		pAwoofy->Change_State(CAwoofy::AWOOFY_FIND, 40.f, false, true);
	}

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
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	m_fTimeDelta += fTimeDelta;

	if(2.f > m_fTimeDelta)
		pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 0.1f , fTimeDelta);
	else
	{
		m_fTimeDelta = 0.f;
		pAwoofy->Change_State(CAwoofy::AWOOFY_BRAKE, 40.f, false, true);
	}
	//if (m_pGameInstance->Get_KeyState(DIK_W, KEY_PRESS))
	//	pController->Move_Dir(pTransformCom, XMVector4Normalize(pTransformCom->Get_State(CTransform::STATE_LOOK)) * 0.1f, fTimeDelta);
	//else
	//	pAwofy->Change_State(CAwoofy::AWOOFY_GROOMING, 40.f, true, true);

	//if (m_pGameInstance->Get_KeyState(DIK_D, KEY_PRESS))
	//	pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);

	//if (m_pGameInstance->Get_KeyState(DIK_A, KEY_PRESS))
	//	pTransformCom->Turn(XMVectorSet(0.f, -1.f, 0.f, 0.f), fTimeDelta);

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


#pragma region FIND STATE
//*******************************************************
//						 FIND STATE
//*******************************************************
CAwoofy_Find_State::CAwoofy_Find_State()
{
}

void CAwoofy_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CAwoofy_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

	if (true == pAwoofy->IsAnimFinished())
		pAwoofy->Change_State(CAwoofy::AWOOFY_RUN, 40.f, true, true);
}

void CAwoofy_Find_State::OnStateExit()
{
}

CAwoofy_Find_State* CAwoofy_Find_State::Create()
{
	CAwoofy_Find_State* pInstance = new CAwoofy_Find_State();
	return pInstance;
}

void CAwoofy_Find_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region FIND STATE
//*******************************************************
//						 FIND STATE
//*******************************************************
CAwoofy_LookAround_State::CAwoofy_LookAround_State()
{
}

void CAwoofy_LookAround_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CAwoofy_LookAround_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	pController->FreeFall(pTransformCom, fTimeDelta, 0.5f);

	if(true == pAwoofy->IsAnimFinished())
		pAwoofy->Change_State(CAwoofy::AWOOFY_GROOMING, 45.f, false, true);
}

void CAwoofy_LookAround_State::OnStateExit()
{
}

CAwoofy_LookAround_State* CAwoofy_LookAround_State::Create()
{
	CAwoofy_LookAround_State* pInstance = new CAwoofy_LookAround_State();
	return pInstance;
}

void CAwoofy_LookAround_State::Free()
{
	__super::Free();
}

#pragma endregion

CAwoofy_Brake_State::CAwoofy_Brake_State()
{
}

void CAwoofy_Brake_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CAwoofy_Brake_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_float fDeceleration = m_fSpeed * m_fSpeed;

	if (0.f < m_fSpeed)
		m_fSpeed -= fTimeDelta * 0.3f;
	else
		m_fSpeed = 1.f;

	pController->FreeFall(pTransformCom, fTimeDelta, 0.5f);

	if (true == pAwoofy->IsAnimFinished())
		pAwoofy->Change_State(CAwoofy::AWOOFY_LOOKAROUND, 45.f, false, true);
	else
		pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 0.1f * fDeceleration, fTimeDelta);
}

void CAwoofy_Brake_State::OnStateExit()
{
}

CAwoofy_Brake_State* CAwoofy_Brake_State::Create()
{
	CAwoofy_Brake_State* pInstance = new CAwoofy_Brake_State();
	return pInstance;
}

void CAwoofy_Brake_State::Free()
{
	__super::Free();
}
