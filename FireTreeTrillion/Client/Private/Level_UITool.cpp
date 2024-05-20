#include "stdafx.h"
#include "Level_UITool.h"

CLevel_UITool::CLevel_UITool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_UITool::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_UITool::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;
}

HRESULT CLevel_UITool::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	_uint iFPS = { 0 };
	++iFPS;

	_tchar szFPS[MAX_PATH] = TEXT("");
	wsprintf(szFPS, TEXT("Level UI Tool, %d FPS"), iFPS);

	if (m_fAccDelta >= 1.f)
	{
 		SetWindowText(g_hWnd, szFPS);
		m_fAccDelta = 0.f;
		iFPS = 0.f;
	}

	return S_OK;
}

HRESULT CLevel_UITool::Ready_Layer_BackGround(const wstring& strLayerTag)
{
	return S_OK;
}

HRESULT CLevel_UITool::Ready_Layer_UI(const wstring& strLayerTag)
{
	// 05.20) IMGUI UI Editor Ãß°¡
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_UI, strLayerTag, TEXT("Prototype_GameObject_UI_Editor"))))
		return E_FAIL;

	return S_OK;
}

CLevel_UITool* CLevel_UITool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_UITool* pInstance = new CLevel_UITool(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : Level_UITool"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_UITool::Free()
{
	__super::Free();
}
