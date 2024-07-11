#include "stdafx.h"
#include "Level_Park.h"
#include "Camera_Free.h"
#include "Camera_Main.h"
#include "BasicMap.h"

#include "Trigger.h"
#include "Kirby.h"
#include "Awoofy.h"
#include "Rabbit.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "PoppyBrosJr.h"
#include "SurprisedBoard.h"
#include "Crumble.h"

#include "BG.h"
#include "HUD.h"
#include "SkySphere.h"
//#include "Kirby.h"

CLevel_Park::CLevel_Park(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Park::Initialize()
{
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_TOOL);

	HRESULT hr;
	hr = __super::Initialize();
	CHECK_FAILED(hr);

	// 환경맵을 추가한다.
	if (FAILED(Add_EnvMap()))
		return E_FAIL;

	hr = Ready_Lights();
	CHECK_FAILED(hr);

	hr = Ready_Layer_Camera(TEXT("Layer_Camera"));
	CHECK_FAILED(hr);

	hr = Ready_Layer_BackGround(TEXT("Layer_BackGround"));
	CHECK_FAILED(hr);

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
	hr = Ready_Objects();
	CHECK_FAILED(hr);
	hr = Ready_UI();
	CHECK_FAILED(hr);


	CGameObject::GAMEOBJECT_DESC ObjDesc{};
	ObjDesc.fSpeedPerSec = 5.f;
	ObjDesc.fRotationPerSec = ToRadian(90.f);
	_float4x4 InitMat = _float4x4::Identity;
	InitMat.Translation({ 0.f, 0.f, 0.f });
	ObjDesc.matWorld = InitMat;

	// Car Test
	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Deform"), TEXT("Prototype_GameObject_DumpCar"), &ObjDesc)))
		return E_FAIL;
	
	m_pGameInstance->Bind_RendererFunc(TRIGGER_SHADER);
	m_pGameInstance->Set_ColorSet_ByIndex(5);


	// 해당 위치의 행렬을 넘긴다.
	//surprisedDesc.matWorld = transformationMatrix;
	//surprisedDesc.eColor = CSurprisedBoard::RED;
	//surprisedDesc.eStartState = CSurprisedBoard::WAIT_L;
	//surprisedDesc.vPosition = _float3(32.f, 5.1f, -92.f);//21.39f, 5.08f, -87.56f);
	//hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
	//CHECK_FAILED(hr);



	return S_OK;
}

void CLevel_Park::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;
}

HRESULT CLevel_Park::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	//윈도우 바 FPS 체크
	++m_iFPS;

	_tchar szFPS[MAX_PATH] = TEXT("");
	wsprintf(szFPS, TEXT("Level Park, %d FPS"), m_iFPS);

	if (m_fAccDelta >= 1.f)
	{
		SetWindowText(g_hWnd, szFPS);
		m_fAccDelta = 0.f;
		m_iFPS = 0;
	}


	return S_OK;
}

HRESULT CLevel_Park::Ready_Lights()
{
	//// 예시코드 1 : 태양광
	LIGHT_DESC			LightDesc{};
	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.f, -1.f, -.3f, 0.f);

	LightDesc.vDiffuse = _float4(0.6f, 0.5f, 0.3f, 1.f);
	LightDesc.vAmbient = _float4(0.3f, 0.3f, 0.3f, 1.f);
	LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	CGameInstance::Get_Instance()->Setting_GodRay({-650.f, 5000.f, 1200.f, 1.f});

	return S_OK;
}

HRESULT CLevel_Park::Ready_Layer_Camera(const wstring& strLayerTag)
{

	CCamera_Main::CAMERA_KIRBY_DESC		MainCamDesc{};
	MainCamDesc.fFovy = XMConvertToRadians(38.0f);
	MainCamDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	MainCamDesc.fNear = 0.1f;
	MainCamDesc.fFar = 1000.0f;
	MainCamDesc.vEye = _float4(0.f, 0.f, 0.f, 1.f);
	MainCamDesc.vAt = _float4(0.f, -.2f, 1.f, 1.f);
	MainCamDesc.fSpeedPerSec = 10.f;
	MainCamDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	MainCamDesc.fOrigDistance = 28.f;
	MainCamDesc.fCamSensor = .3f;

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Main"), &MainCamDesc)))
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

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Park::Ready_Layer_BackGround(const wstring& strLayerTag)
{
	//CSkySphere::SKYSPHERE_DESC LabSkyDesc{};
	//LabSkyDesc.strModelTag = { "SkySphere_Stage1_Day" };
	//LabSkyDesc.strTextureTag = { "SkySphere_Lab_Diffuse" };
	//HRESULT hr = m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_SkySphere"), &LabSkyDesc);
	//CHECK_FAILED(hr);

	////SUB_SKYSPHERE
	//CSkySphere::SKYSPHERE_DESC LabSkySubDesc{};
	//_float4x4 InitMat = _float4x4::Identity;
	//InitMat.Translation({ 0.f, -50.f, -0.f });
	//LabSkySubDesc.matWorld = InitMat;

	//hr = m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_SkySphereSub"), &LabSkySubDesc);
	//CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLevel_Park::Ready_Map()
{
	string strFileName = "../../../objects_txt/Park_Map.txt";
	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park_Map.txt"));
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

		if (wstrGameObjectTag == TEXT("BasicMap"))
			int a = 0;
		if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tMapDesc)))
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

HRESULT CLevel_Park::Ready_Triggers()
{

	string strFileName = "../../../objects_txt/Park_Triggers.txt";
	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park_Triggers.txt"));
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

			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Trigger"), TEXT("Prototype_GameObject_Trigger"), &tTriggerDesc)))
				return E_FAIL;
			continue;
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby"), &tempDesc)))
				return E_FAIL;
		}
		else if ("Ladder" == strModelName)
		{
			CGameObject::GAMEOBJECT_DESC tDesc{};
			tDesc.matWorld = matWorld;
			tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tDesc.iShaderVars = iShaderVars;
			tDesc.fRimWidth = fRimWidth;
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Ladder"), TEXT("Prototype_GameObject_Ladder"), &tDesc)))
				return E_FAIL;
		}
		else if ("Fog" == strModelName)
		{
			CGameObject::GAMEOBJECT_DESC tDesc{};
			tDesc.matWorld = matWorld;
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_TerrainFog"), TEXT("Prototype_GameObject_TerrainFog"), &tDesc)))
				return E_FAIL;
		}
	}

	CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(m_iLevel, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
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

HRESULT CLevel_Park::Ready_Monsters()
{

	string strFileName = "../../../objects_txt/Park_Monsters.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park_Monsters.txt"));
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Awoofy"), &MonsterDesc)))
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Rabbit"), &RabbitDesc)))
				return E_FAIL;
		}
		else if (L"Buffahorn" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Buffahorn"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"BladeKnight" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BladeKnight"), &tempDesc)))
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_PoppyBrosJr"), &PoppyDesc)))
				return E_FAIL;
		}
		else if (L"CappyBody" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_CappyBody"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"Phanta" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Phanta"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"SpookStep" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SpookStep"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"GhostGordo" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_GhostGordo"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"Bomber" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Bomber"), &tempDesc)))
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Kabu"), &KabuDesc)))
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BrontoBurt"), &BrontoBurtDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_Crumble")
		{
			CCrumble::CRUMBLE_DESC CrumbleDesc = {};
			CrumbleDesc.matWorld = matWorld;
			CrumbleDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			CrumbleDesc.iShaderVars = iShaderVars;
			CrumbleDesc.fRimWidth = fRimWidth;
			CrumbleDesc.uInitialState = (_uint)iTriggerIndex;
			CrumbleDesc.fSpeedPerSec = 10.f;
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick"), TEXT("Prototype_GameObject_Crumble"), &CrumbleDesc)))
				return E_FAIL; 
		}
	}
	fileInput.close();

	#pragma region SurprisedBoard 여섯마리 레전드 추가

		// ------------------ 맨 앞 SurprisedBoard ------------------
		// 위치 행렬을 만든다. 
		_float4x4 translationMatrix = XMMatrixTranslation(19.6f, 5.f, -143.f);

		// 회전 행렬을 만든다. X축과 Z축으로 -180도 (PI 라디안).
		_float4x4 rotationMatrixX = XMMatrixRotationX(XM_PI); // XM_PI == -180.f
		_float4x4 rotationMatrixZ = XMMatrixRotationZ(XM_PI);

		// 회전 행렬들을 결합한다.
		_float4x4 rotationMatrix = rotationMatrixX * rotationMatrixZ;
		_float4x4 transformationMatrix = rotationMatrix * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		CSurprisedBoard::SURPRISED_DESC surprisedDesc = {};
		surprisedDesc.matWorld = transformationMatrix;
	
		surprisedDesc.eColor = CSurprisedBoard::RED;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_L;
		surprisedDesc.vPosition = _float3(15.4f, 5.f, -155.f);
		HRESULT hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);


		// ------------------ 두 번째 SurprisedBoard ------------------
		// 위치 수정
		translationMatrix = XMMatrixTranslation(44.54f, 4.97f, -113.189f);
		// 회전값은 그대로 사용
		transformationMatrix = rotationMatrix * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		surprisedDesc.matWorld = transformationMatrix;
		surprisedDesc.eColor = CSurprisedBoard::GREEN;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_R;
		surprisedDesc.vPosition = _float3(48.f, 5.1f, -127.f);
		hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);


		// ------------------ 세 번째 SurprisedBoard ------------------
		// 위치 수정
		translationMatrix = XMMatrixTranslation(21.39f, 5.08f, -89.f);
		// 회전 행렬을 만든다.
		_float4x4 rotationMatrixY = XMMatrixRotationY(XM_PI / 2);
		// 회전값은 그대로 사용
		transformationMatrix = rotationMatrixY * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		surprisedDesc.matWorld = transformationMatrix;
		surprisedDesc.eColor = CSurprisedBoard::RED;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_L;
		surprisedDesc.vPosition = _float3(32.f, 5.1f, -92.f);//21.39f, 5.08f, -87.56f);
		hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);


		// ------------------ 네 번째(1) 오른쪽 SurprisedBoard ------------------
		// 위치 수정
		translationMatrix = XMMatrixTranslation(12.f, 4.5f, -68.5f);
		// 회전값은 그대로 사용
		transformationMatrix = rotationMatrix * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		surprisedDesc.matWorld = transformationMatrix;
		surprisedDesc.eColor = CSurprisedBoard::BLUE;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_L;
		surprisedDesc.vPosition = _float3(4.5f, 4.f, -82.5f);
		hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);


		// ------------------ 네 번째(2) 왼쪽 SurprisedBoard ------------------
		// 위치 수정
		translationMatrix = XMMatrixTranslation(-1.62f, 4.5f, -69.5f);
		// 회전값은 그대로 사용
		transformationMatrix = rotationMatrix * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		surprisedDesc.matWorld = transformationMatrix;
		surprisedDesc.eColor = CSurprisedBoard::BLUE;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_R;
		surprisedDesc.vPosition = _float3(4.5f, 4.f, -82.5f);
		hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);

	#pragma endregion

	return S_OK;
}

HRESULT CLevel_Park::Ready_Items()
{

	string strFileName = "../../../objects_txt/Park_Items.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park_Items.txt"));
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_NoVacuumItem"), TEXT("Prototype_GameObject_Coin"), &tDesc)))
				return E_FAIL;
		}
		else if ("Item_EnergyDrink" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_NoVacuumItem"), TEXT("Prototype_GameObject_EnergyDrink"), &tDesc)))
				return E_FAIL;
		}
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_Park::Ready_Kickables()
{
	string strFileName = "../../../objects_txt/Park_Kickables.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park_Kickables.txt"));
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

		if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Item"), TEXT("Prototype_GameObject_KickableRock"), &tDesc)))
			return E_FAIL;
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_Park::Ready_Objects()
{
	//Map, Triggers, Kickables.. 분류 제외 잔존 오브젝트들

	string strFileName = "../../../objects_txt/Park.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park.txt"));
		return E_FAIL;
	}

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iShaderVars{};
	_float fRimWidth{};

	while (!fileInput.eof())
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		if (fileInput.eof())
			break;

		CGameObject::GAMEOBJECT_DESC tDesc{};
		tDesc.matWorld = matWorld;
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;

#pragma region GIMMICK_OBJECT

		if ("FhEntranceAlien_NonAnim" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick"), TEXT("Prototype_GameObject_Gm_ParkFhEntranceAlien"), &tDesc)))
				continue;
		}

		if ("SolarPanelCharge_NonAnim" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick"), TEXT("Prototype_GameObject_Gm_ParkSolarPanelCharge"), &tDesc)))
				continue;
		}

		if ("SolarPanelOnce_NonAnim" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick"), TEXT("Prototype_GameObject_Gm_ParkSolarPanelOnce"), &tDesc)))
				continue;
		}

#pragma endregion
	}
	fileInput.close();

	return S_OK;
}

HRESULT CLevel_Park::Ready_UI()
{
	CUIObject::UIOBJ_DESC DiscardUIDesc{};
	DiscardUIDesc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f };
	DiscardUIDesc.vPos = { DiscardUIDesc.vCenter.x, DiscardUIDesc.vCenter.y, 0.f };
	DiscardUIDesc.vSize = { 260.f * 0.8f, 120.f * 0.8f, 1.f };

	HRESULT hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_UI_HUD"), TEXT("Prototype_GameObject_HUD_AbilityDiscard"), &DiscardUIDesc);

	return S_OK;
}

HRESULT CLevel_Park::Add_EnvMap()
{
	return S_OK;
}

CLevel_Park* CLevel_Park::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Park* pInstance = new CLevel_Park(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_Park"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Park::Free()
{
	m_pGameInstance->Clear_EventCallBack();
	__super::Free();

	for (auto& tex : m_pEnvTexture)
		Safe_Release(tex);
}


