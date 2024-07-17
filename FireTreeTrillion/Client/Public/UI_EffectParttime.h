#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_EffectParttime : public CUIObject
{
	enum STATE { CIRCLE, SHOOT, STATE_END};

private:
	CUI_EffectParttime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_EffectParttime(const CUI_EffectParttime& rhs);
	virtual ~CUI_EffectParttime() = default;

public:
	virtual HRESULT			Initialize_Prototype()			override;
	virtual HRESULT			Initialize(void* pArg)			override;
	virtual _int			Tick(_float fTimeDelta)			override;
	virtual void			Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT			Render()						override;
#ifdef _DEBUG				
	virtual void			Render_IMGUI()					override;
#endif

private:					
	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources();
							
private:					
	CTexture*				m_pTextureCom = { nullptr };
	//_float					m_fFadeOutRatio = 1.f;
	STATE					m_eState = STATE_END;

public:						
	static CUI_EffectParttime*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;
	virtual void					Free() override;

};


END
