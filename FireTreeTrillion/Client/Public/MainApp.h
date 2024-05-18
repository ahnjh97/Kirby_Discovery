#pragma once

#include "Client_Defines.h"
#include "Base.h"

BEGIN(Engine)
class CGameInstance;
END

BEGIN(Client)

class CMainApp final : public CBase
{
private:
	CMainApp();
	virtual ~CMainApp() = default;

public:
	HRESULT Initialize();
	void Tick(_float fTimeDelta);
	HRESULT Render(_float fTimeDelta);
	HRESULT Open_Level(LEVEL eLevelID);

private:
	CGameInstance*			m_pGameInstance = { nullptr };
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

private:	
	HRESULT Ready_Fonts();
	HRESULT Ready_Prototype_Component_For_Static();

	//Parsing TEST
	void		CreateXML();
	void		Read_XML();
	void		Create_N_ReadJSON();
	void		Create_JSON();

	_bool	ParseJson(Document& doc, const string& jsonData);
	string	JsonDocToString(Document& doc, bool isPretty = false);
	void	TestJson_Parse();
	void	TestJson_AddMember();

public:	
	static CMainApp* Create();
	virtual void Free() override;
};

END


