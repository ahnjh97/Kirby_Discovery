#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_Fading : public CUIObject
{
public:
	enum STATE {FADEOUT, FADEIN, STATE_END};

private:
	CUI_Fading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Fading(const CUI_Fading& rhs);
	virtual ~CUI_Fading() = default;

public:
	virtual HRESULT			Initialize_Prototype()			override;
	virtual HRESULT			Initialize(void* pArg)			override;
	virtual _int			Tick(_float fTimeDelta)			override;
	virtual void			Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT			Render()						override;
#ifdef _DEBUG				
	virtual void			Render_IMGUI()					override;
#endif

	void					Set_InOutState(STATE eState) 
	{
		m_eState = eState; 
		m_fFadeOutRatio = (m_eState == FADEIN) ? 0.f : 1.f;
	}
	STATE					Get_State() const { return m_eState; }
							
	_float					Get_FadeRatio() const { return m_fFadeOutRatio; }

private:					
	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources();
							
private:					
	CTexture*				m_pTextureCom = { nullptr }; // 마스킹을 위한 텍스쳐
							
	_float					m_fTimeDelta = _float();
	_float					m_fFadeOutRatio = 1.f;

	STATE					m_eState = STATE_END;

public:						
	static CUI_Fading*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};


END
