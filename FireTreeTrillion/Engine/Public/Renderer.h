#pragma once

#include "Base.h"

/* 1. 화면에 그려져야할 객체들만 그려지는 순서대로 보관하는 클래스이다.*/
/* 2. 보관하고 있는 순서대로 객체들의 Draw콜(렌더함수를호출한다.)을 수행한다.*/

BEGIN(Engine)

class CRenderer final : public CBase
{
public:
	enum RENDER_MODE {
		MODE_GAMEPLAY, MODE_TOOL, MODE_END
	};

	enum OPTION {
		OPTION_SHADOW, OPTION_SSAO, OPTION_DOF, OPTION_MOTIONBLUR, OPTION_END
	};

	enum RENDERGROUP {
		RENDER_PRIORITY,
		RENDER_SHADOW,
		RENDER_NONBLEND,
		RENDER_NONLIGHT,
		RENDER_BLOOM,
		RENDER_BLEND,
		RENDER_DEFERREDINFO,
		RENDER_UI,
		RENDER_SUPERUI,
		RENDER_END
	};

private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT Initialize();
	void Color_Initialize();

	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Add_BlendModel(class CModel* pBlendModel);
	HRESULT Render(_float fTimeDelta);

	void Set_RenderMode(RENDER_MODE eMode) {
		m_eRenderMode = eMode;
		if (eMode == MODE_TOOL)
		{
			for (size_t i = OPTION_SHADOW; i < OPTION_END; ++i)
			{
				m_bRenderOption[i] = false;
				Update_Option((OPTION)i, m_bRenderOption[i]);

			}
		}
		else
		{
			for (size_t i = OPTION_SHADOW; i < OPTION_END; ++i)
			{
				m_bRenderOption[i] = true;
				Update_Option((OPTION)i, m_bRenderOption[i]);

			}

		}
	}

	void Render_SystemTick(_float fTimeDelta);
	void Key_Input();

	HRESULT Bind_DeferredTexture(CTexture* pTexture, const _char* pConstantName, _uint iIndex);
	HRESULT Bind_DeferredRawValue(const _char* pConstantName, const void* pData, _uint iLength);


	void Set_ColorSet(COLOR_DATA destColorData);
	void Set_ColorSet_ByIndex(_int iSetIdx);
	void Save_ColorSet(string strTag, COLOR_DATA destColorData);
	COLOR_DATA& Find_ColorSet(string strTag);

	// 잠깐의 레디얼 블러를 세팅한다.
	void Setting_RadialBlur(_fvector vWorldPos, _float fRadial, _float fSubtraction);
	void Setting_RadialBlur(_float fRadial, _float fSubtraction);
	// DOF 초점을 업데이트한다.
	void Update_DofFocus(_fvector vWorldPos);
	// GodRay 위치를 설정한다.
	void Setting_GodRay(_fvector vWorldPos, 
		_float fRayExposure = 0.15f, _float fRayDecay = 0.96815f, _float fRayIlluminationDecay = 0.8f, _float fRayDensity = 0.5f, _float fWeight = 0.5f);
	// 랜즈 플레어를 끄고 킨다.
	void Setting_LensFlare(_bool bOnOff) {
		m_bLensFlare = bOnOff;
	}
	_bool	m_bLensFlare = { true };

	HRESULT Render_LightDepth_For_GameObject(class CShader* pShader, class CTransform* pTransform, class CModel* pModel);
	HRESULT Render_LightDepth_For_PartObject(class CShader* pShader, const _float4x4* pMatrix, class CModel* pModel);

	void Update_LightShadow(_fvector vLightPos, _fvector vFocusPos) {
		m_vShadowEyePos = vLightPos;
		m_vShadowFocusPos = vFocusPos;
	}

	void Set_BlackBackGround(_bool bSet) {	m_bBlackBackground = bSet; }

	// 환경설정 업데이트
	void Update_Option(OPTION Option, _bool bOn);
	void Bind_RendererFunc(_int iTriggerType);

#ifdef _DEBUG
public:
	HRESULT Add_DebugComponents(class CComponent* pRenderObject);
	_bool	Get_HitBoxRender() { return m_IsRenderHitBox; }
	_bool	Get_IsRenderRTV() { return m_IsRenderRTV; }
#endif

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	list<class CGameObject*>			m_RenderObjects[RENDER_END];
	list<class CModel*>					m_BlendModelsList;


private:
	class CVIBuffer_Rect* m_pVIBuffer = { nullptr };
	class CShader* m_pShader = { nullptr };
	_float4x4							m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	ID3D11DepthStencilView* m_pLightDepthDSV = { nullptr };
	ID3D11RenderTargetView* m_pUIRTV = { nullptr };
	ID3D11ShaderResourceView* m_pUISRV = { nullptr };

	_float4								m_vShadowEyePos = { 0.f, 0.f, 0.f, 0.f };
	_float4								m_vShadowFocusPos = { 0.f, 0.f, 0.f, 0.f };
	_float								m_fShadowAngle = { 0.f };
	_float								m_fShadowFar = { 0.f };

#ifdef _DEBUG
	// 디버그 랜더타겟뷰 ON / OFF
	_bool								m_IsRenderRTV = { FALSE };
	// 디버그 히트박스 ON / OFF
	_bool								m_IsRenderHitBox = { FALSE };
#endif

	// 현재 게임 모드
	RENDER_MODE							m_eRenderMode = { MODE_END };
	_bool								m_bRenderOption[OPTION_END] = { true, true, true, true };
	_bool								m_bDebugOptionControl = { true };

#ifdef _DEBUG
private:
	list<class CComponent*>				m_DebugComponents;
#endif

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend();

	HRESULT Render_Lights();			HRESULT Render_Light_For_Tool();

	HRESULT Render_Effect();
	HRESULT Render_EffectResult();

	HRESULT Render_DeferredInfo();

	HRESULT Render_Result();			HRESULT Render_Result_For_Tool();

	HRESULT Render_GodRay();

	HRESULT Render_Radial_Result(_float fTimeDelta);
	HRESULT Render_DOF_Result();
	HRESULT Render_MotionBlur();

	HRESULT Render_FinalResult();

	HRESULT Render_UI();

#ifdef _DEBUG
	void Render_IMGUI();
#endif

	void Interpolate_ColorData(_float _fTimeDelta);
	void Interpolate_BlackBackground(_float fTimeDelta);
	void Interpolate_RadialBlur(_float fTimeDelta);

private:
	// For.RadialBlur
	_float2 m_vScreenPos = { 0.f, 0.f };
	_float m_fRadialBlurRadius = { 0.f };
	_float m_fRadialRadiusSubtraction = { 0.f };
	_bool  m_isRadial = { true };
	// For.BackgroundColor
	_bool  m_bBlackBackground = { false };
	_float m_fBlackBackground = { 1.f };
	// For.DOF
	_float2 m_vDofFocus = { 0.f, 0.f };
	// For.GodRay
	_float4 m_vGodPos = { 0.f, 0.f, 0.f, 0.f };

	// For.ColorCorrection
	_bool m_bApplyCorrection = { true };

	_float m_fExposure = { 1.03f };
	_float m_fHue = { 1.f };
	_float m_fSaturation = { .95f };
	_float m_fBrightness = { 1.3f };
	_float m_fGamma = { .85f };
	_float m_fVibrance = { .96f };
	_float m_fContrast = { 1.10f };

	_float m_vWhiteBalance[3] = { .64f, .6f, .6f };
	_float m_vColorBalance[3] = { 1.06f, .96f, 1.04f };

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
	_int					m_iCurColorIdx = { -1 };

	pair<_float, _float> m_fRimLightRatio = { 1.f, 1.f };

	_float m_fSSAOScale = { 0.5f };
	_float m_fSSAOBias = { 0.f };
	_float m_fSSAOSampleRadius = { 2.85f };
	_float m_fSSAOIntensity = { 2.5f };


#ifdef _DEBUG
private:
	HRESULT Render_Debug();
#endif

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;


};

END