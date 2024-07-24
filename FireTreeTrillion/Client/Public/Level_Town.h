#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Engine)
class CTexture;
END

BEGIN(Client)
class CLevel_Town final : public CLevel
{
private:
	CLevel_Town(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Town() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void	Sound_Tick(_float fTimeDelta);

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_UI();

	HRESULT Ready_Map();
	HRESULT Ready_Triggers();
	HRESULT Ready_Dees();
	HRESULT Ready_Monsters();
	HRESULT Ready_Items();
	HRESULT Ready_Kickables();
	HRESULT Ready_Objects();

	void	Change_Levels();
	void	Teleport_Player();

	HRESULT Add_EnvMap();
	enum TEXTURETYPE { TYPE_ENV, TYPE_LUT, TYPE_NORMAL, TYPE_END };
	CTexture* m_pEnvTexture[TYPE_END] = { nullptr };

	enum SOUND_STATE { TOWN_BASIC, PARK_ENTRANCE, STATE_END };
	SOUND_STATE  m_eSoundState = STATE_END;

public:
	static CLevel_Town* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
