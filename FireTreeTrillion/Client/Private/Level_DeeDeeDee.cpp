#include "stdafx.h"
#include "Level_DeeDeeDee.h"

#include "Camera_Main.h"
#include "Camera_Free.h"
#include "BasicMap.h"
#include "Trigger.h"
#include "Kirby.h"

#include "Awoofy.h"
#include "Rabbit.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "PoppyBrosJr.h"
#include "SkySphere.h"

#include "HungryDee.h"

#include "BG.h"
#include "HUD.h"
#include "Dialog.h"
#include "UI_Fading.h"


#define SOUND_TRIGGER_Z		-100.f

CLevel_DeeDeeDee::CLevel_DeeDeeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_DeeDeeDee::Initialize()
{
	m_pGameInstance->Set_RenderMode(CRenderer::MODE_GAMEPLAY);

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

	hr = Ready_Dees();
	CHECK_FAILED(hr);

	hr = Ready_Objects();
	CHECK_FAILED(hr);

	hr = Ready_UI();
	CHECK_FAILED(hr);

	m_pGameInstance->Bind_RendererFunc(TRIGGER_SHADER);
	m_pGameInstance->Set_ColorSet(CRenderer::COLORSET_TOWN);
	m_pGameInstance->Set_ObjectBlack(1.f);


	// DeeTest
	CGameObject::GAMEOBJECT_DESC ObjDesc{};
	ObjDesc.fSpeedPerSec = 5.f;
	ObjDesc.fRotationPerSec = ToRadian(90.f);
	_float4x4 InitMat = _float4x4::Identity;
	InitMat.Translation({ 1.51f, 22.11f, 3.91f });
	ObjDesc.matWorld = InitMat;
	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_DeeDeeDee"), TEXT("Prototype_GameObject_DeeDeeDee"), &ObjDesc)))
		return E_FAIL;

	// 안개 초기화
	m_pGameInstance->Fog_Intialize_ForDeeDeeDee(0);

	return S_OK;
}

void CLevel_DeeDeeDee::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;

	//static _float fTimeAcc = 0.f;
	//fTimeAcc += fTimeDelta;
	//if (fTimeAcc > 2.f) // 2초뒤 페이드인
	Ready_FadeIn();

	Fog_Tick(fTimeDelta);
}

HRESULT CLevel_DeeDeeDee::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	//윈도우 바 FPS 체크
	++m_iFPS;

	_tchar szFPS[MAX_PATH] = TEXT("");
	wsprintf(szFPS, TEXT("Level DeeDeeDee, %d FPS"), m_iFPS);

	if (m_fAccDelta >= 1.f)
	{
		SetWindowText(g_hWnd, szFPS);
		m_fAccDelta = 0.f;
		m_iFPS = 0;
	}

	return S_OK;
}

// 디디디를 만나기전까지의 포그를, 디디디를 만나고 감소시키는 틱입니다.
void CLevel_DeeDeeDee::Fog_Tick(_float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(m_iLevel, TEXT("Layer_Player")));
	_float4 vPos = pKirby->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);
	if (vPos.z >= -20.f)
		m_pGameInstance->Decrease_FogViewValue(fTimeDelta);
}

void CLevel_DeeDeeDee::Ready_FadeIn()
{
	static _bool bOnceFade = false;
	static _bool bOnceChanger = false;

	CGameObject* pUIObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_Fading"));
	CHECK_NULLPTR(pUIObj);
	CUI_Fading* pFadingUI = dynamic_cast<CUI_Fading*>(pUIObj);

	// FadingUI가 이전에 FadeOut 안되어있다면 NO FadeIn
	//if (pFadingUI->Get_State() != CUI_Fading::FADEOUT) return;

	if (bOnceChanger == false)
	{
		if (bOnceFade == false)
		{
			pFadingUI->Set_InOutState(CUI_Fading::FADEIN);
			pFadingUI->Set_IsRender(true);
			bOnceFade = true;
		}
		else if (pFadingUI->Get_FadeRatio() >= 1.f)
		{

			pFadingUI->Set_IsRender(false);
			bOnceChanger = true;
		}
	}
}

HRESULT CLevel_DeeDeeDee::Ready_Lights()
{
	//태양광
	LIGHT_DESC			LightDesc{};
	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(0.f, -1.f, 0.f, 0.f);

	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.8f, 1.f);
	LightDesc.vAmbient = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	CGameInstance::Get_Instance()->Setting_GodRay({ -650.f, 300.f, 1200.f, 1.f });

	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Ready_Layer_Camera(const wstring& strLayerTag)
{
	

	CCamera_Main::CAMERA_KIRBY_DESC		MainCamDesc{};
	MainCamDesc.fFovy = XMConvertToRadians(30.0f);
	MainCamDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	MainCamDesc.fNear = 0.1f;
	MainCamDesc.fFar = g_fCamFar;
	MainCamDesc.vEye = _float4(0.f, 0.f, 0.f, 1.f);
	MainCamDesc.vAt = _float4(0.f, -.15f, 1.f, 1.f);
	MainCamDesc.fSpeedPerSec = 10.f;
	MainCamDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	MainCamDesc.fOrigDistance = 40.f;
	MainCamDesc.fCamSensor = .3f;

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Main"), &MainCamDesc)))
		return E_FAIL;


	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};
	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(30.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = g_fCamFar;
	CameraDesc.vEye = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.vAt = _float4(0.f, -1.f, 1.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Ready_Layer_BackGround(const wstring& strLayerTag)
{
	CSkySphere::SKYSPHERE_DESC DDDSkyDesc{};
	DDDSkyDesc.strModelTag = { "SkySphere_Stage1_Day" };

	HRESULT hr = m_pGameInstance->Add_Clone(m_iLevel, strLayerTag, TEXT("Prototype_GameObject_SkySphere"), &DDDSkyDesc);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Ready_UI()
{
	HRESULT hr = S_OK;

#pragma region PARSING HUD_KIRBYHP, STARPOINT

	vector<string> vecUITag = { "HUD_KirbyStatus", "HUD_StarPoint" };

	string strFileBase = { "../../../UI_txt/" };
	string strFileExt = { "_Orig.txt" };

	for (const auto& strUITag : vecUITag)
	{
		string strFilePath = strFileBase + strUITag.c_str() + strFileExt;
		std::ifstream InputFile(strFilePath, ios::in | std::ios::binary);

		if (!InputFile.is_open()) //==FALSE 
		{
			MSG_BOX(TEXT("Failed to Open : FileData"));
			ALARM_FAIL(TEXT("Failed to Open : FileData"));
			return E_FAIL;
		}

		size_t size = 0;
		InputFile.read(reinterpret_cast<char*>(&size), sizeof(size));

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
			size_t strFileFrontPos = strFilePath.find("txt/");
			if (strFileFrontPos != string::npos)
				strUITag = strFilePath.substr(strFileFrontPos + 4);

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

			wstring wstrLayerTag = {};
			if ("HUD_KirbyStatus" == strUITag)
				wstrLayerTag = TEXT("Layer_UI_HUD_KirbyHP");

			if ("HUD_StarPoint" == strUITag)
				wstrLayerTag = TEXT("Layer_UI_HUD_StarPoint");

			hr = m_pGameInstance->Add_Clone(m_iLevel, wstrLayerTag, CUtils::StrToWstr(strProtoTag), &LayerUIDesc);
			CHECK_FAILED(hr);
		}

		continue;
	}

#pragma endregion
	
	//커비 네임태그
	hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_UI_HUD"), TEXT("Prototype_GameObject_HUD_KirbyNameTag"));

	//능력버리기
	CUIObject::UIOBJ_DESC DiscardUIDesc{};
	DiscardUIDesc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f };
	DiscardUIDesc.vPos = { DiscardUIDesc.vCenter.x, DiscardUIDesc.vCenter.y, 0.f };
	DiscardUIDesc.vSize = { 260.f * 0.8f, 120.f * 0.8f, 1.f };
	hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_UI_HUD"), TEXT("Prototype_GameObject_HUD_AbilityDiscard"), &DiscardUIDesc);

	//다이얼로그 :: 망치 루팅 이후 출력
	CDialog::DIALOG_DESC tDialogDesc{};
	tDialogDesc.strPath = "../Bin/Resources/Data/Dialog_DeeDeeDee.json";
	hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_UI_Dialog"), TEXT("Prototype_GameObject_Dialog"), &tDialogDesc);

	return S_OK; 
}

HRESULT CLevel_DeeDeeDee::Ready_Map()
{


	string strFileName = "../../../objects_txt/DeeDeeDee_Map.txt";
	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee_Map.txt"));
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

HRESULT CLevel_DeeDeeDee::Ready_Triggers()
{

	string strFileName = "../../../objects_txt/DeeDeeDee_Triggers.txt";
	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee_Triggers.txt"));
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

HRESULT CLevel_DeeDeeDee::Ready_Dees()
{



	//CWaddleDee::DEE_DESC ObjDesc{};
	//ObjDesc.fSpeedPerSec = 5.f;
	//ObjDesc.fRotationPerSec = ToRadian(90.f);
	//_float4x4 InitMat = _float4x4::Identity;
	//InitMat.Translation({ 10.2f, 24.7f, 26.f });
	//ObjDesc.matWorld = InitMat;

	//if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Dee"), TEXT("Prototype_GameObject_BattleDee"), &ObjDesc)))
	//	return E_FAIL;



	string strFileName = "../../../objects_txt/DeeDeeDee_Monsters.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee_Monsters.txt"));
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


		if (L"WaddleDee" == tempDesc.wstrModelName)
		{
			CWaddleDee::DEE_DESC DeeDesc = {};
			DeeDesc.matWorld = matWorld;
			DeeDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			DeeDesc.iShaderVars = iShaderVars;
			DeeDesc.fRimWidth = fRimWidth;

			//원래 인덱스를 넘어가면 맨 마지막 놈으로다가 매치
			DeeDesc.eCharacter =
				(DEECHARACTER_END - 1) < (DEE_CHARACTER)iTriggerIndex ?
				DEECHARACTER_SLEEPY : (DEE_CHARACTER)iTriggerIndex;

			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_BattleDee"), TEXT("Prototype_GameObject_BattleDee"), &DeeDesc)))
				return E_FAIL;
		}
	}



	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Ready_Monsters()
{



	//일단 마을에는 몬스터가 없어요
	//하지만 넣어 봤어요
	string strFileName = "../../../objects_txt/DeeDeeDee_Monsters.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee_Monsters.txt"));
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
			CMonster::MONSTER_DESC RabbitDesc = {};
			RabbitDesc.matWorld = matWorld;
			RabbitDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			RabbitDesc.iShaderVars = iShaderVars;
			RabbitDesc.fRimWidth = fRimWidth;
			RabbitDesc.eMonState = CMonster::MONSTER_STATE(iTriggerIndex);
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
			CMonster::MONSTER_DESC PoppyDesc = {};
			PoppyDesc.matWorld = matWorld;
			PoppyDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			PoppyDesc.iShaderVars = iShaderVars;
			PoppyDesc.fRimWidth = fRimWidth;
			PoppyDesc.eMonState = CMonster::MONSTER_STATE(iTriggerIndex);
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_PoppyBrosJr"), &PoppyDesc)))
				return E_FAIL;
		}
		else if (L"CappyBody" == tempDesc.wstrModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_CappyBody"), &tempDesc)))
				return E_FAIL;
		}
		else if (L"Kabu" == tempDesc.wstrModelName)
		{
			CMonster::MONSTER_DESC KabuDesc = {};
			KabuDesc.matWorld = matWorld;
			KabuDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			KabuDesc.iShaderVars = iShaderVars;
			KabuDesc.fRimWidth = fRimWidth;
			KabuDesc.eMonState = CMonster::MONSTER_STATE(iTriggerIndex);
			KabuDesc.vecRallyPoints = vecRallyPoints;
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Kabu"), &KabuDesc)))
				return E_FAIL;
		}
		else if (strModelName == "NonAnim_BrontoBurt")
		{
			CMonster::MONSTER_DESC BrontoBurtDesc = {};
			BrontoBurtDesc.matWorld = matWorld;
			BrontoBurtDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			BrontoBurtDesc.iShaderVars = iShaderVars;
			BrontoBurtDesc.fRimWidth = fRimWidth;
			BrontoBurtDesc.eMonState = CMonster::MONSTER_STATE(iTriggerIndex);
			BrontoBurtDesc.vecRallyPoints = vecRallyPoints;
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_BrontoBurt"), &BrontoBurtDesc)))
				return E_FAIL;
		}
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Ready_Items()
{

	string strFileName = "../../../objects_txt/DeeDeeDee_Items.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee_Items.txt"));
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
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_NoVacuumItem"), TEXT("Prototype_GameObject_Food"), &tDesc)))
				return E_FAIL;
		}
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Ready_Kickables()
{

	string strFileName = "../../../objects_txt/DeeDeeDee_Kickables.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee_Kickables.txt"));
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

HRESULT CLevel_DeeDeeDee::Ready_Objects()
{

	string strFileName = "../../../objects_txt/DeeDeeDee.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : DeeDeeDee.txt"));
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

		if ("NonRenderWall" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_NonRenderWall"), TEXT("Prototype_GameObject_NonRenderWall"), &tDesc)))
				continue;
		}
	}
	fileInput.close();

	return S_OK;
}

HRESULT CLevel_DeeDeeDee::Add_EnvMap()
{
	HRESULT hr;

	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Level_Town_Env"),
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

CLevel_DeeDeeDee* CLevel_DeeDeeDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{

	CLevel_DeeDeeDee* pInstance = new CLevel_DeeDeeDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_DeeDeeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_DeeDeeDee::Free()
{
	m_pGameInstance->Clear_EventCallBack();
	__super::Free();

	for (auto& tex : m_pEnvTexture)
		Safe_Release(tex);

	m_pGameInstance->StopSound(CHANNEL_BGM);
}
