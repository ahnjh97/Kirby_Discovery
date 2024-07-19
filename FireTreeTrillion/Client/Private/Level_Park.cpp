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
#include "SpawnEffect.h"

#include "BG.h"
#include "HUD.h"
#include "SkySphere.h"
#include "TransingStar.h"
#include "Gm_DynamicField.h"
#include "Gm_ParkSolarPanelOnce.h"
#include "Gm_ParkSolarPanelCharge.h"

#define MONSTER_TRIGGER(index) (index - 11)

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
	m_pGameInstance->Set_ColorSet(CRenderer::COLORSET_PARK);
	m_pGameInstance->Set_ObjectBlack(1.f);

	// PARK 도착했으면 오픈해주세요
	CGameObject* pGameObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_TransingStar"));
	CTransingStar* pTransingStar = static_cast<CTransingStar*>(pGameObj);
	pTransingStar->Set_LargeColor(_float3(95.f / 255.f, 28.f / 255.f, 128.f / 255.f));
	pTransingStar->Set_SmallColor(_float3(167.f / 255.f, 42.f / 255.f, 168.f / 255.f));
	pTransingStar->Activate(CTransingStar::OPEN);

	// 플레이어 이동 트리거
	function<void(_int)> func = bind(&CLevel_Park::Teleport_Player, this);
	m_pGameInstance->Emplace_TriggerFunc(TRIGGER_STAR, func);

	// 레벨전환 트리거
	function<void(_int)> funcChanger = bind(&CLevel_Park::Change_Levels, this);
	m_pGameInstance->Emplace_TriggerFunc(TRIGGER_LEVELCHANGER, funcChanger);

	// 몬스터 트리거
	function<void(_int)> funcMonster = bind(&CLevel_Park::SummonEffectForMonster, this, placeholders::_1);
	m_pGameInstance->Emplace_TriggerFunc(TRIGGER_MONSTER, funcMonster);

	return S_OK;
}

void CLevel_Park::Teleport_Player()
{
	CGameObject* pGameObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_TransingStar"));
	CTransingStar* pTransingStar = static_cast<CTransingStar*>(pGameObj);
	pTransingStar->Set_NextLevel(LEVEL_END);
	pTransingStar->Activate(CTransingStar::CLOSE);
	pTransingStar->Set_LargeColor(_float3(95.f / 255.f, 28.f / 255.f, 128.f / 255.f));
	pTransingStar->Set_SmallColor(_float3(167.f / 255.f, 42.f / 255.f, 168.f / 255.f));
}

void CLevel_Park::Change_Levels()
{
	CGameObject* pGameObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_TransingStar"));
	CTransingStar* pTransingStar = static_cast<CTransingStar*>(pGameObj);
	pTransingStar->Set_NextLevel(LEVEL_SIMBA);
	pTransingStar->Set_LargeColor(_float3(85.f / 255.f, 93.f / 255.f, 183.f / 255.f));
	pTransingStar->Set_SmallColor(_float3(48.f / 255.f, 57.f / 255.f, 147.f / 255.f));
	pTransingStar->Activate(CTransingStar::CLOSE);
}

void CLevel_Park::SummonMonsters(_uint iTriggerIndex)
{
	wstring wstrPrototypeTag = TEXT("Prototype_GameObject_");
	wstring wstrTag;

	for (auto& monsterDesc : m_vecMonsterDescs[MONSTER_TRIGGER(iTriggerIndex)])
	{
		wstring wstrModelName = monsterDesc.wstrModelName;
		size_t underscorePos = wstrModelName.find(L'_');
		if (underscorePos != wstring::npos)
			wstrModelName = wstrModelName.substr(underscorePos + 1);
	
		wstrTag = wstrPrototypeTag + wstrModelName;

		if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), wstrTag, &monsterDesc)))
			return;
	}
}

void CLevel_Park::SummonEffectForMonster(_uint iTriggerIndex)
{
	if (m_setActivatedMonsterTriggers.end() != m_setActivatedMonsterTriggers.find(iTriggerIndex))
		return;

	m_setActivatedMonsterTriggers.insert(iTriggerIndex); // 여러번 호출되는거 방지

	m_bTrigger = true;
	m_iTriggerIndex = iTriggerIndex;

	for (auto& monsterDesc : m_vecMonsterDescs[MONSTER_TRIGGER(iTriggerIndex)])
	{
		HRESULT hr;
		CSpawnEffect::SPAWNEFFECT_DESC tDesc{};
		_float4 vPos = _float4(monsterDesc.matWorld._41, monsterDesc.matWorld._42, monsterDesc.matWorld._43, monsterDesc.matWorld._44);
		_float4 vRight = _float4(monsterDesc.matWorld._11, monsterDesc.matWorld._12, monsterDesc.matWorld._13, monsterDesc.matWorld._14);
		_float4 vUp = _float4(monsterDesc.matWorld._21, monsterDesc.matWorld._22, monsterDesc.matWorld._23, monsterDesc.matWorld._24);
		_float4 vLook = _float4(monsterDesc.matWorld._31, monsterDesc.matWorld._32, monsterDesc.matWorld._33, monsterDesc.matWorld._34);
		tDesc.vPosition = vPos + vLook * 0.1f;
		tDesc.fScale = 1.f;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SpawnEffect"), &tDesc);
		CHECK_FAILED(hr);
		tDesc.vPosition = vPos + vUp * 0.5f;
		tDesc.fScale = 1.3f;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SpawnEffect"), &tDesc);
		CHECK_FAILED(hr);
		tDesc.vPosition = vPos + vRight * 0.5f;
		tDesc.fScale = 1.3f;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SpawnEffect"), &tDesc);
		CHECK_FAILED(hr);
		tDesc.vPosition = vPos - vRight * 0.5f;
		tDesc.fScale = 1.3f;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SpawnEffect"), &tDesc);
		CHECK_FAILED(hr);
		tDesc.vPosition = vPos - vUp * 0.5f;
		tDesc.fScale = 1.3f;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SpawnEffect"), &tDesc);
		CHECK_FAILED(hr);
	}
}

void CLevel_Park::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_fAccDelta += fTimeDelta;

	if(true == m_bTrigger)
		m_fSummonTime += fTimeDelta;

	if (2.f < m_fSummonTime)
	{
		m_bTrigger = false;
		m_fSummonTime = 0.f;
		SummonMonsters(m_iTriggerIndex);
	}
}

HRESULT CLevel_Park::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;


	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_PARK, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby")));
	CHECK_NULLPTR(pKirby);
	_float4 vPosKirby = pKirby->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);

	_float fPosKirbyX = round(vPosKirby.x * 1000) / 1000;
	_float fPosKirbyY = round(vPosKirby.y * 1000) / 1000;
	_float fPosKirbyZ = round(vPosKirby.z * 1000) / 1000;

	CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_PARK, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	CHECK_NULLPTR(pCamera);
	_float4 vPosCamera = pCamera->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION);

	_float fPosCameraX = round(vPosCamera.x * 1000) / 1000;
	_float fPosCameraY = round(vPosCamera.y * 1000) / 1000;
	_float fPosCameraZ = round(vPosCamera.z * 1000) / 1000;

	//윈도우 바 FPS 체크
	++m_iFPS;
	wstring wstrMsg = TEXT("Level Park : ") + to_wstring(m_iFPS) 
					+ TEXT(". | Kirby is at ")  + to_wstring(fPosKirbyX)  + TEXT(", ") + to_wstring(fPosKirbyY)  + TEXT(", ") + to_wstring(fPosKirbyZ)
					+ TEXT(". | Camera is at ")	+ to_wstring(fPosCameraX) + TEXT(", ") + to_wstring(fPosCameraY) + TEXT(", ") + to_wstring(fPosCameraZ);

	if (m_fAccDelta >= 1.f)
	{
		SetWindowText(g_hWnd, wstrMsg.c_str());
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

	LightDesc.vDiffuse = _float4(0.f, 0.f, 0.f, 1.f);
	//LightDesc.vAmbient = _float4(0.3f, 0.3f, 0.3f, 1.f);
	//LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 1.f);

	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	CGameInstance::Get_Instance()->Setting_GodRay({-650.f, 5000.f, 1200.f, 1.f});

	string strFileName = "../../../objects_txt/Park_Lights.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		MSG_BOX(TEXT("Failed to open : Park_Lights.txt"));
		return E_FAIL;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iStrLength{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iShaderVars{};
	_float fRimWidth{};

	LIGHT_DESC			tPointLightDesc{};
	tPointLightDesc.eType = LIGHT_DESC::TYPE_POINT;

	// 지역 vector에 임시로 넣어두기
	vector<LIGHT_DESC> vecLight;
	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		tPointLightDesc.fRange = fRimWidth;
		tPointLightDesc.vDiffuse = _float4(matWorld._11, matWorld._12, matWorld._13, 1.f);
		tPointLightDesc.vAmbient = _float4(matWorld._21, matWorld._22, matWorld._23, 1.f);
		tPointLightDesc.vSpecular = _float4(matWorld._31, matWorld._32, matWorld._33, 1.f);
		tPointLightDesc.vPosition = _float4(matWorld._41, matWorld._42, matWorld._43, 1.f);

		vecLight.push_back(tPointLightDesc);
	}

	// z값으로 정렬
	sort(vecLight.begin(), vecLight.end(), [](const LIGHT_DESC& a, const LIGHT_DESC& b) {
		return a.vPosition.z <= b.vPosition.z;
		});

	// 정렬된 vector에서 light를 하나씩 꺼내어, 진짜 Light_Manager에게 보내기
	for (auto& tLight : vecLight)
	{
		HRESULT hr = CGameInstance::Get_Instance()->Add_Light(tLight);
		CHECK_FAILED(hr);
	}

	fileInput.close();

	return S_OK;
}

HRESULT CLevel_Park::Ready_Layer_Camera(const wstring& strLayerTag)
{

	CCamera_Main::CAMERA_KIRBY_DESC		MainCamDesc{};
	//MainCamDesc.fFovy = XMConvertToRadians(38.0f);
	MainCamDesc.fFovy = XMConvertToRadians(30.0f);
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

		//기믹 등으로 활성화되는 동적 필드에 대한 예외처리
		else if ("Gimmick_PkFunHouseDarkness01" == strModelName || "Gimmick_PkFunHouseDarkness02" == strModelName 
			|| "Gimmick_PkFunHouseDarkness03" == strModelName || "Gimmick_PkFunHouseDarkness04" == strModelName 
			|| "Gimmick_PkFunHouseDarkness05" == strModelName || "Gimmick_PkFunHouse06" == strModelName 
			|| "Gimmick_PkFunHouse06A" == strModelName || "Gimmick_PkFunHouse06B" == strModelName
			|| "Gimmick_PkFunHouse06C" == strModelName	|| "Gimmick_PkFunHouse07" == strModelName)
			wstrGameObjectTag = TEXT("DynamicField");
			
		else
			wstrGameObjectTag = TEXT("BasicMap");

		if (wstrGameObjectTag == TEXT("BasicMap"))
			int a = 0;

		if (TEXT("BasicMap") == wstrGameObjectTag || TEXT("BG") == wstrGameObjectTag)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Map"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tMapDesc)))
			{
				wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
				MSG_BOX(wstrErrorMsg.c_str());
				fileInput.close();
				return E_FAIL;
			}
		}
		else
		{
			CGameObject::GAMEOBJECT_DESC tDesc{};
			tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tDesc.matWorld = matWorld;
			tDesc.iShaderVars = _uint(round(vMin.x));

			//동적 필드
			if ("Gimmick_PkFunHouseDarkness01" == strModelName || "Gimmick_PkFunHouseDarkness04" == strModelName
				|| "Gimmick_PkFunHouseDarkness05" == strModelName || "Gimmick_PkFunHouse06" == strModelName
				|| "Gimmick_PkFunHouse06A" == strModelName || "Gimmick_PkFunHouse06B" == strModelName
				|| "Gimmick_PkFunHouse06C" == strModelName)
			{
				if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_DynamicField"), 
					TEXT("Prototype_GameObject_Gm_") + wstrGameObjectTag, &tDesc)))
					continue;
			}

			if ("Gimmick_PkFunHouseDarkness02" == strModelName || "Gimmick_PkFunHouseDarkness03" == strModelName)
			{
				if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_DynamicField_SurpriseBoard"),
					TEXT("Prototype_GameObject_Gm_") + wstrGameObjectTag, &tDesc)))
					continue;
			}

			if ("Gimmick_PkFunHouse07" == strModelName)
			{
				if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_StaticField"),
					TEXT("Prototype_GameObject_Gm_") + wstrGameObjectTag, &tDesc)))
					continue;
			}
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
			
			wstring wstrLayerTag = TEXT("Layer_Trigger");
			if(10 < iTriggerIndex)
				wstrLayerTag = TEXT("Layer_MonsterTrigger");

			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, wstrLayerTag, TEXT("Prototype_GameObject_Trigger"), &tTriggerDesc)))
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

		if (10 < iTriggerIndex)
		{
			CMonster::MONSTER_DESC tMonsterDesc = *static_cast<CMonster::MONSTER_DESC*>(&tempDesc);
			tMonsterDesc.eMonState = CMonster::MONSTER_STATE(iTriggerIndex);

			m_vecMonsterDescs[MONSTER_TRIGGER(iTriggerIndex)].push_back(tMonsterDesc);
			continue;
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

#pragma region SurprisedBoard 일곱마리

	#pragma region 맨 앞 빨강

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
		surprisedDesc.fSpeedPerSec = 7.f;
		surprisedDesc.matWorld = transformationMatrix;
	
		surprisedDesc.eColor = CSurprisedBoard::RED;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_L;
		surprisedDesc.vPosition = _float3(15.4f, 5.f, -155.f);
		HRESULT hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);

#pragma endregion
	
	#pragma region 두번째 연두색

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

	#pragma endregion
	
	#pragma region 세번째 주황색

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

    #pragma endregion
	
	#pragma region 네번째(1) 피랑색 오른쪽

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

    #pragma endregion
	
	#pragma region 네번째(2) 피랑색 왼쪽

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
	
	#pragma region 다섯번째 Land2에서의 앞쪽 연두색

		// 위치 수정
		translationMatrix = XMMatrixTranslation(-27.02f, 57.97f, 33.42f);
		// 회전값은 그대로 사용
		transformationMatrix = rotationMatrix * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		surprisedDesc.matWorld = transformationMatrix;
		surprisedDesc.eColor = CSurprisedBoard::GREEN;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_R; 
		surprisedDesc.vPosition = _float3(-24.27f, 59.f, 31.5f);
		hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_LegendBoard"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);

	#pragma endregion
	
	#pragma region 여섯번째 Land2에서의 중간쯤 빨강색

		// 위치 수정
		translationMatrix = XMMatrixTranslation(39.5f, 57.97f, 81.5f);
		// 회전값은 그대로 사용
		transformationMatrix = rotationMatrix * translationMatrix;

		// 해당 위치의 행렬을 넘긴다.
		surprisedDesc.matWorld = transformationMatrix;
		surprisedDesc.eColor = CSurprisedBoard::RED;
		surprisedDesc.eStartState = CSurprisedBoard::WAIT_L;
		surprisedDesc.vPosition = _float3(35.5f, 58.5f, 78.2f);
		hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_LegendBoard"), TEXT("Prototype_GameObject_SurprisedBoard"), &surprisedDesc);
		CHECK_FAILED(hr);

	#pragma endregion

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

	unordered_set<string> vecCoins = { "Item_Coin", "Item_BlueCoin", "Item_RedCoin" };
	unordered_set<string> vecFood = { "Item_Bread", "Item_Cake", "Item_Cocktail", "Item_EnergyDrink"
		, "Item_Makaron", "Item_Meat", "Item_Omelet", "Item_Onigiri", "Item_Steak", "Item_Sushi" };

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

		if (vecCoins.end() != vecCoins.find(strModelName))
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_NoVacuumItem"), TEXT("Prototype_GameObject_Coin"), &tDesc)))
				return E_FAIL;
		}
		else if (vecFood.end() != vecFood.find(strModelName))
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_NoVacuumItem"), TEXT("Prototype_GameObject_Food"), &tDesc)))
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


		//원더리아 입구
		if ("FhEntranceAlien_NonAnim" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick"), 
				TEXT("Prototype_GameObject_Gm_ParkFhEntranceAlien"), &tDesc)))
				continue;
		}

		//태양광 패널 기믹
		if ("SolarPanelCharge_NonAnim" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick_SolarPanel"), 
				TEXT("Prototype_GameObject_Gm_ParkSolarPanelCharge"), &tDesc)))
				continue;
		}

		if ("SolarPanelOnce_NonAnim" == strModelName)
		{
			if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Gimmick_SolarPanel"), 
				TEXT("Prototype_GameObject_Gm_ParkSolarPanelOnce"), &tDesc)))
				continue;
		}
	}
	fileInput.close();

#pragma region SET_GIMMICK_SOLARPANEL

	list<CGameObject*>* GimmickList = m_pGameInstance->Get_List(m_iLevel, TEXT("Layer_Gimmick_SolarPanel"));
	list<CGameObject*>* DFieldList = m_pGameInstance->Get_List(m_iLevel, TEXT("Layer_DynamicField"));

	for (auto& field : *DFieldList)
	{
		CGm_DynamicField* pField = dynamic_cast<CGm_DynamicField*>(field);
		_uint iFieldIx = pField->Get_GimmickIndex();

		for (auto& gimmick : *GimmickList)
		{
			if (TEXT("Prototype_GameObject_Gm_ParkSolarPanelOnce") == gimmick->Get_PrototypeTag())
			{
				CGm_ParkSolarPanelOnce* pGimmick = dynamic_cast<CGm_ParkSolarPanelOnce*>(gimmick);
				_uint iGimmickIx = pGimmick->Get_GimmickIndex();
				if (iFieldIx == iGimmickIx) {
					pField->Set_SolarPanelOnce(pGimmick);
					break;
				}
			}
			else if (TEXT("Prototype_GameObject_Gm_ParkSolarPanelCharge") == gimmick->Get_PrototypeTag())
			{
				CGm_ParkSolarPanelCharge* pGimmick = dynamic_cast<CGm_ParkSolarPanelCharge*>(gimmick);
				_uint iGimmickIndex = pGimmick->Get_GimmickIndex();
				if (iFieldIx == iGimmickIndex) {
					pField->Set_SolarPanelCharge(pGimmick);
					break;
				}
			}
		}
	}

	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(LEVEL_PARK, TEXT("Layer_Player")));
	if (nullptr == pKirby)
		return S_OK;

	list<CGameObject*>* leftRightList = m_pGameInstance->Get_List(m_iLevel, TEXT("Layer_DynamicField_SurpriseBoard"));
	if(nullptr == leftRightList)
		return S_OK;

	for (auto& leftRight : *leftRightList)
	{
		CGm_DynamicField* pLeftRight = dynamic_cast<CGm_DynamicField*>(leftRight);
		if (nullptr == pLeftRight)
			continue;
		_uint iGimmickIndex = pLeftRight->Get_GimmickIndex();

		CSurprisedBoard* pSurpriseBoard = { nullptr };
		if (11 == iGimmickIndex)
		{
			pSurpriseBoard = dynamic_cast<CSurprisedBoard*>(m_pGameInstance->Get_GameObject(LEVEL_PARK, TEXT("Layer_LegendBoard"), 0));
			pSurpriseBoard->RegisterSurpriseBoardAndDynamicField(pLeftRight);
		}
		else if (12 == iGimmickIndex)
		{
			pSurpriseBoard = dynamic_cast<CSurprisedBoard*>(m_pGameInstance->Get_GameObject(LEVEL_PARK, TEXT("Layer_LegendBoard"), 1));
			pSurpriseBoard->RegisterSurpriseBoardAndDynamicField(pLeftRight);
		}
	}

#pragma endregion

	return S_OK;
}

HRESULT CLevel_Park::Ready_UI()
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

	CUIObject::UIOBJ_DESC DiscardUIDesc{};
	DiscardUIDesc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f };
	DiscardUIDesc.vPos = { DiscardUIDesc.vCenter.x, DiscardUIDesc.vCenter.y, 0.f };
	DiscardUIDesc.vSize = { 260.f * 0.8f, 120.f * 0.8f, 1.f };

	hr = m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_UI_HUD"), TEXT("Prototype_GameObject_HUD_AbilityDiscard"), &DiscardUIDesc);

	return S_OK;
}

HRESULT CLevel_Park::Add_EnvMap()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Level_Park_Env"),
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

	m_setActivatedMonsterTriggers.clear();

	for (auto& tex : m_pEnvTexture)
		Safe_Release(tex);
}


