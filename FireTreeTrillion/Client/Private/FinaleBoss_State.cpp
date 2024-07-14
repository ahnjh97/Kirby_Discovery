#include "stdafx.h"
#include "FinaleBoss_State.h"
#include "FinaleBoss.h"
#include "FinaleKirby.h"
#include "FinaleCut_ControlCenter.h"
#include "Camera_Main.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CFinaleBoss_Idle_State::CFinaleBoss_Idle_State()
{
}

void CFinaleBoss_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinaleBoss_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinaleBoss* pFinaleBoss = static_cast<CFinaleBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CFinaleCut_ControlCenter* pCenter = static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));

	CFinaleKirby* pFinaleKirby = static_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pFinaleKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));
	_float fMaxDist = 400.f;

	if (0.f < vPos.m128_f32[1])
		vPos.m128_f32[1] = vKirbyPos.m128_f32[1] + 60.f;

	vPos.m128_f32[2] = vKirbyPos.m128_f32[2];
	if (400.f > fDistance)
	{
		vPos += XMVector3Normalize(XMVectorSet(1.f, 0.f, 0.f, 0.f)) * fTimeDelta * 50.f;
		pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	}

	_int iAnimIndex = pCenter->Get_CutScene();

	if (1 == iAnimIndex)
	{
		_float4 NewLook = _float4(1.f, 0.f, 0.f, 0.f);
		_float4 NewUp = _float4(0.f, 1.f, 0.f, 0.f);
		_float4 NewRight = XMVector3Cross(NewUp, NewLook);

		pTransformCom->Set_State(CTransform::STATE_LOOK, NewLook);
		pTransformCom->Set_State(CTransform::STATE_UP, NewUp);
		pTransformCom->Set_State(CTransform::STATE_RIGHT, NewRight);

		pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(2550.f, 239.f, -136.f, 1.f));

		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT1, 50.f, false, false);
	}
	//else if (fDistance <= fMaxDist)
	//{
	//	// 대상 객체에 가까워지는 방향으로 이동
	//	vPos += vKirbyPos + XMVectorSet(1.f, 0.f, 0.f, 0.f) * (fDistance - fMaxDist);
	//	pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	//}

}

void CFinaleBoss_Idle_State::OnStateExit()
{
}

CFinaleBoss_Idle_State* CFinaleBoss_Idle_State::Create()
{
	CFinaleBoss_Idle_State* pInstance = new CFinaleBoss_Idle_State();
	return pInstance;
}

void CFinaleBoss_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region CUTSCENE STATE
//*********************************
//			 CUTSCENE STATE
//*********************************
CFinaleBoss_CutScene_State::CFinaleBoss_CutScene_State()
{
}

void CFinaleBoss_CutScene_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinaleBoss_CutScene_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinaleBoss* pFinaleBoss = static_cast<CFinaleBoss*>(pGameObject);
	CTransform* pTransform = pFinaleBoss->Get_TransformCom();
	CFinaleCut_ControlCenter* pCenter = static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));


	_int iAnimIndex = pCenter->Get_CutScene();

	if (1 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT1, 50.f, false, false);
	else if (2 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT2, 50.f, false, false);
	else if (3 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT3, 50.f, false, false);
	else if (4 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT4, 50.f, false, false);
	else if (5 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT5, 50.f, false, false);
	else if (6 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT6, 70.f, false, false);
	else if (7 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT7, 50.f, false, false);
	else if (8 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT8, 50.f, false, false);
	else if (9 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT9, 50.f, false, false);
	else if (10 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT10, 50.f, false, false);
	else if (11 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT11, 50.f, false, false);
	else if (12 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT12, 50.f, false, false);
	else if (13 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT13, 50.f, true, false);
	else if (14 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT14, 50.f, false, false);
	else if (15 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT15, 50.f, false, false);
	else if (16 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT16, 50.f, false, false);
	else if (17 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT17, 50.f, false, false);
	else if (18 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT18, 50.f, true, false);
	else if (19 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT19, 50.f, false, false);
	else if (20 == pCenter->Get_CutScene())
		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT20, 50.f, false, true);



	//if (true == pFinaleBoss->IsAnimFinished())
	//{
	//	switch (pFinaleBoss->Get_State())
	//	{
	//	case CFinaleBoss::FINALEBOSS_CUT1:
	//		//pCenter->Set_CutScene(2);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT2, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT2:
	//		//pCenter->Set_CutScene(3);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT3, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT3:
	//		//pCenter->Set_CutScene(4);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT4, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT4:
	//		//pCenter->Set_CutScene(5);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT5, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT5:
	//		//pCenter->Set_CutScene(6);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT6, 70.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT6:
	//		//pCenter->Set_CutScene(7);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT7, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT7:
	//		//pCenter->Set_CutScene(8);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT8, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT8:
	//		//pCenter->Set_CutScene(9);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT9, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT9:
	//		//pCenter->Set_CutScene(10);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT10, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT10:
	//		//pCenter->Set_CutScene(11);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT11, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT11:
	//		//pCenter->Set_CutScene(12);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT12, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT12:
	//	{
	//		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_FINALE, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	//		CHECK_NULLPTR(pCamera);
	//		pCamera->Set_CamFocus(CCamera::FOCUS_BATTLE);

	//		//pCenter->Set_CutScene(13);

	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT13, 50.f, true, false);
	//	}
	//	break;
	//	/*case CFinaleBoss::FINALEBOSS_CUT13:
	//		pCenter->Set_CutScene(14);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT14, 50.f, false, false);
	//		break;*/
	//	case CFinaleBoss::FINALEBOSS_CUT14:
	//		//pCenter->Set_CutScene(15);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT15, 50.f, false, false);
	//		break;
	//	case CFinaleBoss::FINALEBOSS_CUT15:
	//		//pCenter->Set_CutScene(16);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT16, 50.f, false, false);
	//		break;
	//		//case CFinaleBoss::FINALEBOSS_CUT16:
	//		//	pCenter->Set_CutScene(17);
	//		//	pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT17, 50.f, false, false);
	//		//	break;
	//	case CFinaleBoss::FINALEBOSS_CUT17:
	//		//pCenter->Set_CutScene(18);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT18, 50.f, true, false);
	//		break;
	//		//case CFinaleBoss::FINALEBOSS_CUT18:
	//		//	pCenter->Set_CutScene(19);
	//		//	pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT19, 50.f, false, false);
	//		//	break;
	//	case CFinaleBoss::FINALEBOSS_CUT19:
	//		//pCenter->Set_CutScene(20);
	//		pFinaleBoss->Change_State(CFinaleBoss::FINALEBOSS_CUT20, 50.f, false, true);
	//		break;
	//	}
	//}
}

void CFinaleBoss_CutScene_State::OnStateExit()
{
}

CFinaleBoss_CutScene_State* CFinaleBoss_CutScene_State::Create()
{
	CFinaleBoss_CutScene_State* pInstance = new CFinaleBoss_CutScene_State();
	return pInstance;
}

void CFinaleBoss_CutScene_State::Free()
{
	__super::Free();
}
#pragma endregion
