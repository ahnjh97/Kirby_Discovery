#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

#define PARTTIME_TEX_CNT	19

// 15 : FINISH
// 17 : READY
// 18 : GO
// ---------------------------
// 16 : FADE-INOUT

BEGIN(Client)
class CUI_PartTime : public CUIObject
{
public:
	enum STATE { BASIC, START, FADE, STATE_END };

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

	void						Add_TimeBar(_float _fTimeBar);
	void						Add_Score(_int _fPlusScore);

	void						Set_PreRatioBar() { m_fRatioBarSub = m_fRatioTimeBar; }
	void						Set_RenderState(STATE _eState, _bool _bState);

	_bool						Get_TimeStart() const { return m_bTimeStart; }
	
private:
	HRESULT						Add_Components();
	
	HRESULT						Bind_ShaderResources();
	void						Setup_PosSizeColor(_int iTextureNum);
	_bool						Setup_DeeFace(_int iTextureNum);
	void						Compute_Timer(_float fTimeDelta);
	void						Compute_TimerBar(_float fTimeDelta);
	void						Compute_TimeScore(_float fTimeDelta);
	void						Change_TimeTexures(_float _fTime);

	void						Render_READY();
	void						Render_GO();
	void						Render_Finish();
	void						Render_Fade();

	void						Reset_ShaderValue();
	_float						EaseOutBounce(_float _value);

private:
	array<CTexture*, PARTTIME_TEX_CNT>		m_arrTexures;
	array<_float2,	 PARTTIME_TEX_CNT>		m_arrSize;
	array<_float2,	 PARTTIME_TEX_CNT>		m_arrPosition;

	// -------------- for IMGUI
	array<_float2,	 PARTTIME_TEX_CNT>		m_arrOriginalSize;
	array<_float,	 PARTTIME_TEX_CNT>		m_arrSizeRatio;
	array<_float3,	 PARTTIME_TEX_CNT>		m_arrColor;

	CTexture*					m_pTexMask = { nullptr }; // 마스킹을 위한 텍스쳐

	// UI 사이즈 픽싱 
	_float2						m_SizeBar2D = _float2(1024.f * 1.3f, 128.f * 1.3f);
	_float2						m_SizeTimeBarBlank2D = _float2(1600 * .65f, 61.f * .65f);
	_float2						m_SizeScoreBar2D = _float2(438.f, 156.f);
	_float2						m_SizeCategory2D = _float2(256.f, 256.f);
	_float2						m_SizeDeeFace2D = _float2(200.f, 116.f);
	_float2						m_SizeDigits2D = _float2(72.f, 72.f);

	// TimeBar Ratio
	_float						m_fRatioTimeBar = 1.f;
	_float						m_fGoalTimeBar = 0.f;
	_float						m_fRatioBarSub = 1.f;

	// Mediate-Timer
	_float						m_fStandardTime = 0.f;
	_float						m_fBeforeTime = 0.f;  
	_float						m_fCurTime = 50.f;
	_bool						m_bGoing = false;

	// Timer-Digits
	array<_int, 2>				m_arrTimerDigits;
	// Score-Digits
	array<_int, 3>				m_arrScoreDigits;

	_float						m_fTimeDelta = 0.f;
	
	// GAME-OVER 텍스트
	_float						m_fSizeRatio = 0.f;
	_float2						m_fStandardSize2D = _float2();
	// 시작할 때 텍스트
	_float						m_fMoveRatio = 0.f;
	_float2						m_fMovePosition2D = _float2(); // this

	_bool						m_bOnce = false;
	array<_bool, STATE_END>		m_arrRenderState;
	_bool						m_bTimeStart = false;

public:
	static CUI_PartTime*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};


END