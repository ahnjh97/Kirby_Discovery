#include "stdafx.h"
#include "PartTimeHelper.h"

//#include "Kirby.h"

IMPLEMENT_SINGLETON(CPartTimeHelper)

CPartTimeHelper::CPartTimeHelper()
{
}
//
//PARTTIME_ITEM CPartTimeHelper::Make_RandomItem()
//{
//	return DRINK;
//}
//
////셀렉한 친구가 와들디가 달라고하는 친구가 맞는 지 확인한다.
//_bool CPartTimeHelper::Check_Item()
//{
//	return _bool();
//}
//
//// 커비가 맞췄는지 틀렸는지에 대한 결과값을 와들디에게 송출한다.
//void CPartTimeHelper::Notify_Result()
//{
//}

void CPartTimeHelper::Free()
{
	__super::Free();
}

