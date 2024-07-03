#include "stdafx.h"
#include "KirbyContents_State.h"
#include "Kirby_State_Function.h"
#include "Ability.h"


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
		DESC(m_eMouthState) = CKirby::MOUTH_HAPPY;
		CTransform* pCameraTransform = pCamera->Get_TransformCom();
		_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
		_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
		DESC(m_vTargetDir) = vCamLook * -1.f;
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);


		if (pKirby->isAnimFinish())
		{
			DESC(m_eMouthState) = CKirby::MOUTH_IDLE;
			m_pGameInstance->Set_BlackBackGround(false);
			m_pGameInstance->Set_SecondTimerRatio(1.f);

			static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr())->Set_FOVY(30.f);

			if (pKirby->Get_AbilityType() == ABILITY_SWORD)
				pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
			else if (pKirby->Get_AbilityType() == ABILITY_BOMB)
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			else if (pKirby->Get_AbilityType() == ABILITY_HAMMER)
				pKirby->Change_State(CKirby::HAMMERSTATE_IDLE, 60.f, true, true, CKirby::BODY_HAMMER, CKirby::OFFSET_HAMMER);

			return;
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
			HRESULT hr = S_OK;

			CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
			AbilityItemDesc.vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			AbilityItemDesc.eAbilityType = pKirby->Get_AbilityType();
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
			CHECK_FAILED(hr);

			pKirby->Set_AbilityType(ABILITY_DEFAULT);
			
			if (JoyStick_controller(Kirbydesc, pCamera) == true)
				pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);
			else
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);

			return;
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