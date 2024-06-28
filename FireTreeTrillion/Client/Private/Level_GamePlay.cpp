#include "stdafx.h"
#include "Level_GamePlay.h"
#include "Camera_Main.h"
#include "Camera_Free.h"
#include "BasicMap.h"
#include "UIObject.h"
#include "Trigger.h"
#include "Kirby.h"
#include "Awoofy.h"
#include "Rabbit.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "PoppyBrosJr.h"
#include "SkySphere.h"

#include "BG.h"
#include "HUD.h"

#include "Starblock.h"
#include "StarblockPiece.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_GAMEPLAY);


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

	HRESULT hr{};
	hr = Ready_Map();
	CHECK_FAILED(hr);
	hr = Ready_Triggers();
	CHECK_FAILED(hr);
	hr = Ready_Monsters();
	CHECK_FAILED(hr);
	hr = Ready_Items();
	CHECK_FAILED(hr);
	hr = Ready_Kickables();
	CHECK_FAILED(hr);

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

	hr = m_pGameInstance->Add_Clone(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_LevelChangeStar"));
	CHECK_FAILED(hr);

	//// Ladder Test
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Ladder"), TEXT("Prototype_GameObject_Ladder"))))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Item"), TEXT("Prototype_GameObject_Coin"))))
	//	return E_FAIL;
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Item"), TEXT("Prototype_GameObject_EnergyDrink"))))
	//	return E_FAIL;

	//CStarBlockPiece::PIECE_DESC desc{};
	//desc.vInitialPos = _float4(1.f, 15.f, -179.f,1.f);
	//HRESULT hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_MapObject"), TEXT("Prototype_GameObject_StarBlockPiece"), &desc);
	//CHECK_FAILED(hr);

	//if (FAILED(Ready_Layer_MapObject(TEXT("Layer_MapObject"))))
	//	return E_FAIL;

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
	CSkySphere::SKYSPHERE_DESC GPSkyDesc{};
	GPSkyDesc.strModelTag = { "SkySphere_Stage1_Day" };

	HRESULT hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_SkySphere"), &GPSkyDesc);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_UI(const wstring& _wstrLayerTag)
{
	//모든 HUD를 준비
	string strUITag = { "LayerUI" };
	CHUD::UI_TAG eHUDType = CHUD::TAG_NONE;

	map<CHUD::UI_TAG, string> HUDmap =
	{
		{CHUD::HUD_KIRBYHP, "HUD_KirbyStatus"},
		{CHUD::HUD_STARPOINT, "HUD_StarPoint"},
		{CHUD::HUD_ABILITYDISCARD, "HUD_AbilityDiscard"},
	};

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

HRESULT CLevel_GamePlay::Ready_Map()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;
	string strFileName = "../../../objects_txt/Stage1_Map.txt";
	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Stage1_Map.txt"));
		return E_FAIL;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_float3 vMin{}, vMax{};
	wstring wstrGameObjectTag;

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileInput.read(reinterpret_cast<char*>(&vMin), sizeof(vMin));
		fileInput.read(reinterpret_cast<char*>(&vMax), sizeof(vMax));

		CBasicMap::MAP_DESC tMapDesc{};
		tMapDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tMapDesc.matWorld = matWorld;
		tMapDesc.vMin = vMin;
		tMapDesc.vMax = vMax;

		if ("BG0" == strModelName || "BG1" == strModelName)
			wstrGameObjectTag = TEXT("BG");
		else
			wstrGameObjectTag = TEXT("BasicMap");

		if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tMapDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return E_FAIL;
		}
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Triggers()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;
	string strFileName = "../../../objects_txt/Stage1_Triggers.txt";
	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Stage1_Triggers.txt"));
		return E_FAIL;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iShaderVars{};
	_float fRimWidth{};

	_int iTriggerIndex{};
	_int triggerType{};
	_int iCamType{};
	_float fRadius{};
	wstring wstrGameObjectTag = TEXT("MapToolObject");

	map<_int, _float4x4> camMatrices;
	map<_int, pair<_vector, _float>> frontDirRadii;
	map<_int, pair<_vector, _float>> rearDirRadii;
	map<_int, pair<_float4x4, _float>> triggerInfos;

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		fileInput.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
		fileInput.read(reinterpret_cast<char*>(&triggerType), sizeof(triggerType));
		fileInput.read(reinterpret_cast<char*>(&iCamType), sizeof(iCamType));
		fileInput.read(reinterpret_cast<char*>(&fRadius), sizeof(fRadius));

		if ("Camera" == strModelName)
			camMatrices.emplace(iTriggerIndex, matWorld);
		else if ("Dummy" == strModelName) {
			_vector vDir = XMVector3Normalize(XMVectorSet(matWorld._31, matWorld._32, matWorld._33, 0));
			if (CAM_FRONT == iCamType)
				frontDirRadii.emplace(iTriggerIndex, pair<_vector, _float>(vDir, fRadius));
			else if (CAM_REAR == iCamType)
				rearDirRadii.emplace(iTriggerIndex, pair<_vector, _float>(vDir, fRadius));
		}
		else if ("Trigger" == strModelName)
		{
			_vector vDeterminant{};
			_float4x4 matInverse{};
			matInverse = XMMatrixInverse(&vDeterminant, matWorld);
			triggerInfos.emplace(iTriggerIndex, pair<_float4x4, _float>(matInverse, matWorld._33));

			CTrigger::TRIGGER_DESC tTriggerDesc{};
			tTriggerDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tTriggerDesc.matWorld = matWorld;
			tTriggerDesc.iTriggerIndex = iTriggerIndex;
			tTriggerDesc.iTriggerType = triggerType;

			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Trigger"), TEXT("Prototype_GameObject_Trigger"), &tTriggerDesc)))
				return E_FAIL;
		}
		else if ("NonAnim_Kirby" == strModelName)
		{
			CGameObject::GAMEOBJECT_DESC tempDesc = {};
			tempDesc.matWorld = matWorld;
			tempDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tempDesc.iShaderVars = iShaderVars;
			tempDesc.fRimWidth = fRimWidth;
			if (strModelName.size() >= 8) { // NonAnim_ 부분 지우기
				if ("NonAnim" == strModelName.substr(0, 7))
					tempDesc.wstrModelName.erase(0, 8);
			}
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby"), &tempDesc)))
				return E_FAIL;
		}
		else if ("Ladder" == strModelName)
		{
			CGameObject::GAMEOBJECT_DESC tDesc{};
			tDesc.matWorld = matWorld;
			tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tDesc.iShaderVars = iShaderVars;
			tDesc.fRimWidth = fRimWidth;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Ladder"), TEXT("Prototype_GameObject_Ladder"), &tDesc)))
				return E_FAIL;
		}
		else if ("Fog" == strModelName)
		{
			CGameObject::GAMEOBJECT_DESC tDesc{};
			tDesc.matWorld = matWorld;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_TerrainFog"), TEXT("Prototype_GameObject_TerrainFog"), &tDesc)))
				return E_FAIL;
		}
	}

	CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(eLevel, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	if (nullptr == pCamera)
		return E_FAIL;

	fileInput.close();

	if (!camMatrices.empty()) { // 카메라 행렬 세팅
		for (auto& pair : camMatrices)
			pCamera->EmplaceBackCamMatrix(pair.second);

		pCamera->Set_MatrixIndex(0);
	}

	for (auto& pair : frontDirRadii)
		pCamera->EmplaceBackDirRadius(CAM_FRONT, pair.second.first, pair.second.second);


	for (auto& pair : rearDirRadii)
		pCamera->EmplaceBackDirRadius(CAM_REAR, pair.second.first, pair.second.second);


	for (auto& pair : triggerInfos)
		pCamera->EmplaceBackTriggerInfo(pair.second.first, pair.second.second);


	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Monsters()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;
	string strFileName = "../../../objects_txt/Stage1_Monsters.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Stage1_Monsters.txt"));
		return E_FAIL;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iShaderVars{};
	_float fRimWidth{};
	_int iTriggerIndex{};
	_uint iNumRallyPoints{};
	vector<_float4> vecRallyPoints;
	wstring wstrGameObjectTag;

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		fileInput.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
		fileInput.read(reinterpret_cast<char*>(&iNumRallyPoints), sizeof(iNumRallyPoints));

		vecRallyPoints.clear();
		_float3 vRallyPointPos{};
		for (_uint iRallyPointIdx = 0; iRallyPointIdx < iNumRallyPoints; iRallyPointIdx++)
		{
			fileInput.read(reinterpret_cast<char*>(&vRallyPointPos), sizeof(vRallyPointPos));
			vecRallyPoints.push_back(_float4(vRallyPointPos.x, vRallyPointPos.y, vRallyPointPos.z, 1));
		}

		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		tempDesc.matWorld = matWorld;
		tempDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tempDesc.iShaderVars = iShaderVars;
		tempDesc.fRimWidth = fRimWidth;
		if (strModelName.size() >= 8) { // NonAnim_ 부분 지우기
			if ("NonAnim" == strModelName.substr(0, 7))
				tempDesc.wstrModelName.erase(0, 8);
		}

		if (L"Awoofy" == tempDesc.wstrModelName)
		{
			CMonster::MONSTER_DESC MonsterDesc = {};
			MonsterDesc.matWorld = matWorld;
			MonsterDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			MonsterDesc.iShaderVars = iShaderVars;
			MonsterDesc.fRimWidth = fRimWidth;
			MonsterDesc.eMonState = CMonster::MONSTER_STATE(iTriggerIndex);
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Awoofy"), &MonsterDesc)))
				return E_FAIL;
		}
		else if (L"Rabbit" == tempDesc.wstrModelName)
		{
			CRabbit::RABBIT_DESC RabbitDesc = {};
			RabbitDesc.matWorld = matWorld;
			RabbitDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			RabbitDesc.iShaderVars = iShaderVars;
			RabbitDesc.fRimWidth = fRimWidth;
			RabbitDesc.eRabbitState = CRabbit::RABBIT_STATE(iTriggerIndex);
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Rabbit"), &RabbitDesc)))
				return E_FAIL;
		}
		else if (L"Buffahorn" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Buffahorn"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"BladeKnight" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BladeKnight"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"PoppyBrosJr" == tempDesc.wstrModelName)
		{
			CPoppyBrosJr::POPPY_DESC PoppyDesc = {};
			PoppyDesc.matWorld = matWorld;
			PoppyDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			PoppyDesc.iShaderVars = iShaderVars;
			PoppyDesc.fRimWidth = fRimWidth;
			PoppyDesc.ePoppyState = CPoppyBrosJr::POPPY_STATE(iTriggerIndex);
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_PoppyBrosJr"), &PoppyDesc)))
				return E_FAIL;
		}
		else if (L"CappyBody" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_CappyBody"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"Kabu" == tempDesc.wstrModelName)
		{
			CKabu::KABU_DESC KabuDesc = {};
			KabuDesc.matWorld = matWorld;
			KabuDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			KabuDesc.iShaderVars = iShaderVars;
			KabuDesc.fRimWidth = fRimWidth;
			KabuDesc.eMonState = CKabu::MONSTER_STATE(iTriggerIndex);
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
			BrontoBurtDesc.eMonState = CBrontoBurt::MONSTER_STATE(iTriggerIndex);
			BrontoBurtDesc.vecRallyPoints = vecRallyPoints;
			if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BrontoBurt"), &BrontoBurtDesc)))
				return E_FAIL;
		}
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Items()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;
	string strFileName = "../../../objects_txt/Stage1_Items.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Stage1_Items.txt"));
		return E_FAIL;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iShaderVars{};
	_float fRimWidth{};

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		CGameObject::GAMEOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;

		if ("Item_Coin" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Item"), TEXT("Prototype_GameObject_Coin"), &tDesc)))
				return E_FAIL;
		}
	}
	fileInput.close();

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Kickables()
{
	LEVEL eLevel = LEVEL_GAMEPLAY;
	string strFileName = "../../../objects_txt/Stage1_Kickables.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Stage1_Kickables.txt"));
		return E_FAIL;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iShaderVars{};
	_float fRimWidth{};

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		CGameObject::GAMEOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;

		if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Item"), TEXT("Prototype_GameObject_KickableRock"), &tDesc)))
			return E_FAIL;
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_GamePlay::Load_FileData(const string& _strFilePath, FILE_TYPE _eFileType, const wstring& _wstrLayerTag)
{
	std::ifstream InputFile(_strFilePath, ios::in | std::ios::binary);

	if (!InputFile.is_open()) //==FALSE 
	{
		MSG_BOX(TEXT("Failed to Open : FileData"));
		ALARM_FAIL(TEXT("Failed to Open : FileData"));
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
	m_pGameInstance->Clear_EventCallBack();

	__super::Free();

	for(auto& tex : m_pEnvTexture)
		Safe_Release(tex);

}
