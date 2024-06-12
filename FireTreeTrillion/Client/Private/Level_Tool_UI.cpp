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
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_TOOL);

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
	CameraDesc.fFovy = XMConvertToRadians(30.0f);

	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;

	CameraDesc.vEye = _float4(0.f, 0.f, -25.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 1.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

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
