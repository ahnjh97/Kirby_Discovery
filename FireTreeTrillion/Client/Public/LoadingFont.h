#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CLoadingFont : public CUIObject
{
public:
	struct LOADINGFONT_DESC : public CGameObject::GAMEOBJECT_DESC {
		_bool	bDeadRender = { false };
		_uint	iTexIndex = { 0 };
		_float	fPosX = { 0.f };
		_float	fEndPos = { 0.f };
		_float	fDisappearPos = { 0.f };
		wstring	strTag = { L"" };
	};
private:
	CLoadingFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLoadingFont(const CLoadingFont& rhs);
	virtual ~CLoadingFont() = default;

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
	CTexture*	m_pTextureCom = { nullptr };

	_float		m_fPosX = { 0.f };
	_float		m_fEndPos = { 0.f };
	_float		m_fRatio = { 0.f };
	_float		m_fDisappearPos = { 0.f };

	_bool		m_bDeadRender = { false };
	wstring		m_strTag = { L"" };

private:
	HRESULT		Add_Components();
	HRESULT		Bind_ShaderResources();

	_float		Compute_Easing(_float vStartPos, _float vEndPos, _float fSpeed);

public:
	static CLoadingFont* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void			Free() override;
};


END
