#include "stdafx.h"
#include "UIObject.h"
#include "Level_Tool_UI.h"
#include "Camera_Free.h"
#include "Level.h"

CLevel_Tool_UI::CLevel_Tool_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Tool_UI::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

#ifdef _DEBUG
	//IMGUI_STYLE 세팅
	m_pGameInstance->Set_IMGUIStyle(CImGUI_Manager::STYLE::PURPLE);
#endif // DEBUG

	if (FAILED(Ready_Layer_IMGUI(TEXT("Layer_IMGUI"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_Tool_UI::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;
}

HRESULT CLevel_Tool_UI::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	//윈도우 바 FPS 체크
	++m_iFPS;

	_tchar szFPS[MAX_PATH] = TEXT("");
	wsprintf(szFPS, TEXT("Level Tool_UI, %d FPS"), m_iFPS);

	if (m_fAccDelta >= 1.f)
	{
 		SetWindowText(g_hWnd, szFPS);
		m_fAccDelta = 0.f;
		m_iFPS = 0;
	}

	return S_OK;
}

HRESULT CLevel_Tool_UI::Ready_Layer_Camera(const wstring& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(5.0f);

	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 10000.0f;
	CameraDesc.vEye = _float4(0.f, 0.f, -250.f/*g_iWinSizeX * -0.1f*/, 1.f);

	CameraDesc.vAt = _float4(0.f, 0.f, 1.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(0.0f);

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_UI, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_UI::Ready_Layer_IMGUI(const wstring& strLayerTag)
{	
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_UI, strLayerTag, TEXT("Prototype_GameObject_Editor_UI"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_UI::Ready_Layer_UI(const wstring& strLayerTag)
{
	//CUIObject::UIOBJ_DESC SingleUI_Desc{};
	//SingleUI_Desc.wstrUITag = { TEXT("Single_UI") };
	//SingleUI_Desc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	//SingleUI_Desc.vSize = { 100.f, 100.f };
	//SingleUI_Desc.vPos = { SingleUI_Desc.vCenter.x/* - 200.f*/,
	//						SingleUI_Desc.vCenter.y/* - 200.f */ };
	//SingleUI_Desc.fDegree = { 0.f };
	//SingleUI_Desc.iTexIndex = { 0 };

	////단일 
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_UI, strLayerTag, TEXT("Prototype_GameObject_Single_UI"), &SingleUI_Desc)))
	//	return E_FAIL;


	//다중 (상속)
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_UI, strLayerTag, TEXT("Prototype_GameObject_Multi_UI"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_UI::Ready_RTV(const wstring& strLayerTag)
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};
	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	//렌더타겟 뷰 생성 및 준비
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_UI"), (_uint)ViewportDesc.Width, (_uint)ViewportDesc.Height, 
		DXGI_FORMAT_R32G32B32A32_FLOAT,_float4(0.f, 1.f, 0.f, 1.f))))
	{
		MSG_BOX(TEXT("Failed to Add : RenderTarget"));
		return E_FAIL;
	}

#ifdef _DEBUG
	if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_UI"), 50.f, g_iWinSizeY - 50.f, g_iWinSizeX * 0.75f, g_iWinSizeY * 0.75f)))
	{
		MSG_BOX(TEXT("Failed to Ready : RenderTarget"));
		return E_FAIL;
	}
#endif
	//XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	//XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizex, g_iWinSizeY, 0.f, 1.f));

	//텍스처 생성 및 정보 설정
	ID3D11Texture2D* pTexture2D = { nullptr };
	D3D11_TEXTURE2D_DESC	Tex2DDesc{};
	ZeroMemory(&Tex2DDesc, sizeof(D3D11_TEXTURE2D_DESC));
	//TextureDesc.Width = 256;
	//TextureDesc.Height = 256;
	Tex2DDesc.MipLevels = 1;
	Tex2DDesc.ArraySize = 1;
	Tex2DDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //DXGI_FORMAT_R8G8B8A8_UNORM;

	Tex2DDesc.SampleDesc.Quality = 0;
	Tex2DDesc.SampleDesc.Count = 1;

	Tex2DDesc.Usage = D3D11_USAGE_DEFAULT; //| D3D11_USAGE_DYNAMIC; //텍스처 값 임의 변경 여부 설정
	Tex2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //텍스처의 색상 샘플링 후 사용 용도

	Tex2DDesc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE; //CPU 연산 옵션
	Tex2DDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&Tex2DDesc, nullptr, &pTexture2D))) //텍스처 생성
	{
		MSG_BOX(TEXT("Failed to Create : ID3D11Texture2D"));
		return E_FAIL;
	}

	ID3D11RenderTargetView* pRTV = { nullptr };
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc{};
	RTVDesc.Format = Tex2DDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	if (FAILED(m_pDevice->CreateRenderTargetView(pTexture2D, &RTVDesc, &pRTV))) //렌더타겟 뷰 생성
	{
		MSG_BOX(TEXT("Failed to Create : ID3D11RenderTargetView"));
		return E_FAIL;
	}

	Safe_Release(pTexture2D);
	Safe_Release(pRTV);

	return S_OK;
}

CLevel_Tool_UI* CLevel_Tool_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Tool_UI* pInstance = new CLevel_Tool_UI(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : Level_Tool_UI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Tool_UI::Free()
{
	__super::Free();
}
