#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CHUD_KirbyNameTag : public CUIObject
{
public:
	enum TEX_NAMETAG {
		TEXNT_KIRBY, TEXNT_SWORD, TEXNT_BOMB, TEXNT_TOYHAMMER, TEXNT_CRASH,
		TEXNT_DEFORMCAR, TEXNT_DEFORMBULB, TEXNT_NONE
	};

	enum NAMETAG_STATE { NAMETAG_IDLE, NAMETAG_SHOW, NAMETAG_HIDE, NAMETAG_NONE };

private:
	CHUD_KirbyNameTag(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_KirbyNameTag(const CHUD_KirbyNameTag& rhs);
	virtual ~CHUD_KirbyNameTag() = default;

#pragma region GETTER/SETTER
public:
	void Set_NameTagState(NAMETAG_STATE _eNameTagState) { m_eCurState = _eNameTagState;	}
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
	TEX_NAMETAG					Check_TexIndex();

private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CHUD_KirbyNameTag*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	class CCharacter*			m_pKirby = { nullptr };

	_float4						m_vInitPos = { 0.f, 0.f, 0.f, 1.f };

	NAMETAG_STATE				m_eCurState = { NAMETAG_NONE };
		
};
END