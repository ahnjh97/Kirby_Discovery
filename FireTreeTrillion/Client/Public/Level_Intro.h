#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Intro final : public CLevel
{
private:
	CLevel_Intro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Intro() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_Layer_UI(const wstring& strLayerTag);
	HRESULT Ready_ParsedObjects();
	HRESULT Ready_Map();
	HRESULT Ready_Triggers();
	HRESULT Ready_Monsters();
	HRESULT Ready_Items();

public:
	static CLevel_Intro* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END

