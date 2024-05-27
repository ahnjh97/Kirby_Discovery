#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Client)

class CHUD : public CUIObject
{
protected:
	enum HUD_STATUS { STAT_HP, STAT_SP, STAT_NONE };
	enum HUD_STATFONT { STFONT_1, STFONT_10, STFONT_100, STFONT_SLASH, STFONT_NONE };

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
	virtual void				Render_IMGUI()								override;

private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CHUD*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

};


END