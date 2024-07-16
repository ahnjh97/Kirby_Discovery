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
	HRESULT Ready_Fonts();
	HRESULT Ready_Object_For_Static();
	HRESULT Ready_Prototype_Component_For_Static();

#ifdef _DEBUG
	_bool	Render_RTVFonts();
#endif // _DEBUG

private:
	CGameInstance*			m_pGameInstance = { nullptr };
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

public:	
	static CMainApp* Create();
	virtual void Free() override;
};

END


