#include "stdafx.h"
#include "PartTimeHelper.h"

#include "PartTimerKirby.h"
#include "HungryDee.h"
#include "UI_PartTime.h"
#include "UI_PartTimeDee.h"
#include "UI_PartTimeResult.h"
#include "Camera_Free.h"
#include "Camera_Main.h"

IMPLEMENT_SINGLETON(CPartTimeHelper)

CPartTimeHelper::CPartTimeHelper()
{
}

void CPartTimeHelper::Register_FirstDee(CHungryDee* pDee)
{
	Safe_Release(m_pHungryDee);
	m_pHungryDee = pDee;
	Safe_AddRef(m_pHungryDee);
}

void CPartTimeHelper::Register_PartTimerKirby(CPartTimerKirby* pKirby)
{
	Safe_Release(m_pPartTimerKirby);
	m_pPartTimerKirby = pKirby;
	Safe_AddRef(m_pPartTimerKirby);
}

void CPartTimeHelper::Register_UI(CUI_PartTime* pUI)
{
	Safe_Release(m_pUI_PartTime);
	m_pUI_PartTime = pUI;
	Safe_AddRef(m_pUI_PartTime);
}

void CPartTimeHelper::Register_PartTimeResult(CUI_PartTimeResult* pResult)
{
	Safe_Release(m_pUI_PartTimeResult);
	m_pUI_PartTimeResult = pResult;
	Safe_AddRef(m_pUI_PartTimeResult);
}

void CPartTimeHelper::Register_Camera(CCamera_Main* pCamera)
{
	Safe_Release(m_pCamera);
	m_pCamera = pCamera;
	Safe_AddRef(m_pCamera);
}

// PartTimer Kirby state에서 맞추거나 틀리고 나서 불리우는 함수.
// 실질적 문제내는 함수
void CPartTimeHelper::Make_RandomItem()
{
	_int iRandom = CUtils::Make_RandomInt(0,3);
	m_eFood = (PARTTIME_ITEM)iRandom;
	m_pHungryDee->Change_Dialog(m_eFood);
	m_pHungryDee->Set_RenderDialog(true);
}

// 셀렉한 친구가 와들디가 달라고하는 친구가 맞는 지 확인한다.
// 매개변수는 커비가 주겠다고 선택한 아이템
_bool CPartTimeHelper::Check_Item(PARTTIME_ITEM eITEM)
{
	if(m_eFood == eITEM) // 커비가 맞췄을 때
	{
		m_pHungryDee->Bring_Food(eITEM);
		// 맞췄을 때 점수판 COUNT-UP
		m_fScore += 30;
		m_pUI_PartTime->Add_Score(30);
		m_pUI_PartTime->Set_PreRatioBar();
		// 맞췄을 때 타임bar 길어지기
		m_pUI_PartTime->Add_TimeBar(0.2f);
		return true;
	}
	else
	{
		m_pHungryDee->Bring_Food(PARTTIME_ITEM::ITEM_END);
		return false;
	}
}

// 점심시간 알리는 용도로만 현재 사용하고 있습니다. 추후 추가될때 제게 말씀주세요 JYWI
void CPartTimeHelper::NotifyObserver()
{
	if(m_pHungryDee != nullptr)
		m_pHungryDee->OnNotify();
	if(m_pPartTimerKirby != nullptr)
		m_pPartTimerKirby->OnNotify();
}

// 카메라 변경되면서 게임 스타트 UI를 띄워주는 함수
_bool CPartTimeHelper::Handle_GameStart()
{
	// Player 세팅 : 바깥쪽을 바라보고 있던 커비를 안쪽으로 보게 만듭니다.
	CTransform* pTransform = m_pPartTimerKirby->Get_TransformCom();
	pTransform->Set_State(CTransform::STATE_POSITION, _float4(15.8f, 23.8f, 28.9f, 1.f));
	pTransform->Rotation(_float3{ 0.f, 1.f, 0.f }, ToRadian(-10.f));
	m_pPartTimerKirby->Set_MouthState(CPartTimerKirby::MOUTH_IDLE);

	// Start 안내하는 UI
	m_pUI_PartTime->Set_RenderState(CUI_PartTime::BASIC, true);
	m_pUI_PartTime->Set_RenderState(CUI_PartTime::START, true);

	m_pHungryDee->Set_RenderDialog(true);

	return true;
}

_bool CPartTimeHelper::Handle_GameOver()
{
	//m_pCamera->Lock_Camera({ 21.44f, 28.98f, 8.84f }, { -0.13f, -0.29f, 0.95f }, 33.f);
	m_pCamera->Lock_All({ 21.44f, 28.98f, 8.84f }, { -0.13f, -0.29f, 0.95f });
	// Player 세팅
	CTransform* pTransform = m_pPartTimerKirby->Get_TransformCom();
	pTransform->Set_State(CTransform::STATE_POSITION, _float4(17.85f, 23.8f, 27.f, 1.f));
	pTransform->Rotation(_float3{ 0.f, 1.f, 0.f }, ToRadian(170.f));

	// Dee들 세팅
	// 효선아 여기야

	// UI 세팅
	m_pHungryDee->Set_RenderDialog(false);
	_int iLevel = *CGameInstance::Get_Instance()->Get_CurrentLevelID();
	auto layerList = CGameInstance::Get_Instance()->Get_List(iLevel, TEXT("Layer_Dee"));
	for (auto pGameObj : *layerList)
	{
		CHungryDee* pDee = static_cast<CHungryDee*>(pGameObj);
		pDee->Set_RenderDialog(false);
	}

	m_pUI_PartTime->Set_RenderState(CUI_PartTime::BASIC, false);

	return true;
}

// 게임 흐름과 관련된 일을 처리하는 함수
void CPartTimeHelper::Handle_UI(TYPE eContent)
{
	if (eContent == GAMEOVER)  // 게임 종료 : GAME OVER 띄우기
		m_pUI_PartTime->Set_RenderState(CUI_PartTime::FADE, true);
	else if (eContent == OVER) // 게임 종료 : 카메라 전환된 상태에서 UI
	{
		m_pUI_PartTime->Set_RenderState(CUI_PartTime::FADE, false);
		m_pUI_PartTimeResult->Set_IsRender(true);
		m_pUI_PartTimeResult->Set_Score(m_fScore);
	}
}

void CPartTimeHelper::Free()
{
	__super::Free();
	Safe_Release(m_pHungryDee);
	Safe_Release(m_pUI_PartTime);
	Safe_Release(m_pUI_PartTimeResult);
	Safe_Release(m_pPartTimerKirby);
	Safe_Release(m_pCamera);
}

