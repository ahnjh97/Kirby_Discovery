#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "UI_MessageWindow.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_BtnIcon : public CUIObject
{
private:
	enum TEX_BTNICON { TEXBTN_BASE, TEXBTN_BRIGHT, TEXBTN_NONE };
	enum BTN_STATE { BTN_IDLE, BTN_BLINK, BTN_SELECT_DOWN, BTN_SELECT_UP, BTN_NONE };

private:
	CUI_BtnIcon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_BtnIcon(const CUI_BtnIcon& rhs);
	virtual ~CUI_BtnIcon() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif

private:
	//HRESULT						Add_Transform(void* _pArg);
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CUI_BtnIcon*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	CTexture*					m_pTexCom[TEXBTN_NONE] = {nullptr};
	CUI_MessageWindow*			m_pMWindow = { nullptr };
	
	_float						m_fBtnAlpha = { 0.f };

	_float						m_fBlinkAlpha = { 0.f };
	_float						m_fBlinkTime = { 0.f };
	_float						m_fSelectTime = { 0.f };
	_float3						m_vOrigScale = { };

	BTN_STATE					m_ePreState = { BTN_NONE };
	BTN_STATE					m_eCurState = { BTN_NONE };
		
};
END