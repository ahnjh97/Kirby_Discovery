#include "stdafx.h"
#include "PartTimeHelper.h"

#include "PartTimerKirby.h"
#include "HungryDee.h"
#include "UI_PartTime.h"
#include "UI_PartTimeDee.h"
#include "UI_PartTimeResult.h"
#include "Camera_Free.h"

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

void CPartTimeHelper::Register_Camera(CCamera* pCamera)
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
	m_pHungryDee->Set_MaskValueUI(0.f);
}

// 셀렉한 친구가 와들디가 달라고하는 친구가 맞는 지 확인한다.
// 매개변수는 커비가 주겠다고 선택한 아이템
_bool CPartTimeHelper::Check_Item(PARTTIME_ITEM eITEM)
{
	if(m_eFood == eITEM) // 커비가 맞췄을 때
	{
		m_pHungryDee->Bring_Food(eITEM);
		// 맞췄을 때 점수판 COUNT-UP
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
	// 효선아 여기야
	// 카메라 다 내려오고 나서 카메라 전환~ 까지 다하면 true 반환
	
	// Player 세팅 : 바깥쪽을 바라보고 있던 커비를 안쪽으로 보게 만듭니다.
	//CTransform* pTransform = m_pPartTimerKirby->Get_TransformCom();
	//pTransform->Rotation(_float3{ 0.f, 1.f, 0.f }, ToRadian(170.f));

	// Start 안내하는 UI
	m_pUI_PartTime->Set_RenderState(CUI_PartTime::START, true);

	return true;
}

/// <summary> 점심시간 안내에 대한 이벤트 처리 </summary>
/// <returns> true : 얼음 땡, false : 얼음 중 </returns>
_bool CPartTimeHelper::Handle_LunchTime()
{
	// 효선아 여기야
	// 카메라이동하고 ui띄우고 다시 카메라 돌아오면 true반환해주시면 됩니다.
	// true를 받으면 secondTimer로 돌아가고 있던 커비와 디가 다시 움직이기 시작합니다~
	return true;
}

_bool CPartTimeHelper::Handle_GameOver()
{
	m_pCamera->Lock_Camera({ 21.44f, 28.98f, 8.84f }, { -0.13f, -0.29f, 0.95f }, 33.f);
	
	// Player 세팅
	CTransform* pTransform = m_pPartTimerKirby->Get_TransformCom();
	pTransform->Set_State(CTransform::STATE_POSITION, _float4(17.85f, 23.8f, 27.f, 1.f));
	pTransform->Rotation(_float3{ 0.f, 1.f, 0.f }, ToRadian(170.f));

	// Dee들 세팅
	// 효선아 여기야

	// UI 세팅
	m_pHungryDee->Erase_DialogUI();
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

