#pragma once

#include "Client_Defines.h"
#include "Base.h"

/* 다음레벨에 대한 자원을 로드한다. */

BEGIN(Engine)
class CGameInstance;
END

BEGIN(Client)

class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public:

	HRESULT Initialize(LEVEL eNextLevelID);
	HRESULT Start();
	_bool IsFinished() const {
		return m_IsFinished;
	}
	void Output() 
	{
		SetWindowText(g_hWnd, m_strLoadingText.c_str());
	}

private:
	HRESULT Loading_ObjectAll();

	HRESULT Loading_For_Logo();
	HRESULT Loading_For_GamePlay();
	HRESULT Loading_For_Tool_UI();
	HRESULT Loading_For_Tool_FX();
	HRESULT Loading_For_Tool_Anim();
	//HRESULT Loading_For_Tool_Map();

	// 240518
	HRESULT Add_Models(LEVEL eLevel);
	void SetUp_ModelScaleRotation(LEVEL eLevel);

	// 셰이더 추가하고
	HRESULT Add_Shaders(LEVEL eLevel);

	HRESULT Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures = 1);
	HRESULT Add_KirbyFaceTexture(LEVEL eLevel);

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	HANDLE						m_hThread;
	CRITICAL_SECTION			m_Critical_Section;
	LEVEL						m_eNextLevelID = { LEVEL_END };
	wstring						m_strLoadingText;
	_bool						m_IsFinished = { false };

	vector<MODEL>				m_vecModelInfo;

	CGameInstance*				m_pGameInstance = { nullptr };

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END