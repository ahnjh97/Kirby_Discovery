#include "stdafx.h"
#include "Level_GamePlay.h"
#include "LevelChanger.h"
#include "Camera_Main.h"
#include "Camera_Free.h"
#include "BasicMap.h"
#include "UIObject.h"
#include "Trigger.h"
#include "Kirby.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "BG.h"
#include "HUD.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_GAMEPLAY);
	CLevelChanger::Get_Instance()->Load();

	if (FAILED(__super::Initialize()))
		return E_FAIL;

	// 환경맵을 추가한다.
	if (FAILED(Add_EnvMap()))
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
	
	if (FAILED(Ready_ParsedObjects()))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

	// TEST (아이템 맨)
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Item"), TEXT("Prototype_GameObject_EnergyDrink"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_MapObject(TEXT("Layer_MapObject"))))
		return E_FAIL;


	m_pGameInstance->Bind_RendererFunc(TRIGGER_SHADER);

	return S_OK;
}

void CLevel_GamePlay::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;
}

HRESULT CLevel_GamePlay::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	//CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby")));
	//_float4 vPos = pKirby->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);
	//wstring wstrMsg = TEXT("게임플레이레벨입니다. 커비 위치 : ")
	//	+ to_wstring(vPos.x) + TEXT(", ") + to_wstring(vPos.y) + TEXT(", ") + to_wstring(vPos.z);
	//SetWindowText(g_hWnd, wstrMsg.c_str());

	//윈도우 바 FPS 체크
	++m_iFPS;

	_tchar szFPS[MAX_PATH] = TEXT("");
	wsprintf(szFPS, TEXT("Level GamePlay, %d FPS"), m_iFPS);

	if (m_fAccDelta >= 1.f)
	{
		SetWindowText(g_hWnd, szFPS);
		m_fAccDelta = 0.f;
		m_iFPS = 0;
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{



	//// 예시코드 1 : 태양광
	LIGHT_DESC		LightDesc{};
	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.3f, -1.f, -0.3f, 0.f);

	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.8f, 1.f);
	LightDesc.vAmbient = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	m_pGameInstance->Setting_GodRay(_float4(320.f, 600.f, 1800.f, 1.f));

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const wstring & strLayerTag)
{
	LEVEL eLevel = LEVEL_GAMEPLAY;


	CCamera_Main::CAMERA_KIRBY_DESC		MainCamDesc{};
	MainCamDesc.fFovy = XMConvertToRadians(30.0f);
	MainCamDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	MainCamDesc.fNear = 0.1f;
	MainCamDesc.fFar = 1000.0f;
	MainCamDesc.vEye = _float4(0.f, 3.f, -1.f, 1.f);
	MainCamDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	MainCamDesc.fSpeedPerSec = 10.f;
	MainCamDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	MainCamDesc.fOrigDistance = 20.f;
	MainCamDesc.fCamSensor = .3f;

	if (FAILED(m_pGameInstance->Add_Clone(eLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Main"), &MainCamDesc)))
		return E_FAIL;



	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(30.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, .5f, -1.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	
	if (FAILED(m_pGameInstance->Add_Clone(eLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const wstring& strLayerTag)
{
	HRESULT hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_SkySphere"));
	CHECK_FAILED(hr);

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
	//// Awoofy
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Awoofy"))))
	//	return E_FAIL;

	//// Rabbit
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Rabbit"))))
	//	return E_FAIL;

	// Buffahorn
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Buffahorn"))))
	//	return E_FAIL;

	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BladeKnight"))))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_UI(const wstring& _wstrLayerTag)
{
	//모든 HUD를 준비
	string strUITag = { "LayerUI" };
	CHUD::HUD_STATUS eHUDType = CHUD::STAT_NONE;

	map<CHUD::HUD_STATUS, string> HUDmap = 
	{
		{CHUD::STAT_KIRBY, "HUD_KirbyStatus"},
		{CHUD::STAT_STARPOINT, "HUD_StarPoint"},
		//{CHUD::STAT_NONE, "LayerUI"},
	};

	//auto it = HUDmap.find(eHUDType);
	//if (it != HUDmap.end()) { strUITag = it->second;	}
	//else {	strUITag = "LayerUI"; }

	for (const auto& [eHUDType, strUITag] : HUDmap)
	{
		string strFilePath = { "../../../UI_txt/" };
		string strFileExt = { "_Orig.txt" };

		strFilePath += strUITag.c_str() + strFileExt;
		if (FAILED(Load_FileData(strFilePath, FILE_UI, _wstrLayerTag)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_ParsedObjects()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;

	string strFileName = "../../../objects_txt/Stage1.txt";
	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Stage1.txt"));
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
	_uint iNumRallyPoints{};
	_float3 vRallyPointPos{};
	vector<_float4> vecRallyPoints;
	string strConnectedMonster;

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

			if(CAM_FRONT == iCamType)
				frontDirRadii.emplace(iTriggerIndex, pair<_vector, _float>(vDir, fRadius));
			else if(CAM_REAR == iCamType)
				rearDirRadii.emplace(iTriggerIndex, pair<_vector, _float>(vDir, fRadius));
		}
		else if ("NonAnim" == strModelName.substr(0, 7) && strModelName.substr(strModelName.size() - 5) != "Kirby")
		{
			vecRallyPoints.clear();
			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex)); // Monster Enum
			fileStream.read(reinterpret_cast<char*>(&iNumRallyPoints), sizeof(iNumRallyPoints));
			for (_uint iRallyPointIdx = 0; iRallyPointIdx < iNumRallyPoints; iRallyPointIdx++) {
				fileStream.read(reinterpret_cast<char*>(&vRallyPointPos), sizeof(vRallyPointPos));
				vecRallyPoints.push_back(_float4(vRallyPointPos.x, vRallyPointPos.y, vRallyPointPos.z, 1));
			}
		}
		else if ("RallyPoint" == strModelName)
		{
			_uint iStrSize{};
			fileStream.read(reinterpret_cast<char*>(&iStrSize), sizeof(iStrSize));
			strConnectedMonster.resize(iStrSize);
			fileStream.read(&strConnectedMonster[0], iStrSize);
			_int iTempIdx{};
			fileStream.read(reinterpret_cast<char*>(&iTempIdx), sizeof(iTempIdx));
			continue;
		}

		if (fileStream.eof())
			break;

		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		tempDesc.matWorld = matWorld;
		tempDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tempDesc.iShaderVars = iShaderVars;
		tempDesc.fRimWidth = fRimWidth;
		if (strModelName.size() >= 8) { // NonAnim_ 부분 지우기
			if ("NonAnim" == strModelName.substr(0, 7))
				tempDesc.wstrModelName.erase(0, 8);
		}
			
		if ("NonAnim_Kirby" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Awoofy")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Awoofy"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Rabbit")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Rabbit"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Buffahorn")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Buffahorn"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_BladeKnight")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BladeKnight"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_PoppyBrosJr")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_PoppyBrosJr"), &tempDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Kabu")
		{
			CKabu::KABU_DESC KabuDesc = {};
			KabuDesc.matWorld = matWorld;
			KabuDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			KabuDesc.iShaderVars = iShaderVars;
			KabuDesc.fRimWidth = fRimWidth;
			KabuDesc.eMoveState = CKabu::KABUMOVING_STATE(iTriggerIndex);
			KabuDesc.vecRallyPoints = vecRallyPoints;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Kabu"), &KabuDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_BrontoBurt")
		{
			CBrontoBurt::BRONTOBURT_DESC BrontoBurtDesc = {};
			BrontoBurtDesc.matWorld = matWorld;
			BrontoBurtDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			BrontoBurtDesc.iShaderVars = iShaderVars;
			BrontoBurtDesc.fRimWidth = fRimWidth;
			BrontoBurtDesc.eMoveState = CBrontoBurt::BRONTOBURTMOVING_STATE(iTriggerIndex);
			BrontoBurtDesc.vecRallyPoints = vecRallyPoints;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BrontoBurt"), &BrontoBurtDesc)))
				return E_FAIL;
		}
		else if (strModelName == "Level1Stage1Step01" || strModelName == "Level1Stage1Step01_Blend")
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
		else if (strModelName == "Level1Stage1Step01_Blend")
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_BasicMap"), &tempDesc)))
				return E_FAIL;
		}
		else if ("Trigger" == strModelName)
		{
			CTrigger::TRIGGER_DESC tTriggerDesc{};
			tTriggerDesc.matWorld = matWorld;
			tTriggerDesc.wstrModelName = tempDesc.wstrModelName;
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

	CCamera_Main* pCamera = dynamic_cast<CCamera_Main*> (m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Camera")));
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

HRESULT CLevel_GamePlay::Ready_Layer_MapObject(const wstring& strLayerTag)
{
	HRESULT hr;
	hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_WasteCan"));
	CHECK_FAILED(hr);
	return S_OK;
}

HRESULT CLevel_GamePlay::Load_FileData(const string& _strFilePath, FILE_TYPE _eFileType, const wstring& _wstrLayerTag)
{
	std::ifstream InputFile(_strFilePath, ios::in | std::ios::binary);

	if (!InputFile.is_open()) //==FALSE 
	{
		MSG_BOX(TEXT("Failed to Open : FileData"));
		return E_FAIL;
	}

	size_t size = 0;
	InputFile.read(reinterpret_cast<char*>(&size), sizeof(size));
	//m_LayerUIs.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		string strProtoTag = {};
		_uint iProtoTagLen = {};
		InputFile.read(reinterpret_cast<char*>(&iProtoTagLen), sizeof(iProtoTagLen));
		strProtoTag.resize(iProtoTagLen);
		InputFile.read(&strProtoTag[0], iProtoTagLen);

		if (0 == strProtoTag.size())
			return E_FAIL;

		CUIObject::UIOBJ_DESC LayerUIDesc{};
		string strUITag = {};
		_uint iUITagLen = {};
		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.eUIType), sizeof(LayerUIDesc.eUIType));
		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.eUIProj), sizeof(LayerUIDesc.eUIProj));

		InputFile.read(reinterpret_cast<char*>(&iUITagLen), sizeof(iUITagLen));
		strUITag.resize(iUITagLen);
		InputFile.read(&strUITag[0], iUITagLen);
		LayerUIDesc.wstrUITag = CUtils::StrToWstr(strUITag);

		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.vCenter), sizeof(LayerUIDesc.vCenter));
		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.vSize), sizeof(LayerUIDesc.vSize));
		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.vPos), sizeof(LayerUIDesc.vPos));
		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.vDegree), sizeof(LayerUIDesc.vDegree));

		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.iTexIndex), sizeof(LayerUIDesc.iTexIndex));

		string strText = {};
		_uint iUIextLen = {};
		InputFile.read(reinterpret_cast<char*>(&iUIextLen), sizeof(iUIextLen));
		strText.resize(iUIextLen);
		InputFile.read(&strText[0], iUIextLen);
		LayerUIDesc.wstrText = CUtils::StrToWstr(strText);

		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.vColorRGB), sizeof(LayerUIDesc.vColorRGB));
		InputFile.read(reinterpret_cast<char*>(&LayerUIDesc.fAlpha), sizeof(LayerUIDesc.fAlpha));

		//파일 경로명으로 prototag를 받아 생성하는 방식
		size_t strFileFrontPos = _strFilePath.find("txt/");
		if (strFileFrontPos != string::npos)
			strUITag = _strFilePath.substr(strFileFrontPos + 4);

		//Prototype_GameObject_
		size_t strFileBackPos = strUITag.find("_Orig");
		if (strFileBackPos != string::npos)
			strUITag = strUITag.substr(0, strFileBackPos);

		size_t strProtoPos = strProtoTag.find("t_"); //찾을 문자열 위치
		if (strProtoPos != string::npos)
		{
			strProtoTag = strProtoTag.substr(0, strProtoPos + 2); //해당 문자열 이후만 남김
			strProtoTag += strUITag;
		}

		HRESULT hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, _wstrLayerTag, CUtils::StrToWstr(strProtoTag), &LayerUIDesc);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Add_EnvMap()
{
	HRESULT hr;

	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Level_0_Env"),
		TEXT("Com_Texture1"), (CComponent**)&m_pEnvTexture[TYPE_ENV]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_BRDF_LUT"),
		TEXT("Com_Texture2"), (CComponent**)&m_pEnvTexture[TYPE_LUT]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_RandomNormal"),
		TEXT("Com_Texture3"), (CComponent**)&m_pEnvTexture[TYPE_NORMAL]);
	CHECK_FAILED(hr);




	// 환경맵을 던진다.
	if (FAILED(m_pGameInstance->Bind_DeferredTexture(m_pEnvTexture[TYPE_ENV], "g_EnvTexture")))
		return E_FAIL;

	//LUT 던진다.
	if (FAILED(m_pGameInstance->Bind_DeferredTexture(m_pEnvTexture[TYPE_LUT], "g_LUTTexture")))
		return E_FAIL;

	//Normal 던진다.
	if (FAILED(m_pGameInstance->Bind_DeferredTexture(m_pEnvTexture[TYPE_NORMAL], "g_RandomNormalTexture")))
		return E_FAIL;

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

	for(auto& tex : m_pEnvTexture)
		Safe_Release(tex);

	CLevelChanger::Get_Instance()->Save();
}
