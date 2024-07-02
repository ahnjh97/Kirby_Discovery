#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "HUD.h"

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
	//virtual void				Render_IMGUI()								override;
#endif

private:
	HRESULT						Add_Transform(void* _pArg);
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CUI_BtnIcon*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	//CTransform*				m_pTransCom[TEXBTN_NONE] = { nullptr };
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom[TEXBTN_NONE] = {nullptr};
	CVIBuffer_Rect*				m_pVIBufferCom = { nullptr };
};
END