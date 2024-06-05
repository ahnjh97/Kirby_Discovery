#pragma once
#include "Client_Defines.h"

BEGIN(Client)

class CLevelChanger final : public CBase
{
	DECLARE_SINGLETON(CLevelChanger)

public:
	struct LEVEL_DATA
	{
		_float	fKirbyHP;
		_float	fKirbyCoin;
		
		// 추후 Kirby의 변신상태를 츄가한다.
		//_uint	uKirbyState;
		// 등등
	};

private:
	CLevelChanger(/*ID3D11Device* pDevice, ID3D11DeviceContext* pContext*/);
	virtual ~CLevelChanger() = default;

public:
	void		Save();
	void		Load();

private:
	LEVEL_DATA		m_tLevelData;

public:
	void Release_LevelChanger();
	virtual void Free() override;

};

END