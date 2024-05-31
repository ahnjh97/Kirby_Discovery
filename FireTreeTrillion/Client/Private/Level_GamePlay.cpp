#include "stdafx.h"
#include "..\Public\Level_GamePlay.h"

#include "Kirby.h"
#include "Level_GamePlay.h"
#include "Camera_Free.h"
#include "Trigger.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	
	if(FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	/* 구한정보들을 각 랜드오브젝트르 생성할 때 던진다. */
	if (FAILED(Ready_Layer_Monster(TEXT("Layer_Monster"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;
	
	if (FAILED(Ready_ParsedObjects()))
		return E_FAIL;

	// TEST (블러와 블랜드의 관계)
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Moon"), TEXT("Prototype_GameObject_Moon"))))
		return E_FAIL;


	return S_OK;
}

void CLevel_GamePlay::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

}

HRESULT CLevel_GamePlay::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby")));
	_float4 vPos = pKirby->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);
	wstring wstrMsg = TEXT("게임플레이레벨입니다. 커비 위치 : ")
		+ to_wstring(vPos.x) + TEXT(", ") + to_wstring(vPos.y) + TEXT(", ") + to_wstring(vPos.z);
	SetWindowText(g_hWnd, wstrMsg.c_str());

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{
	//// 예시코드 1 : 태양광
	LIGHT_DESC			LightDesc{};
	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.f, -1.f, 0.f, 0.f);

	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.8f, 1.f);
	LightDesc.vAmbient = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const wstring & strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(40.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, 2.f, -1.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const wstring& strLayerTag)
{

	HRESULT hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_SkySphere"));
	CHECK_FAILED(hr);

	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_TestMap"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Player(const wstring & strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_TestModel"))))
	//	return E_FAIL;

	// Kirby
	/*if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Kirby"))))
		return E_FAIL;*/

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Monster(const wstring & strLayerTag)
{
	// Awoofy
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Awoofy"))))
		return E_FAIL;

	//// Rabbit
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Rabbit"))))
	//	return E_FAIL;

	// Buffahorn
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Buffahorn"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_UI(const wstring& strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_UI_Test"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_ParsedObjects()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;

	string strFileName = "../../../objects_txt/GamePlay.txt";
	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : GamePlay.txt"));
		return E_FAIL;
	}

	string strModelName;
	_float4x4 matWorld{};
	_int iTriggerType{};
	_int iTriggerIndex{};
	map<_int, _float4x4> camMatrices;

	while (!fileStream.eof())
	{
		_uint iStrLength;
		fileStream.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileStream.read(&strModelName[0], iStrLength);
		fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));

		if ("Camera" == strModelName || "Trigger" == strModelName) {
			if("Trigger" == strModelName)
				fileStream.read(reinterpret_cast<char*>(&iTriggerType), sizeof(iTriggerType));
			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
			if("Camera" == strModelName)
				camMatrices.emplace(iTriggerIndex, matWorld);
		}
		if (fileStream.eof())
			break;

		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		tempDesc.matWorld = matWorld;
		tempDesc.wstrModelName = CUtils::StrToWstr(strModelName);

		if (strModelName == "NonAnim_Kirby")
		{
			tempDesc.wstrModelName.erase(0, 8); // NonAnim_ 부분 지우기
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "Level1Stage1Step01" || strModelName == "Level1Stage1Step01_Blend")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_BasicMap"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "Trigger")
		{
			CTrigger::TRIGGER_DESC tTriggerDesc{};
			tTriggerDesc.matWorld = matWorld;
			tTriggerDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tTriggerDesc.iTriggerType = iTriggerType;
			tTriggerDesc.iTriggerIndex = iTriggerIndex;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Trigger"), TEXT("Prototype_GameObject_Trigger"), &tTriggerDesc)))
				return E_FAIL;
		}
	}
	fileStream.close();

	CCamera_Free* pCamera = dynamic_cast<CCamera_Free*> (m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Camera")));
	if (nullptr == pCamera)
		return E_FAIL;

	if (!camMatrices.empty()) { // 카메라 행렬 세팅
		for (auto& pair : camMatrices)
			pCamera->EmplaceBackCamMatrix(pair.second);

		pCamera->Set_MatrixIndex(0);
	}
	
	return S_OK;
}

CLevel_GamePlay * CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_GamePlay*		pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_GamePlay"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_GamePlay::Free()
{
	__super::Free();
	m_pGameInstance->Clear_EventCallBack();
}
