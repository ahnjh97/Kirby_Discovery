#pragma once
#include "Client_Defines.h"

BEGIN(Client)

class CLevelChanger final : public CBase
{
	DECLARE_SINGLETON(CLevelChanger)

public:
	struct LEVEL_DATA
	{
		string	strObjectName;
		_float	fKirbyHP;
		_float	fKirbyCoin;
		//_uint	uKirbyState; // 커비 변신 상태
	};

private:
	CLevelChanger();
	virtual ~CLevelChanger() = default;

public:
	void			Save(LEVEL_DATA _tData);
	LEVEL_DATA		Load();

private:
	LEVEL_DATA		m_tLevelData;

public:
	virtual void Free() override;

};

END