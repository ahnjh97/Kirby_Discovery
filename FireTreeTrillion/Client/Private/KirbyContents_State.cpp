#include "stdafx.h"
#include "KirbyContents_State.h"
#include "Kirby_State_Function.h"


#pragma region GET_STATE

CKirbyGet_State::CKirbyGet_State()
{
}

void CKirbyGet_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	if(_iAnimIndex == CKirby::STATE_ABILITYDUMP)
		LadderStart_FX(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0)->Get_TransformCom());
}

void CKirbyGet_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{


	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// 능력을 획득 하였을 때 나오는 애니메이션이다.
	if (pKirby->Get_State() == CKirby::STATE_GETABILITY)
	{
		CTransform* pCameraTransform = pCamera->Get_TransformCom();
		_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
		_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
		DESC(m_vTargetDir) = vCamLook * -1.f;
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);


		if (pKirby->isAnimFinish())
		{
			m_pGameInstance->Set_BlackBackGround(false);
			m_pGameInstance->Set_SecondTimerRatio(1.f);


			if (pKirby->Get_AbilityType() == ABILITY_SWORD)
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			if (pKirby->Get_AbilityType() == ABILITY_BOMB)
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);

			static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr())->Zoom(0.f);

		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_ITEMGET)
	{

	}
	else if (pKirby->Get_State() == CKirby::STATE_ITENGETWAIT)
	{

	}
	else if (pKirby->Get_State() == CKirby::STATE_ABILITYDUMP)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
			Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (pKirby->isAnimFinish())
		{
			// 어빌리티 타입에 맞는 아이템 드랍. (미구현)

			pKirby->Set_AbilityType(ABILITY_DEFAULT);
			
			if (JoyStick_controller(Kirbydesc, pCamera) == true)
				pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);
			else
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
		}
	}
}

void CKirbyGet_State::OnStateExit()
{


}


CKirbyGet_State* CKirbyGet_State::Create()
{
	CKirbyGet_State* pInstance = new CKirbyGet_State();
	return pInstance;
}

void CKirbyGet_State::Free()
{
	__super::Free();
}

#pragma endregion