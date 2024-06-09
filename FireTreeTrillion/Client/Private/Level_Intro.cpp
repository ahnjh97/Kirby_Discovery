#include "stdafx.h"
#include "LevelChanger.h"
#include "Level_Intro.h"
#include "Camera_Free.h"
#include "BasicMap.h"
#include "Trigger.h"
#include "Kirby.h"
#include "BG.h"

CLevel_Intro::CLevel_Intro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Intro::Initialize()
{
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_GAMEPLAY);
	CLevelChanger::Get_Instance()->Load();

	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

	if (FAILED(Ready_ParsedObjects()))
		return E_FAIL;

	m_pGameInstance->Bind_RendererFunc(TRIGGER_SHADER);

	return S_OK;
}

void CLevel_Intro::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;
}

HRESULT CLevel_Intro::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	//윈도우 바 FPS 체크
	++m_iFPS;

	_tchar szFPS[MAX_PATH] = TEXT("");
	wsprintf(szFPS, TEXT("Level Intro, %d FPS"), m_iFPS);

	if (m_fAccDelta >= 1.f)
	{
		SetWindowText(g_hWnd, szFPS);
		m_fAccDelta = 0.f;
		m_iFPS = 0;
	}


	return S_OK;
}

HRESULT CLevel_Intro::Ready_Lights()
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

HRESULT CLevel_Intro::Ready_Layer_Camera(const wstring& strLayerTag)
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

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_INTRO, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Intro::Ready_Layer_BackGround(const wstring& strLayerTag)
{

	HRESULT hr = m_pGameInstance->Add_Clone(LEVEL_INTRO, strLayerTag, TEXT("Prototype_GameObject_SkySphere"));
	CHECK_FAILED(hr);

	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_INTRO, strLayerTag, TEXT("Prototype_GameObject_TestMap"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Intro::Ready_Layer_UI(const wstring& strLayerTag)
{
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_INTRO, strLayerTag, TEXT("Prototype_GameObject_UI_Test"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Intro::Ready_ParsedObjects()
{
	LEVEL eLevel = LEVEL_INTRO;

	string strFileName = "../../../objects_txt/Intro.txt";
	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Intro.txt"));
		return E_FAIL;
	}

	string strModelName;
	_float4x4 matWorld{};
	_int iTriggerType{};
	_int iTriggerIndex{};
	_int iCamType{};
	_float fRadius{};
	_float4x4 matInverse{};
	_uint iShaderVars{};
	_float fRimWidth{};
	map<_int, _float4x4> camMatrices;
	map<_int, pair<_vector, _float>> frontDirRadii;
	map<_int, pair<_vector, _float>> rearDirRadii;
	map<_int, pair<_float4x4, _float>> triggerInfos;

	while (!fileStream.eof())
	{
		_uint iStrLength;
		fileStream.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileStream.read(&strModelName[0], iStrLength);
		fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));
		fileStream.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileStream.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		if ("Camera" == strModelName)
		{
			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
			camMatrices.emplace(iTriggerIndex, matWorld);
		}
		else if ("Trigger" == strModelName)
		{
			fileStream.read(reinterpret_cast<char*>(&iTriggerType), sizeof(iTriggerType));
			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
			_vector vDeterminant{};
			matInverse = XMMatrixInverse(&vDeterminant, matWorld);
			triggerInfos.emplace(iTriggerIndex, pair<_float4x4, _float>(matInverse, matWorld._33));
		}
		else if ("Dummy" == strModelName)
		{
			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
			fileStream.read(reinterpret_cast<char*>(&iCamType), sizeof(iCamType));
			fileStream.read(reinterpret_cast<char*>(&fRadius), sizeof(fRadius));

			_vector vDir = XMVector3Normalize(XMVectorSet(matWorld._31, matWorld._32, matWorld._33, 0));

			if (CAM_FRONT == iCamType)
				frontDirRadii.emplace(iTriggerIndex, pair<_vector, _float>(vDir, fRadius));
			else if (CAM_REAR == iCamType)
				rearDirRadii.emplace(iTriggerIndex, pair<_vector, _float>(vDir, fRadius));
		}

		if (fileStream.eof())
			break;

		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		tempDesc.matWorld = matWorld;
		tempDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tempDesc.iShaderVars = iShaderVars;
		tempDesc.fRimWidth = fRimWidth;

		if ("NonAnim_Kirby" == strModelName)
		{
			tempDesc.wstrModelName.erase(0, 8); // NonAnim_ 부분 지우기
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Awoofy")
		{
			tempDesc.wstrModelName.erase(0, 8); // NonAnim_ 부분 지우기
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Awoofy"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Rabbit")
		{
			tempDesc.wstrModelName.erase(0, 8); // NonAnim_ 부분 지우기
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Rabbit"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Buffahorn")
		{
			tempDesc.wstrModelName.erase(0, 8); // NonAnim_ 부분 지우기
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Buffahorn"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_BladeKnight")
		{
			tempDesc.wstrModelName.erase(0, 8); // NonAnim_ 부분 지우기
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BladeKnight"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "Level0Stage1Step01")
		{
			CBasicMap::MAP_DESC tMapDesc{};
			tMapDesc.matWorld = tempDesc.matWorld;
			tMapDesc.wstrModelName = tempDesc.wstrModelName;

			_float3 vMin{}, vMax{};
			fileStream.read(reinterpret_cast<char*>(&vMin), sizeof(vMin));
			fileStream.read(reinterpret_cast<char*>(&vMax), sizeof(vMax));

			tMapDesc.vMin = vMin;
			tMapDesc.vMax = vMax;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_BasicMap"), &tMapDesc)))
				return E_FAIL;
		}
		else if (strModelName == "Level0Stage1Step01_Blend")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_BasicMap"), &tempDesc)))
				return E_FAIL;
		}
		else if ("Trigger" == strModelName)
		{
			CTrigger::TRIGGER_DESC tTriggerDesc{};
			tTriggerDesc.matWorld = matWorld;
			tTriggerDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tTriggerDesc.iTriggerType = iTriggerType;
			tTriggerDesc.iTriggerIndex = iTriggerIndex;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Trigger"), TEXT("Prototype_GameObject_Trigger"), &tTriggerDesc)))
				return E_FAIL;
		}
		else if (strModelName == "BG1")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_BG"), &tempDesc)))
				return E_FAIL;
		}
	}
	fileStream.close();

	CCamera_Free* pCamera = dynamic_cast<CCamera_Free*> (m_pGameInstance->Get_GameObject(LEVEL_INTRO, TEXT("Layer_Camera")));
	if (nullptr == pCamera)
		return E_FAIL;

	if (!camMatrices.empty()) { // 카메라 행렬 세팅
		for (auto& pair : camMatrices)
			pCamera->EmplaceBackCamMatrix(pair.second);

		pCamera->Set_MatrixIndex(0);
	}

	if (!frontDirRadii.empty())
	{
		for (auto& pair : frontDirRadii)
			pCamera->EmplaceBackDirRadius(CAM_FRONT, pair.second.first, pair.second.second);
	}

	if (!rearDirRadii.empty())
	{
		for (auto& pair : rearDirRadii)
			pCamera->EmplaceBackDirRadius(CAM_REAR, pair.second.first, pair.second.second);
	}

	if (!triggerInfos.empty())
	{
		for (auto& pair : triggerInfos)
			pCamera->EmplaceBackTriggerInfo(pair.second.first, pair.second.second);
	}

	return S_OK;
}

CLevel_Intro* CLevel_Intro::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Intro* pInstance = new CLevel_Intro(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_Intro"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Intro::Free()
{
	__super::Free();
	m_pGameInstance->Clear_EventCallBack();
}
