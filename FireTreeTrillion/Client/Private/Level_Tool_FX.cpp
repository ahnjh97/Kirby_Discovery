#include "stdafx.h"
#include "Level_Tool_FX.h"

#include "Camera_Free.h"

CLevel_Tool_FX::CLevel_Tool_FX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Tool_FX::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;


	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;


	return S_OK;
}

void CLevel_Tool_FX::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

}

HRESULT CLevel_Tool_FX::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	SetWindowText(g_hWnd, TEXT("FX Tool"));

	return S_OK;
}

HRESULT CLevel_Tool_FX::Ready_Layer_Camera(const wstring& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(40.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, 3.f, -5.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_FX, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_FX::Ready_Layer_UI(const wstring& strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_FX, strLayerTag, TEXT("Prototype_GameObject_FXToolDirector"))))
		return E_FAIL;

	return S_OK;
}

CLevel_Tool_FX* CLevel_Tool_FX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Tool_FX* pInstance = new CLevel_Tool_FX(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_Tool_FX"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Tool_FX::Free()
{
	__super::Free();
}

