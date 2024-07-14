#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Engine)
class CTexture;
END

BEGIN(Client)
class CLevel_DeeDeeDee final : public CLevel
{
private:
	CLevel_DeeDeeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_DeeDeeDee() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void	Ready_FadeIn();

	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_Layer_UI(const wstring& strLayerTag);

	HRESULT Ready_Map();
	HRESULT Ready_Triggers();
	HRESULT Ready_Dees();
	HRESULT Ready_Monsters();
	HRESULT Ready_Items();
	HRESULT Ready_Kickables();
	HRESULT Ready_Objects();

	HRESULT	Load_FileData(const string& _strFilePath, FILE_TYPE _eFileType, const wstring& _strLayerTag);

	HRESULT Add_EnvMap();
	enum TEXTURETYPE { TYPE_ENV, TYPE_LUT, TYPE_NORMAL, TYPE_END };
	CTexture* m_pEnvTexture[TYPE_END] = { nullptr };

public:
	static CLevel_DeeDeeDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
