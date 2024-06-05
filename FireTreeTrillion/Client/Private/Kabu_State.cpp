#include "stdafx.h"
#include "Kabu_State.h"
#include "Kabu.h"

CKabu_Idle_State::CKabu_Idle_State()
{
}

void CKabu_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CKabu_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKabu* pKabu = static_cast<CKabu*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));


}

void CKabu_Idle_State::OnStateExit()
{
}

CKabu_Idle_State* CKabu_Idle_State::Create()
{
	CKabu_Idle_State* pInstance = new CKabu_Idle_State();
	return pInstance;
}

void CKabu_Idle_State::Free()
{
	__super::Free();
}
