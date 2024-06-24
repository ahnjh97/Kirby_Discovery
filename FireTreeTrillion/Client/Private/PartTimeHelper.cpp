#include "stdafx.h"
#include "PartTimeHelper.h"

#include "PartTimerKirby.h"
#include "HungryDee.h"

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

// 메뉴판 앞으로 온 와들디 OR UI 쪽에서 생성
void CPartTimeHelper::Make_RandomItem()
{
	_int iRandom = CUtils::Make_RandomInt(0,3);
	m_eFood = (PARTTIME_ITEM)iRandom;
}

// 셀렉한 친구가 와들디가 달라고하는 친구가 맞는 지 확인한다.
// 매개변수는 커비가 주겠다고 선택한 아이템
_bool CPartTimeHelper::Check_Item(PARTTIME_ITEM eITEM)
{
	if(m_eFood == eITEM) 
	{
		m_pHungryDee->Bring_Food(eITEM);
		return true;
	}
	else
	{
		m_pHungryDee->Bring_Food(PARTTIME_ITEM::ITEM_END);
		return false;
	}
}

void CPartTimeHelper::NotifyObserver()
{
	if(m_pHungryDee != nullptr)
		m_pHungryDee->OnNotify();
	if(m_pPartTimerKirby != nullptr)
		m_pPartTimerKirby->OnNotify();
}

void CPartTimeHelper::Free()
{
	__super::Free();
	Safe_Release(m_pHungryDee);
	Safe_Release(m_pPartTimerKirby);
}

