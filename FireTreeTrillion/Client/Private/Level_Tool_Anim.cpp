#include "stdafx.h"
#include "Level_Tool_Anim.h"

#include "Camera_Free.h"

CLevel_Tool_Anim::CLevel_Tool_Anim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Tool_Anim::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;
	
	if(FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	//if (FAILED(Ready_Layer_Monster(TEXT("Layer_Monster"))))
	//	return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;
	

	return S_OK;
}

void CLevel_Tool_Anim::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

}

HRESULT CLevel_Tool_Anim::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	SetWindowText(g_hWnd, TEXT("애니메이션 툴 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Lights()
{
	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_Camera(const wstring & strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(60.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, 2.f, -1.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_Player(const wstring & strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_TestModel"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_Monster(const wstring & strLayerTag)
{
	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_BackGround(const wstring & strLayerTag)
{
	/*if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_TestMap"))))
		return E_FAIL;*/

	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_UI(const wstring& strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_UI_Test"))))
	//	return E_FAIL;

	return S_OK;
}

CLevel_Tool_Anim * CLevel_Tool_Anim::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Tool_Anim*		pInstance = new CLevel_Tool_Anim(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevel_Tool_Anim"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Tool_Anim::Free()
{
	__super::Free();

}
