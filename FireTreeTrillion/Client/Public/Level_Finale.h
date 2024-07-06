#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Finale final : public CLevel
{
private:
	CLevel_Finale(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Finale() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_FinaleRoad();
	HRESULT Ready_Map();
	HRESULT Ready_Triggers();
	HRESULT Ready_Monsters();
	HRESULT Ready_Items();
	HRESULT Ready_Kickables();
	HRESULT Ready_Objects();
	HRESULT Ready_UI();

public:
	static CLevel_Finale* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END

