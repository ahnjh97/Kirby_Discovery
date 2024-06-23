#include "stdafx.h"
#include "PartTimeHelper.h"
#include "PartTimerKirby.h"

IMPLEMENT_SINGLETON(CPartTimeHelper)

CPartTimeHelper::CPartTimeHelper()
{
}

// 메뉴판 앞으로 온 와들디 OR UI 쪽에서 생성
void CPartTimeHelper::Make_RandomItem()
{
	m_eFood = DRINK;
}

// 셀렉한 친구가 와들디가 달라고하는 친구가 맞는 지 확인한다.
// 매개변수는 커비가 주겠다고 선택한 아이템
_bool CPartTimeHelper::Check_Item(PARTTIME_ITEM eITEM)
{
	//if(m_eFoode == eITEM) 
	{
		return true;
	}
	m_eFood = DRINK;

	// 혹시모를 초기화
	m_eFood = ITEM_END;
	
	return true;
}

void CPartTimeHelper::Free()
{
	__super::Free();
}

