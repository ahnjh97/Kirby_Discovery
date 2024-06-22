#include "stdafx.h"
#include "Dee_State.h"
#include "WaddleDee.h"
#include "Kirby.h"


CDee_State::CDee_State()
{
}

void CDee_State::Free()
{
	__super::Free();
}

void CDee_State::Setup_BaseInfo(BASE_INFO& _baseInfo, CGameObject* pGameObject)
{
	_baseInfo.pDee = static_cast<CWaddleDee*>(pGameObject);
	_baseInfo.pTransformCom = pGameObject->Get_TransformCom();
	_baseInfo.pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_baseInfo.pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	_baseInfo.pKirbyTransformCom = _baseInfo.pKirby->Get_TransformCom();

	_baseInfo.vMyPos = (_float3)_baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
	_baseInfo.vKirbyPos = (_float3)_baseInfo.pKirbyTransformCom->Get_State(CTransform::STATE_POSITION);

	_baseInfo.fDistance = (_baseInfo.vMyPos - _baseInfo.vKirbyPos).Length();


}


#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CDee_Idle_State::CDee_Idle_State()
{
}

void CDee_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);


}


void CDee_Idle_State::OnStateExit()
{
}

CDee_Idle_State* CDee_Idle_State::Create()
{
	CDee_Idle_State* pInstance = new CDee_Idle_State();
	return pInstance;
}

void CDee_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region WALK STATE
//*********************************
//			 WALK STATE
//*********************************
CDee_Move_State::CDee_Move_State()
{
}

void CDee_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CWaddleDee* pDee = static_cast<CWaddleDee*>(pGameObject);
}

void CDee_Move_State::OnStateExit()
{
}

CDee_Move_State* CDee_Move_State::Create()
{
	CDee_Move_State* pInstance = new CDee_Move_State();
	return pInstance;
}

void CDee_Move_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region EMOTION STATE
//*********************************
//			 EMOTION STATE
//*********************************
CDee_Emotion_State::CDee_Emotion_State()
{
}

void CDee_Emotion_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Emotion_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CWaddleDee* pDee = static_cast<CWaddleDee*>(pGameObject);
}

void CDee_Emotion_State::OnStateExit()
{
}

CDee_Emotion_State* CDee_Emotion_State::Create()
{
	CDee_Emotion_State* pInstance = new CDee_Emotion_State();
	return pInstance;
}

void CDee_Emotion_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region HUNGRY STATE
//*********************************
//			 HUNGRY STATE
//*********************************
CDee_Hungry_State::CDee_Hungry_State()
{
}

void CDee_Hungry_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Hungry_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CWaddleDee* pDee = static_cast<CWaddleDee*>(pGameObject);
}

void CDee_Hungry_State::OnStateExit()
{
}

CDee_Hungry_State* CDee_Hungry_State::Create()
{
	CDee_Hungry_State* pInstance = new CDee_Hungry_State();
	return pInstance;
}

void CDee_Hungry_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region STUN STATE
//*********************************
//			 STUN STATE
//*********************************
CDee_Stun_State::CDee_Stun_State()
{
}

void CDee_Stun_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Stun_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CWaddleDee* pDee = static_cast<CWaddleDee*>(pGameObject);
}

void CDee_Stun_State::OnStateExit()
{
}

CDee_Stun_State* CDee_Stun_State::Create()
{
	CDee_Stun_State* pInstance = new CDee_Stun_State();
	return pInstance;
}

void CDee_Stun_State::Free()
{
	__super::Free();
}

#pragma endregion

