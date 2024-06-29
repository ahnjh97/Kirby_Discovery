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
	enum TEX_STATE { DISCARD_IDLE, DISCARD_HIDE, DISCARD_SHOW, DISCARD_NONE	};

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
	//virtual void				Render_IMGUI()								override;
#endif

private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CHUD_AbilityDiscard*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;
	virtual void					Free() override;

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom[TEX_NONE] = { nullptr };
	CVIBuffer_Rect*				m_pVIBufferCom = { nullptr };

	TEX_STATE					m_eTexState = { DISCARD_NONE };

	_bool						m_IsGaugeUP = { FALSE };
	_bool						m_IsKirbyExist = { FALSE };
	
	_float						m_GaugeRatio = { 0.f };
	_float						m_fDumpAbilityTime = { 0.f };
	
	_float4						m_vInitPos = { 0.f, 0.f, 0.f, 1.f };
	_float3						m_vInitSize = { 0.f, 0.f, 0.f };
	_float						m_fInitAlpha = { 0.f };

};


END