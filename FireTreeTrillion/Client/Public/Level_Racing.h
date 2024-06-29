#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Racing final : public CLevel
{
private:
	CLevel_Racing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Racing() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_Layer_UI(const wstring& strLayerTag);

	HRESULT Ready_Map(_float fXOffset, _float fZOffset);
	HRESULT Ready_Triggers(_float fXOffset, _float fZOffset);
	HRESULT Ready_Monsters(_float fXOffset, _float fZOffset);
	HRESULT Ready_Items(_float fXOffset, _float fZOffset);
	HRESULT Ready_Kickables(_float fXOffset, _float fZOffset);
	HRESULT Ready_Objects(_float fXOffset, _float fZOffset);

	void	Change_Levels();

	HRESULT	Load_FileData(const string& _strFilePath, FILE_TYPE _eFileType, const wstring& _strLayerTag);

	HRESULT Add_EnvMap();
	enum TEXTURETYPE { TYPE_ENV, TYPE_LUT, TYPE_NORMAL, TYPE_END };
	CTexture* m_pEnvTexture[TYPE_END] = { nullptr };

public:
	static CLevel_Racing* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END


