#include "stdafx.h"
#include "Level_Tool_Anim.h"

#include "PhysX.h"
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

	if (FAILED(Ready_Layer_Ground(TEXT("Layer_Ground"))))
		return E_FAIL;

	//if (FAILED(Ready_Layer_Character(TEXT("Layer_Character"))))
	//	return E_FAIL;

	if (FAILED(Ready_Layer_IMGUI(TEXT("Layer_IMGUI"))))
		return E_FAIL;

#ifdef _DEBUG
	m_pGameInstance->Set_IMGUIStyle(1);
#endif // DEBUG

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
	//// 예시코드 1 : 태양광
	LIGHT_DESC			LightDesc{};
	LightDesc.eType		 = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.f, -1.f, 0.f, 0.f);

	LightDesc.vDiffuse	 = _float4(0.8f, 0.8f, 0.8f, 1.f);
	LightDesc.vAmbient	 = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.vSpecular  = _float4(0.2f, 0.2f, 0.2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

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

HRESULT CLevel_Tool_Anim::Ready_Layer_Ground(const wstring& strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_Grid"))))
		return E_FAIL;

	m_pGameInstance->Ready_TestGround();

	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_Character(const wstring & strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_Kirby"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_Anim::Ready_Layer_IMGUI(const wstring& strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_ANIM, strLayerTag, TEXT("Prototype_GameObject_AnimToolHelper"))))
		return E_FAIL;

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

