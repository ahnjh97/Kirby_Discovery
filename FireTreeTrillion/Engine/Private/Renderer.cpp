#include "Renderer.h"
#include "GameObject.h"
#include "GameInstance.h"

_uint		g_iSizeX = 8192;
_uint		g_iSizeY = 4608;
_uint		g_iOriginSizeX = 1600; //1280;
_uint		g_iOriginSizeY = 900; //720;

CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CRenderer::Initialize()
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_vShadowEyePos = { 0.f, 20.f, -1.f, 1.f };
	m_vShadowFocusPos = { 0.f, 0.f, 0.f, 1.f };
	m_fShadowAngle = { 120.f };
	m_fShadowFar = { 2000.f };

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	Save_ColorSet("Default",
		COLOR_DATA{
		m_fExposure, m_fHue, m_fSaturation,	m_fBrightness, m_fGamma, m_fVibrance, m_fContrast,
		{m_vWhiteBalance[0], m_vWhiteBalance[1], m_vWhiteBalance[2]},
		{m_vColorBalance[0], m_vColorBalance[1], m_vColorBalance[2]},
		{m_vShadowColor[0], m_vShadowColor[1], m_vShadowColor[2]},
		m_fShadowIntensity,
		{m_vMidtoneColor[0], m_vMidtoneColor[1], m_vMidtoneColor[2]},
		m_fMidtoneIntensity,
		{m_vHighlightColor[0], m_vHighlightColor[1], m_vHighlightColor[2]},
		m_fHighlightIntensity,
		m_fShadowThreshold,
		m_fHighlightThreshold
		});

	Save_ColorSet("Tutorial",
		COLOR_DATA{
		.39f, 1.f, .80f,	1.74f, .99f, .90f, 1.05f,
		{.94f, .6f, .6f},
		{1.05f, 1.01f, 1.22f},
		{73.f / 255.f, 15.f / 255.f, 89.f / 255.f},
		.29f,
		{61.f / 255.f, 186.f / 255.f, 173.f / 255.f},
		.48f,
		{1.f, 216.f / 255.f, 65.f / 255.f},
		.34f,
		.33f,
		.60f
		});

	Save_ColorSet("Night",
		COLOR_DATA{
		0.75f,
		1.f, 1.14f,
		1.01f,
		0.63f,
		0.62f,
		1.03f,
		0.8f,
		0.6f,
		0.6f,
		1.6f, 0.72f, 1.82f, 0.376471f, 0.0352941f, 0.372549f,
		0.07f, 0.579137f, 0.633162f, 0.769912f, 0.51f, 0.973451f, 0.771999f, 0.323048f, 0.72f, 0.08f, 0.63f
		});



	//function<void(_int)> func = bind(&CCamera_Free::Set_MatrixIndex, this, placeholders::_1);
	//auto ColorFunc = bind((CRenderer::Set_ColorSet), this, placeholders::_1);

	//function<void(_int)> ColorFunc = bind(&CRenderer::Set_ColorSet, this, placeholders::_1);

	//m_pGameInstance->SetUp_TriggerFunc(1, ColorFunc);


#pragma region MRT_Sky
	// 스카이와 블룸이 공존할 수 있게 랜더타겟으로 별도로 스카이박스를 처리한다.
	/* For.Target_Sky */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Sky"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Sky"), TEXT("Target_Sky"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_GameObjects
	/* For.Target_Diffuse */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Diffuse"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* For.Target_Normal */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Normal"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	/* For.Target_Depth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Depth"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 1.f, 0.f, 1.f))))
		return E_FAIL;
	/* For.Target_RimLight */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_RimLight"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	/* For.Target_FieldDepth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_FieldDepth"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	/* For.Target_Stencil */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Stencil"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	/* For.Target_MotionBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_MotionBlur"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	/* For.Target_MRA */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_MRA"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;


	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_RimLight"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_FieldDepth"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_Stencil"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_MotionBlur"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GameObjects"), TEXT("Target_MRA"))))
		return E_FAIL;

#pragma endregion

#pragma region MRT_LightAcc
	/* For.Target_Shade */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Shade"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
		return E_FAIL;
	/* For.Target_Specular */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Specular"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_ShadowObject
	/* For.Target_LightDepth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_LightDepth"), g_iSizeX, g_iSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ShadowObject"), TEXT("Target_LightDepth"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_DefferredInfo
	/* For.Target_LightDepth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DeferredInfo"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DeferredInfo"), TEXT("Target_DeferredInfo"))))
		return E_FAIL;
#pragma endregion


#pragma region MRT_NonLight
	/* For.Target_RadialBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_NonLight"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_NonLight"), TEXT("Target_NonLight"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_Bloom
	/* For.Target_Blur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Blend"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect"), TEXT("Target_Effect"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect"), TEXT("Target_Blend"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Blur_X"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur_X"), TEXT("Target_Blur_X"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Blur_Y"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur_Y"), TEXT("Target_Blur_Y"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_RadialBlur
	/* For.Target_RadialBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_RadialBlur"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_RadialBlur"), TEXT("Target_RadialBlur"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_DOFBlur
	/* For.Target_RadialBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DOFBlur"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DOFBlur"), TEXT("Target_DOFBlur"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_MotionBlur
	/* For.Target_RadialBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DiffuseMotionBlur"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_MotionBlur"), TEXT("Target_DiffuseMotionBlur"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_ColorCorrrection
	/* For.Target_ColorCorrrection (Final) */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Final"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ColorCorrrection"), TEXT("Target_Final"))))
		return E_FAIL;

#pragma endregion


	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	m_WorldMatrix._11 = ViewportDesc.Width;
	m_WorldMatrix._22 = ViewportDesc.Height;
	m_WorldMatrix._41 = 0.f;
	m_WorldMatrix._42 = 0.f;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f));

	ID3D11Texture2D* pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = g_iSizeX;
	TextureDesc.Height = g_iSizeY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL
		/*| D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE*/;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pLightDepthDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);


#ifdef _DEBUG

	// GameObject
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Diffuse"), 50.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Normal"), 150.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Depth"), 250.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_FieldDepth"), 350.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Stencil"), 450.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_RimLight"), 550.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_MotionBlur"), 1200.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_MRA"), 1300.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;


	// LightAcc
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Shade"), 700.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Specular"), 800.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;


	// ShadowObject
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_LightDepth"), 50.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;


	// Effect
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_NonLight"), 900.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blend"), 800.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Effect"), 400.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blur_X"), 200.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blur_Y"), 300.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;


	// Sky
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Sky"), 550.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// RadialBlur
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_RadialBlur"), 700.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// DOFBlur
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_DOFBlur"), 1000.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// MotionBlur
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_DiffuseMotionBlur"), 1100.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// DEFERRED INFO
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_DeferredInfo"), 900.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

#endif


	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	if (eRenderGroup >= RENDER_END)
		return E_FAIL;

	m_RenderObjects[eRenderGroup].push_back(pRenderObject);

	Safe_AddRef(pRenderObject);

	return S_OK;
}

HRESULT CRenderer::Render(_float fTimeDelta)
{
	// Sky 등을 그린다.
	if (FAILED(Render_Priority()))
		return E_FAIL;

	// 그림자를 그린다.
	if (FAILED(Render_Shadow()))
		return E_FAIL;

	// 빛에 영향을 받는 친구들을 그린다.
	if (FAILED(Render_NonBlend()))
		return E_FAIL;

	// 디퍼드에 필요한 값을 따로 저장한다.
	if (FAILED(Render_DeferredInfo()))
		return E_FAIL;


	// 빛 연산을 시작한다.
	if (FAILED(Render_Lights()))
		return E_FAIL;

	// 빛 영향을 받지 않는 이펙트 등을 그린다.
	if (FAILED(Render_Effect()))
		return E_FAIL;

	// 이펙트 끼리 연산을 한다.
	if (FAILED(Render_EffectResult()))
		return E_FAIL;



	//**** 모든 그리기가 완료되었다. ****//

	if (FAILED(Render_Result()))
		return E_FAIL;

	if (m_bLowPass == false)
	{
		// Radial블러 적용
		if (FAILED(Render_Radial_Result(fTimeDelta)))
			return E_FAIL;
		// DOF블러 적용
		if (FAILED(Render_DOF_Result()))
			return E_FAIL;
		// Motion블러 적용
		if (FAILED(Render_MotionBlur()))
			return E_FAIL;
		//**** 후처리 완료 ****//

		/////////////////////// UI를 제외하고 그려진 상황에서, 화면 색 보정 처리한다.

		Interpolate_ColorData(fTimeDelta);

		if (FAILED(Render_FinalResult()))
			return E_FAIL;
	}

	if (FAILED(Render_UI()))
		return E_FAIL;

	if (FAILED(Render_SuperUI()))
		return E_FAIL;



	/// 림 라이트
	if (m_pGameInstance->Get_DIKeyState(DIK_R, KEY_DOWN))
		m_bRimTest = !m_bRimTest;
	
	if (m_bRimTest == true)
	{
		m_fRimWidth += (0.2f - m_fRimWidth) * (fTimeDelta * 5.f);
		if ((0.2f - m_fRimWidth) < 0.001f)
		{
			m_fRimWidth = 0.2f;
		}
	}
	else
	{
		m_fRimWidth -= m_fRimWidth * (fTimeDelta * 5.f);
		if (m_fRimWidth < 0.01f)
		{
			m_fRimWidth = 0.f;
		}
	}

	// 고사양, 저사양 모드
	if (m_pGameInstance->Get_DIKeyState(DIK_E, KEY_DOWN))
		m_bLowPass = !m_bLowPass;
	
	//레벨 별 사양 처리
	//int iCurrLevel = m_pGameInstance->Get_CurrentLevelID();
	//if (3 == iCurrLevel) //LEVEL_GAMEPLAY

#ifdef _DEBUG

	//렌더 타겟 뷰 ON/OFF
	if (m_pGameInstance->Get_DIKeyState(DIK_F1, KEY_DOWN))
		m_IsRenderRTV = !m_IsRenderRTV;

	if (m_IsRenderRTV)
	{
		if (FAILED(Render_Debug()))
			return E_FAIL;
	}

	Render_IMGUI();
#endif

	return S_OK;
}

void CRenderer::Set_ColorSet(COLOR_DATA destColorData)
{
	//일단 -1로 초기화
	m_DestColorData = {};

	m_DestColorData = destColorData;
}

void CRenderer::Set_ColorSet(_int iSetIdx)
{
	switch (iSetIdx)
	{
	case 0:
		m_DestColorData = Find_ColorSet("Default");
		break;
	case 1:
		m_DestColorData = Find_ColorSet("Forest");
		break;
	case 2:
		m_DestColorData = Find_ColorSet("Night");
		break;
	default:
		break;
	}
}

void CRenderer::Save_ColorSet(string strTag, COLOR_DATA destColorData)
{
	m_ColorSets.emplace(strTag, destColorData);
}

COLOR_DATA& CRenderer::Find_ColorSet(string strTag)
{
	auto iter = m_ColorSets.find(strTag);

	if (iter == m_ColorSets.end())
		return COLOR_DATA{};

	return (iter->second);
}

void CRenderer::Setting_RadialBlur(_fvector vWorldPos, _float fRadial, _float fSubtraction)
{
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vWorldPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	m_vScreenPos = _float2(fScreenX, 1.f - fScreenY);
	m_fRadialBlurRadius = fRadial;
	m_fRadialRadiusSubtraction = fSubtraction;
}

void CRenderer::Setting_RadialBlur(_float fRadial, _float fSubtraction)
{
	m_vScreenPos = _float2(0.5f, 0.5f);
	m_fRadialBlurRadius = fRadial;
	m_fRadialRadiusSubtraction = fSubtraction;
}

void CRenderer::Update_DofFocus(_fvector vWorldPos)
{
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vWorldPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	m_vDofFocus = _float2(fScreenX, 1.f - fScreenY);
}

HRESULT CRenderer::Render_LightDepth_For_GameObject(CShader* pShader, CTransform* pTransform, CModel* pModel)
{
	if (nullptr == pShader || nullptr == pTransform || nullptr == pModel)
		return E_FAIL;

	if (FAILED(pTransform->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;

	_float4x4		ViewMatrix, ProjMatrix;
	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMLoadFloat4(&m_vShadowEyePos), XMLoadFloat4(&m_vShadowFocusPos), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fShadowAngle), (_float)(g_iOriginSizeX / g_iOriginSizeY), 0.1f, m_fShadowFar));

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", &ProjMatrix)))
		return E_FAIL;

	_uint iNumMeshes = pModel->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(pModel->Bind_ShaderResource(pShader, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (pModel->Get_ModelInfo().eType == TYPE_ANIM)
		{
			if (FAILED(pModel->Bind_BoneMatrices(pShader, "g_BoneMatrices", i)))
				return E_FAIL;
		}

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(pShader->Begin(2)))
			return E_FAIL;

		pModel->Render(i);
	}

	return S_OK;
}


#ifdef _DEBUG

HRESULT CRenderer::Add_DebugComponents(CComponent* pRenderComponent)
{
	m_DebugComponents.emplace_back(pRenderComponent);

	Safe_AddRef(pRenderComponent);

	return S_OK;
}

#endif

HRESULT CRenderer::Render_Priority()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Sky"))))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[RENDER_PRIORITY])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_PRIORITY].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = (_float)g_iSizeX;
	ViewPortDesc.Height = (_float)g_iSizeY;
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &ViewPortDesc);


	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_ShadowObject"), m_pLightDepthDSV)))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[RENDER_SHADOW])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_LightDepth();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_SHADOW].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = (_float)g_iOriginSizeX;
	ViewPortDesc.Height = (_float)g_iOriginSizeY;
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &ViewPortDesc);

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	/* 렌더타겟을 교체한다. */
	/* 이 그룹에 있는 객체들을 다 빛연산이 필요하다. => 빛연산을 후처리로 할꺼다. */
	/* 후처리를 위해서는 빛연산을 위한 데이터가 필요하다. => 빛 : 빛매니져, ☆노멀,재질☆ : 이새끼를 받아오고 싶어서!!!!! 렌더타겟에 ㅈ2ㅓ장해서 받아올라고!! */
	/* Diffuse를 0번째에 셋, Normal를 1번째에 셋 */
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_GameObjects"))))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[RENDER_NONBLEND])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_NONBLEND].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Lights()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", &m_pGameInstance->Get_Transform_Float4x4_Inverse(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", &m_pGameInstance->Get_Transform_Float4x4_Inverse(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Normal"), "g_NormalTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_LightAcc"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Render_Lights(m_pShader, m_pVIBuffer)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Effect()
{
	// 불투명한 객체들
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_NonLight"))))
		return E_FAIL;
	for (auto& pRenderObject : m_RenderObjects[RENDER_NONLIGHT])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_NONLIGHT].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;


	// 투명한 객체 + 블룸
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Effect"))))
		return E_FAIL;

	list<CGameObject*> RenderObjects;
	RenderObjects.insert(RenderObjects.end(), m_RenderObjects[RENDER_BLOOM].begin(), m_RenderObjects[RENDER_BLOOM].end());
	RenderObjects.insert(RenderObjects.end(), m_RenderObjects[RENDER_BLEND].begin(), m_RenderObjects[RENDER_BLEND].end());
	RenderObjects.sort([](CGameObject* pSour, CGameObject* pDest)->_bool
		{
			return ((CGameObject*)pSour)->Get_ViewZ() > ((CGameObject*)pDest)->Get_ViewZ();
		});


	for (auto& pRenderObject : RenderObjects)
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}

	m_RenderObjects[RENDER_BLEND].clear();
	m_RenderObjects[RENDER_BLOOM].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;


	return S_OK;
}

HRESULT CRenderer::Render_EffectResult()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur_X"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Effect"), "g_EffectTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blend"), "g_BlendTexture")))
		return E_FAIL;


	m_pVIBuffer->Bind_Buffers();

	m_pShader->Begin(DEFERRED_BLUR_X);

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur_Y"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blur_X"), "g_EffectTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blend"), "g_BlendTexture")))
		return E_FAIL;

	m_pVIBuffer->Bind_Buffers();

	m_pShader->Begin(DEFERRED_BLUR_Y);

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_DeferredInfo()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DeferredInfo"))))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[RENDER_DEFERREDINFO])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_DeferredInfo();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_DEFERREDINFO].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Result()
{
	if (m_bLowPass == false)
	{
		// 최종적으로 레디얼 블러 및 다른 블러를 먹일 생각이다.
		if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_RadialBlur"))))
			return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Diffuse"), "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Specular"), "g_SpecularTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_FieldDepth"), "g_FieldDepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Shade"), "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_LightDepth"), "g_LightDepthTexture")))
		return E_FAIL;

	// 그림자 안 그려지게끔 하는 용도이다.
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Stencil"), "g_StencilTexture")))
		return E_FAIL;

	// 섞을 이펙트들 (빛 상관 없는 애들)
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blur_Y"), "g_BlurTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Effect"), "g_EffectTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_NonLight"), "g_NonLightTexture")))
		return E_FAIL;

	// 림 라이트 + 카메라 포지션
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_RimLight"), "g_RimLightTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fRimWidth", &m_fRimWidth, sizeof(_float))))
		return E_FAIL;

	// 섞을 스카이 박스
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Sky"), "g_SkyTexture")))
		return E_FAIL;

	// 각종 디퍼드 자원
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_DeferredInfo"), "g_DeferredInfoTexture")))
		return E_FAIL;


	_float4x4		ViewMatrix, ProjMatrix;

	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMLoadFloat4(&m_vShadowEyePos), XMLoadFloat4(&m_vShadowFocusPos), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fShadowAngle), (_float)(g_iOriginSizeX / g_iOriginSizeY), 0.1f, m_fShadowFar));

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrix", &ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrix", &ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(DEFERRED_FINAL)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return  E_FAIL;

	if (m_bLowPass == false)
	{
		if (FAILED(m_pGameInstance->End_MRT()))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Render_Radial_Result(_float fTimeDelta)
{
	// DOF에 담는다.
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DOFBlur"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	// 레디얼 블러를 적용시킬 텍스쳐를 던진다.
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_RadialBlur"), "g_RadialBlur")))
		return E_FAIL;

	if (m_fRadialBlurRadius > 0.f)
		m_fRadialBlurRadius -= fTimeDelta * m_fRadialRadiusSubtraction;

	if (m_fRadialBlurRadius < 0.f)
		m_fRadialBlurRadius = 0.f;

	if (FAILED(m_pShader->Bind_RawValue("g_fRadialblurRaduis", &m_fRadialBlurRadius, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fRadialblurCenter", &m_vScreenPos, sizeof(_float2))))
		return E_FAIL;

	// 레디얼 블러를 적용시킨다.
	m_pShader->Begin(DEFERRED_BLUR_R);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_DOF_Result()
{
	// 모션블러에 담는다.
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_MotionBlur"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	// DOF를 적용시킬 텍스쳐를 던진다.
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_DOFBlur"), "g_DOFBlur")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vDOFFocus", &m_vDofFocus, sizeof(_float2))))
		return E_FAIL;


	// DOF 를 적용시킨다.
	m_pShader->Begin(8);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_MotionBlur()
{
	// 최종 컬러 보정할 최종 MRT에 담는다.
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_ColorCorrrection"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	// 모션 블러를 적용시킬 원본 텍스쳐를 던진다.
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_DiffuseMotionBlur"), "g_DiffuseMotionBlur")))
		return E_FAIL;
	// 모션 블러의 정보(방향 벨로시티)를 가진것을 던진다.
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_MotionBlur"), "g_MotionBlur")))
		return E_FAIL;

	// 모션 블러를 적용시킨다.
	m_pShader->Begin(9);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_FinalResult()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;


#pragma region 색감 보정 변수 바인딩


	if (FAILED(m_pShader->Bind_RawValue("g_bApplyCorrection", &m_bApplyCorrection, sizeof(_bool))))
		return E_FAIL;


	if (FAILED(m_pShader->Bind_RawValue("g_fExposure", &m_fExposure, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fHue", &m_fHue, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fSaturation", &m_fSaturation, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fBrightness", &m_fBrightness, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fGamma", &m_fGamma, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fVibrance", &m_fVibrance, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_fContrast", &m_fContrast, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vColorBalance", m_vColorBalance, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vWhiteBalance", m_vWhiteBalance, sizeof(_float3))))
		return E_FAIL;



	if (FAILED(m_pShader->Bind_RawValue("g_fShadowThreshold", &m_fShadowThreshold, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fHighlightThreshold", &m_fHighlightThreshold, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vShadowColor", &m_vShadowColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fShadowIntensity", &m_fShadowIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vMidtoneColor", &m_vMidtoneColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fMidtoneIntensity", &m_fMidtoneIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vHighlightColor", &m_vHighlightColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fHighlightIntensity", &m_fHighlightIntensity, sizeof(_float))))
		return E_FAIL;

#pragma endregion

	// 최종 작업물 던지기
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Final"), "g_FinalTexture")))
		return E_FAIL;

	m_pShader->Begin(DEFERRED_COLORCORRECT);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();


	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_UI])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_UI].clear();

	return S_OK;
}

HRESULT CRenderer::Render_SuperUI()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_SUPERUI])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_SUPERUI].clear();

	return S_OK;
}

void CRenderer::Render_IMGUI()
{

	if (m_pGameInstance->Get_KeyState(DIK_1, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Default"));
	if (m_pGameInstance->Get_KeyState(DIK_2, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Tutorial"));
	if (m_pGameInstance->Get_KeyState(DIK_3, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Night"));

	ImGui::Begin(u8"컬러 코렉션");

	ImGui::Checkbox(u8"적용", &m_bApplyCorrection);


	ImGui::DragFloat(u8"노출", &m_fExposure, .01f, 0.f, 3.f, "%.2f");
	//ImGui::DragFloat(u8"색조", &m_fHue, .01f, 0.f, 1.5f, "%.2f");
	ImGui::DragFloat(u8"채도", &m_fSaturation, .01f, 0.f, 2.f, "%.2f");
	ImGui::DragFloat(u8"명도", &m_fBrightness, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat(u8"감마", &m_fGamma, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat(u8"활기", &m_fVibrance, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat(u8"대비", &m_fContrast, .01f, 0.f, 3.f, "%.2f");

	ImGui::DragFloat3(u8"화이트 밸런스", m_vWhiteBalance, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat3(u8"색상 균형", m_vColorBalance, .01f, 0.f, 3.f, "%.2f");

	ImGui::ColorEdit3(u8"그림자 색상", m_vShadowColor);
	ImGui::DragFloat(u8"그림자 세기", &m_fShadowIntensity, .01f, 0.f, 1.f, "%.2f");

	ImGui::ColorEdit3(u8"중간 색상", m_vMidtoneColor);
	ImGui::DragFloat(u8"중간 세기", &m_fMidtoneIntensity, .01f, 0.f, 1.f, "%.2f");

	ImGui::ColorEdit3(u8"하이라이트 색상", m_vHighlightColor);
	ImGui::DragFloat(u8"하이라이트 세기", &m_fHighlightIntensity, .01f, 0.f, 1.f, "%.2f");

	ImGui::DragFloat(u8"그림자 임계", &m_fShadowThreshold, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat(u8"하이라이트 임계", &m_fHighlightThreshold, .01f, 0.f, 1.f, "%.2f");


	ostringstream oss;
	oss << m_fExposure << ", "
		<< m_fHue << ", "
		<< m_fSaturation << ", "
		<< m_fBrightness << ", "
		<< m_fGamma << ", "
		<< m_fVibrance << ", "
		<< m_fContrast << ", "
		<< m_vWhiteBalance[0] << ", " << m_vWhiteBalance[1] << ", " << m_vWhiteBalance[2] << ", "
		<< m_vColorBalance[0] << ", " << m_vColorBalance[1] << ", " << m_vColorBalance[2] << ", "
		<< m_vShadowColor[0] << ", " << m_vShadowColor[1] << ", " << m_vShadowColor[2] << ", "
		<< m_fShadowIntensity << ", "
		<< m_vMidtoneColor[0] << ", " << m_vMidtoneColor[1] << ", " << m_vMidtoneColor[2] << ", "
		<< m_fMidtoneIntensity << ", "
		<< m_vHighlightColor[0] << ", " << m_vHighlightColor[1] << ", " << m_vHighlightColor[2] << ", "
		<< m_fHighlightIntensity << ", "
		<< m_fShadowThreshold << ", "
		<< m_fHighlightThreshold;

	std::string colorDataStr = oss.str();

	// ImGui 텍스트 박스에 표시
	ImGui::InputTextMultiline("Color Data", &colorDataStr[0], colorDataStr.size() + 1, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);


	ImGui::End();

}

void CRenderer::Interpolate_ColorData(_float _fTimeDelta)
{
	_float fInterpolateSpeed = 3.f * _fTimeDelta;

	if (m_DestColorData.fExposure != -1.f)
		m_fExposure += (m_DestColorData.fExposure - m_fExposure) * fInterpolateSpeed;

	if (m_DestColorData.fHue != -1.f)
		m_fHue += (m_DestColorData.fHue - m_fHue) * fInterpolateSpeed;

	if (m_DestColorData.fSaturation != -1.f)
		m_fSaturation += (m_DestColorData.fSaturation - m_fSaturation) * fInterpolateSpeed;

	if (m_DestColorData.fBrightness != -1.f)
		m_fBrightness += (m_DestColorData.fBrightness - m_fBrightness) * fInterpolateSpeed;

	if (m_DestColorData.fGamma != -1.f)
		m_fGamma += (m_DestColorData.fGamma - m_fGamma) * fInterpolateSpeed;

	if (m_DestColorData.fVibrance != -1.f)
		m_fVibrance += (m_DestColorData.fVibrance - m_fVibrance) * fInterpolateSpeed;

	if (m_DestColorData.fContrast != -1.f)
		m_fContrast += (m_DestColorData.fContrast - m_fContrast) * fInterpolateSpeed;

	if (m_DestColorData.fShadowIntensity != -1.f)
		m_fShadowIntensity += (m_DestColorData.fShadowIntensity - m_fShadowIntensity) * fInterpolateSpeed;

	if (m_DestColorData.fMidtoneIntensity != -1.f)
		m_fMidtoneIntensity += (m_DestColorData.fMidtoneIntensity - m_fMidtoneIntensity) * fInterpolateSpeed;

	if (m_DestColorData.fHighlightIntensity != -1.f)
		m_fHighlightIntensity += (m_DestColorData.fHighlightIntensity - m_fHighlightIntensity) * fInterpolateSpeed;

	if (m_DestColorData.fShadowThreshold != -1.f)
		m_fShadowThreshold += (m_DestColorData.fShadowThreshold - m_fShadowThreshold) * fInterpolateSpeed;

	if (m_DestColorData.fHighlightThreshold != -1.f)
		m_fHighlightThreshold += (m_DestColorData.fHighlightThreshold - m_fHighlightThreshold) * fInterpolateSpeed;




	if (m_DestColorData.vWhiteBalance[0] != -1.f)
	{
		m_vWhiteBalance[0] += (m_DestColorData.vWhiteBalance[0] - m_vWhiteBalance[0]) * fInterpolateSpeed;
		m_vWhiteBalance[1] += (m_DestColorData.vWhiteBalance[1] - m_vWhiteBalance[1]) * fInterpolateSpeed;
		m_vWhiteBalance[2] += (m_DestColorData.vWhiteBalance[2] - m_vWhiteBalance[2]) * fInterpolateSpeed;
	}

	if (m_DestColorData.vColorBalance[0] != -1.f)
	{
		m_vColorBalance[0] += (m_DestColorData.vColorBalance[0] - m_vColorBalance[0]) * fInterpolateSpeed;
		m_vColorBalance[1] += (m_DestColorData.vColorBalance[1] - m_vColorBalance[1]) * fInterpolateSpeed;
		m_vColorBalance[2] += (m_DestColorData.vColorBalance[2] - m_vColorBalance[2]) * fInterpolateSpeed;
	}

	if (m_DestColorData.vShadowColor[0] != -1.f)
	{
		m_vShadowColor[0] += (m_DestColorData.vShadowColor[0] - m_vShadowColor[0]) * fInterpolateSpeed;
		m_vShadowColor[1] += (m_DestColorData.vShadowColor[1] - m_vShadowColor[1]) * fInterpolateSpeed;
		m_vShadowColor[2] += (m_DestColorData.vShadowColor[2] - m_vShadowColor[2]) * fInterpolateSpeed;
	}

	if (m_DestColorData.vMidtoneColor[0] != -1.f)
	{
		m_vMidtoneColor[0] += (m_DestColorData.vMidtoneColor[0] - m_vMidtoneColor[0]) * fInterpolateSpeed;
		m_vMidtoneColor[1] += (m_DestColorData.vMidtoneColor[1] - m_vMidtoneColor[1]) * fInterpolateSpeed;
		m_vMidtoneColor[2] += (m_DestColorData.vMidtoneColor[2] - m_vMidtoneColor[2]) * fInterpolateSpeed;
	}

	if (m_DestColorData.vHighlightColor[0] != -1.f)
	{
		m_vHighlightColor[0] += (m_DestColorData.vHighlightColor[0] - m_vHighlightColor[0]) * fInterpolateSpeed;
		m_vHighlightColor[1] += (m_DestColorData.vHighlightColor[1] - m_vHighlightColor[1]) * fInterpolateSpeed;
		m_vHighlightColor[2] += (m_DestColorData.vHighlightColor[2] - m_vHighlightColor[2]) * fInterpolateSpeed;
	}
}


#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{
	if (*m_pGameInstance->Get_CurrentLevelID() == 4)
		return S_OK;

	for (auto& pDebugCom : m_DebugComponents)
	{
		if (nullptr != pDebugCom)
			pDebugCom->Render();

		Safe_Release(pDebugCom);
	}
	m_DebugComponents.clear();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_GameObjects"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_ShadowObject"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_Effect"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_Blur_X"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_Blur_Y"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_Sky"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_RadialBlur"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_NonLight"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_DOFBlur"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_MotionBlur"), m_pShader, m_pVIBuffer)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_DeferredInfo"), m_pShader, m_pVIBuffer)))
		return E_FAIL;

	return S_OK;
}

#endif

CRenderer* CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer* pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CRenderer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CRenderer::Free()
{
	__super::Free();


	for (auto& RenderList : m_RenderObjects)
	{
		for (auto& pRenderObject : RenderList)
			Safe_Release(pRenderObject);
		RenderList.clear();
	}


	Safe_Release(m_pLightDepthDSV);
	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
