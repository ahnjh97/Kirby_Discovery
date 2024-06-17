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

	Color_Initialize();
	Set_ColorSet(Find_ColorSet("Tutorial"));


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

#pragma region MRT_ResultColor

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_ResultColor"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Specular"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_SSAO"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_LensFlare"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ResultColor"), TEXT("Target_ResultColor"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ResultColor"), TEXT("Target_Specular"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ResultColor"), TEXT("Target_SSAO"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_ResultColor"), TEXT("Target_LensFlare"))))
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
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_SSAO_X"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur_X"), TEXT("Target_Blur_X"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur_X"), TEXT("Target_SSAO_X"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Blur_Y"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_SSAO_Y"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur_Y"), TEXT("Target_Blur_Y"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Blur_Y"), TEXT("Target_SSAO_Y"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_GodRay
	/* For.Target_RadialBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_GodRay"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_GodRay"), TEXT("Target_GodRay"))))
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
	/* For.Target_DOFBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DOFBlur"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DOFBlur"), TEXT("Target_DOFBlur"))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_DOFBlur_Result"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DOFBlur_Result"), TEXT("Target_DOFBlur_Result"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_MotionBlur
	/* For.Target_MotionBlur */
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

#pragma region MRT_SuperFinal
	/* For.Target_SuperFinal (SuperFinal) */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_SuperFinal"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_SuperFinal"), TEXT("Target_SuperFinal"))))
		return E_FAIL;

#pragma endregion

#pragma region MRT_UI


	//06.04) UI 렌더타겟 뷰 생성 및 준비
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_UI"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height,
		DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	//06.04) UI 렌더타겟 뷰 생성 및 준비
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_UI"), TEXT("Target_UI"))))
		return E_FAIL;

#pragma endregion


	// 기본 세팅이니 열지 마시오
#pragma region 후처리 셰이더 기본 세팅

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

#pragma endregion


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

	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_LensFlare"), 1400.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// LightAcc
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_ResultColor"), 700.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Specular"), 800.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_GodRay"), 900.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_SSAO"), 1000.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
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

	//// DEFERRED INFO 이 자리 god ray 확인용으로 좀 써주게여
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_DeferredInfo"), 900.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
	//	return E_FAIL;
	
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_UI"), ViewportDesc.Width * 0.1f / 2.f, ViewportDesc.Height * 0.1f / 2.f,
		ViewportDesc.Width * 0.1f, ViewportDesc.Height * 0.1f)))
		return E_FAIL;

#endif

	return S_OK;
}

void CRenderer::Color_Initialize()
{
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
		1.00999f,
		1.f,
		0.940155f,
		1.30984f,0.859977f, 1.04722f, 1.05019f, 0.690178f,0.6f,0.6f,1.02006f,0.960001f,1.04f,0.253046f,0.0210087f,0.0958922f,0.129992f,0.917647f,0.513726f,
		0.145098f, 0.16998f, 1.f, 0.847059f, 0.254902f, 0.330203f, 0.13988f, 0.559918f
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

	Save_ColorSet("Stage1",
		COLOR_DATA{
		0.829539f, 1.f, 1.00999f, 1.44943f, 1.17964f, 1.14037f, 1.11018f, 0.720338f, 0.6f, 0.6f, 1.3f, 1.06f, 1.1f, 0.0649942f, 0.0378847f, 0.199115f, 0.00958735f, 0.466084f, 0.676991f, 0.218674f, 0.0796085f, 0.499961f, 0.912908f, 0.99115f, 0.209722f, 0.209559f, 0.340393f
		});

	//쉐이더 타입 트리거는 idx 1, 접촉하면 해당 함수를 호출!
	function<void(_int)> TriggerFunc = bind(&CRenderer::Set_ColorSet_ByIndex, this, placeholders::_1);
	m_pGameInstance->Emplace_TriggerFunc(1, TriggerFunc);

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
	// 랜더러 기본 수치 세팅
	Render_SystemTick(fTimeDelta);

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

	// 빛 연산을 시작한다. ( PBR 연산, SSAO 연산, 스페큘러 연산 )
	if (FAILED(m_eRenderMode == MODE_TOOL ? Render_Light_For_Tool() : Render_Lights()))
		return E_FAIL;

	// 빛 영향을 받지 않는 이펙트 등을 그린다.
	if (FAILED(Render_Effect()))
		return E_FAIL;

	// 이펙트 끼리 연산을 한다.
	if (FAILED(Render_EffectResult()))
		return E_FAIL;

	//**** 모든 그리기가 완료되었다. ****//
	if (FAILED(m_eRenderMode == MODE_TOOL ? Render_Result_For_Tool() : Render_Result()))
		return E_FAIL;


	/// 블러 및 고사양 모드 시작

	if (m_eRenderMode == MODE_GAMEPLAY)
	{

		// 갓 레이 적용
		if (FAILED(Render_GodRay()))
			return E_FAIL;

		// 레디얼 블러 값이 있을때만 레디얼블러를 가동한다.
		if (m_isRadial == true)
		{
			// Radial블러 적용
			if (FAILED(Render_Radial_Result(fTimeDelta)))
				return E_FAIL;
		}

		// DOF블러 적용
		if (FAILED(Render_DOF_Result()))
			return E_FAIL;
		// Motion블러 적용
		if (FAILED(Render_MotionBlur()))
			return E_FAIL;
		// UI를 제외하고 그려진 상황에서, 화면 색 보정 처리한다.
		if (FAILED(Render_FinalResult()))
			return E_FAIL;
	}

	//**** 후처리 완료 ****//
	if (FAILED(Render_UI()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_IsRenderRTV)
	{
		if (FAILED(Render_Debug()))
			return E_FAIL;
	}
	Render_IMGUI();
#endif

	return S_OK;
}


void CRenderer::Render_SystemTick(_float fTimeDelta)
{
	// 레디얼블러가 가동중일때만, 레디얼블러 셰이더가 작동하도록 한다.
	Interpolate_RadialBlur(fTimeDelta);

	// 컬러를 보간해준다.
	Interpolate_ColorData(fTimeDelta);

	// 컨텐츠 색상변화 기능
	Interpolate_BlackBackground(fTimeDelta);

	Key_Input();
}

void CRenderer::Key_Input()
{
	// 고사양, 저사양 모드
	//if (m_pGameInstance->Get_DIKeyState(DIK_E, KEY_DOWN) && m_eRenderMode == MODE_GAMEPLAY)
	//{
	//	m_bDebugOptionControl = !m_bDebugOptionControl;
	//	for (size_t i = OPTION_SHADOW; i < OPTION_END; ++i)
	//	{
	//		m_bRenderOption[i] = m_bDebugOptionControl;
	//		Update_Option((OPTION)i, m_bRenderOption[i]);
	//	}
	//}
	
	//리얼 초저사양모드
	if (m_pGameInstance->Get_DIKeyState(DIK_E, KEY_DOWN))
	{
		m_eRenderMode == MODE_GAMEPLAY ? Set_RenderMode(MODE_TOOL) : Set_RenderMode(MODE_GAMEPLAY);
	}


#ifdef _DEBUG
	//렌더 타겟 뷰 ON/OFF
	if (m_pGameInstance->Get_DIKeyState(DIK_F1, KEY_DOWN))
		m_IsRenderRTV = !m_IsRenderRTV;
#endif

	if (m_pGameInstance->Get_KeyState(DIK_F5, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Default"));
	if (m_pGameInstance->Get_KeyState(DIK_F6, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Tutorial"));
	if (m_pGameInstance->Get_KeyState(DIK_F7, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Night"));
	if (m_pGameInstance->Get_KeyState(DIK_F8, KEY_DOWN))
		Set_ColorSet(Find_ColorSet("Stage1"));
}

HRESULT CRenderer::Bind_DeferredTexture(CTexture* pTexture, const _char* pConstantName, _uint iIndex)
{
	if (FAILED(pTexture->Bind_ShaderResource(m_pShader, pConstantName, iIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Bind_DeferredRawValue(const _char* pConstantName, const void* pData, _uint iLength)
{
	if (FAILED(m_pShader->Bind_RawValue(pConstantName, pData, iLength)))
		return E_FAIL;

	return S_OK;
}

void CRenderer::Set_ColorSet(COLOR_DATA destColorData)
{
	//일단 -1로 초기화
	m_DestColorData = {};

	m_DestColorData = destColorData;
}

void CRenderer::Set_ColorSet_ByIndex(_int iSetIdx)
{
	switch (iSetIdx)
	{
	case 0:
		m_DestColorData = Find_ColorSet("Default");
		break;
	case 1:
		m_DestColorData = Find_ColorSet("Tutorial");
		break;
	case 2:
		m_DestColorData = Find_ColorSet("Tutorial");
		break;
	case 3:
		m_DestColorData = Find_ColorSet("Stage1");
		break;
	default:
		m_DestColorData = Find_ColorSet("Default");
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

	m_isRadial = true;

}

void CRenderer::Setting_RadialBlur(_float fRadial, _float fSubtraction)
{
	m_vScreenPos = _float2(0.5f, 0.5f);
	m_fRadialBlurRadius = fRadial;
	m_fRadialRadiusSubtraction = fSubtraction;

	m_isRadial = true;

}

void CRenderer::Update_DofFocus(_fvector vWorldPos)
{
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vWorldPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	m_vDofFocus = _float2(fScreenX, 1.f - fScreenY);
}

void CRenderer::Setting_GodRay(_fvector vWorldPos)
{
	m_vGodPos = vWorldPos;

	if (FAILED(m_pShader->Bind_RawValue("g_vGodPos", &m_vGodPos, sizeof(_float4))))
		return;
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

HRESULT CRenderer::Render_LightDepth_For_PartObject(CShader* pShader, const _float4x4* pMatrix, CModel* pModel)
{
	if (nullptr == pShader || nullptr == pModel)
		return E_FAIL;

	if (FAILED(pShader->Bind_Matrix("g_WorldMatrix", pMatrix)))
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

void CRenderer::Bind_RendererFunc(_int iTriggerType)
{
	function<void(_int)> TriggerFunc = bind(&CRenderer::Set_ColorSet_ByIndex, this, placeholders::_1);
	m_pGameInstance->Emplace_TriggerFunc(iTriggerType, TriggerFunc);
}

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
		if (nullptr != pRenderObject && m_bRenderOption[OPTION_SHADOW] == true)
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
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_ResultColor"))))
		return E_FAIL;

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
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_MRA"), "g_MRATexture")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_GodViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_GodProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Render_Lights(m_pShader, m_pVIBuffer)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Light_For_Tool()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_ResultColor"))))
		return E_FAIL;


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

	if (FAILED(m_pGameInstance->Render_Lights(m_pShader, m_pVIBuffer, true)))
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

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Specular"), "g_SpecularTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_SSAO"), "g_SSAOTexture")))
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

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_SSAO_X"), "g_SSAOTexture")))
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
	// God에 보낸다.
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_GodRay"))))
		return E_FAIL;


	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Diffuse"), "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_FieldDepth"), "g_FieldDepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_ResultColor"), "g_LinearTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_LightDepth"), "g_LightDepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_RimLight"), "g_RimLightTexture")))
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

	//// SSAO 연산
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_SSAO_Y"), "g_SSAOTexture")))
		return E_FAIL;
	// 카메라 포지션
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	// 검은 배경의 color 배율
	if (FAILED(m_pShader->Bind_RawValue("g_fBlackBackGround", &m_fBlackBackground, sizeof(_float))))
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


	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Result_For_Tool()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_SuperFinal"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Diffuse"), "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_FieldDepth"), "g_FieldDepthTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_LightDepth"), "g_LightDepthTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Stencil"), "g_StencilTexture")))
		return E_FAIL;

	// 섞을 이펙트들 (빛 상관 없는 애들)
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blur_Y"), "g_BlurTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Effect"), "g_EffectTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_NonLight"), "g_NonLightTexture")))
		return E_FAIL;

	// SSAO 연산
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_SSAO"), "g_SSAOTexture")))
		return E_FAIL;

	// 카메라 포지션
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	// 섞을 스카이 박스
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Sky"), "g_SkyTexture")))
		return E_FAIL;

	_float4x4		ViewMatrix, ProjMatrix;

	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMLoadFloat4(&m_vShadowEyePos), XMLoadFloat4(&m_vShadowFocusPos), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fShadowAngle), (_float)(g_iOriginSizeX / g_iOriginSizeY), 0.1f, m_fShadowFar));

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrix", &ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(DEFERRED_FINAL_TOOL)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return  E_FAIL;

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_GodRay()
{
	// 최적화를 위해 레디얼 블러가 작동중일때만, MRT_RadialBlur가 작동하도록 한다.
	if (m_isRadial == true)
	{
		// 최종적으로 레디얼 블러 및 다른 블러를 먹일 생각이다.
		if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_RadialBlur"))))
			return E_FAIL;
	}
	else
	{
		// DOF에 담는다.
		if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DOFBlur"))))
			return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_GodRay"), "g_GodRayTexture")))
		return E_FAIL;


	if (FAILED(m_pShader->Bind_Matrix("g_GodViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_GodProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	//_float4 vTemp = Dir(m_pGameInstance->Get_Transform_Inv(CPipeLine::D3DTS_VIEW).Forward());
	_float4 asd = CUtils::Get_State_Vector_Matrix(m_pGameInstance->Get_Transform_Inv(CPipeLine::D3DTS_VIEW), CUtils::STATE_LOOK);

	// 카메라 포지션
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vCamLook", &asd, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(DEFERRED_GODRAY)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return  E_FAIL;

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;


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

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DOFBlur_Result"))))
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
	m_pShader->Begin(DEFERRED_DOF_X);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	///////////////////////////////

	// 모션블러에 담는다.
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_MotionBlur"))))
		return E_FAIL;

	// DOF를 적용시킬 텍스쳐를 던진다.
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_DOFBlur_Result"), "g_DOFBlur_Result")))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_RawValue("g_vDOFFocus", &m_vDofFocus, sizeof(_float2))))
		return E_FAIL;

	// DOF 를 적용시킨다.
	m_pShader->Begin(DEFERRED_DOF_Y);

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
	m_pShader->Begin(DEFERRED_MOTIONBLUR);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_FinalResult()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_SuperFinal"))))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	_float4 vCamLook = CUtils::Get_State_Vector_Matrix(m_pGameInstance->Get_Transform_Inv(CPipeLine::D3DTS_VIEW), CUtils::STATE_LOOK);

	// 카메라 포지션
	if (FAILED(m_pShader->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_vCamLook", &vCamLook, sizeof(_float4))))
		return E_FAIL;


	//// LensFlare 연산
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_LensFlare"), "g_LensFlareTexture")))
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

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_UI"))))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[RENDER_UI])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_UI].clear();

	//SuperUI :: Fadein FadeOut/트랜잭션 효과 등을 표현할 때 최우선 순위로 렌더할 UI 요소
	for (auto& pRenderObject : m_RenderObjects[RENDER_SUPERUI])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_SUPERUI].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_SuperFinal"), "g_FinalTexture")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_UI"), "g_UITexture")))
		return E_FAIL;

	m_pShader->Begin(DEFERRED_UI);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();

	return S_OK;
}

#ifdef _DEBUG

void CRenderer::Render_IMGUI()
{


	ImGui::Begin(u8"렌더러");

	ImGui::SeparatorText(u8"렌더옵션");
	if (ImGui::Checkbox(u8"그림자", &m_bRenderOption[OPTION_SHADOW]))
		Update_Option(OPTION_SHADOW, m_bRenderOption[OPTION_SHADOW]);
	if (ImGui::Checkbox(u8"싸오", &m_bRenderOption[OPTION_SSAO]))
		Update_Option(OPTION_SSAO, m_bRenderOption[OPTION_SSAO]);
	if (ImGui::Checkbox(u8"디오에프", &m_bRenderOption[OPTION_DOF]))
		Update_Option(OPTION_DOF, m_bRenderOption[OPTION_DOF]);
	if (ImGui::Checkbox(u8"모샨블라", &m_bRenderOption[OPTION_MOTIONBLUR]))
		Update_Option(OPTION_MOTIONBLUR, m_bRenderOption[OPTION_MOTIONBLUR]);

	ImGui::SeparatorText(u8"컬러코렉션");

	ImGui::DragFloat(u8"노출", &m_DestColorData.fExposure, .01f, 0.f, 3.f, "%.2f");
	//ImGui::DragFloat(u8"색조", &m_DestColorData.fHue, .01f, 0.f, 1.5f, "%.2f");
	ImGui::DragFloat(u8"채도", &m_DestColorData.fSaturation, .01f, 0.f, 2.f, "%.2f");
	ImGui::DragFloat(u8"명도", &m_DestColorData.fBrightness, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat(u8"감마", &m_DestColorData.fGamma, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat(u8"활기", &m_DestColorData.fVibrance, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat(u8"대비", &m_DestColorData.fContrast, .01f, 0.f, 3.f, "%.2f");

	ImGui::DragFloat3(u8"화이트 밸런스", m_DestColorData.vWhiteBalance, .01f, 0.f, 3.f, "%.2f");
	ImGui::DragFloat3(u8"색상 균형", m_DestColorData.vColorBalance, .01f, 0.f, 3.f, "%.2f");

	ImGui::ColorEdit3(u8"그림자 색상", m_DestColorData.vShadowColor);
	ImGui::DragFloat(u8"그림자 세기", &m_DestColorData.fShadowIntensity, .01f, 0.f, 1.f, "%.2f");

	ImGui::ColorEdit3(u8"중간 색상", m_DestColorData.vMidtoneColor);
	ImGui::DragFloat(u8"중간 세기", &m_DestColorData.fMidtoneIntensity, .01f, 0.f, 1.f, "%.2f");

	ImGui::ColorEdit3(u8"하이라이트 색상", m_DestColorData.vHighlightColor);
	ImGui::DragFloat(u8"하이라이트 세기", &m_DestColorData.fHighlightIntensity, .01f, 0.f, 1.f, "%.2f");

	ImGui::DragFloat(u8"그림자 임계", &m_DestColorData.fShadowThreshold, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat(u8"하이라이트 임계", &m_DestColorData.fHighlightThreshold, .01f, 0.f, 1.f, "%.2f");


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

#endif

void CRenderer::Interpolate_ColorData(_float _fTimeDelta)
{
	if (!m_bApplyCorrection)
		return;

	_float fInterpolateSpeed = 2.f * _fTimeDelta;

	if (.01f < abs(m_fExposure - m_DestColorData.fExposure))
	{
		m_fExposure += (m_DestColorData.fExposure - m_fExposure) * fInterpolateSpeed;

		if (abs(m_fExposure - m_DestColorData.fExposure) < 0.001f)
		{
			m_fExposure = m_DestColorData.fExposure;
			//m_DestColorData.fExposure = -1.f;
		}
	}

	if (.01f < abs(m_fSaturation - m_DestColorData.fSaturation))
	{
		m_fSaturation += (m_DestColorData.fSaturation - m_fSaturation) * fInterpolateSpeed;
		if (abs(m_fSaturation - m_DestColorData.fSaturation) < 0.001f)
		{
			m_fSaturation = m_DestColorData.fSaturation;
			//m_DestColorData.fSaturation = -1.f;
		}
	}

	if (.01f < abs(m_fBrightness - m_DestColorData.fBrightness))
	{
		m_fBrightness += (m_DestColorData.fBrightness - m_fBrightness) * fInterpolateSpeed;
		if (abs(m_fBrightness - m_DestColorData.fBrightness) < 0.001f)
		{
			m_fBrightness = m_DestColorData.fBrightness;
			//m_DestColorData.fBrightness = -1.f;
		}
	}

	if (.01f < abs(m_fGamma - m_DestColorData.fGamma))
	{
		m_fGamma += (m_DestColorData.fGamma - m_fGamma) * fInterpolateSpeed;
		if (abs(m_fGamma - m_DestColorData.fGamma) < 0.001f)
		{
			m_fGamma = m_DestColorData.fGamma;
			//m_DestColorData.fGamma = -1.f;
		}
	}

	if (.01f < abs(m_fVibrance - m_DestColorData.fVibrance))
	{
		m_fVibrance += (m_DestColorData.fVibrance - m_fVibrance) * fInterpolateSpeed;
		if (abs(m_fVibrance - m_DestColorData.fVibrance) < 0.001f)
		{
			m_fVibrance = m_DestColorData.fVibrance;
			//m_DestColorData.fHue = -1.f;
		}
	}

	if (.01f < abs(m_fContrast - m_DestColorData.fContrast))
	{
		m_fContrast += (m_DestColorData.fContrast - m_fContrast) * fInterpolateSpeed;
		if (abs(m_fContrast - m_DestColorData.fContrast) < 0.001f)
		{
			m_fContrast = m_DestColorData.fContrast;
			//m_DestColorData.fContrast = -1.f;
		}
	}

	if (.01f < abs(m_fShadowIntensity - m_DestColorData.fShadowIntensity))
	{
		m_fShadowIntensity += (m_DestColorData.fShadowIntensity - m_fShadowIntensity) * fInterpolateSpeed;
		if (abs(m_fShadowIntensity - m_DestColorData.fShadowIntensity) < 0.001f)
		{
			m_fShadowIntensity = m_DestColorData.fShadowIntensity;
			//m_DestColorData.fShadowIntensity = -1.f;
		}
	}

	if (.01f < abs(m_fMidtoneIntensity - m_DestColorData.fMidtoneIntensity))
	{
		m_fMidtoneIntensity += (m_DestColorData.fMidtoneIntensity - m_fMidtoneIntensity) * fInterpolateSpeed;
		if (abs(m_fMidtoneIntensity - m_DestColorData.fMidtoneIntensity) < 0.001f)
		{
			m_fMidtoneIntensity = m_DestColorData.fMidtoneIntensity;
			//m_DestColorData.fMidtoneIntensity = -1.f;
		}
	}

	if (.01f < abs(m_fHighlightIntensity - m_DestColorData.fHighlightIntensity))
	{
		m_fHighlightIntensity += (m_DestColorData.fHighlightIntensity - m_fHighlightIntensity) * fInterpolateSpeed;
		if (abs(m_fHighlightIntensity - m_DestColorData.fHighlightIntensity) < 0.001f)
		{
			m_fHighlightIntensity = m_DestColorData.fHighlightIntensity;
			//m_DestColorData.fHighlightIntensity = -1.f;
		}
	}

	if (.01f < abs(m_fShadowThreshold - m_DestColorData.fShadowThreshold))
	{
		m_fShadowThreshold += (m_DestColorData.fShadowThreshold - m_fShadowThreshold) * fInterpolateSpeed;
		if (abs(m_fShadowThreshold - m_DestColorData.fShadowThreshold) < 0.001f)
		{
			m_fShadowThreshold = m_DestColorData.fShadowThreshold;
			//m_DestColorData.fShadowThreshold = -1.f;
		}
	}

	if (.01f < abs(m_fHighlightThreshold - m_DestColorData.fHighlightThreshold))
	{
		m_fHighlightThreshold += (m_DestColorData.fHighlightThreshold - m_fHighlightThreshold) * fInterpolateSpeed;


		if (abs(m_fHighlightThreshold - m_DestColorData.fHighlightThreshold) < 0.001f)
		{
			m_fHighlightThreshold = m_DestColorData.fHighlightThreshold;
			//m_DestColorData.fHighlightThreshold = -1.f;
		}
	}


	if (.01f < abs(m_vWhiteBalance[0] - m_DestColorData.vWhiteBalance[0]))
	{

		m_vWhiteBalance[0] += (m_DestColorData.vWhiteBalance[0] - m_vWhiteBalance[0]) * fInterpolateSpeed;
		m_vWhiteBalance[1] += (m_DestColorData.vWhiteBalance[1] - m_vWhiteBalance[1]) * fInterpolateSpeed;
		m_vWhiteBalance[2] += (m_DestColorData.vWhiteBalance[2] - m_vWhiteBalance[2]) * fInterpolateSpeed;

		if (abs(m_vWhiteBalance[0] - m_DestColorData.vWhiteBalance[0]) < 0.001f)
		{

			memcpy(m_vWhiteBalance, m_DestColorData.vWhiteBalance, sizeof(_float3));


		}
	}

	if (.01f < abs(m_vColorBalance[0] - m_DestColorData.vColorBalance[0]))
	{

		m_vColorBalance[0] += (m_DestColorData.vColorBalance[0] - m_vColorBalance[0]) * fInterpolateSpeed;
		m_vColorBalance[1] += (m_DestColorData.vColorBalance[1] - m_vColorBalance[1]) * fInterpolateSpeed;
		m_vColorBalance[2] += (m_DestColorData.vColorBalance[2] - m_vColorBalance[2]) * fInterpolateSpeed;

		if (abs(m_vColorBalance[0] - m_DestColorData.vColorBalance[0]) < 0.001f)
		{
			memcpy(m_vColorBalance, m_DestColorData.vColorBalance, sizeof(_float3));
		}
	}

	if (.01f < abs(m_vShadowColor[0] - m_DestColorData.vShadowColor[0]))
	{
		m_vShadowColor[0] += (m_DestColorData.vShadowColor[0] - m_vShadowColor[0]) * fInterpolateSpeed;
		m_vShadowColor[1] += (m_DestColorData.vShadowColor[1] - m_vShadowColor[1]) * fInterpolateSpeed;
		m_vShadowColor[2] += (m_DestColorData.vShadowColor[2] - m_vShadowColor[2]) * fInterpolateSpeed;

		if (abs(m_vShadowColor[0] - m_DestColorData.vShadowColor[0]) < 0.001f)
		{
			memcpy(m_vShadowColor, m_DestColorData.vShadowColor, sizeof(_float3));

		}
	}

	if (.01f < abs(m_vMidtoneColor[0] - m_DestColorData.vMidtoneColor[0]))
	{
		m_vMidtoneColor[0] += (m_DestColorData.vMidtoneColor[0] - m_vMidtoneColor[0]) * fInterpolateSpeed;
		m_vMidtoneColor[1] += (m_DestColorData.vMidtoneColor[1] - m_vMidtoneColor[1]) * fInterpolateSpeed;
		m_vMidtoneColor[2] += (m_DestColorData.vMidtoneColor[2] - m_vMidtoneColor[2]) * fInterpolateSpeed;

		if (abs(m_vMidtoneColor[0] - m_DestColorData.vMidtoneColor[0]) < 01.f)
		{
			memcpy(m_vMidtoneColor, m_DestColorData.vMidtoneColor, sizeof(_float3));

		}
	}

	if (.01f < abs(m_vHighlightColor[0] - m_DestColorData.vHighlightColor[0]))
	{
		m_vHighlightColor[0] += (m_DestColorData.vHighlightColor[0] - m_vHighlightColor[0]) * fInterpolateSpeed;
		m_vHighlightColor[1] += (m_DestColorData.vHighlightColor[1] - m_vHighlightColor[1]) * fInterpolateSpeed;
		m_vHighlightColor[2] += (m_DestColorData.vHighlightColor[2] - m_vHighlightColor[2]) * fInterpolateSpeed;

		if (abs(m_vHighlightColor[0] - m_DestColorData.vHighlightColor[0]) < 01.f)
		{
			memcpy(m_vHighlightColor, m_DestColorData.vHighlightColor, sizeof(_float3));

		}
	}
}

void CRenderer::Interpolate_BlackBackground(_float fTimeDelta)
{
	if (m_bBlackBackground == false)
	{
		m_fBlackBackground += fTimeDelta * 1.2f;
		if (m_fBlackBackground > 1.f)
			m_fBlackBackground = 1.f;
	}
	else
	{
		m_fBlackBackground -= fTimeDelta * 1.2f;
		if (m_fBlackBackground < 0.5f)
			m_fBlackBackground = 0.5f;
	}
}

void CRenderer::Interpolate_RadialBlur(_float fTimeDelta)
{

	if (m_fRadialBlurRadius > 0.f)
		m_fRadialBlurRadius -= fTimeDelta * m_fRadialRadiusSubtraction;

	if (m_fRadialBlurRadius < 0.f)
		m_fRadialBlurRadius = 0.f;


	if (m_fRadialBlurRadius == 0.f)
		m_isRadial = false;

}

void CRenderer::Update_Option(OPTION Option, _bool bOn)
{
	m_bRenderOption[Option] = bOn;

	string strValue = "";

	switch (Option) {
	case OPTION_SHADOW: strValue = "g_bRenderShadow"; break;
	case OPTION_SSAO: strValue = "g_bRenderSSAO"; break;
	case OPTION_DOF: strValue = "g_bRenderDOF"; break;
	case OPTION_MOTIONBLUR: strValue = "g_bRenderMotionBlur"; break;
	}

	if (FAILED(m_pShader->Bind_RawValue(strValue.c_str(), &m_bRenderOption[Option], sizeof(_bool))))
		return;
}

#ifdef _DEBUG

HRESULT CRenderer::Render_Debug()
{

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
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_ResultColor"), m_pShader, m_pVIBuffer)))
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
	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_GodRay"), m_pShader, m_pVIBuffer)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Draw_RTVDebug(TEXT("MRT_UI"), m_pShader, m_pVIBuffer)))
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

	Safe_Release(m_pUISRV);
	Safe_Release(m_pLightDepthDSV);
	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
