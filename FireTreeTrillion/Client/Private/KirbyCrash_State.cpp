#include "stdafx.h"
#include "KirbyCrash_State.h"
#include "Kirby_State_Function.h"


#pragma region Attack STATE

CKirbyCrash_Attack_State::CKirbyCrash_Attack_State()
{
}

void CKirbyCrash_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCrash_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


}

void CKirbyCrash_Attack_State::OnStateExit()
{
}

CKirbyCrash_Attack_State* CKirbyCrash_Attack_State::Create()
{
	CKirbyCrash_Attack_State* pInstance = new CKirbyCrash_Attack_State();
	return pInstance;
}

void CKirbyCrash_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region Big Attack STATE

CKirbyCrash_BigAttack_State::CKirbyCrash_BigAttack_State()
{
}

void CKirbyCrash_BigAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCrash_BigAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


}

void CKirbyCrash_BigAttack_State::OnStateExit()
{
}

CKirbyCrash_BigAttack_State* CKirbyCrash_BigAttack_State::Create()
{
	CKirbyCrash_BigAttack_State* pInstance = new CKirbyCrash_BigAttack_State();
	return pInstance;
}

void CKirbyCrash_BigAttack_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region Charge STATE

CKirbyCrash_Charge_State::CKirbyCrash_Charge_State()
{
}

void CKirbyCrash_Charge_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCrash_Charge_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();





}

void CKirbyCrash_Charge_State::OnStateExit()
{
}

CKirbyCrash_Charge_State* CKirbyCrash_Charge_State::Create()
{
	CKirbyCrash_Charge_State* pInstance = new CKirbyCrash_Charge_State();
	return pInstance;
}

void CKirbyCrash_Charge_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region Big Charge STATE

CKirbyCrash_BigCharge_State::CKirbyCrash_BigCharge_State()
{
}

void CKirbyCrash_BigCharge_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyCrash_BigCharge_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();



}

void CKirbyCrash_BigCharge_State::OnStateExit()
{
}

CKirbyCrash_BigCharge_State* CKirbyCrash_BigCharge_State::Create()
{
	CKirbyCrash_BigCharge_State* pInstance = new CKirbyCrash_BigCharge_State();
	return pInstance;
}

void CKirbyCrash_BigCharge_State::Free()
{
	__super::Free();
}

#pragma endregion
