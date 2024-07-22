#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CLoadingStart : public CUIObject
{
public:
	enum TEXTURETYPE { TYPE_DIFFUSE_1, TYPE_DIFFUSE_2, TYPE_MASK_1, TYPE_MASK_2, TYPE_END };

private:
	CLoadingStart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLoadingStart(const CLoadingStart& rhs);
	virtual ~CLoadingStart() = default;

public:
	void Set_Activate(_bool bActivate) {
		m_bActivate = bActivate;
	}
	void Set_Alpha(_float fAlpha) {
		m_fAlpha = fAlpha;
	}

public:
	virtual HRESULT			Initialize_Prototype()			override;
	virtual HRESULT			Initialize(void* pArg)			override;
	virtual _int			Tick(_float fTimeDelta)			override;
	virtual void			Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT			Render()						override;
	virtual HRESULT			Render(_float fTimeDelta);
#ifdef _DEBUG				
	virtual void			Render_IMGUI()					override;
#endif

private:
	CTexture*				m_pTextureCom[TYPE_END] = { nullptr };

	_bool					m_bActivate = { false };

	_uint					m_iMaskIndex = { 0 };

	_float					m_fTimeDelta = { 0.f };

private:
	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources(_float fAlpha);

	HRESULT					Render_Again(_float fSizeX, _float fSizeY, _float fPosX, _float fPosY);

public:
	static CLoadingStart* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void			Free() override;

};


END
