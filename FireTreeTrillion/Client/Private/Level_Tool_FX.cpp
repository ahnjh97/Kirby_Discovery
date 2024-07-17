#include "stdafx.h"
#include "Level_Tool_FX.h"

#include "Camera_Free.h"

CLevel_Tool_FX::CLevel_Tool_FX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Tool_FX::Initialize()
{
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_TOOL);

	if (FAILED(__super::Initialize()))
		return E_FAIL;


	//// �¾籤
	LIGHT_DESC			LightDesc{};
	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.3f, -1.f, 0.3f, 0.f);

	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(.2f, .2f, .2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
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
	CameraDesc.fFar = g_fCamFar;
	CameraDesc.vEye = _float4(0.f, 3.f, -5.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_FX::Ready_Layer_UI(const wstring& strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_FXToolDirector"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Tool_FX::Ready_Layer_BackGround(const wstring& strLayerTag)
{

	CGameObject::GAMEOBJECT_DESC ObjDesc{};
	_float4x4 InitMat = _float4x4::Identity;
	InitMat.Translation({ 4.f, 0.f, 0.f });
	ObjDesc.matWorld = InitMat;

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_AnimToolObject"), &ObjDesc)))
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
	m_pGameInstance->Clear_EventCallBack();
	__super::Free();
}

