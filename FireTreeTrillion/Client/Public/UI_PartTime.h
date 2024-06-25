#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

#define TEXTURECNT	10

BEGIN(Client)
class CUI_PartTime : public CUIObject
{
	enum UI_NAME { TIME_BAR_BG, UI_END };
private:
	CUI_PartTime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_PartTime(const CUI_PartTime& rhs);
	virtual ~CUI_PartTime() = default;

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
	HRESULT						Add_Components();
	
	HRESULT						Bind_ShaderResources();
	void						Setup_PosSizeColor(_int iTextureNum);
	_bool						Setup_DeeFace(_int iTextureNum);
	void						Compute_Timer(_float fTimeDelta);
	void						Compute_TimeScore(_float fTimeDelta);
	void						Change_TimeTexures(_float _fTime);

private:
	array<CTexture*, TEXTURECNT>		m_arrTexures;
	array<_float2,	 TEXTURECNT>		m_arrSize;
	array<_float2,	 TEXTURECNT>		m_arrPosition;

	// -------------- for IMGUI
	array<_float2,	 TEXTURECNT>			m_arrOriginalSize;
	array<_float,	 TEXTURECNT>			m_arrSizeRatio;
	array<_float3,	 TEXTURECNT>			m_arrColor;

	CTexture*					m_pTexMask = { nullptr }; // 마스킹을 위한 텍스쳐

	// UI 사이즈 픽싱 
	_float2						m_SizeBar2D = _float2(1024.f * 1.3f, 128.f * 1.3f);
	_float2						m_SizeTimeBarBlank2D = _float2(1600 * .65f, 61.f * .65f);
	_float2						m_SizeScoreBar2D = _float2(438.f, 156.f);
	_float2						m_SizeCategory2D = _float2(256.f, 256.f);
	_float2						m_SizeDeeFace2D = _float2(200.f, 116.f);

	// TimeBar Ratio
	_float						m_fRatioTimeBar = 1.f;
	_float						m_fRatioSubBar = _float();

	// Mediate-Timer
	_float						m_fStandardTime = 0.f;
	_float						m_fBeforeTime = 0.f;  
	_float						m_fCurTime = 0.f;  

public:
	static CUI_PartTime*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};


END