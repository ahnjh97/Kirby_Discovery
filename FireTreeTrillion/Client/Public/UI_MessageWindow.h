#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "Dialog.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_MessageWindow : public CUIObject
{
public:
	enum TEX_MESSAGEWINDOW { TEXMW_BASE, TEXMW_BTNBASE, TEXMW_NONE };
	enum MESSAGEWINDOW_STATE { WINDOW_IDLE, WINDOW_HIDE, WINDOW_SHOW, WINDOW_NONE	};

private:
	CUI_MessageWindow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_MessageWindow(const CUI_MessageWindow& rhs);
	virtual ~CUI_MessageWindow() = default;

#pragma region GETTER/SETTER
public:
	MESSAGEWINDOW_STATE Get_MWindowState() { return m_eCurState; }
#pragma endregion


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
	HRESULT						Add_Transform(void* _pArg);
	HRESULT						Add_Dialog(void* _pArg);
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CUI_MessageWindow*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	CTransform*					m_pTransCom[TEXMW_NONE] = { nullptr };
	CDialog*					m_pDialog = { nullptr };
	
	_float						m_fHideTime = { 0.f };
	_float3						m_vOrigScale = { 0.f, 0.f, 1.f };

	MESSAGEWINDOW_STATE			m_eCurState = { WINDOW_NONE };
};
END