#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_Interactable : public CUIObject
{
public:
	enum STATE { PLUS, MINUS, STATE_END };

private:
	CUI_Interactable(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Interactable(const CUI_Interactable& rhs);
	virtual ~CUI_Interactable() = default;

public:
	virtual HRESULT				Initialize_Prototype()			override;
	virtual HRESULT				Initialize(void* pArg)			override;
	virtual _int				Tick(_float fTimeDelta)			override;
	virtual void				Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT				Render()						override;
#ifdef _DEBUG					
	virtual void				Render_IMGUI()					override;
#endif

	void						Set_Owner(CGameObject* pOwner) { m_pOwner = pOwner;	}
	void						Set_Offset(_float fOffset) { m_fOffset = fOffset;	}

private:						
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources();
	void						Update_Pos(_float3 _vPosition);

private:						
	CTexture*					m_pTextureCom = { nullptr }; // 마스킹을 위한 텍스쳐
	CGameObject*				m_pOwner = nullptr;

	STATE						m_eState = STATE_END;
	_float2						m_InitialSize = _float2();
	_float						m_fFadeOutRatio = 1.f;
	_float						m_fOffset = _float();

public:						
	static CUI_Interactable*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

};


END
