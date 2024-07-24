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
class CHUD_AbilityDiscard : public CUIObject
{
public:
	enum ABILITYDISCARD_STATE { DISCARD_IDLE, DISCARD_HIDE, DISCARD_SHOW, DISCARD_NONE	};
	enum TEX_DISCARD { TEXDC_ABILITYBASE, TEXDC_DEFORMBASE, TEXDC_GAUGE, TEXDC_BTN, TEXDC_NONE };

private:
	CHUD_AbilityDiscard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_AbilityDiscard(const CHUD_AbilityDiscard& rhs);
	virtual ~CHUD_AbilityDiscard() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif

	void						ChaseUI_To_Player();
	void						Compute_PlayerDumpAbiliyTime();
	_bool						Key_InputSystem(_float fTimeDelta);
	TEX_DISCARD					Check_TexIndex();

private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CHUD_AbilityDiscard*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;
	virtual void					Free() override;

private:
	CTexture*					m_pTexCom[TEX_NONE] = { nullptr };
	class CCharacter*			m_pKirby = { nullptr };
	
	ABILITYDISCARD_STATE			m_eCurState = { DISCARD_NONE };

	_float						m_fGaugeRatio = { 0.f };
	_float						m_fDumpAbilityTime = { 0.f };
	_float						m_fKeyInputTime = { 0.f };

};


END