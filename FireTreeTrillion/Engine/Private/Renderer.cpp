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
	, m_pGameInstance {CGameInstance::Get_Instance()}
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

#pragma region MRT_Sky
	// 스카이와 블룸이 공존할 수 있게 랜더타겟으로 별도로 스카이박스를 처리한다.
	/* For.Target_Sky */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Sky"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Sky"), TEXT("Target_Sky"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_RadialBlur
	/* For.Target_RadialBlur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_RadialBlur"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_RadialBlur"), TEXT("Target_RadialBlur"))))
		return E_FAIL;
#pragma endregion

#pragma region MRT_Bloom
	/* For.Target_Blur */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Effect"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Effect"), TEXT("Target_Effect"))))
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

	ID3D11Texture2D*		pDepthStencilTexture = nullptr;

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

	/* RenderTarget */
	/* ShaderResource */
	/* DepthStencil */

	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pLightDepthDSV)))
		return E_FAIL;

	Safe_Release(pDepthStencilTexture);


#ifdef _DEBUG

	// GameObject
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Diffuse"), 25.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Diffuse"), 50.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Normal"), 75.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Normal"), 150.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Depth"), 125.0f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Depth"), 250.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_FieldDepth"), 175.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_FieldDepth"), 350.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Stencil"), 225.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Stencil"), 450.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_RimLight"), 275.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_RimLight"), 550.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	// LightAcc
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Shade"), 325.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Shade"), 700.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Specular"), 800.f, ViewportDesc.Height - 50.f, 100.f, 100.f)))
		return E_FAIL;

	// ShadowObject
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_LightDepth"), 425.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_LightDepth"), 50.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// Bloom (Blur)
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blur_X"), 475.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blur_X"), 200.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blur_Y"), 525.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Blur_Y"), 300.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Effect"), 575.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Effect"), 400.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// Sky
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Sky"), 625.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_Sky"), 550.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;

	// RadialBlur
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_RadialBlur"), 675.f, 695.f, 50.f, 50.f)))
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_RadialBlur"), 700.f, ViewportDesc.Height - 150.f, 100.f, 100.f)))
		return E_FAIL;
#endif


	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject * pRenderObject)
{
	if (eRenderGroup >= RENDER_END)
		return E_FAIL;

	m_RenderObjects[eRenderGroup].push_back(pRenderObject);

	Safe_AddRef(pRenderObject);

	return S_OK;
}

HRESULT CRenderer::Render(_float fTimeDelta)
{
	if (FAILED(Render_Priority()))
		return E_FAIL;

	if (FAILED(Render_Shadow()))
		return E_FAIL;

	if (FAILED(Render_NonBlend()))
		return E_FAIL;

	if (FAILED(Render_Lights()))
		return E_FAIL;

	/////////////////////// 빛 처리가 완료되었다.

	if (FAILED(Render_Bloom()))
		return E_FAIL;
	if (FAILED(Render_BloomResult()))
		return E_FAIL;
	if (FAILED(Render_Result()))
		return E_FAIL;
	if (FAILED(Render_Blur_Result(fTimeDelta)))
		return E_FAIL;

	/////////////////////// 블룸 및 Sky와의 결합과 레디얼 블러까지 완료되었다.
	if (FAILED(Render_NonLight()))
		return E_FAIL;
	if (FAILED(Render_Blend()))
		return E_FAIL;

	if (FAILED(Render_UI()))
		return E_FAIL;

	if (FAILED(Render_SuperUI()))
		return E_FAIL;


#ifdef _DEBUG
	if (FAILED(Render_Debug()))
		return E_FAIL;
#endif

	return S_OK;
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

HRESULT CRenderer::Render_LightDepth_For_GameObject(CShader* pShader, CTransform* pTransform, CModel* pModel)
{
	if (nullptr == pShader || nullptr == pTransform || nullptr == pModel)
		return E_FAIL;

	if (FAILED(pTransform->Bind_ShaderResource(pShader, "g_WorldMatrix")))
		return E_FAIL;

	_float4x4		ViewMatrix, ProjMatrix;
	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMLoadFloat4(&m_vShadowEyePos), XMLoadFloat4(&m_vShadowFocusPos), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fShadowAngle), (_float)g_iOriginSizeX / g_iOriginSizeY, 0.1f, m_fShadowFar));

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

HRESULT CRenderer::Add_DebugComponents(CComponent * pRenderComponent)
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
		if(nullptr != pRenderObject)
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
	//ViewPortDesc.Width = (_float)1280.0f;
	//ViewPortDesc.Height = (_float)720.0f;
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

HRESULT CRenderer::Render_NonLight()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_NONLIGHT])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}

	m_RenderObjects[RENDER_NONLIGHT].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Bloom()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Effect"))))
		return E_FAIL;

	m_RenderObjects[RENDER_BLOOM].sort([](CGameObject* pSour, CGameObject* pDest)->_bool
		{
			return ((CGameObject*)pSour)->Get_ViewZ() > ((CGameObject*)pDest)->Get_ViewZ();
		});

	for (auto& pRenderObject : m_RenderObjects[RENDER_BLOOM])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}

	m_RenderObjects[RENDER_BLOOM].clear();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_BloomResult()
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

	m_pVIBuffer->Bind_Buffers();

	m_pShader->Begin(4);

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Blur_Y"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blur_X"), "g_EffectTexture")))
		return E_FAIL;

	m_pVIBuffer->Bind_Buffers();

	m_pShader->Begin(5);

	m_pVIBuffer->Render();

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{	
	m_RenderObjects[RENDER_BLEND].sort([](CGameObject* pSour, CGameObject* pDest)->_bool
		{
			return ((CGameObject*)pSour)->Get_ViewZ() > ((CGameObject*)pDest)->Get_ViewZ();
		});

	for (auto& pRenderObject : m_RenderObjects[RENDER_BLEND])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_BLEND].clear();

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

HRESULT CRenderer::Render_Result()
{
	// 최종적으로 레디얼 블러 및 다른 블러를 먹일 생각이다.
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_RadialBlur"))))
		return E_FAIL;

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

	// 블러 처리. Effect는 원본 블러는 가중치로 그린것이다. (Y는 X와 같이 그린것임.)
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Blur_Y"), "g_BlurTexture")))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Effect"), "g_EffectTexture")))
		return E_FAIL;

	// 섞을 스카이 박스
	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShader, TEXT("Target_Sky"), "g_SkyTexture")))
		return E_FAIL;


	_float4x4		ViewMatrix, ProjMatrix;

	XMStoreFloat4x4(&ViewMatrix, XMMatrixLookAtLH(XMLoadFloat4(&m_vShadowEyePos), XMLoadFloat4(&m_vShadowFocusPos), XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fShadowAngle), (_float)g_iOriginSizeX / g_iOriginSizeY, 0.1f, m_fShadowFar));

	if (FAILED(m_pShader->Bind_Matrix("g_LightViewMatrix", &ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Bind_Matrix("g_LightProjMatrix", &ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(3)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return  E_FAIL;

	if (FAILED(m_pGameInstance->End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Blur_Result(_float fTimeDelta)
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	// 최종 작업물 던지기
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

	m_pShader->Begin(6);

	m_pVIBuffer->Bind_Buffers();

	m_pVIBuffer->Render();
	return S_OK;
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

	return S_OK;	
}

#endif

CRenderer * CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer*		pInstance = new CRenderer(pDevice, pContext);

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
