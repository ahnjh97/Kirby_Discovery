#pragma once

#include "Base.h"

/* 1. 화면에 그려져야할 객체들만 그려지는 순서대로 보관하는 클래스이다.*/
/* 2. 보관하고 있는 순서대로 객체들의 Draw콜(렌더함수를호출한다.)을 수행한다.*/

BEGIN(Engine)

class CRenderer final : public CBase
{
public:
	enum RENDERGROUP {
		RENDER_PRIORITY,
		RENDER_SHADOW,
		RENDER_NONBLEND,
		RENDER_NONLIGHT,
		RENDER_BLOOM,
		RENDER_BLEND,
		RENDER_UI,
		RENDER_SUPERUI,
		RENDER_END
	};
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;


public:
	HRESULT Initialize();
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Render(_float fTimeDelta);


	void Set_ColorSet(COLOR_DATA destColorData);
	void Set_ColorSet(_int iSetIdx);
	void Save_ColorSet(string strTag, COLOR_DATA destColorData);
	COLOR_DATA& Find_ColorSet(string strTag);

	// 잠깐의 레디얼 블러를 세팅한다.
	void Setting_RadialBlur(_fvector vWorldPos, _float fRadial, _float fSubtraction);
	void Setting_RadialBlur(_float fRadial, _float fSubtraction);

	void Update_DofFocus(_fvector vWorldPos);

	HRESULT Render_LightDepth_For_GameObject(class CShader* pShader, class CTransform* pTransform, class CModel* pModel);

	void Update_LightShadow(_fvector vLightPos, _fvector vFocusPos) {
		m_vShadowEyePos = vLightPos;
		m_vShadowFocusPos = vFocusPos;
	}


#ifdef _DEBUG
public:
	HRESULT Add_DebugComponents(class CComponent* pRenderObject);
#endif

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	list<class CGameObject*>			m_RenderObjects[RENDER_END];



private:
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	class CShader* m_pShader = { nullptr };
	_float4x4							m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	ID3D11DepthStencilView* m_pLightDepthDSV = { nullptr };

	_float4								m_vShadowEyePos = { 0.f, 0.f, 0.f, 0.f };
	_float4								m_vShadowFocusPos = { 0.f, 0.f, 0.f, 0.f };
	_float								m_fShadowAngle = { 0.f };
	_float								m_fShadowFar = { 0.f };

	// 저 사양 모드
	_bool								m_bLowPass = { true };



#ifdef _DEBUG
private:
	list<class CComponent*>				m_DebugComponents;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();

	HRESULT Render_Lights();

	HRESULT Render_Effect();
	HRESULT Render_EffectResult();

	HRESULT Render_Result();
	HRESULT Render_Radial_Result(_float fTimeDelta);
	HRESULT Render_DOF_Result();
	HRESULT Render_MotionBlur();

	HRESULT Render_FinalResult();

	HRESULT Render_UI();
	HRESULT Render_SuperUI();

	void Render_IMGUI();

	void Interpolate_ColorData(_float _fTimeDelta);

private:
	_float2 m_vScreenPos = { 0.f, 0.f };
	_float m_fRadialBlurRadius = { 0.f };
	_float m_fRadialRadiusSubtraction = { 0.f };


	_float m_fRimWidth = { 0.f };
	_bool  m_bRimTest = { false };

	_float2 m_vDofFocus = { 0.f, 0.f };

	//색감 보정 변수
	_bool m_bApplyCorrection = { true };

	_float m_fExposure = { 1.03f };
	_float m_fHue = { 1.f };
	_float m_fSaturation = { .95f };
	_float m_fBrightness = { 1.3f };
	_float m_fGamma = { .85f };
	_float m_fVibrance = { .96f };
	_float m_fContrast = { 1.10f };

	_float m_vWhiteBalance[3] = {.64f, .6f, .6f};
	_float m_vColorBalance[3] = {1.06f, .96f, 1.04f};

	_float m_vShadowColor[3] = { 62.f / 255.f, 2.f / 255.f, 2.f / 255.f };
	_float m_fShadowIntensity = { 0.12f };
	_float m_vMidtoneColor[3] = { 234.f / 255.f,131.f / 255.f, 37.f / 255.f };
	_float m_fMidtoneIntensity = { 0.16f };
	_float m_vHighlightColor[3] = { 255.f / 255.f, 216.f / 255.f, 65.f / 255.f };
	_float m_fHighlightIntensity = { 0.34f };
	_float m_fShadowThreshold = { .13f };
	_float m_fHighlightThreshold = { .55f };

	COLOR_DATA m_DestColorData{};
	map<string, COLOR_DATA> m_ColorSets;

#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;


};

END