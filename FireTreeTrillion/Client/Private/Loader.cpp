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

//UI 툴
#pragma region TOOL_UI
#ifdef _DEBUG
#include "Editor_UI.h"
#include "LayerUI.h"
#endif
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
//카메라
#include "Camera_Free.h"
#include "Camera_Main.h"

#include "TestModel.h"
#include "TestTerrain.h"
#include "Kirby.h"
#include "BombOrbitGlow.h"
#include "BombOrbit.h"
#include "KirbyBomb.h"
#include "PartTimerKirby.h"


// 몬스터
#include "KirbyWeapons.h"
#include "KirbyArmours.h"
#include "Awoofy.h"
#include "Rabbit.h"
#include "Buffahorn.h"
#include "BladeKnight.h"
#include "BladeKnightSword.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "PoppyBrosJr.h"
#include "PoppyBomb.h"
#include "CappyBody.h"
#include "CappyHat.h"

//와들디
#include "FoodShopDee.h"

// 맵 오브젝트
#include "Moon.h"
#include "KickableRock.h"
#include "WasteCan.h"
#include "StarBlock.h"
#include "StarBlockPiece.h"
#include "TerrainFog.h"
#include "BreakableRock.h"
#include "Car.h"
#include "BreakableRock.h"
#include "BreakableRockPartical.h"

//UI
#include "BackGround.h"
#include "HUD.h"
#include "HUD_KirbyStatus.h"
#include "HUD_StarPoint.h"
#include "BombOrbit.h"
#include "BombOrbitGlow.h"
#include "HUD_AbilityDiscard.h"


// 아이템
#include "EnergyDrink.h"
#include "Coin.h"
#include "Ability.h"

// 콜라이더
#include "HitBox.h"


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
	}
	break;

	case LEVEL_GAMEPLAY:
		hr = Loading_For_GamePlay();
		break;
	// 05.20) UI Tool 레벨 추가
	case LEVEL_TOOL_UI:
		hr = Loading_For_Tool_UI();
		break;

	case LEVEL_TOOL_FX:
		hr = Loading_For_Tool_FX();
		break;

	case LEVEL_TOOL_ANIM:
		hr = Loading_For_Tool_Anim();
		break;

	case LEVEL_TOOL_MAP:
		hr = Loading_For_Tool_Map();
		break;

	//////////////////////////////////////// IN GAME
	case LEVEL_INTRO:
		hr = Loading_For_Intro();
		break;
	case LEVEL_RACING:
		hr = Loading_For_Racing();
		break;
	case LEVEL_TOWN:
		hr = Loading_For_Town();
		break;
	case LEVEL_PARTTIME:
		hr = Loading_For_Parttime();
		break;

	case LEVEL_FINALBOSS:
		hr = Loading_For_FinalBoss();
		break;
	}

	LeaveCriticalSection(&m_Critical_Section);

	if (FAILED(hr))
		return E_FAIL;

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
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("LayerUI"), CLayerUI);

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

	// For HitBox
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HitBox"), CHitBox);
#pragma region UI

	// UI
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD"), CHUD);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_KirbyStatus"), CHUD_KirbyStatus);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_StarPoint"), CHUD_StarPoint);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_AbilityDiscard"), CHUD_AbilityDiscard);
	// 
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_HPBoss"), CHUD_HPBoss);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_Mission"), CHUD_Mission);

#pragma endregion
	
#pragma region FOR CLIENT
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kirby"), CKirby);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyWeapons"), CKirbyWeapons);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyArmours"), CKirbyArmours);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BombOrbit"), CBombOrbit);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BombOrbitGlow"), CBombOrbitGlow);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyBomb"), CKirbyBomb);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PartTimerKirby"), CPartTimerKirby);

	// Deform
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Car"), CCar);

	// Monster
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Awoofy"), CAwoofy);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Rabbit"), CRabbit);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Buffahorn"), CBuffahorn);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BladeKnight"), CBladeKnight);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BladeKnightSword"), CBladeKnightSword);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kabu"), CKabu);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BrontoBurt"), CBrontoBurt);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PoppyBrosJr"), CPoppyBrosJr);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PoppyBomb"), CPoppyBomb);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CappyBody"), CCappyBody);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CappyHat"), CCappyHat);

	//Dee
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FoodShopDee"), CFoodShopDee);


	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BackGround"), CBackGround);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Main"), CCamera_Main);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestMap"), CTestTerrain);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestModel"), CTestModel);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Moon"), CMoon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("WasteCan"), CWasteCan);

	// Item
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("EnergyDrink"), CEnergyDrink);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Coin"), CCoin);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Ability"), CAbility);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Ladder"), CLadder);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KickableRock"), CKickableRock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("StarBlock"), CStarBlock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("StarBlockPiece"), CStarBlockPiece);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TerrainFog"), CTerrainFog);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BreakableRock"), CBreakableRock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BreakableRockPartical"), CBreakableRockPartical);

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

	// 쉐이더
	Add_Shaders();

	//이펙트 텍스쳐
	Add_FXTexture();

	wstring wstrPrototypeTag = L"Prototype_Component_Shader_";
	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	CHECK_FAILED(hr);

	//hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("VtxInstance_Point"),
	//	CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Point.hlsl"), VTXINSTANCE_POINT::Elements, VTXINSTANCE_POINT::iNumElements));
	//CHECK_FAILED(hr);
	//

	wstrPrototypeTag = L"Prototype_Component_VIBuffer_";

	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);

	hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + TEXT("Instance_Point"),
		CVIBuffer_Instance_Point::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);


#pragma region LEVEL_SKYSPHERE

	hr = Add_Texture(eLevel, "SkySphere_LabDiscovera_2Pase", "SkySphere/SkySphere_LabDiscovera_2Pase_Diffuse.dds");
	CHECK_FAILED(hr);

	hr = Add_Texture(eLevel, "SkySphere_LabDiscovera_2Pase_Normal", "SkySphere/SkySphere_LabDiscovera_2Pase_Normal.dds");
		CHECK_FAILED(hr);

	hr = Add_Texture(eLevel, "SkySphere_LabDiscovera_2Pase_Emissive", "SkySphere/SkySphere_LabDiscovera_2Pase_Emissive.dds");
	CHECK_FAILED(hr);

	hr = Add_Texture(eLevel, "SkySphere_LabDiscovera_2Pase_Height", "SkySphere/SkySphere_LabDiscovera_2Pase_Height.dds");
	CHECK_FAILED(hr);

#pragma endregion

	return S_OK;
}

HRESULT CLoader::Add_Shaders()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_STATIC;
	
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
	if (FAILED(Add_Texture(eLevel, "Level_0_Env", "Map/Level_0_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;

	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

#pragma region UI

	//KirbyHP
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");

	//StarPoint
	hr = Add_Texture(eLevel, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);

	//Ability Discard
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 17);
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);

	CHECK_FAILED(hr);

#pragma endregion

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);
#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
#pragma region 모델
	Load_AnimToolInfo();
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

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Racing()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_RACING;


	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Moon", "Moon.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Level_0_Env", "Map/Level_0_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;

	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

#pragma region UI

	//KirbyHP
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");

	//StarPoint
	hr = Add_Texture(eLevel, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);

	//Ability Discard
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 17);
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);

	CHECK_FAILED(hr);

#pragma endregion

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);
#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
#pragma region 모델
	Load_AnimToolInfo();
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
	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

#pragma region UI

	//KirbyHP
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");

	//StarPoint
	hr = Add_Texture(eLevel, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);

	//Ability Discard
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 17);
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);

	CHECK_FAILED(hr);

#pragma endregion

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
	
	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Town()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOWN;


	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Level_Town_Env", "Map/Level_Town_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;

	//마스크용
	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

#pragma region UI

	//KirbyHP
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");

	//StarPoint
	hr = Add_Texture(eLevel, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);

	//Ability Discard
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 17);
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);

	CHECK_FAILED(hr);
#pragma endregion

	// 얼굴, 눈 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);
#pragma endregion


	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
#pragma region 모델
	Load_AnimToolInfo();
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


	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Parttime()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_PARTTIME;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Level_Town_Env", "Map/Level_Town_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;

	//마스크용
	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

	// 얼굴, 눈 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);

#pragma endregion


	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
#pragma region 모델
	Load_AnimToolInfo();
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


	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_FinalBoss()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_FINALBOSS;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Moon", "Moon.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Level_0_Env", "Map/Level_0_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;

	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

#pragma region UI

	//KirbyHP
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");

	//StarPoint
	hr = Add_Texture(eLevel, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);

	//Ability Discard
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 17);
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);

	CHECK_FAILED(hr);

#pragma endregion

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);
#pragma endregion

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
#pragma region 모델
	Load_AnimToolInfo();
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
	// 애님툴에서 저장한 데이터들을 읽어온다.
	Load_AnimToolInfo();
	// 애님툴에서 필요한 모델들의 프로토타입을 추가한다. 또한 필요한 데이터정리를 시행한다.
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

	m_strLoadingText = TEXT("모델(을) 로딩 중 입니다.");

	if (FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM, L"MapDeco/")))
		return E_FAIL;
	if (FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM, L"MapObjs/")))
		return E_FAIL;
	if (FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM, L"Monsters/")))
		return E_FAIL;

	if (FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM, L"TownDeco/")))
		return E_FAIL;

	//LEVEL_FINALBOSS (MapName :: LAB_Discovera)
	if (FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM, L"LabDiscovera_Deco/")))
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

	//KirbyHP
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");

	//StarPoint
	hr = Add_Texture(eLevel, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);

	//Ability Discard
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 17);
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);
	
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("Loading For Texture : Complete!");

#pragma endregion

	m_strLoadingText = TEXT("Loading For UITool : Complete!");

	m_IsFinished = TRUE;
	return S_OK;
}

HRESULT CLoader::Add_Models(LEVEL eLevel)
{
	HRESULT hr = S_OK;

	// SetUp_ModelScaleRotation 함수에서 모아놓은 Model들을 타입에 따라서 Component 생성한다.
	for (auto& ModelInfo : m_vecModelInfo)
	{
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

		hr = m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CModel::Create(m_pDevice, m_pContext, ModelInfo));
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CLoader::Add_FXTexture()
{
	HRESULT hr;

	//테스트용
	hr = Add_Texture(LEVEL_STATIC, "FX_Test", "Effects/test.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SimpleStar", "Effects/simpleStar.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Logo", "Effects/simpleStar.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SimpleSolid", "Simple/simpleSolid_%d.png", 2);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_BombOrbit", "Effects/BombOrbitGlow.dds");	CHECK_FAILED(hr);

	//마스크
	hr = Add_Texture(LEVEL_STATIC, "FX_Mask_Bubble", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Mask_Updown", "Effects/Mask/UpDownMask.png");	CHECK_FAILED(hr);

	
	// 주로 사용되는 텍스쳐들
	hr = Add_Texture(LEVEL_STATIC, "FX_Star", "Effects/Basic/common_star.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Bubble", "Effects/Basic/common_bubble.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Circles", "Effects/Basic/common_circle_%d.png", 4);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Grad", "Effects/Basic/common_gradation.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Hit", "Effects/Basic/common_ring_0.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Smoke", "Effects/Basic/common_smoke.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Line", "Effects/Basic/common_line_0.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Jump", "Effects/Basic/common_jump.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Sparkle", "Effects/Basic/common_sparkle.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Lead", "Effects/Basic/leaf.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Collide", "Effects/Basic/common_hit.png");	CHECK_FAILED(hr);


	hr = Add_Texture(LEVEL_STATIC, "FX_VacuumTornado", "Effects/Basic/wind01.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_VacuumWind", "Effects/Basic/scroll07.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_VacuumDGB", "Effects/Basic/twinkle02.png");	CHECK_FAILED(hr);

	hr = Add_Texture(LEVEL_STATIC, "FX_Wind", "Effects/Basic/wind_%d.png", 2);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Shockwave", "Effects/Basic/shockwave_%d.png", 1);	CHECK_FAILED(hr);

	//칼
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordSlash", "Effects/Sword/slash_%d.png", 6);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordTail", "Effects/Sword/sword_tail.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordDecal", "Effects/Sword/sword_decal.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordLine", "Effects/Sword/line.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordShockwave", "Effects/Sword/shockwave.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordRingNoFrip", "Effects/Sword/ringnofrip.png");	CHECK_FAILED(hr);

	//잎
	hr = Add_Texture(LEVEL_STATIC, "FX_BushCut", "Effects/Basic/BushColor_%d.png", 4);	CHECK_FAILED(hr);

	return E_NOTIMPL;
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


		//이펙트 친구들...

		m_vecModelInfo.emplace_back("SmokeCenter", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeFadeLarge", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeOriginal", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeSplit", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeTail", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Tornado", TYPE_NONANIM );

		//부쉬 쪼가리
		m_vecModelInfo.emplace_back("BushCutS", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("BushCutM", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("BushCutL", TYPE_NONANIM);

		//이펙트 입히는 원기둥
		m_vecModelInfo.emplace_back("CylinderA", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("CylinderB", TYPE_NONANIM);

		m_vecModelInfo.emplace_back("Dash", TYPE_NONANIM);

		//커비 회오리
		m_vecModelInfo.emplace_back("VacuumTornado", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("VacuumWind", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("VacuumDGB", TYPE_NONANIM);

		//칼 관련
		m_vecModelInfo.emplace_back("SwordTrail", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("SwordSlash_hrzt", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("UpwardSlash", TYPE_NONANIM);

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
		m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);


		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyBombDefault", TYPE_ANIM, 1.3f, 180.f);

		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);


		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level0Stage1Step01_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG0", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		// 자동차 관련 (자동차와 부수는 돌멩이들)
		m_vecModelInfo.emplace_back("Car", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("RockA", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("RockB", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("RockPartical", TYPE_NONANIM, 1.f);


		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.2f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("CappyBody", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("CappyHat", TYPE_ANIM, 1.f, 180.f);

		//와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

		// For Mab Interactive Object
		m_vecModelInfo.emplace_back("GsPebble", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("SeShell", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("WasteCanYellow", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		// For Item
		m_vecModelInfo.emplace_back("Item_EnergyDrink", TYPE_NONANIM, 3.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Item_Coin", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Item_Sword", TYPE_NONANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("Item_Bomb", TYPE_NONANIM, 1.f, 0.f);

		// For Interaction Decor
		m_vecModelInfo.emplace_back("GsWoodBridgeA", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("PopFlower", TYPE_ANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("WoodParts", TYPE_ANIM, 1.f, 0.f, 0, string("MapDeco/"));


	}
	else if (eLevel == LEVEL_TOWN)
	{
		m_vecModelInfo.emplace_back("Town", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));


		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);
		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyPartTimer", TYPE_ANIM, 1.f, 180.f);


		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyBombDefault", TYPE_ANIM, 1.3f, 180.f);

		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);

		// 와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

	}

	else if (eLevel == LEVEL_RACING)
	{
		m_vecModelInfo.emplace_back("Level0Stage1Step02", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level0Stage1Step02_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG0", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));


		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);
		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyBombDefault", TYPE_ANIM, 1.3f, 180.f);

		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);


		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.2f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("CappyBody", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("CappyHat", TYPE_ANIM, 1.f, 180.f);

		// For Mab Interactive Object
		m_vecModelInfo.emplace_back("GsPebble", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("SeShell", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("WasteCanYellow", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		// For Item
		m_vecModelInfo.emplace_back("Item_EnergyDrink", TYPE_NONANIM, 3.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Item_Coin", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Item_Sword", TYPE_NONANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("Item_Bomb", TYPE_NONANIM, 1.f, 0.f);

		// For Interaction Decor
		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("PopFlower", TYPE_ANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("WoodParts", TYPE_ANIM, 1.f, 0.f, 0, string("MapDeco/"));
	}

	else if (eLevel == LEVEL_PARTTIME)
	{
		m_vecModelInfo.emplace_back("TownShop", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyPartTimer", TYPE_ANIM, 1.f, 180.f);

		// 와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

		// 음식 나갑니다~
		m_vecModelInfo.emplace_back("Item_EnergyDrink", TYPE_NONANIM, 3.f, 0.f, 0, string("MapObjs/"));
	}

	else if (eLevel == LEVEL_FINALBOSS)
	{
		m_vecModelInfo.emplace_back("LbLastBossStage", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));


		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);
		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyBombDefault", TYPE_ANIM, 1.3f, 180.f);

		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);
	}
	else if (eLevel == LEVEL_GAMEPLAY)
	{
		m_vecModelInfo.emplace_back("Fiona", TYPE_ANIM );
		m_vecModelInfo.emplace_back("Dee", TYPE_ANIM, 0.01f);
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyBombDefault", TYPE_ANIM, 1.3f, 180.f);

		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);
		
		m_vecModelInfo.emplace_back("GsBenchAL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("Level1Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level1Stage1Step01_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.2f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 1.5f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("CappyBody", TYPE_ANIM, 1.2f, 180.f);
		m_vecModelInfo.emplace_back("CappyHat", TYPE_ANIM, 1.2f, 180.f);

		// For Mab Interactive Object
		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("GsPebble", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("SeShell", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("WasteCanYellow", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("StarBlockL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("StarBlockM", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("StarBlockS", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("WoodParts", TYPE_ANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("StarBlockPiece", TYPE_NONANIM, 0.5f, 180.f);
		m_vecModelInfo.emplace_back("StarBlockPieceStar", TYPE_NONANIM, 0.5f, 180.f);


		// For Item
		m_vecModelInfo.emplace_back("Item_EnergyDrink", TYPE_NONANIM, 3.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Item_Coin", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Item_Sword", TYPE_NONANIM, 1.f, 0.f);

		// 와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

	}
	else if (eLevel == LEVEL_TOOL_MAP)
	{
		// 맵툴에서는 크기나 회전 상태 바꾸고 싶은 모델만 여기에 등록. 안바꾸고싶으면 NonAnim, 크기1, 회전 0도로 자동 추가됨
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("Camera", TYPE_NONANIM, 0.2f, 270.f);
		m_vecModelInfo.emplace_back("Dummy", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("RallyPoint", TYPE_NONANIM, 2.f);
		m_vecModelInfo.emplace_back("LightBulb", TYPE_NONANIM, 0.02f);
		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level1Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Fog", TYPE_NONANIM, 0.002f);
		m_vecModelInfo.emplace_back("zBat", TYPE_NONANIM, 0.002f, 0.f, 0, string("TownDeco/"));
	}
	else if (eLevel == LEVEL_TOOL_ANIM)
	{
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);


		m_vecModelInfo.emplace_back("KirbyPartTimer", TYPE_ANIM, 1.f, 0.f);
		
		// For Kirby Weapon
		m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);
		// For Kirby Armour
		m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);

		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 2.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);

		//와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);
	}
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

	// Kirby Guide UI
	if (FAILED(Add_Texture(eLevel, "BombOrbit", "KirbyBombOrbit/BombOrbit.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BombOrbitGlow", "KirbyBombOrbit/BombOrbitGlow.dds")))
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
	// BrontoBurt Eye
	if (FAILED(Add_Texture(eLevel, "BrontoBurt_Eye", "BrontoBurtEye/Face.0%d.dds", 2)))
		return E_FAIL;

	// WaddleDee Eye
	HRESULT hr = Add_Texture(eLevel, "Dee_Eye", "WaddleDeeEye/eye_%d.png", 6);
		CHECK_FAILED(hr);

	return S_OK;
}

// TOOL_MAP, TOOL_ANIM에서 사용중인 함수.
HRESULT CLoader::Add_AllModelTxts(LEVEL eLevel, TYPE eType, wstring wstrFolder)
{
	HRESULT hr = S_OK;

	wstring wstrRootFolderPath = TEXT("../../../model_txt/");
	wstrRootFolderPath += wstrFolder;

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
		
		MODEL tModelInfo = MODEL{ strModelName ,  eType };
		for (auto& modelInfo : m_vecModelInfo)
		{
			if (modelInfo.strModelName == strModelName)
			{
				tModelInfo = modelInfo;
				break;
			}
		}

		if (strModelName.size() > 8 && "NonAnim" == strModelName.substr(0, 7))
			tModelInfo.fDegree = tModelInfo.fDegree + 180.f;

		tModelInfo.strFolder = CUtils::WstrToStr(wstrFolder);

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
				string animName = Remove_BeforeLastPipe(pAnimElement->GetText());
				if (!animName.empty())
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
								if (eventName && strcmp(eventName, "Notify") == 0)
									continue;
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
					animMap[animName] = animInfo;
				}
			}
		}
	}
}

string CLoader::Remove_BeforeLastPipe(const string& str)
{
	size_t pos = str.find_last_of('|');
	if (pos != string::npos)
	{
		return str.substr(pos + 1); // 마지막 '|' 이후의 문자열 반환
	}
	return str; // '|'가 없으면 원래 문자열 반환
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

