#include "stdafx.h"
#include "LevelChanger.h"

IMPLEMENT_SINGLETON(CLevelChanger)

CLevelChanger::CLevelChanger()
{
}

void CLevelChanger::Save()
{
	_int a = 3;
}

void CLevelChanger::Load()
{
	_int a = 3;
}

void CLevelChanger::Release_LevelChanger()
{
	CGameInstance::Get_Instance()->Free();
	Destroy_Instance();
}

void CLevelChanger::Free()
{
	__super::Free();
}

