#include "stdafx.h"
#include "Loader.h"
#include <process.h>
#include <codecvt>
#include <locale>
#include "GameInstance.h"
#include "tinyxml2.h"

//맵툴, 맵
#include "OrbitingCamera.h"
#include "MapToolHelper.h"
#include "MapToolObject.h"
#include "NonRenderWall.h"
#include "NonAnimDeco.h"
#include "ShadowDeco.h"
#include "AnimDeco.h"
#include "BasicMap.h"
#include "Trigger.h"
#include "Grid.h"
#include "BG.h"

//스카이 스피어
#include "SkySphere.h"
#include "SkySphereSub.h"

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
#include "FinaleKirby.h"
#include "BombOrbitGlow.h"
#include "BombOrbit.h"
#include "KirbyBomb.h"
#include "PartTimerKirby.h"
#include "BulbFlare.h"

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
#include "Phanta.h"
#include "SurprisedBoard.h"
#include "SpookStep.h"
#include "GhostGordo.h"
#include "Bomber.h"

// 보스 몬스터
#include "FinalBoss.h"
#include "FinalBossSpear.h"
#include "RayArrow.h"
#include "Laser.h"
#include "SpikeSpear.h"
#include "DimensionGate.h"
#include "Meteor.h"
#include "BossClone.h"
#include "Gully.h"

#include "FinaleBoss.h"

// 보스
#include "DeeDeeDee.h"
#include "DeeDeeDeeHammer.h"

//와들디
#include "Dee_Part.h"
#include "OriginalDee.h"
#include "FoodShopDee.h"
#include "HungryDee.h"
#include "BattleDee.h"

// 맵 오브젝트
#include "Moon.h"
#include "KickableRock.h"
#include "WasteCan.h"
#include "StarBlock.h"
#include "StarBlockPiece.h"
#include "TerrainFog.h"
#include "BreakableRock.h"
#include "BreakableRockParticle.h"
#include "Car.h"
#include "Dump.h"
#include "Bulb.h"
#include "BlendMapObject.h"
#include "PortalSoftEffect.h"

// Racing Gimmick Objects
#include "CarShopWall.h"
#include "CarShopWallFrame.h"
#include "ToppleableBridge.h"
#include "AnimBridge.h"
#include "Tunnel.h"
#include "TunnelRock.h"
#include "Radio.h"
#include "Fog_Instance.h"
#include "Box.h"
#include "Debris.h"

// Park Gimmick Objects
#include "Crumble.h"

// Simba Gimmick Objects
#include "Turbine.h"
#include "SimbaRoomGlass.h"
#include "RoomGlass.h"
#include "Throne.h"

// Simba
#include "Simba.h"

// 피날레 스테이지 기믹들
#include "Baum.h"
#include "BaumPiece.h"
#include "Disaster_Master.h"
#include "FinaleRoad.h"
#include "FinaleRoadGrouper.h"
#include "PopStar.h"
#include "PopStar_StarSmall.h"
#include "BreakableBlock.h"
#include "FinalePartical.h"
#include "FinaleBuildingPartical.h"
#include "FinalePartical_Maker.h"
#include "FinaleCut_ControlCenter.h"

#include "Finale_SpecialDebris_A.h"
#include "Finale_SpecialDebris_B.h"
#include "Finale_SpecialDebris_C.h"
#include "BuildingCluster_A.h"
#include "BuildingCluster_B.h"
#include "BuildingCluster_C.h"

#pragma region LEVEL_FINALBOSS (LAB_DISCOVERA)
//BOSS
#include "BossChimera.h"

//GIMMICK
#include "Gm_LabAntenna.h"
#include "Gm_LabBossRoomDoor.h"

#pragma endregion

//기믹
#include "Gm_ParkFhEntranceAlien.h"
#include "Gm_ParkSolarPanelCharge.h"
#include "Gm_ParkSolarPanelOnce.h"
#include "Gm_DynamicField.h"

//UI
#include "BackGround.h"
#include "HUD.h"
#include "HUD_KirbyStatus.h"
#include "HUD_StarPoint.h"
#include "BombOrbit.h"
#include "BombOrbitGlow.h"
#include "HUD_AbilityDiscard.h"
#include "UI_PartTime.h"
#include "UI_PartTimeDee.h"
#include "UI_PartTimeResult.h"
#include "HUD_BossHpBar.h"
#include "TransingStar.h"
#include "UI_MessageWindow.h"
#include "UI_BtnIcon.h"
#include "UI_Fading.h"

// 아이템
#include "Food.h"
#include "Coin.h"
#include "Ability.h"

// 콜라이더
#include "HitBox.h"

// 미니게임(와들디마을)
#include "PartTimeFood.h"

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
	case LEVEL_DEEDEEDEE:
		hr = Loading_For_DeeDeeDee();
		break;

	case LEVEL_TOWN:
		hr = Loading_For_Town();
		break;
	case LEVEL_PARTTIME:
		hr = Loading_For_Parttime();
		break;
	case LEVEL_PARK:
		hr = Loading_For_Park();
		break;

	case LEVEL_SIMBA:
		hr = Loading_For_Simba();
		break;
	case LEVEL_FINALBOSS:
		hr = Loading_For_FinalBoss();
		break;
	case LEVEL_FINALE:
		hr = Loading_For_Finale();
		break;
	}

	LeaveCriticalSection(&m_Critical_Section);
	CHECK_FAILED(hr);

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
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SkySphereSub"), CSkySphereSub);
	
	// MapTool GameObject Prototypes
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BasicMap"), CBasicMap);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Trigger"), CTrigger);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("OrbitingCamera"), COrbitingCamera);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BG"), CBG);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("AnimDeco"), CAnimDeco);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("NonAnimDeco"), CNonAnimDeco);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("NonRenderWall"), CNonRenderWall);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PortalSoftEffect"), CPortalSoftEffect);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("ShadowDeco"), CShadowDeco);

	// For HitBox
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HitBox"), CHitBox);
#pragma region UI

	// UI
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD"), CHUD);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_KirbyStatus"), CHUD_KirbyStatus);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_StarPoint"), CHUD_StarPoint);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_AbilityDiscard"), CHUD_AbilityDiscard);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_BossHpBar"), CHUD_BossHpBar);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("HUD_Mission"), CHUD_Mission);
	
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_PartTime"), CUI_PartTime);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_PartTimeDee"), CUI_PartTimeDee);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_PartTimeResult"), CUI_PartTimeResult);
	
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_TransingStar"), CTransingStar);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_Fading"), CUI_Fading);

	//DIALOG
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Dialog"), CDialog);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_MessageWindow"), CUI_MessageWindow);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_BtnIcon"), CUI_BtnIcon);

#pragma endregion
	

#pragma region FOR CLIENT
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kirby"), CKirby);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinaleKirby"), CFinaleKirby);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyWeapons"), CKirbyWeapons);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyArmours"), CKirbyArmours);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BombOrbit"), CBombOrbit);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BombOrbitGlow"), CBombOrbitGlow);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KirbyBomb"), CKirbyBomb);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PartTimerKirby"), CPartTimerKirby);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BulbFlare"), CBulbFlare);

	// Deform
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Car"), CCar);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("DumpCar"), CDump);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Bulb"), CBulb);

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
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Phanta"), CPhanta);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SurprisedBoard"), CSurprisedBoard);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Crumble"), CCrumble);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SpookStep"), CSpookStep);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("GhostGordo"), CGhostGordo);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Bomber"), CBomber);
	
	// FinalBoss
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinalBoss"), CFinalBoss);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinalBossSpear"), CFinalBossSpear);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("RayArrow"), CRayArrow);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Laser"), CLaser);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SpikeSpear"), CSpikeSpear);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("DimensionGate"), CDimensionGate);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Meteor"), CMeteor);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BossClone"), CBossClone);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gully"), CGully);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinaleBoss"), CFinaleBoss);

	// Finale
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Baum"), CBaum);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BaumPiece"), CBaumPiece);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Disaster_Master"), CDisaster_Master);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinaleRoad"), CFinaleRoad);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinaleRoadGrouper"), CFinaleRoadGrouper);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PopStar"), CPopStar);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PopStar_StarSmall"), CPopStar_StarSmall);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BreakableBlock"), CBreakableBlock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinalePartical_Maker"), CFinalePartical_Maker);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinalePartical"), CFinalePartical);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinaleBuildingPartical"), CFinaleBuildingPartical);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FinaleCut_ControlCenter"), CFinaleCut_ControlCenter);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Finale_SpecialDebris_A"), CFinale_SpecialDebris_A);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Finale_SpecialDebris_B"), CFinale_SpecialDebris_B);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Finale_SpecialDebris_C"), CFinale_SpecialDebris_C);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BuildingCluster_A"), CBuildingCluster_A);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BuildingCluster_B"), CBuildingCluster_B);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BuildingCluster_C"), CBuildingCluster_C);


	//Dee
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("DeePart"), CDee_Part);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FoodShopDee"), CFoodShopDee);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HungryDee"), CHungryDee);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("OriginalDee"), COriginalDee);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BattleDee"), CBattleDee);

	//Boss
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("DeeDeeDee"), CDeeDeeDee);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("DeeDeeDeeHammer"), CDeeDeeDeeHammer);


	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BackGround"), CBackGround);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Main"), CCamera_Main);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestMap"), CTestTerrain);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestModel"), CTestModel);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Moon"), CMoon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("WasteCan"), CWasteCan);

	// Item
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Food"), CFood);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Coin"), CCoin);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Ability"), CAbility);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Ladder"), CLadder);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("KickableRock"), CKickableRock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("StarBlock"), CStarBlock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("StarBlockPiece"), CStarBlockPiece);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TerrainFog"), CTerrainFog);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Radio"), CRadio);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BreakableRock"), CBreakableRock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BreakableRockPartical"), CBreakableRockParticle);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CarShopWall"), CCarShopWall);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CarShopWallFrame"), CCarShopWallFrame);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("ToppleableBridge"), CToppleableBridge);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BlendMapObject"), CBlendMapObject);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("AnimBridge"), CAnimBridge);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tunnel"), CTunnel);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TunnelRock"), CTunnelRock);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Fog_Instance"), CFog_Instance);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Box"), CBox);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Debris"), CDebris);

	#pragma region LEVEL_FINALBOSS :: LAB_DISCOVERA

	//BOSS
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("BossChimera"), CBossChimera);

	//GIMMICK
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gm_LabAntenna"), CGm_LabAntenna);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gm_LabBossRoomDoor"), CGm_LabBossRoomDoor);

	#pragma endregion

	#pragma region GIMMICK::LEVEL_PARK

	//기믹 오브젝트
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gm_ParkFhEntranceAlien"), CGm_ParkFhEntranceAlien);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gm_ParkSolarPanelCharge"), CGm_ParkSolarPanelCharge);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gm_ParkSolarPanelOnce"), CGm_ParkSolarPanelOnce);

	//기믹 활성화 시 이동하는 동적 필드
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Gm_DynamicField"), CGm_DynamicField);

	#pragma endregion

	// 미니게임 in 와들디마을
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("PartTimeFood"), CPartTimeFood);

	#pragma region LEVEL_SIMBA
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Turbine"), CTurbine);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SimbaRoomGlass"), CSimbaRoomGlass);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("RoomGlass"), CRoomGlass);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Throne"), CThrone);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Simba"), CSimba);
	#pragma endregion

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

	//UI 텍스처
	Add_StaticUITexture();

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

#pragma region MAP_PORTAL::YW

	hr = Add_Texture(eLevel, "Portal_Soft_Black", "Map/Portal_Soft/Portal_Soft_Black.png");
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

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);

	#pragma region TUNNEL_TEXTURE

	hr = Add_Texture(eLevel, "TunnelMask_BaseColor", "Map/Tunnel/TunnelMask_BaseColor.dds");
	hr = Add_Texture(eLevel, "TunnelMask_Normal", "Map/Tunnel/TunnelMask_Normal.dds");
	hr = Add_Texture(eLevel, "TunnelMask_MRA", "Map/Tunnel/TunnelMask_MRA.dds");
	CHECK_FAILED(hr);

	#pragma endregion

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

HRESULT CLoader::Loading_For_DeeDeeDee()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_DEEDEEDEE;

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

	//HUD_BOSSHPBAR
	hr = Add_Texture(eLevel, "HUD_BossBar", "UI/HUD/Boss/BossBar_%d.png", 5);

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
	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "FoodShape", "UI/MGameFood/FoodShape.png");
	CHECK_FAILED(hr);
#pragma region UI

	// 와들디 주문 말풍선
	hr = Add_Texture(eLevel, "OrderCloud",			"UI/MGameFood/OrderCloud.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "ThinkingCloud",		"UI/MGameFood/ThinkingCloud.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "tomato",				"UI/MGameFood/tomato.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "burger",				"UI/MGameFood/burger.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "cake",				"UI/MGameFood/cake.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "energydrink",			"UI/MGameFood/energydrink.png");
	CHECK_FAILED(hr);

	// 타임 바
	hr = Add_Texture(eLevel, "Clock_orig",			"UI/MGameFood/Clock_orig.png");
	CHECK_FAILED(hr);

	// 게임 마무리 UI
	hr = Add_Texture(eLevel, "score_bar",			"UI/MGameFood/score bar.png");
	CHECK_FAILED(hr);	
	hr = Add_Texture(eLevel, "NewScoreBanner_bw",	"UI/MGameFood/new score banner_bw.png");
	CHECK_FAILED(hr);

	// 게임 DIGITS
	hr = Add_Texture(eLevel, "TimeDigits", "UI/Parttime/TimeDigits/%d.png", 10);
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "ScoreDigits", "UI/Parttime/ScoreDigits/%d.png", 10);
	CHECK_FAILED(hr);

#pragma endregion

	// 얼굴, 눈 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);

#pragma endregion

#pragma region UI

	// 타임 바
	hr = Add_Texture(eLevel, "GameFoodUI_BaseBar",			"UI/MGameFood/base bar.png");
	hr = Add_Texture(eLevel, "GameFoodUI_TimeBarBW",		"UI/MGameFood/time bar_bw.png");
	hr = Add_Texture(eLevel, "GameFoodUI_DeeBGBW",			"UI/MGameFood/dee bg_bw.png");
	hr = Add_Texture(eLevel, "GameFoodUI_Clock_Orig",		"UI/MGameFood/Clock_orig.png");

	// 왼쪽 아래 점수판
	hr = Add_Texture(eLevel, "GameFoodUI_ScoreBar",			"UI/MGameFood/score bar.png");
	
	// 와들디 안내 표정 
	hr = Add_Texture(eLevel, "GameFoodUI_AngryDee",			"UI/MGameFood/angrydee.png");
	hr = Add_Texture(eLevel, "GameFoodUI_IdleDee",			"UI/MGameFood/idledee.png");
	hr = Add_Texture(eLevel, "GameFoodUI_SadDee",			"UI/MGameFood/saddee.png");

	// 와들디 요구사항
	hr = Add_Texture(eLevel, "GameFoodUI_OrderCloud",		"UI/MGameFood/OrderCloud.png");
	hr = Add_Texture(eLevel, "GameFoodUI_ThinkingCloud",	"UI/MGameFood/ThinkingCloud.png");
	hr = Add_Texture(eLevel, "GameFoodUI_cake",				"UI/MGameFood/cake.png");
	hr = Add_Texture(eLevel, "GameFoodUI_tomato",			"UI/MGameFood/tomato.png");
	hr = Add_Texture(eLevel, "GameFoodUI_energydrink",		"UI/MGameFood/energydrink.png");
	hr = Add_Texture(eLevel, "GameFoodUI_burger",			"UI/MGameFood/burger.png");

	// 시작 안내
	hr = Add_Texture(eLevel, "Parttime_Ready",				"UI/Parttime/Text/Ready.png");
	hr = Add_Texture(eLevel, "Parttime_Go",					"UI/Parttime/Text/Go.png");

	// 결과 안내
	hr = Add_Texture(eLevel, "Parttime_Finish",				"UI/Parttime/Text/Finish.png");

	// 결과창
	hr = Add_Texture(eLevel, "GameFoodUI_ResultBar",		"UI/MGameFood/result bar.png");
	
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

HRESULT CLoader::Loading_For_Park()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_PARK;

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

	//HUD_BOSSHPBAR
	hr = Add_Texture(eLevel, "HUD_BossBar", "UI/HUD/Boss/BossBar_%d.png", 5);

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);

	//Gm_ParkSolarPanelCharge
	hr = Add_Texture(eLevel, "SolarPanelCharge_Lamp", "Gimmick/ParkSolarPanelCharge/AOsubC._%d.dds", 3);

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

HRESULT CLoader::Loading_For_Simba()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_SIMBA;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Moon", "Moon.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Level_Simba_Env", "Map/Level_Simba_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;

	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

	//HUD_BOSSHPBAR
	hr = Add_Texture(eLevel, "HUD_BossBar", "UI/HUD/Boss/BossBar_%d.png", 5);

	if (FAILED(Add_Texture(eLevel, "SimbaEye_Diffuse", "SimbaEye/Eye_BaseColor%d.dds", 3)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "SimbaEye_Normal", "SimbaEye/Eye_Normal%d.dds", 3)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "SimbaEye_MRA", "SimbaEye/BaseMRA.dds")))
		return E_FAIL;

#pragma region SKYSPHERE::LEVEL_SIMBA
	////FIELD
	//hr = Add_Texture(eLevel, "SkySphere_Lab_CloudNoize", "SkySphere/SkySphere_Lab_CloudNoizeC_MRA.dds"); CHECK_FAILED(hr);
	//hr = Add_Texture(eLevel, "SkySphere_Lab_Diffuse", "SkySphere/SkySphere_Lab_Diffuse_%d.dds", 3);	CHECK_FAILED(hr);
	//hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Normal", "SkySphere/SkySphere_LabBoss_2Pase_Normal.dds");	CHECK_FAILED(hr);
	//hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Emissive", "SkySphere/SkySphere_LabBoss_2Pase_Emissive.dds");	CHECK_FAILED(hr);
	//hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Height", "SkySphere/SkySphere_LabBoss_2Pase_Height.dds");	CHECK_FAILED(hr);
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

	//HUD_BOSSHPBAR
	hr = Add_Texture(eLevel, "HUD_BossBar", "UI/HUD/Boss/BossBar_%d.png", 5);

#pragma region SKYSPHERE::LEVEL_FINALBOSS

	//FIELD
	hr = Add_Texture(eLevel, "SkySphere_Lab_CloudNoize", "SkySphere/SkySphere_Lab_CloudNoizeC_MRA.dds"); CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_Lab_Diffuse", "SkySphere/SkySphere_Lab_Diffuse_%d.dds", 3);	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Normal", "SkySphere/SkySphere_LabBoss_2Pase_Normal.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Emissive", "SkySphere/SkySphere_LabBoss_2Pase_Emissive.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Height", "SkySphere/SkySphere_LabBoss_2Pase_Height.dds");	CHECK_FAILED(hr);

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

HRESULT CLoader::Loading_For_Finale()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_FINALE;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

#pragma region 텍스쳐
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Moon", "Moon.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Level_FInale_Env", "Map/Level_FInale_Env.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "BRDF_LUT", "Map/BRDF_LUT.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "RandomNormal", "Map/RandomNormal.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain_Fog", "Map/Fog/Sand_%d.png", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "FinalePartical", "FinalePartical/Finale_%d.dds", 9)))
		return E_FAIL;


	hr = Add_Texture(eLevel, "FX_Mask_Bubble2", "Effects/Mask/noise_bubble_%d.png", 4);	CHECK_FAILED(hr);

	//HUD_BOSSHPBAR
	hr = Add_Texture(eLevel, "HUD_BossBar", "UI/HUD/Boss/BossBar_%d.png", 5);

#pragma region SKYSPHERE::LEVEL_FINALE

	//FIELD
	hr = Add_Texture(eLevel, "SkySphere_Lab_CloudNoize", "SkySphere/SkySphere_Lab_CloudNoizeC_MRA.dds"); CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_Lab_Diffuse", "SkySphere/SkySphere_Lab_Diffuse_%d.dds", 3);	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Normal", "SkySphere/SkySphere_LabBoss_2Pase_Normal.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Emissive", "SkySphere/SkySphere_LabBoss_2Pase_Emissive.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SkySphere_LabBoss_2Pase_Height", "SkySphere/SkySphere_LabBoss_2Pase_Height.dds");	CHECK_FAILED(hr);
	//hr = Add_Texture(eLevel, "SkySphere_Space", "SkySphere/SkySphere_Space2.dds");	CHECK_FAILED(hr);

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
	HRESULT  hr = S_OK;
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
	#pragma region 와들디 주문 말풍선
	hr = Add_Texture(eLevel, "OrderCloud", "UI/MGameFood/OrderCloud.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "ThinkingCloud", "UI/MGameFood/ThinkingCloud.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "tomato", "UI/MGameFood/tomato.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "burger", "UI/MGameFood/burger.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "cake", "UI/MGameFood/cake.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "energydrink", "UI/MGameFood/energydrink.png");
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SimbaEye_Diffuse", "SimbaEye/Eye_BaseColor%d.dds", 3);
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SimbaEye_Normal", "SimbaEye/Eye_Normal%d.dds", 3);
	CHECK_FAILED(hr);
	hr = Add_Texture(eLevel, "SimbaEye_MRA", "SimbaEye/BaseMRA.dds");
	CHECK_FAILED(hr);
	#pragma endregion
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
	if (FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM, L"ParkDeco/")))
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
	hr = Add_Texture(eLevel, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 3);
	//hr = Add_Texture(eLevel, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");
	//hr = Add_Texture(eLevel, "HUD_BtnIcon", "UI/HUD/Kirby/BtnIcon/BtnIcon_%d.dds", 4);
	
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("Loading For Texture : Complete!");

#pragma endregion

	m_strLoadingText = TEXT("Loading For UITool : Complete!");

	m_IsFinished = TRUE;
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
	hr = Add_Texture(LEVEL_STATIC, "FX_Star", "Effects/Basic/common_star.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Bubble", "Effects/Basic/common_bubble.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Circles", "Effects/Basic/common_circle_%d.png", 4);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Grad", "Effects/Basic/common_gradation.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Hit", "Effects/Basic/common_ring_0.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Smoke", "Effects/Basic/common_smoke.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Line", "Effects/Basic/common_line_0.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Jump", "Effects/Basic/common_jump.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Sparkle", "Effects/Basic/common_sparkle.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Leaf", "Effects/Basic/leaf.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Collide", "Effects/Basic/common_hit.png");	CHECK_FAILED(hr);


	hr = Add_Texture(LEVEL_STATIC, "FX_VacuumTornado", "Effects/Basic/wind01.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_VacuumWind", "Effects/Basic/scroll07.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_VacuumDGB", "Effects/Basic/twinkle02.png");	CHECK_FAILED(hr);

	hr = Add_Texture(LEVEL_STATIC, "FX_Wind", "Effects/Basic/wind_%d.png", 2);	CHECK_FAILED(hr);

	hr = Add_Texture(LEVEL_STATIC, "FX_Scroll", "Effects/Basic/scroll_%d.png", 3);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Shockwave", "Effects/Basic/shockwave_%d.png", 5);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Swing", "Effects/Basic/swing_%d.png", 1);	CHECK_FAILED(hr);

	//칼
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordSlash", "Effects/Sword/slash_%d.png", 6);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordTail", "Effects/Sword/sword_tail.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordDecal", "Effects/Sword/sword_decal.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordLine", "Effects/Sword/line.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordShockwave", "Effects/Sword/shockwave.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_SwordRingNoFrip", "Effects/Sword/ringnofrip.png");	CHECK_FAILED(hr);

	//잎
	hr = Add_Texture(LEVEL_STATIC, "FX_BushCut", "Effects/Basic/BushColor_%d.png", 4);	CHECK_FAILED(hr);

	//UI용
	hr = Add_Texture(LEVEL_STATIC, "FX_Food_LunchTimeIcon", "Effects/UI/MGameFood/lunch time icon.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Food_LunchTimeBanner", "Effects/UI/MGameFood/lunch time banner.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Food_LunchTimeTypo", "UI/MGameFood/LunchType_%d.png", 6);	CHECK_FAILED(hr);

	hr = Add_Texture(LEVEL_STATIC, "FX_Food_ResultTop", "Effects/UI/MGameFood/result top.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_Food_ResultBottom", "Effects/UI/MGameFood/result bottom.png");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_TitleLogo", "Effects/UI/TitleLogo.png");	CHECK_FAILED(hr);

	//파티클
	hr = Add_Texture(LEVEL_STATIC, "FX_Rock", "Effects/Particle/rock_%d.png", 4);	CHECK_FAILED(hr);

	//팝스타
	hr = Add_Texture(LEVEL_STATIC, "FX_PopstarFallWind", "Effects/Popstar/PopStarFallEffectWind.dds");	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "FX_PopstarSkyCloud", "Effects/Popstar/PopStarSkyCloud.dds");	CHECK_FAILED(hr);


	return S_OK;
}

HRESULT CLoader::Add_StaticUITexture()
{
	HRESULT hr;

	//KirbyHP
	hr = Add_Texture(LEVEL_STATIC, "HUD_StatusBar_Kirby", "UI/HUD/Kirby/StatusBar/StatusBar_Hard_%d.dds", 23);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "HUD_StatusBar_Kirby_Mask", "UI/HUD/Kirby/StatusBar/KirbyHPMask.png");	CHECK_FAILED(hr);

	//StarPoint
	hr = Add_Texture(LEVEL_STATIC, "HUD_StarPoint", "UI/HUD/Kirby/StarPoint/StarPoint_%d.dds", 10);	CHECK_FAILED(hr);

	//Ability Discard
	hr = Add_Texture(LEVEL_STATIC, "HUD_AbilityDiscard", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_%d.dds", 3);	CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "HUD_AbilityDiscard_Mask", "UI/HUD/Kirby/AbilityDiscard/AbilityDiscard_Mask.dds");	CHECK_FAILED(hr);


	//UI_MessageWindow
	hr = Add_Texture(LEVEL_STATIC, "UI_MessageWindow_Base", "UI/MessageWindow/MessageWindow_Base_%d.dds", 3); CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "UI_MessageWindow_Base_Claw", "UI/MessageWindow/MessageWindow_Base_Claw.dds"); CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "UI_MessageWindow_BtnBase", "UI/MessageWindow/MessageWindow_BtnBase_%d.dds", 3); CHECK_FAILED(hr);

	//UI_Button
	hr = Add_Texture(LEVEL_STATIC, "UI_BtnIconBase", "UI/BtnIcon/BtnIcon_Base_%d.dds", 3); CHECK_FAILED(hr);
	hr = Add_Texture(LEVEL_STATIC, "UI_BtnIconBright", "UI/BtnIcon/BtnIcon_Bright.dds"); CHECK_FAILED(hr);
	
	//UI_Fading
	hr = Add_Texture(LEVEL_STATIC, "Fade", "UI/Fade.png");
	CHECK_FAILED(hr);

	return S_OK;
}

// 여기다가 모든 Model을 셋업한다.
void CLoader::SetUp_ModelScaleRotation(LEVEL eLevel)
{
	// MODEL 구조체 생성자 순서		: 이름 (파일이름) / ANIMTYPE / Scale / Degree (Y) / Root
	// MODEL 구조체 생성자 기본 값  : ""			  / TYPE_END /  1.f  /    0.f     / 4
	if (eLevel == LEVEL_STATIC)
	{
		//DEFAULT_SKYSPHERE
		m_vecModelInfo.emplace_back("SkySphere_Stage1_Day", TYPE_NONANIM );

		//이펙트 친구들...
		m_vecModelInfo.emplace_back("SmokeCenter", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeFadeLarge", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeOriginal", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeSplit", TYPE_NONANIM );
		m_vecModelInfo.emplace_back("SmokeTail", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Tornado", TYPE_NONANIM );

		//부쉬 쪼가리
		m_vecModelInfo.emplace_back("BushCutL", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("BushCutM", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("BushCutS", TYPE_NONANIM);

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

		//토루스
		m_vecModelInfo.emplace_back("Torus", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("TorusHalf", TYPE_NONANIM);

		//팝스타
		m_vecModelInfo.emplace_back("PopStar_Debris", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("PopStar_StarRiver", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("PopStar_StarSmall", TYPE_NONANIM);

		//워프 스타
		//m_vecModelInfo.emplace_back("WarpStar_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		//m_vecModelInfo.emplace_back("WarpStar_Anim", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	}
	else if (eLevel == LEVEL_LOGO)
	{

	}
	else if (eLevel == LEVEL_INTRO)
	{
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level0Stage1Step01_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG0", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

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
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("CappyBody", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("CappyHat", TYPE_ANIM, 1.f, 180.f);

		//와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

		// Boss
		m_vecModelInfo.emplace_back("DeeDeeDee", TYPE_ANIM, 3.0f, 180.f);
		m_vecModelInfo.emplace_back("DeeDeeDeeHammer", TYPE_NONANIM, 1.0f);

		// For Map Interactive Object
		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoxWood", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoxWood_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));

		// For Item
		Load_ItemModels();

		// For Kickables
		Load_KickableModels();

		// For StarBlocks
		Load_StarBlockModels();

		// For Interaction Decor
		m_vecModelInfo.emplace_back("BushLRemainder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("BushMRemainder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("BushSRemainder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
	}
	else if (eLevel == LEVEL_TOWN)
	{
		m_vecModelInfo.emplace_back("Town", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));


		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		//와들디 파트오브젝트
		m_vecModelInfo.emplace_back("DeePart_Arena", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Bass", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Delivery", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Drum", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_DrumStick", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_FoodShop", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Guitar", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Keyboard", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Knowledge", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Mike", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Pharmacy", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Plate", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_RollingBall", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Theater", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_WateringCan", TYPE_NONANIM, 1.f);

		// 와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);
		m_vecModelInfo.emplace_back("WaddleDeeHungry", TYPE_ANIM, 1.1f, 180.f);

		// Boss
		m_vecModelInfo.emplace_back("DeeDeeDee", TYPE_ANIM, 3.0f, 180.f);
		m_vecModelInfo.emplace_back("DeeDeeDeeHammer", TYPE_NONANIM, 1.3f);

		//기믹 오브젝트
		m_vecModelInfo.emplace_back("FhEntranceAlien_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("FhEntranceAlien_NonAnim", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);

		// For Item
		Load_ItemModels();


	}
	else if (eLevel == LEVEL_RACING)
	{
		m_vecModelInfo.emplace_back("Level0Stage1Step02", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level0Stage1Step02_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG0", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Town", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));



		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.2f, 180.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 180.f);
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);
		m_vecModelInfo.emplace_back("CappyBody", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("CappyHat", TYPE_ANIM, 1.f, 180.f);

		// 자동차 관련 (자동차와 부수는 돌멩이들)
		m_vecModelInfo.emplace_back("Car", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("RockA", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("RockB", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("RockPartical", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("CarShopBreakableWall", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("CarShopWallFrame", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("CarShopFrameBefore", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("CarShopWallParticle", TYPE_NONANIM);

		m_vecModelInfo.emplace_back("Radio", TYPE_ANIM, 1.f, 180.f);


		// For Item
		Load_ItemModels();

		// For Kickables
		Load_KickableModels();

		// For StarBlocks
		Load_StarBlockModels();

		// For Interaction Decor
		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BushLRemainder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("BushMRemainder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("BushSRemainder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapDeco/"));
		m_vecModelInfo.emplace_back("BoardA", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoardB", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoardC", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoardA_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoardB_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoardC_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("TunnelRocks", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		for (_uint i = 0; i <= 16; i++) {
			string strTunnelRock = "TunnelRock" + to_string(i);
			m_vecModelInfo.emplace_back(strTunnelRock, TYPE_NONANIM);
		}
		m_vecModelInfo.emplace_back("BoxPlastic", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BoxPlastic_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
	}
	else if (eLevel == LEVEL_DEEDEEDEE)
	{
		m_vecModelInfo.emplace_back("DeeDeeDeeMap", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("DeeDeeDee", TYPE_ANIM, 3.0f, 180.f);
		m_vecModelInfo.emplace_back("DeeDeeDeeHammer", TYPE_NONANIM, 1.3f);

		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("NonRenderWall", TYPE_NONANIM, 0.01f, 1.f, 0, string("MapObjs/"));


		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		//와들디 파트오브젝트
		m_vecModelInfo.emplace_back("DeePart_Arena", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Bass", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Delivery", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Drum", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_DrumStick", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_FoodShop", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Guitar", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Keyboard", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Knowledge", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Mike", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Pharmacy", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Plate", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_RollingBall", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_Theater", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DeePart_WateringCan", TYPE_NONANIM, 1.f);

		// 와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);
		m_vecModelInfo.emplace_back("WaddleDeeHungry", TYPE_ANIM, 1.1f, 180.f);

		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 180.f);


		// For Item
		Load_ItemModels();


		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

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
		m_vecModelInfo.emplace_back("WaddleDeeHungry", TYPE_ANIM, 1.1f, 180.f);
		
		// 커비 모자 뺏어 씁니다
		m_vecModelInfo.emplace_back("DeePart_FoodShop", TYPE_NONANIM, 1.f);

		// 음식 나갑니다~
		m_vecModelInfo.emplace_back("Item_EnergyDrink", TYPE_NONANIM, 1.6f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("FoodBurger", TYPE_NONANIM, 0.6f);
		m_vecModelInfo.emplace_back("FoodCake", TYPE_NONANIM, 0.6f);
		m_vecModelInfo.emplace_back("FoodTomato", TYPE_NONANIM, 1.6f);
	}
	else if (eLevel == LEVEL_PARK)
	{
		// For Item
		Load_ItemModels();

		// For Kickables
		Load_KickableModels();

		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		
		// For Map
		m_vecModelInfo.emplace_back("PkFunHouse", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		//전구하우스 BulbZone
		m_vecModelInfo.emplace_back("Land_PkFunHouseDarkness", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouseDarkness01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouseDarkness02", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouseDarkness03", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouseDarkness04", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouseDarkness05", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouse06", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Gimmick_PkFunHouse07", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		//기믹 오브젝트
		m_vecModelInfo.emplace_back("FhEntranceAlien_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("FhEntranceAlien_NonAnim", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("SolarPanelCharge_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("SolarPanelCharge_NonAnim", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("SolarPanelOnce_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("SolarPanelOnce_NonAnim", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));


		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		// Deform
		m_vecModelInfo.emplace_back("DumpCar", TYPE_ANIM, 0.8f, 90.f);

		// Monster
		m_vecModelInfo.emplace_back("Phanta", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("SpookStep", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("GhostGordo", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Bomber", TYPE_ANIM, 1.f, 180.f);

		m_vecModelInfo.emplace_back("SurprisedBoardBlue", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("NonAnim_SurprisedBoardBlue",	TYPE_NONANIM, 1.f, 0.f, 0, string("Monsters/"));
		m_vecModelInfo.emplace_back("SurprisedBoardGreen", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("NonAnim_SurprisedBoardGreen",	TYPE_NONANIM, 1.f, 0.f, 0, string("Monsters/"));
		m_vecModelInfo.emplace_back("SurprisedBoardRed", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("NonAnim_SurprisedBoardRed",	TYPE_NONANIM, 1.f, 0.f, 0, string("Monsters/"));

		// Gimmick
		m_vecModelInfo.emplace_back("Crumble", TYPE_ANIM);
		m_vecModelInfo.emplace_back("NonAnim_Crumble", TYPE_NONANIM, 1.f, 0.f, 0, string("Monsters/"));
	}
	else if (eLevel == LEVEL_SIMBA)
	{
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("NonRenderWall", TYPE_NONANIM, 0.01f, 1.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// Level_Simba 맵
		m_vecModelInfo.emplace_back("LbBossLoom01L", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("LbBossLoom01L_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		
		// 맵 오브젝트
		m_vecModelInfo.emplace_back("LbBossTurbine01L_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("LbBossRing01L_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("JhGlass", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("RoomGlass_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Throne_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("OriginCage_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));


		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		// For Boss 
		m_vecModelInfo.emplace_back("Simba", TYPE_ANIM, 1.f, 180.f);

		// For Item
		Load_ItemModels();
	}
	else if (eLevel == LEVEL_FINALBOSS)
	{
		for (_uint i = 0; i <= 17; i++) {
			string strTunnelRock = "TunnelRock" + to_string(i);
			m_vecModelInfo.emplace_back(strTunnelRock, TYPE_NONANIM, 0.1f);
		}

		//보스전 필드에서만 생성하는 SUB_SKYSPHERE (BackGround 요소)
		m_vecModelInfo.emplace_back("LbBuildingFrame", TYPE_NONANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("LbFarPiller", TYPE_NONANIM, 1.f, 76.117f);

		//보스전 진입 전 필드
		m_vecModelInfo.emplace_back("Land_LbLastBossBeforeStep", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("Land_LbLastBossBeforeStep_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		
		//기믹 오브젝트
		m_vecModelInfo.emplace_back("LbAntenna_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("LbBossRoomDoor_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		Load_ItemModels();
		Load_KickableModels();

		// 액체괴물 :: Fecto_Forgo
		//m_vecModelInfo.emplace_back("", TYPE_ANIM, 1.f, 180.f);

		// For Boss 
		m_vecModelInfo.emplace_back("FinalBoss", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("FinalBossSpear", TYPE_ANIM, 1.f);
		m_vecModelInfo.emplace_back("RayArrow", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DimensionLaser", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("DimensionGateL", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BigDebris", TYPE_NONANIM, 0.8f, 180.f);
		m_vecModelInfo.emplace_back("SmallDebris", TYPE_NONANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("MoundPiece", TYPE_NONANIM, 0.2f, 180.f);
	}
	else if (eLevel == LEVEL_FINALE)
	{
		//보스전 필드에서만 생성하는 SUB_SKYSPHERE (BackGround 요소)
		m_vecModelInfo.emplace_back("LbBuildingFrame", TYPE_NONANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("LbFarPiller", TYPE_NONANIM, 1.f, 76.117f);
		
		//보스전 필드
		m_vecModelInfo.emplace_back("Land_LbLastBossStage", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("FinaleCave", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));


		//피날레 오브젝트들
		m_vecModelInfo.emplace_back("PopStar", TYPE_ANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("PopStar_StarSmall", TYPE_NONANIM, 1.f, 0.f, 0);

		//보스전 진입 전 필드
		//m_vecModelInfo.emplace_back("Land_LbLastBossBeforeStep", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		//m_vecModelInfo.emplace_back("Land_LbLastBossBeforeStep_Blend", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		//기믹 오브젝트
		//m_vecModelInfo.emplace_back("LbAntenna_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));
		//m_vecModelInfo.emplace_back("LbBossRoomDoor_Anim", TYPE_ANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("Baum", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BaumPieceA", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BaumPieceB", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BaumPieceC", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BaumPieceD", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BaumPieceE", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BaumPieceF", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BuildingPiece", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("StarPiece", TYPE_NONANIM, 1.f, 0.f, 0);

		m_vecModelInfo.emplace_back("MovableBuildingA", TYPE_NONANIM, 0.8f, 0.f, 0);
		m_vecModelInfo.emplace_back("MovableBuildingB", TYPE_NONANIM, 0.8f, 0.f, 0);
		m_vecModelInfo.emplace_back("MovableBuildingC", TYPE_NONANIM, 0.8f, 0.f, 0);
		m_vecModelInfo.emplace_back("MovableBuildingD", TYPE_NONANIM, 0.8f, 0.f, 0);

		m_vecModelInfo.emplace_back("BigMovableBuildingA", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BigMovableBuildingB", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BigMovableBuildingC", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("BigMovableBuildingD", TYPE_NONANIM, 1.f, 0.f, 0);

		m_vecModelInfo.emplace_back("CutDebrisA", TYPE_ANIM, 0.8f, 0.f, 0);
		m_vecModelInfo.emplace_back("CutDebrisB", TYPE_ANIM, 0.8f, 0.f, 0);
		m_vecModelInfo.emplace_back("CutDebrisC", TYPE_ANIM, 0.8f, 0.f, 0);


		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

		m_vecModelInfo.emplace_back("LbBrokenBuildingAC", TYPE_NONANIM, 1.f, 0.f, 0, string("LabDiscovera_Deco/"));
		m_vecModelInfo.emplace_back("LbBrokenBuildingBL", TYPE_NONANIM, 1.f, 0.f, 0, string("LabDiscovera_Deco/"));
		m_vecModelInfo.emplace_back("LbBrokenBuildingCL", TYPE_NONANIM, 1.f, 0.f, 0, string("LabDiscovera_Deco/"));
		m_vecModelInfo.emplace_back("LbBrokenBuildingDL", TYPE_NONANIM, 1.f, 0.f, 0, string("LabDiscovera_Deco/"));

		m_vecModelInfo.emplace_back("BuildingCluster01", TYPE_ANIM, 0.8f);
		m_vecModelInfo.emplace_back("BuildingCluster02", TYPE_ANIM, 0.8f);
		m_vecModelInfo.emplace_back("BuildingSub", TYPE_ANIM, 0.8f);

		 
		m_vecModelInfo.emplace_back("Road", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("RoadEnd", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("RoadParticle", TYPE_NONANIM, 0.2f, 0.f, 0);

		m_vecModelInfo.emplace_back("BreakableBlock", TYPE_ANIM, 0.2f, 0.f, 0);

		m_vecModelInfo.emplace_back("RoadBreak", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("RoadLBreak", TYPE_NONANIM, 1.f, 0.f, 0);
		m_vecModelInfo.emplace_back("RoadLongBreak", TYPE_NONANIM, 1.f, 0.f, 0);


		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("BG1", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		//m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		// Deform
		m_vecModelInfo.emplace_back("DumpCar", TYPE_ANIM, 0.8f, 90.f);

		// For Boss 
		m_vecModelInfo.emplace_back("FinaleBoss", TYPE_ANIM, 1.f);

		m_vecModelInfo.emplace_back("FinalBoss", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("FinalBossSpear", TYPE_ANIM, 1.f);
		m_vecModelInfo.emplace_back("RayArrow", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("DimensionLaser", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("DimensionGateL", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BigDebris", TYPE_NONANIM, 0.8f, 180.f);
		m_vecModelInfo.emplace_back("SmallDebris", TYPE_NONANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("MoundPiece", TYPE_NONANIM, 0.2f, 180.f);
	}
	else if (eLevel == LEVEL_GAMEPLAY)
	{
		m_vecModelInfo.emplace_back("Fiona", TYPE_ANIM );
		m_vecModelInfo.emplace_back("Dee", TYPE_ANIM, 0.01f);
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		// 자동차 관련 (자동차와 부수는 돌멩이들) :: 디버깅용으로 추가
		m_vecModelInfo.emplace_back("Car", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("RockA", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("RockB", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("RockPartical", TYPE_NONANIM, 1.f);

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

		// For Boss 옮겨야함
		m_vecModelInfo.emplace_back("FinalBoss", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("FinalBossSpear", TYPE_NONANIM, 1.f);

		// For Mab Interactive Object
		m_vecModelInfo.emplace_back("Ladder", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		//GAMEPLAY 크래시로 인한 임시 처리 MapDeco > MapObj
		m_vecModelInfo.emplace_back("StarBlockL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("StarBlockM", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
		m_vecModelInfo.emplace_back("StarBlockS", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

		m_vecModelInfo.emplace_back("StarBlockPiece", TYPE_NONANIM, 0.5f, 180.f);
		m_vecModelInfo.emplace_back("StarBlockPieceStar", TYPE_NONANIM, 0.5f, 180.f);

		// For Item
		Load_ItemModels();
		Load_KickableModels();

		// 와들디
		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);
	}
	else if (eLevel == LEVEL_TOOL_MAP)
	{
		// 맵툴에서는 크기나 회전 상태 바꾸고 싶은 모델만 여기에 등록. 안바꾸고싶으면 NonAnim, 크기1, 회전 0도로 자동 추가됨
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f/*, 1.f, 0, string("MapObjs/")*/);
		m_vecModelInfo.emplace_back("Camera", TYPE_NONANIM, 0.2f, 270.f);
		m_vecModelInfo.emplace_back("Dummy", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("RallyPoint", TYPE_NONANIM, 2.f);
		m_vecModelInfo.emplace_back("LightBulb", TYPE_NONANIM, 0.02f);
		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Level1Stage1Step01", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"), true);
		m_vecModelInfo.emplace_back("Fog", TYPE_NONANIM, 0.002f);
		m_vecModelInfo.emplace_back("zBat", TYPE_NONANIM, 0.002f, 0.f, 0, string("TownDeco/"));
		m_vecModelInfo.emplace_back("NonRenderWall", TYPE_NONANIM, 0.01f, 1.f, 0, string("MapObjs/"));
	}
	else if (eLevel == LEVEL_TOOL_ANIM)
	{
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM, 1.f, 180.f);

		// For Kirby Body
		Load_KirbyBodyModels();
		m_vecModelInfo.emplace_back("KirbyPartTimer", TYPE_ANIM, 1.f, 0.f);
		
		// For Kirby Weapon
		Load_KirbyWeaponModels();
		// For Kirby Armour
		Load_KirbyArmourModels();

		// For Monster
		m_vecModelInfo.emplace_back("Awoofy", TYPE_ANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("Rabbit", TYPE_ANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("Buffahorn", TYPE_ANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("BladeKnight", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("BladeKnightSword", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Kabu", TYPE_ANIM, 2.f, 0.f);
		m_vecModelInfo.emplace_back("BrontoBurt", TYPE_ANIM, 2.f, 0.f);
		m_vecModelInfo.emplace_back("PoppyBrosJr", TYPE_ANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("PoppyBomb", TYPE_ANIM, 1.3f, 0.f);
		m_vecModelInfo.emplace_back("FinalBoss", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("FinalBossSpear", TYPE_NONANIM, 1.f);
		m_vecModelInfo.emplace_back("Phanta", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("SpookStep", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("GhostGordo", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("Bomber", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("FinaleBoss", TYPE_ANIM, 1.f);
		
		// Boss
		m_vecModelInfo.emplace_back("DeeDeeDee", TYPE_ANIM, 3.0f, 180.f);
		m_vecModelInfo.emplace_back("DeeDeeDeeHammer", TYPE_NONANIM, 1.0f);

		m_vecModelInfo.emplace_back("WaddleDeeBase", TYPE_ANIM, 1.1f, 180.f);

		// Monster
		m_vecModelInfo.emplace_back("SurprisedBoardBlue", TYPE_ANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("SurprisedBoardGreen", TYPE_ANIM, 1.f, 0.f);
		m_vecModelInfo.emplace_back("SurprisedBoardRed", TYPE_ANIM, 1.f, 0.f);

		m_vecModelInfo.emplace_back("BreakableBlock", TYPE_ANIM, 0.5f, 0.f, 0);
		m_vecModelInfo.emplace_back("Crumble", TYPE_ANIM);
		m_vecModelInfo.emplace_back("Simba", TYPE_ANIM);
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

	if (FAILED(Add_Texture(eLevel, "BulbFlare", "KirbyBulbFlare/BulbFlare.dds")))
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
						animInfo.fAnimSpeed = 60.f;// animSpeed;
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

void CLoader::Load_ItemModels()
{
	// For ItemModels
	m_vecModelInfo.emplace_back("Item_EnergyDrink", TYPE_NONANIM, 3.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Bread", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Cake", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Cocktail", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Makaron", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Meat", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Omelet", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Onigiri", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Steak", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_Sushi", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

	// For Coins
	m_vecModelInfo.emplace_back("Item_Coin", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_BlueCoin", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("Item_RedCoin", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));

	// For AbilityItemModels
	m_vecModelInfo.emplace_back("Item_Sword", TYPE_NONANIM, 1.f, 0.f);
	m_vecModelInfo.emplace_back("Item_Bomb", TYPE_NONANIM, 1.f, 0.f);
	m_vecModelInfo.emplace_back("Item_Hammer", TYPE_NONANIM, 1.f, 0.f);
	m_vecModelInfo.emplace_back("Item_Crash", TYPE_NONANIM, 1.f, 0.f);
	m_vecModelInfo.emplace_back("Item_Star", TYPE_NONANIM, 0.5f, 0.f);
}

void CLoader::Load_KickableModels()
{
	m_vecModelInfo.emplace_back("GsPebble", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("GsRubbleA", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("GsRubbleB", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("GsRubbleC", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("GsTireAL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("GsTireBL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("GsTireCL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("SeShell", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("WasteCanYellow", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
}

void CLoader::Load_StarBlockModels()
{
	m_vecModelInfo.emplace_back("StarBlockS", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("StarBlockM", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("StarBlockL", TYPE_NONANIM, 1.f, 0.f, 0, string("MapObjs/"));
	m_vecModelInfo.emplace_back("StarBlockPiece", TYPE_NONANIM, 0.5f, 180.f);
	m_vecModelInfo.emplace_back("StarBlockPieceStar", TYPE_NONANIM, 0.5f, 180.f);
}

void CLoader::Load_KirbyBodyModels()
{
	// For Kirby Body
	m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbySwordDefault", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbySwordBalloon", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyBoomDefault", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyCarDefault", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyCarVacuum", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyHammerDefault", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyDumpDefault", TYPE_ANIM, 0.8f, 180.f);
	m_vecModelInfo.emplace_back("KirbyBulbDefault", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyBulbVacuum", TYPE_ANIM, 1.f, 180.f);
	m_vecModelInfo.emplace_back("KirbyCrashDefault", TYPE_ANIM, 1.f, 180.f);

	m_vecModelInfo.emplace_back("KirbyDumpVacuum", TYPE_ANIM, 0.8f);
	m_vecModelInfo.emplace_back("KirbyDumpCut", TYPE_ANIM, 0.8f);

	// Deform
	m_vecModelInfo.emplace_back("Bulb", TYPE_ANIM, 1.f, 180.f);
}

void CLoader::Load_KirbyWeaponModels()
{
	// For Kirby Weapon
	m_vecModelInfo.emplace_back("KirbyWeapon_Sword", TYPE_NONANIM, 1.f);
	m_vecModelInfo.emplace_back("KirbyWeapon_Hammer", TYPE_ANIM, 1.f);
	m_vecModelInfo.emplace_back("KirbyBombDefault", TYPE_ANIM, 1.3f, 180.f);

}

void CLoader::Load_KirbyArmourModels()
{
	// For Kirby Armour
	m_vecModelInfo.emplace_back("KirbyArmour_Boom", TYPE_NONANIM, 1.f);
	m_vecModelInfo.emplace_back("KirbyArmour_Sword", TYPE_NONANIM, 1.f);
	m_vecModelInfo.emplace_back("KirbyArmour_Hammer", TYPE_NONANIM, 1.f);
	m_vecModelInfo.emplace_back("KirbyArmour_Crash", TYPE_NONANIM, 1.f);
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
