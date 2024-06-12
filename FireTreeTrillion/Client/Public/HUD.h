#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Client)

class CHUD : public CUIObject
{
public:
	enum HUD_STATUS { STAT_KIRBY, STAT_STARPOINT, STAT_NONE };

protected:
	CHUD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD(const CHUD& rhs);
	virtual ~CHUD() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;
#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif

public:
	static CHUD*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

};


END