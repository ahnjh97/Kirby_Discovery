#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Simba final : public CLevel
{
private:
	CLevel_Simba(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Simba() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_Map();
	HRESULT Ready_Triggers();
	HRESULT Ready_Monsters();
	HRESULT Ready_Items();
	HRESULT Ready_Kickables();
	HRESULT Ready_Objects();
	HRESULT Ready_UI();

	HRESULT Add_EnvMap();
	enum TEXTURETYPE { TYPE_ENV, TYPE_LUT, TYPE_NORMAL, TYPE_END };
	CTexture* m_pEnvTexture[TYPE_END] = { nullptr, nullptr, nullptr };

private:
	_bool m_bWave1DeadNotified = { false };
	_bool m_bWave2DeadNotified = { false };

public:
	static CLevel_Simba* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END

