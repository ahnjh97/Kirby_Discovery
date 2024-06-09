#include "stdafx.h"
#include "Loader.h"
#include <process.h>
#include <codecvt>
#include <locale>
#include "GameInstance.h"
#include "tinyxml2.h"

//맵툴
#include "OrbitingCamera.h"
#include "MapToolHelper.h"
#include "MapToolObject.h"
#include "BasicMap.h"
#include "Trigger.h"
#include "Grid.h"
#include "BG.h"


//스카이 스피어
#include "SkySphere.h"


#pragma region TOOL_UI

#include "Editor_UI.h"
#include "BackGround.h"

#ifdef _DEBUG
#include "Editor_UI.h"
#endif

//#include "TestUI.h"
#include "LayerUI.h"
#pragma endregion

//이펙트 툴
#ifdef _DEBUG
#include "FXToolDirector.h"
#endif
#include "SingleEffect.h"
#include "Particle.h"
#include "MultiEffect.h"

//애님 툴
#include "AnimToolHelper.h"
#include "AnimToolObject.h"

// 클라이언트

#pragma region 컴포넌트
#include "RigidBody.h"
#include "CharacterController.h"
#pragma endregion

#pragma region 객체
#include "Camera_Free.h"
#include "TestModel.h"
#include "TestTerrain.h"
#include "Kirby.h"

// 몬스터
#include "KirbyWeapons.h"
#include "KirbyArmours.h"
#include "Awoofy.h"
#include "Rabbit.h"
#include "Buffahorn.h"
#include "BladeKnight.h"
#include "BladeKnightSword.h"
#include "Kabu.h"

#include "Moon.h"
#include "WasteCan.h"
#pragma endregion


CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

_uint APIENTRY LoadingMain(void* pArg)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	/* 로더에게 지정된 레벨을 준비해라*/
	CLoader* pLoader = (CLoader*)pArg;

	if (FAILED(pLoader->Start()))
		return 1;

	CoUninitialize();

	return 0;
}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_Critical_Section);

	/* 스레드를 생성하낟. */
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;


	return S_OK;
}

HRESULT CLoader::Start()
{
	EnterCriticalSection(&m_Critical_Section);

	HRESULT		hr = { 0 };
	SetUp_ModelScaleRotation(m_eNextLevelID);
	switch (m_eNextLevelID)
	{
	case LEVEL_LOGO:
	{
		hr = Loading_ObjectAll();
		CHECK_FAILED(hr);

		SetUp_ModelScaleRotation(LEVEL_STATIC);
		hr = Loading_StaticComponentAll();

		CHECK_FAILED(hr);
		hr = Loading_For_Logo();
		break;
	}
	case LEVEL_INTRO:
		hr = Loading_For_Intro();
		break;

	case LEVEL_GAMEPLAY:
		hr = Loading_For_GamePlay();
		break;

	// 05.20) UI Tool 레벨 추가
	case LEVEL_TOOL_UI:
		hr = Loading_For_Tool_UI();
		break;

	case LEVEL_TOOL_FX:
	{
		hr = Loading_For_Tool_FX();
		break;
	}

	case LEVEL_TOOL_ANIM:
	{
		hr = Loading_For_Tool_Anim();
		break;
	}

	case LEVEL_TOOL_MAP:
	{
		hr = Loading_For_Tool_Map();
		break;
	}

	}
	if (FAILED(hr))
		return E_FAIL;

	LeaveCriticalSection(&m_Critical_Section);

	return S_OK;
}

/// <summary> 게임에서 필요한 오브젝트 프로토타입을 추가한다. </summary>
HRESULT CLoader::Loading_ObjectAll()
{
	m_strLoadingText = TEXT("객체의 원형를(을) 로딩 중 입니다.");

#ifdef _DEBUG
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FXToolDirector"), CFXToolDirector);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Grid"), CGrid);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("MapToolHelper"), CMapToolHelper);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("MapToolObject"), CMapToolObject);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Editor_UI"), CEditor_UI);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("AnimToolHelper"), CAnimToolHelper);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("AnimToolObject"), CAnimToolObject);
#endif

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SingleEffect"), CSingleEffect);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("MultiEffect"), CMultiEffect);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Particle"), CParticle);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SkySphere"), CSkySphere);
	
	// MapTool GameObject Prototypes
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BasicMap"), CBasicMap);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Trigger"), CTrigger);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("OrbitingCamera"), COrbitingCamera);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BG"), CBG);

	// UI
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("LayerUI"), CLayerUI);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("Multi_UI"), CMulti_UI);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD"), CHUD);
	
#pragma region FOR CLIENT
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kirby"), CKirby);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyWeapons"), CKirbyWeapons);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyArmours"), CKirbyArmours);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Awoofy"), CAwoofy);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Rabbit"), CRabbit);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Buffahorn"), CBuffahorn);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BladeKnight"), CBladeKnight);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BladeKnightSword"), CBladeKnightSword);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kabu"), CKabu);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BackGround"), CBackGround);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_Test"), CTestUI);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestMap"), CTestTerrain);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestModel"), CTestModel);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Moon"), CMoon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("WasteCan"), CWasteCan);
#pragma endregion

	return S_OK;
}

//static 컴포넌트들을 로드한다.
HRESULT CLoader::Loading_StaticComponentAll()
{
	HRESULT hr;
	LEVEL eLevel = LEVEL_STATIC;

	// Static Model 한번에 생성.
	hr = Add_Models(eLevel);
	CHECK_FAILED(hr);

	//이펙트 텍스쳐
	hr = Add_Texture(eLevel, "FX_Test", "Effects/test.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "FX_Logo", "Logo/Logo.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "FX_SimpleSolid", "Simple/simpleSolid_%d.png", 2);
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "FX_SimpleStar", "Effects/SimpleStar.png");
	CHECK_FAILED(hr);

	hr = Add_Texture(eLevel, "FX_VacuumTornado", "Effects/wind01.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "FX_VacuumWind", "Effects/scroll07.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "FX_VacuumDGB", "Effects/twinkle02.png");
	CHECK_FAILED(hr);

	hr = Add_Texture(eLevel, "FX_Wind", "Effects/wind_%d.png", 2);
	CHECK_FAILED(hr);

	hr = Add_Texture(eLevel, "FX_Shockwave", "Effects/shockwave_%d.png", 1);
	CHECK_FAILED(hr);

	wstring wstrPrototypeTag = L"Prototype_Component_Shader_";
	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	CHECK_FAILED(hr);

	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("VtxInstance_Point"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Point.hlsl"), VTXINSTANCE_POINT::Elements, VTXINSTANCE_POINT::iNumElements));
	CHECK_FAILED(hr);
	

	wstrPrototypeTag = L"Prototype_Component_VIBuffer_";

	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);

	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("Instance_Point"),
		CVIBuffer_Instance_Point::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);


	return S_OK;
}

HRESULT CLoader::Loading_For_Logo()
{
	LEVEL eLevel = LEVEL_LOGO;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	/* For.Prototype_Component_Texture_Logo */
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Intro()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_INTRO;


	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Moon", "Moon.png")))
		return E_FAIL;

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);
#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
#pragma region 모델
	// 모아놓은 Model 한번에 생성.
	hr = Add_Models(eLevel);
	CHECK_FAILED(hr);
#pragma endregion

	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
#pragma region 물리 컴포넌트
	/* 리지드바디 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
	/* 캐릭터 컨트롤러 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_CharacterController"), CCharacterController::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
#pragma endregion

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
#pragma region 셰이더
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);
#pragma endregion

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_GamePlay()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_GAMEPLAY;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Moon", "Moon.png")))
		return E_FAIL;
	if(FAILED(Add_Texture(eLevel, "GsLandTopNoize_Fur", "Map/GsLandTopNoize_Fur.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Level_0_Env", "Map/Level_0_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;


	hr = Add_Texture(eLevel, "GameComplete", "UI/GAMECOMPLETE/GameComplete_%d.png", 21);
	CHECK_FAILED(hr);

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);
	#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	#pragma region 모델
	// 모아놓은 Model 한번에 생성.
	Load_AnimToolInfo();
	hr = Add_Models(eLevel);
	CHECK_FAILED(hr);
	#pragma endregion
	
	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	#pragma region 물리 컴포넌트
	/* 리지드바디 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
	/* 캐릭터 컨트롤러 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_CharacterController"), CCharacterController::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
	#pragma endregion
	
	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	#pragma region 셰이더
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_FX()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOOL_FX;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	#pragma region 텍스쳐
	//if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
	//	return E_FAIL;
	#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	#pragma region 모델
	// 모아놓은 Model 한번에 생성.
	//hr = Add_Models(eLevel);
	//CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	#pragma region 물리 컴포넌트
	//hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	//CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	#pragma region 셰이더
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_Anim()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOOL_ANIM;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	#pragma region 텍스쳐
	//if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
	//	return E_FAIL;
	Add_KirbyFaceTexture(eLevel);
	#pragma endregion

	m_strLoadingText = TEXT("VI버퍼(을) 로딩 중 입니다.");
	#pragma region VI버퍼
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Grid"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 5, 5));
	CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	#pragma region 모델
	Load_AnimToolInfo();
	if (FAILED(Add_AllModelTxts(eLevel, TYPE_ANIM)))
		return E_FAIL;
	hr = Add_Models(eLevel);
	CHECK_FAILED(hr);

	#pragma endregion

	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	#pragma region 물리 컴포넌트
	/* 리지드바디 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
	/* 캐릭터 컨트롤러 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_CharacterController"), CCharacterController::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	#pragma region 셰이더
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);
	#pragma endregion

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_Map()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOOL_MAP;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	//if (FAILED(Add_Texture(eLevel, "GsLandTopNoize_Fur", "Map/GsLandTopNoize_Fur.dds")))
	//	return E_FAIL;
	//if (FAILED(Add_Texture(eLevel, "GsDefaultSideRockC_Height", "Map/GsDefaultSideRockC_Height.dds")))
	//	return E_FAIL;

	m_strLoadingText = TEXT("VI버퍼(을) 로딩 중 입니다.");
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Grid"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 200, 200))))
		return E_FAIL;

	m_strLoadingText = TEXT("쉐이더(을) 로딩 중 입니다.");
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("모델(을) 로딩 중 입니다.");
	if(FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM)))
		return E_FAIL;
	
	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_UI()
{
	LEVEL eLevel = LEVEL_TOOL_UI;
	HRESULT hr = S_OK;

#pragma region TEXTURE

	//hr = Add_Texture(eLevel, "KirbyBarHard", "UI/HUD/Hero/BarHard/HeroPanelBarHard_%d.png", 3);
	hr = Add_Texture(eLevel, "GameComplete", "UI/GAMECOMPLETE/GameComplete_%d.png", 21);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("Loading For Texture : Complete!");

#pragma endregion

	m_strLoadingText = TEXT("Loading For UITool : Complete!");

	m_IsFinished = TRUE;
	return S_OK;
}

HRESULT CLoader::Add_Models(LEVEL eLevel)
{
	// SetUp_ModelScaleRotation 함수에서 모아놓은 Model들을 타입에 따라서 Component 생성한다.
	for (auto& ModelInfo : m_vecModelInfo)
	{
		if (ModelInfo.strModelName == "KirbyDefault"){
			_int i = 0;
		}

		// 애님툴에서 조정하여 저장한 값을 불러서
		// 모델 이름이 같을 경우, model의 정보들을 읽어오기
		for (auto& pair : m_mapSequence)
		{
			if (ModelInfo.strModelName == pair.first)
			{
				ModelInfo.umapAnimInfo = pair.second;
			}
		}

		wstring wstrModelName = CUtils::StrToWstr(ModelInfo.strModelName);
		wstring wstrPrototypeTag = L"Prototype_Component_Model_" + wstrModelName;
		if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, ModelInfo))))
			return E_FAIL;
	}

	return S_OK;
}

// 여기다가 모든 Model을 셋업한다.
void CLoader::SetUp_ModelScaleRotation(LEVEL eLevel)
{
	// MODEL 구조체 생성자 순서		: 이름 (파일이름) / ANIMTYPE / Scale / Degree (Y) / Root
	// MODEL 구조체 생성자 기본 값  : ""			  / TYPE_END /  1.f  /    0.f     / 4
	if (eLevel == LEVEL_STATIC)
	{
		//sky sphere
		m_vecModelInfo.emplace_back("SkySphere_Stage1_Day", TYPE_NONANIM );

		m_vecModelInfo.emplace_back("SmokeCenter", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeFadeLarge", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeOriginal", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeSplit", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeTail", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Tornado", TYPE_NONANIM );

		//커비 회오리
		m_vecModelInfo.emplace_back("VacuumTornado", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("VacuumWind", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("VacuumDGB", TYPE_NONANIM);

		m_vecModelInfo.emplace_back("SwordTrail", TYPE_NONANIM);

	}
	else if (eLevel == LEVEL_LOGO)
	{

	}
	else if (eLevel == LEVEL_INTRO)
	{
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);
		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);

		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, true);
		m_vecModelInfo.emplace_back("Level0Stage1Step01_Blend", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f);

		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);

		// For Mab Interactive Object
		m_vecModelInfo.emplace_back("WasteCanYellow", TYPE_NONANIM);
	}
	else if (eLevel == LEVEL_GAMEPLAY)
	{
		m_vecModelInfo.emplace_back("Fiona", TYPE_ANIM );
		m_vecModelInfo.emplace_back("Dee", TYPE_ANIM, 0.01f);
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		m_vecModelInfo.emplace_back("TestMap", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("TestMap2", TYPE_NONANIM, 0.01f);

		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);


		m_vecModelInfo.emplace_back("GsBenchAL", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Level1Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, true);
		m_vecModelInfo.emplace_back("Level1Stage1Step01_Blend", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM);

		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);

		// For Mab Interactive Object
		m_vecModelInfo.emplace_back("WasteCanYellow", TYPE_NONANIM);
	}
	else if (eLevel == LEVEL_TOOL_MAP)
	{
		// 맵툴에서는 크기나 회전 상태 바꾸고 싶은 모델만 여기에 등록. 안바꾸고싶으면 NonAnim, 크기1, 회전 0도로 자동 추가됨
		m_vecModelInfo.emplace_back("Book", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("TestMap2", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("Camera", TYPE_NONANIM, 0.2f, 270.f);
		m_vecModelInfo.emplace_back("Dummy", TYPE_NONANIM, 0.01f);

		//m_vecModelInfo.emplace_back("Level1Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, true);
	}
	else if (eLevel == LEVEL_TOOL_ANIM)
	{
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("BladeKnightSword",  TYPE_NONANIM, 1.f);
	}

}

HRESULT CLoader::Add_Shaders(LEVEL eLevel)
{
	HRESULT hr = S_OK;
	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxCube */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxCube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxInstance_Rect */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxInstance_Rect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Rect.hlsl"), VTXINSTANCE_RECT::Elements, VTXINSTANCE_RECT::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxInstance_Point */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Point.hlsl"), VTXINSTANCE_POINT::Elements, VTXINSTANCE_POINT::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel_Map */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel_Map"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel_Map.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	//이펙트용 쉐이더

	/* For.Prototype_Component_Shader_FXPosTex */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_FXPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	CHECK_FAILED(hr);

	/* For.Prototype_Component_Shader_FXModel */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_FXModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoader::Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures)
{
	wstring wstrPrototypeTag = L"Prototype_Component_Texture_" + CUtils::StrToWstr(strPrototypeName); 
	wstring wstrFullPath = L"../Bin/Resources/Textures/" + CUtils::StrToWstr(strFolderAndFileName);

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CTexture::Create(m_pDevice, m_pContext, wstrFullPath, iNumTextures))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Add_KirbyFaceTexture(LEVEL eLevel)
{
	if (FAILED(Add_Texture(eLevel, "anger", "KirbyFace/anger.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "blink", "KirbyFace/blink.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "close", "KirbyFace/close.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "doubt", "KirbyFace/doubt.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "idle", "KirbyFace/idle.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "pupil", "KirbyFace/pupil.png")))
		return E_FAIL;


	if (FAILED(Add_Texture(eLevel, "mouth_anger", "KirbyFace/mouth_anger.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_base", "KirbyFace/mouth_base.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_happy", "KirbyFace/mouth_happy.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_smile", "KirbyFace/mouth_smile.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_surprise", "KirbyFace/mouth_surprise.png")))
		return E_FAIL;

	// Awoofy Eye
	if (FAILED(Add_Texture(eLevel, "Awoofy_Eye", "AwoofyEye/NormalEnemyEye%d.dds", 5)))
		return E_FAIL;

	// Rabbit Eye
	if (FAILED(Add_Texture(eLevel, "Rabbit_Eye", "RabbitEye/RabbitEnemyEye.0%d.dds", 5)))
		return E_FAIL;

	// Buffahorn Eye
	if (FAILED(Add_Texture(eLevel, "Buffahorn_Eye", "BuffahornEye/TackleEnemyEye.0%d.dds", 4)))
		return E_FAIL;

	return S_OK;
}

// TOOL_MAP, TOOL_ANIM에서 사용중인 함수.
HRESULT CLoader::Add_AllModelTxts(LEVEL eLevel, TYPE eType)
{
	HRESULT hr = S_OK;

	wstring wstrRootFolderPath = TEXT("../../../model_txt/");
	if (TYPE_ANIM == eType)
		wstrRootFolderPath += TEXT("Anim/");
	else if (TYPE_NONANIM == eType)
		wstrRootFolderPath += TEXT("NonAnim/");

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((wstrRootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return E_FAIL;
	}

	list<wstring> txtList;
	TraverseModelTxts(wstrRootFolderPath, txtList);

	for (auto& listIter : txtList)
	{
		wstring wstrModelName = listIter.substr(0, listIter.length() - 4);
		string strModelName = CUtils::WstrToStr(wstrModelName);
		
		_bool bFound = { false };
		
		// 원래 버전
		//MODEL tModelInfo = MODEL{ strModelName ,  eType };
		//for (auto& modelInfo : m_vecModelInfo)
		//{
		//	if (modelInfo.strModelName == strModelName)
		//	{
		//		tModelInfo = modelInfo;
		//		break;
		//	}
		//}
		
		MODEL tModelInfo = MODEL{ strModelName ,  eType };
		for (auto& modelInfo : m_vecModelInfo)
		{
			// 애님툴에서 조정하여 저장한 값을 불러서
			// 모델 이름이 같을 경우, model의 정보들을 읽어오기
			for (auto& pair : m_mapSequence)
			{
				if (modelInfo.strModelName == pair.first)
				{
					modelInfo.umapAnimInfo = pair.second;
				}
			}
			if (modelInfo.strModelName == strModelName)
			{
				tModelInfo = modelInfo;
				break;
			}
		}

		if (strModelName.size() > 8 && "NonAnim" == strModelName.substr(0, 7))
			tModelInfo.fDegree = tModelInfo.fDegree + 180.f;

		wstring wstrPrototypeTag = TEXT("Prototype_Component_Model_") + CUtils::StrToWstr(tModelInfo.strModelName);
		hr = m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CModel::Create(m_pDevice, m_pContext, tModelInfo));
		CHECK_FAILED(hr);
	}

	FindClose(hFind);

 	return S_OK;
}

void CLoader::TraverseModelTxts(const wstring& rootFolderPath, list<wstring>& fileList)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((rootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return;
	}

	do 
	{
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
			if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
				// 재귀적으로 하위 폴더도 순회
				TraverseModelTxts(rootFolderPath + L"\\" + findFileData.cFileName, fileList);
			}
		}
		else 
		{
			// 파일이면 리스트에 추가
			fileList.push_back(wstring(findFileData.cFileName));
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}

// AnimTool에서 만들어놓은 파일을 읽어서 가지고있는 함수
void CLoader::Load_AnimToolInfo()
{
	// XML 파일을 읽어올 경로 설정
	const char* filePath = "../Bin/Resources/Data/AnimationData.xml";

	// XMLDocument 객체 생성
	tinyxml2::XMLDocument m_xmlDocument;

	// XML 파일 로드
	if (m_xmlDocument.LoadFile(filePath) != tinyxml2::XML_SUCCESS)
	{
		MSG_BOX(TEXT("Failed to load XML file"));
		return;
	}

	// 루트 요소 가져오기
	tinyxml2::XMLElement* pRoot = m_xmlDocument.RootElement();

	// ModelName 및 Animation 정보를 읽어옴
	for (tinyxml2::XMLElement* pModelElement = pRoot->FirstChildElement("ModelName");
		pModelElement != nullptr;
		pModelElement = pModelElement->NextSiblingElement("ModelName"))
	{

		// 문자열 끝 부분의 공백 제거
		string modelNameStr(pModelElement->GetText());
		modelNameStr.erase(std::find_if(modelNameStr.rbegin(), modelNameStr.rend(), [](_ubyte ch) {
			return !std::isspace(ch);
			}).base(), modelNameStr.end());

		if (!modelNameStr.empty())
		{
			// ModelName에 해당하는 AnimMap을 생성
			AnimToolMap::mapped_type& animMap = m_mapSequence[string(modelNameStr)];

			// Animation 정보 읽기
			for (tinyxml2::XMLElement* pAnimElement = pModelElement->FirstChildElement("Animation");
				pAnimElement != nullptr;
				pAnimElement = pAnimElement->NextSiblingElement("Animation"))
			{
				const char* animName = pAnimElement->GetText();
				if (animName)
				{
					// ANIM_INFO 객체 생성 및 초기화
					ANIM_INFO animInfo;

					// AnimSpeed 읽기
					tinyxml2::XMLElement* pAnimSpeedElement = pAnimElement->NextSiblingElement("AnimSpeed");
					if (pAnimSpeedElement)
					{
						_float animSpeed;
						pAnimSpeedElement->QueryFloatText(&animSpeed);
						animInfo.fAnimSpeed = animSpeed;
					}

					// Count 값 읽기
					tinyxml2::XMLElement* pCountElement = pAnimElement->NextSiblingElement("Count");
					if (pCountElement)
					{
						_uint count;
						pCountElement->QueryUnsignedText(&count);

						// Event 정보 읽기
						for (unsigned int i = 0; i < count; ++i)
						{
							std::string dataName = "Data" + std::to_string(i);
							tinyxml2::XMLElement* pDataElement = pCountElement->NextSiblingElement(dataName.c_str());
							if (pDataElement)
							{
								EVENT_INFO eventInfo;

								// EventName, StartFrame, EndFrame 읽기
								const char* eventName = pDataElement->Attribute("EventName");
								int startFrame, endFrame;
								pDataElement->QueryIntAttribute("StartFrame", &startFrame);
								pDataElement->QueryIntAttribute("EndFrame", &endFrame);

								eventInfo.strEventName = eventName ? eventName : "";
								eventInfo.iStartFrame = startFrame;
								eventInfo.iEndFrame = endFrame;

								// ANIM_INFO의 vecEventInfo에 추가
								animInfo.vecEventInfo.push_back(eventInfo);
							}
						}
					}

					// ANIM_INFO 객체를 AnimMap에 추가
					animMap[string(animName)] = animInfo;
				}
			}
		}
	}
}

CLoader * CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLoader* pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Create : CLoader"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	WaitForSingleObject(m_hThread, INFINITE);

	DeleteObject(m_hThread);

	CloseHandle(m_hThread);

	DeleteCriticalSection(&m_Critical_Section);

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

