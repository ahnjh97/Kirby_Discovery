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
	HRESULT Ready_Prototype_Effect();

#ifdef _DEBUG
	void	Render_RTVFonts();
	void	Render_RTVFont(string strTag, _float2 vPos, _float4 vColor = {1.f, 1.f, 1.f, 1.f});
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


