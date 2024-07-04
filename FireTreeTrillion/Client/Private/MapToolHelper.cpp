#include "stdafx.h"
#include "MapToolHelper.h"
#include "MapToolObject.h"
#include "BasicMap.h"
#include "Utils.h"

static _int s_iMapTxtIdx = -1;
static _int s_iTriggerTxtIdx = -1;
static _int s_iMonsterTxtIdx = -1;
static _int s_iObjectIdx = -1;
static _int s_iMapDecoIdx = -1;
static _int s_iItemIdx = -1;
static _int s_iKickableIdx = -1;

static _int s_iTownDecoIdx = -1;
static _int s_iLabDecoIdx = -1;

static _int s_iLevelIndex = 0;
static _int s_iTempLevelIdx = -1;

static const _char* s_triggerTypes[] = {"Camera", "Shader", "Star", "LevelChanger"};
static _int s_iTriggerType = -1;
static const _char* s_triggerIndices[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
									"11", "12", "13", "14", "15", "16", "17", "18", "19", "20" };
static _int s_iTriggerIdx = -1;

static _int s_iMapMeshIndex = -1;
static _int s_iSelectedMeshIndex = -1; 

static const _char* s_ShaderPasses[] = { "0. Blend X, NormalO", "1. Blend X, Normal X", "2. LightDepth", "3. Blend O, Normal O", "4. Blend O, Normal X"
		, "5. BLEND X, DISCARD X", "6. BLEND O, DISCARD X" };
static vector<vector<_int>> s_vecPassIndices;
static vector<vector<_float>> s_vecSamplingFactors;
static _int s_iMapIndex = 0;
static _int s_iPickedMeshIndex = 0;

static _float s_fRadius = 0;

static const _char* s_camTypes[] = { "Front", "Rear"};
static _int s_iCamType = -1;

static _bool s_bHideTriggers = { false };
static _bool s_bHideGrid = { true };
static _bool s_bHideDecos = { false };
static _bool s_bHideWalls = { false };

static _int s_iConnectedMonster = -1;
static const _char* s_ModelPassIndices[] = { "0. NORMAL_0", "1. NORMAL_X", "2. SHADOW", "3. SKY", "4. BLOOM", "5. NON_BLUR"
	,"6. TRIGGER", "7.DEFAULTFX", "8. BLENDFX", "9. DEFERREDINFO", "10. WHITEFX", "11. KIRBYPART", "12. NEARCLIP",
	/*"12. NORMAL_O AND NONCULL", */"13. BLEND O, NORMAL O", "14. MONSTERPARTOBJECT" };

static const _char* s_PosTexPasses[] = { "0. DEFAULT", "1. ALPHABLEND", "2. BLENDFX", "3. BLOOM", "4. DEFAULTFX", "5. BLEND_NOZTEXT"
	,"6. WHITEFX", "7. UI_MASK", "8. UI_MASK2", "9. SOFTFX", "10. SOFTALPHAFX"};
static _int s_iPassIndex = -1;
static _char s_ObjectsFilter[MAX_PATH] = "";
static _char s_MapDecoFilter[MAX_PATH] = "";
static _char s_TownDecoFilter[MAX_PATH] = "";
static _char s_LabDecoFilter[MAX_PATH] = "";
static _bool s_bWasObjectsOpen = false;
static _bool s_bWasMapDecosOpen = false;
static _bool s_bWasTownDecosOpen = false;
static _bool s_bWasLabDecosOpen = false;

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

CMapToolHelper::CMapToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMapToolHelper::CMapToolHelper(const CMapToolHelper& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CMapToolHelper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapToolHelper::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	m_vecTxtIndices = { &s_iMapTxtIdx, &s_iTriggerTxtIdx, &s_iMonsterTxtIdx, &s_iObjectIdx
		, &s_iMapDecoIdx, &s_iItemIdx, &s_iKickableIdx, &s_iTownDecoIdx, &s_iLabDecoIdx };

	m_vecLevelName = { "Level_Static", "Level_Loading", "Level_Logo", "GamePlay",
			"Level_Tool_UI", "Level_Tool_FX", "Level_Tool_Anim", "Level_Tool_Map",
		"Intro", "Racing", "DeeDeeDee", "Town", "PartTime", "Simba", "FinalBoss", "Finale", "Level_End" };

	//UV 조절 가능
	//BasicMap (UV 편집 가능)
	m_vecMapModelNames = { "Level0Stage1Step01", "Level0Stage1Step02",  "Level1Stage1Step01", "DeeDeeDeeMap", "Town", "TownShop"
		, "Land_VcLabo", "Land_LbLastBossBeforeStep", "LevelFinale_LbLastBuilding" };

	vector<string> vecBGs = { "BG0", "BG1" };
	m_setMapNames.insert(vecBGs.begin(), vecBGs.end());
	m_setMapNames.insert(m_vecMapModelNames.begin(), m_vecMapModelNames.end());

	m_setTriggerNames = { "NonAnim_Kirby", "Trigger", "Camera", "Dummy", "Fog", "Ladder", "NonAnim_KirbyPartTimer" };
	m_setRallyingMonsters = { "NonAnim_Kabu", "NonAnim_BrontoBurt", "NonAnim_FinalBoss" };

	/*m_setNonColDecos = { "BushMCut" };*/
	m_setAnimDecos = { "BushL", "BushM", "BushS", "PopFlower" };

	//아래는 피직스 처리가 필요한 오브젝트들. (지형 충돌 필요)
	m_setActorDecos = {  "CMBillBoardC", "CmBuilding1stRoof", "CMBuildingParts", "CMGuardrailAL", "CMGuardrailBL"
		, "CMFenceAL", "CMFenceA2L", "CMFenceA3L", "CMFenceB3L", "CMFenceCL", "CMFenceCornerCL"
		, "CMHighwayGuardrailACL", "CMHighwayGuardrailAL", "CMHighwayGuardrailALL", "CMHighwayGuardrailARL"
		, "CMHighwayGuardrailBL", "CMHighwayGuardrailBLL", "CMHighwayGuardrailBRL", "CMHighwayGuardrailCCL"
		, "CMHighwayGuardrailCL", "CMHighwayGuardrailCLL", "CMHighwayGuardrailCRL"
		, "CMStreeLightLampA", "CMStreeLightLampE", "CMWaterTankL", "CvBarricadeBL", "CvPipingDuctA05L"
		, "GsBenchAL", "GsCarFloor", "GsCarShop", "GsCircleBench", "GsClock", "GsFlowerPotAL", "GsFlowerPotBL", "GsSteelFenceA"
		, "GsSteelFenceB", "GsStone", "GsStreetWallA", "GsStreetWallB", "GsTelephonePoleA", "GsTelephonePoleB"
		, "GsTrafficSignalAL", "GsTrafficSignalBL", "GsTreeA", "GsTreeB", "GsTreeC", "GsWallRockA", "GsWallRockB"
		, "GsWoodBridgeA", "GsWoodBridgeB", "GsRockCL", "GsRockDL", "GsRockEL", "GsRockFL", "GsRockGL"
		, "JgGrassB", "JgGrassL", "JgGrasslongB", "JgGrassN", "JgWoodD", "JgGrassO"
		, "SeDriftWoodAL", "SeDriftWoodBL", "SeDriftWoodCL"
		, "VpFactoryPart", "VpFactoryParts", "VpFactoryPartsBlend", "WoodBox"


#pragma region DEEDEEDEE OBJ
		, "TwGougeGround01", "TwGougeGround02"
#pragma endregion


#pragma region LEVEL_RACING OBJECT
		, "CMBuildingFenceA01","CMBuildingFenceA02", "CmFillerObjectD", "CmFillerObjectG", "CmFillerObjectH"
		, "CmFillerObjectH02", "CmFillerObjectI"

		, "CvBarricadeA", "CvGasCylinderAL", "CvGasCylinderBL"
		, "CvGasCylinderCL", "CvGasCylinderDL", "CvGasCylinderEL", "CvGasCylinderHoseA", "CvGasCylinderHoseD"

		, "DsBuildingFenceAL", "DsBuildingFenceBL", "DsBuildingFenceCL", "DsBuildingFenceDL", "DsBuildingFenceEL"
		, "DsBuildingFenceFL", "DsBuildingFenceGL", "DsBuildingFenceHL", "DsBuildingFenceIL"
		, "DsPalmABottomL", "DsPalmAMiddleL", "DsPalmATopL", "DsPalmBL", "DsPalmCL"

		, "GsCarBaseAL", "GsCarCounterAL", "GsCarDirtyRack01L", "GsCarDisplayRack", "GsCarHoistCrane01L"
		, "GsCarMachineAL", "GsCarMeshRack", "GsCarSteelPartsAL", "GsCarStop"
		, "GsRubbleAsphalt01L", "GsRubbleAsphalt02L", "GsRubbleAsphalt03L", "GsRubbleAsphalt04L"
		, "GsRubbleAsphalt05L", "GsRubbleAsphalt06L", "GsRubbleAsphalt07L", "GsRubbleAsphalt08L"
		, "GsRubbleAsphalt09L", "GsScrappedCar"

		, "FarBuildingABL", "FarBuildingAM1L", "FarBuildingAM6L", "FarBuildingC01", "FarBuildingC02"

		, "JgGuardrailAL", "JgGuardrailBL"

		, "MlFlowerPot01L"
#pragma endregion


#pragma region TOWN OBJECT
		, "TwArena", "TwArenaA", "TwArenaB", "TwArenaClerk", "TwDeliveryService", "TwFoodStore", "TwFoodStoreChair", "TwFoodStoreTable"
		, "TwKirbyHouse", "TwPharmacy", "TwPlanterA", "TwPlanterB", "TwRollingBallBooth", "TwSideHouseA", "TwSideHouseC", "TwSideHouseD"
		, "TwSignboardStreetLive", "TwSlope", "TwBenchA", "TwBenchB", "TwBenchC", "TwBenchD", "TwFishingPartsA", "TwFishingPartsB", "TwFishingPartsC"
		,"TwCentralSquare","TwStreetLiveSetC", "TwSideBridge", "TwConstructionAreaStepway", "TwConstructionAreaPond"
#pragma endregion


#pragma region LEVEL_FINALBOSS (LAB_DISCOVERA) OBJECT
		//Field 
		, "LbBossRoom", "LbLastBossStage", "LbLastBuilding", "Land_LbLastBossBeforeStep"

		//LbLastBuilding Object :: 보스전 필드의 오브젝트
		,"LbLastStairs"//, "LbLastTank", "LbLastOutFrame1", "LbLastOutFrame2", :: 준수 오더로 삭제. 이제 Anim으로 대체되어 사용안함
		,"LbBossRoomDoorAL","LbBossRoomDoorBL", "LbOutBuildingWallL"

		//LbLastBossBeforeStep Object :: Rubble 
		, "LbRubble01L", "LbRubble02L", "LbRubble03L", "LbRubble04L", "LbRubble05L", "LbRubble06L", "LbRubble07L", "LbRubble08L"
		, "LbRubbleTile01L", "LbRubbleTile02L", "LbRubbleTile03L"
		, "GsRubbleD", "GsRubbleE", "GsRubbleF", "GsRubbleG"
		
		//CmFillerObject, Ml~ :: 채우기용 잡오브젝트
		, "CmFillerObjectAL", "CmFillerObjectA02L", "CmFillerObjectA03L", "CmFillerObjectBL", "CmFillerObjectCL", "CmFillerObjectEL", "CmFillerObjectFL"
		, "MlBossBenchL", "MlBossChairL", "MlFlowerPot01L", "MlSofaFL"

#pragma endregion
	};
	
	m_setKickables = { "GsRubbleA", "GsRubbleB", "GsRubbleC", "GsPebble", "GsTireAL", "GsTireBL", "GsTireCL",  "SeShell", "WasteCanYellow" };

	m_setItemTxts = { "Item_BlueCoin", "Item_Bread", "Item_Cake", "Item_Cocktail", "Item_Coin", "Item_EnergyDrink", "Item_Makaron",
		"Item_Meat", "Item_Omelet", "Item_Onigiri", "Item_RedCoin", "Item_Steak", "Item_Sushi" };

	m_setTrees = { "GsTreeA", "GsTreeB", "GsTreeC", "DsPalmABottomL", "DsPalmAMiddleL", "DsPalmATopL", "DsPalmBL", "DsPalmCL"
		, "GsFlowerBedTreeAL", "GsFlowerBedTreeBL", "GsIvyGroundMiddlePlus", "GsIvyB", "JgGrasslongB"
		, "GsCarSteelPartsAL", "GsScrappedCar", "GsCarHoistCrane01L", "CvSteelPartsDL" };

	//블렌드 적용이 필요한 데코오브젝트
	m_setBlendDecos = {"LbOutBuildingWallL", "LbOutBuildingFenceL", "GsCarFloor" };

	s_vecPassIndices.resize(m_vecMapModelNames.size());
	s_vecSamplingFactors.resize(m_vecMapModelNames.size());

	ReadMapDecoTxts();
	ReadMapObjTxts();
	ReadMonsterTxts();

	ReadTownDecoTxts();
	ReadLabDecoTxts();

	TraverseBlendDecoInfoTxts();

	HideGrid(s_bHideGrid);
	HideTriggers(s_bHideTriggers);
	HideDecos(s_bHideDecos);
	HideWalls(s_bHideWalls);

	return S_OK;
}

_int CMapToolHelper::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CMapToolHelper::Late_Tick(_float fTimeDelta)
{
	ImGui::Begin("MapTool");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
	ImGui::PushFont(ImGui::GetFont()); // 기본 폰트를 가져와서 스케일 조정
	ImGui::SetWindowFontScale(1.1f); // 텍스트 크기를 키움

	Menu_Level();
	Menu_NonAnimModels();

	if (nullptr != m_pPickedObject) 
	{
		Menu_TriggerInfo();
		Menu_MapShaderInfo();
		Menu_MonsterInfo();
		Menu_RallyPointInfo();
		Menu_BlendDecoInfo();
	}

	// 스타일 복원
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopFont();
	ImGui::PopStyleVar();

	ImGui::End();

	if(m_pGameInstance->Get_KeyState(DIMKS_RBUTTON, KEY_DOWN))
		OnRightClick();

	if (!ImGui::IsAnyItemHovered() && m_pGameInstance->Get_KeyState(DIMKS_LBUTTON, KEY_DOWN) && !m_pGameInstance->Get_DIKeyState(DIK_LSHIFT, KEY_PRESS))
		OnLeftClick();

	if (m_pGameInstance->Get_DIKeyState(DIK_ESCAPE, KEY_DOWN))
		On_DIK_Escape();

	if (m_pGameInstance->Get_DIKeyState(DIK_DELETE, KEY_DOWN))
		On_DIK_Delete();

	//SAVE, LOAD 단축키 추가
	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_DOWN))
			Save_Level();

		if (m_pGameInstance->Get_DIKeyState(DIK_L, KEY_DOWN))
			Load_Level();
	}

	Edit_Object();
}

HRESULT CMapToolHelper::Render()
{
	return S_OK;
}

void CMapToolHelper::ReadMapDecoTxts()
{
	string strPath = "../../../model_txt/MapDeco/NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();
			string strModelName = strFilePath.substr(0, strFilePath.length() - 4);
			m_vecMapDecoTxts.emplace_back(strModelName);
		}
		++dir_iter;
	}

	for (auto& objTxt : m_vecMapDecoTxts)
		m_setMapDecoTxts.insert(objTxt);
}

void CMapToolHelper::ReadMapObjTxts()
{
	string strPath = "../../../model_txt/MapObjs/NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();
			string strModelName = strFilePath.substr(0, strFilePath.length() - 4);
			if (true == IsMap(strModelName))
				m_vecMapTxts.emplace_back(strModelName);
			else if (true == IsTrigger(strModelName))
				m_vecTriggerTxts.emplace_back(strModelName);
			else if (true == IsItem(strModelName))
				m_vecItemTxts.emplace_back(strModelName);
			else if (true == IsKickable(strModelName))
				m_vecKickableTxts.emplace_back(strModelName);
			else if("RallyPoint" == strModelName)
				m_vecTriggerTxts.emplace_back(strModelName);
			else
				m_vecObjectTxts.emplace_back(strModelName);
		}
		++dir_iter;
	}

	for (auto& objTxt : m_vecObjectTxts)
		m_setObjectTxts.insert(objTxt);
}

void CMapToolHelper::ReadMonsterTxts()
{
	string strPath = "../../../model_txt/Monsters/NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();
			string strModelName = strFilePath.substr(0, strFilePath.length() - 4);
			m_vecMonsterTxts.emplace_back(strModelName);
		}
		++dir_iter;
	}

	for (auto& objTxt : m_vecMonsterTxts)
		m_setMonsterNames.insert(objTxt);
}

void CMapToolHelper::ReadTownDecoTxts()
{
	string strPath = "../../../model_txt/TownDeco/NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();
			string strModelName = strFilePath.substr(0, strFilePath.length() - 4);
			m_vecTownDecoTxts.emplace_back(strModelName);
		}
		++dir_iter;
	}

	for (auto& objTxt : m_vecTownDecoTxts)
		m_setTownDecoTxts.insert(objTxt);
}

void CMapToolHelper::ReadLabDecoTxts()
{
	string strPath = "../../../model_txt/LabDiscovera_Deco/NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();
			string strModelName = strFilePath.substr(0, strFilePath.length() - 4);
			m_vecLabDecoTxts.emplace_back(strModelName);
		}
		++dir_iter;
	}

	for (auto& objTxt : m_vecLabDecoTxts)
		m_setLabDecoTxts.insert(objTxt);
}

void CMapToolHelper::Menu_Level()
{
	ImGui::SeparatorText("Level");

	for (_int i = LEVEL_INTRO; i <= LEVEL_FINALE; i++)
	{
		if (ImGui::RadioButton(m_vecLevelName[i].c_str(), s_iLevelIndex == i - LEVEL_INTRO)) {
			ImGui::OpenPopup("Level Change");
			s_iTempLevelIdx = i - LEVEL_INTRO;
		}

		if (s_iTempLevelIdx == i - LEVEL_INTRO && ImGui::BeginPopup("Level Change"))
		{
			string strPopUp = "Switch to " + m_vecLevelName[i] + "?";

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
			ImGui::PushFont(ImGui::GetFont()); // 기본 폰트를 가져와서 스케일 조정
			ImGui::SetWindowFontScale(1.2f); // 텍스트 크기를 1.2배로 키움
			// 텍스트를 가운데 정렬
			_float windowWidth = ImGui::GetWindowSize().x;
			_float textWidth = ImGui::CalcTextSize(strPopUp.c_str()).x;
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);

			ImGui::Text(strPopUp.c_str());

			if (ImGui::Button("Yes", ImVec2(90, 35)))
			{
				s_iLevelIndex = s_iTempLevelIdx;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (ImGui::Button("Close", ImVec2(90, 35)))
				ImGui::CloseCurrentPopup();

			// 스타일 복원
			ImGui::SetWindowFontScale(1.0f);
			ImGui::PopFont();
			ImGui::PopStyleVar();

			ImGui::EndPopup();
		}

		if (i % 2 == 0 && i != LEVEL_FINALE)
			ImGui::SameLine();
	}
	//ImGui::NewLine(); 
	HelpMarker(u8"저장 : Ctrl+S / 로드 : Ctrl+L");

	if (ImGui::Button("Save", ImVec2(100, 40)))
		Save_Level();
	ImGui::SameLine(); 
	if (ImGui::Button("Load", ImVec2(100, 40)))
		Load_Level();

	ImGui::SeparatorText("Options");
	if (ImGui::RadioButton("Hide Triggers", s_bHideTriggers)) {
		s_bHideTriggers = !s_bHideTriggers;
		HideTriggers(s_bHideTriggers);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Hide Grid", s_bHideGrid)) {
		s_bHideGrid = !s_bHideGrid;
		HideGrid(s_bHideGrid);
	}
	if (ImGui::RadioButton("Hide Decos", s_bHideDecos)) {
		s_bHideDecos = !s_bHideDecos;
		HideDecos(s_bHideDecos);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Hide Walls", s_bHideWalls)) {
		s_bHideWalls = !s_bHideWalls;
		HideWalls(s_bHideWalls);
	}
}

void CMapToolHelper::Menu_NonAnimModels()
{
	ImGui::SeparatorText("Models");
	if (ImGui::CollapsingHeader("Map"))
	{
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecMapNames(m_vecMapTxts.size());
		for (_int i = 0; i < m_vecMapTxts.size(); ++i)
			vecMapNames[i] = m_vecMapTxts[i].c_str();
		if (ImGui::ListBox("##Maps", &s_iMapTxtIdx, vecMapNames.data(), m_vecMapTxts.size(), 5)) {
			DisableOtherGroups(&s_iMapTxtIdx);
			m_strSelectedTxt = m_vecMapTxts[s_iMapTxtIdx];
		}
	}

	if (ImGui::CollapsingHeader("Triggers"))
	{
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecTriggerNames(m_vecTriggerTxts.size());
		for (_int i = 0; i < m_vecTriggerTxts.size(); ++i)
			vecTriggerNames[i] = m_vecTriggerTxts[i].c_str();
		if (ImGui::ListBox("##Triggers", &s_iTriggerTxtIdx, vecTriggerNames.data(), m_vecTriggerTxts.size(), 5)) {
			DisableOtherGroups(&s_iTriggerTxtIdx);
			m_strSelectedTxt = m_vecTriggerTxts[s_iTriggerTxtIdx];
		}
	}

	if (ImGui::CollapsingHeader("Monsters"))
	{
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecMonsterNames(m_vecMonsterTxts.size());
		for (_int i = 0; i < m_vecMonsterTxts.size(); ++i)
			vecMonsterNames[i] = m_vecMonsterTxts[i].c_str();
		if (ImGui::ListBox("##Monsters", &s_iMonsterTxtIdx, vecMonsterNames.data(), m_vecMonsterTxts.size(), 5)) {
			DisableOtherGroups(&s_iMonsterTxtIdx);
			m_strSelectedTxt = m_vecMonsterTxts[s_iMonsterTxtIdx];
		}
	}

	if (ImGui::CollapsingHeader("Objects"))
	{
		ImGui::InputText("##ObjectsFilter", s_ObjectsFilter, IM_ARRAYSIZE(s_ObjectsFilter)); // 필터 입력받기
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecObjectNames;
		FilterListBoxStrings(s_ObjectsFilter, vecObjectNames, m_vecObjectTxts);

		if (ImGui::ListBox("##Objects", &s_iObjectIdx, vecObjectNames.data(), vecObjectNames.size(), 10)) {
			DisableOtherGroups(&s_iObjectIdx);
			m_strSelectedTxt = string(vecObjectNames[s_iObjectIdx]);
		}
		s_bWasObjectsOpen = true;
	}
	else
		ClearSearchFilter(s_ObjectsFilter, s_bWasObjectsOpen);
		
	if (ImGui::CollapsingHeader("MapDecos"))
	{
		ImGui::InputText("##MapDecoFilter", s_MapDecoFilter, IM_ARRAYSIZE(s_MapDecoFilter)); // 필터 입력받기
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecMapDecoNames;
		FilterListBoxStrings(s_MapDecoFilter, vecMapDecoNames, m_vecMapDecoTxts);
		
		if (ImGui::ListBox("##MapDecos", &s_iMapDecoIdx, vecMapDecoNames.data(), vecMapDecoNames.size(), 13)) {
			DisableOtherGroups(&s_iMapDecoIdx);
			m_strSelectedTxt = string(vecMapDecoNames[s_iMapDecoIdx]);
		}
		s_bWasMapDecosOpen = true;
	}
	else
		ClearSearchFilter(s_MapDecoFilter, s_bWasMapDecosOpen);

	if (ImGui::CollapsingHeader("Items"))
	{
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecItemNames(m_vecItemTxts.size());
		for (_int i = 0; i < m_vecItemTxts.size(); ++i)
			vecItemNames[i] = m_vecItemTxts[i].c_str();
		if (ImGui::ListBox("##Items", &s_iItemIdx, vecItemNames.data(), m_vecItemTxts.size(), 5)) {
			DisableOtherGroups(&s_iItemIdx);
			m_strSelectedTxt = m_vecItemTxts[s_iItemIdx];
		}
	}

	if (ImGui::CollapsingHeader("Kickables"))
	{
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecKickableNames(m_vecKickableTxts.size());
		for (_int i = 0; i < m_vecKickableTxts.size(); ++i)
			vecKickableNames[i] = m_vecKickableTxts[i].c_str();
		if (ImGui::ListBox("##Kickables", &s_iKickableIdx, vecKickableNames.data(), m_vecKickableTxts.size(), 5)) {
			DisableOtherGroups(&s_iKickableIdx);
			m_strSelectedTxt = m_vecKickableTxts[s_iKickableIdx];
		}
	}

	if (ImGui::CollapsingHeader("TownDecos"))
	{
		ImGui::InputText("##TownDecoFilter", s_TownDecoFilter, IM_ARRAYSIZE(s_TownDecoFilter)); // 필터 입력받기
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecTownDecoNames;
		FilterListBoxStrings(s_TownDecoFilter, vecTownDecoNames, m_vecTownDecoTxts);
		
		if (ImGui::ListBox("##TownDecos", &s_iTownDecoIdx, vecTownDecoNames.data(), vecTownDecoNames.size(), 13)) {
			DisableOtherGroups(&s_iTownDecoIdx);
			m_strSelectedTxt = string(vecTownDecoNames[s_iTownDecoIdx]);
		}
		s_bWasTownDecosOpen = true;
	}
	else
		ClearSearchFilter(s_TownDecoFilter, s_bWasTownDecosOpen);

	if (ImGui::CollapsingHeader("LabDecos"))
	{
		ImGui::InputText("##LabDecoFilter", s_LabDecoFilter, IM_ARRAYSIZE(s_LabDecoFilter)); // 필터 입력받기
		ImGui::SetNextItemWidth(200.0f);
		vector<const _char*> vecLabDecoNames;
		FilterListBoxStrings(s_LabDecoFilter, vecLabDecoNames, m_vecLabDecoTxts);

		if (ImGui::ListBox("##LabDecos", &s_iLabDecoIdx, vecLabDecoNames.data(), vecLabDecoNames.size(), 13)) {
			DisableOtherGroups(&s_iLabDecoIdx);
			m_strSelectedTxt = string(vecLabDecoNames[s_iLabDecoIdx]);
		}
		s_bWasLabDecosOpen = true;
	}
	else
		ClearSearchFilter(s_LabDecoFilter, s_bWasLabDecosOpen);
}

void CMapToolHelper::Menu_TriggerInfo()
{
	if (false == IsTrigger(m_strCurModel))
		return;

	if (m_strCurModel == "Camera" || m_strCurModel == "Trigger" || m_strCurModel == "Dummy")
	{
		CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
		s_iTriggerType = pMapToolObject->Get_TriggerType();
		s_iTriggerIdx = pMapToolObject->Get_TriggerIndex();

		string strTrigger = "[ " + m_strCurModel + " ]";
		ImGui::Begin(strTrigger.c_str());
		if (m_strCurModel == "Trigger" || m_strCurModel == "Dummy") {
			ImGui::SetCursorPosX(37);
			ImGui::Text("TYPE");
			ImGui::SameLine();
			ImGui::SetCursorPosX(120);
		}
		else
			ImGui::SetCursorPosX(33);
		ImGui::Text("INDEX");


		if (m_strCurModel == "Trigger")
		{
			ImGui::SetNextItemWidth(80);
			if (ImGui::Combo("##Type", &s_iTriggerType, s_triggerTypes, IM_ARRAYSIZE(s_triggerTypes)))
				pMapToolObject->Set_TriggerType(s_iTriggerType);
			ImGui::SameLine();
		}
		else if (m_strCurModel == "Dummy")
		{
			ImGui::SetNextItemWidth(80);
			if (ImGui::Combo("##Type", &s_iCamType, s_camTypes, IM_ARRAYSIZE(s_camTypes)))
				pMapToolObject->Set_CamType(s_iCamType);
			ImGui::SameLine();
		}
		
		//-------------------------------------------
		ImGui::SetNextItemWidth(80);
		if (ImGui::Combo("##Index", &s_iTriggerIdx, s_triggerIndices, IM_ARRAYSIZE(s_triggerIndices))) {
			pMapToolObject->Set_TriggerIndex(s_iTriggerIdx);
		}

		if (m_strCurModel == "Dummy")
			Menu_CamLerpInfo(pMapToolObject);

		ImGui::End();
	}
	else if ("Fog" == m_strCurModel)
	{
		CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
		s_iTriggerIdx = pMapToolObject->Get_TriggerIndex(); // Rows
		s_fRadius = pMapToolObject->Get_Radius(); // NumInstances
		string strTrigger = "[ " + m_strCurModel + " ]";

		ImGui::Begin(strTrigger.c_str());
		ImGui::SetCursorPosX(17);
		ImGui::Text("ROWS");
		ImGui::SameLine();
		ImGui::SetCursorPosX(77);

		ImGui::SetNextItemWidth(80);
		if (ImGui::Combo("##InstanceRows", &s_iTriggerIdx, s_triggerIndices, IM_ARRAYSIZE(s_triggerIndices)))
			pMapToolObject->Set_TriggerIndex(s_iTriggerIdx);

		ImGui::Text("INSTANCES");
		ImGui::SameLine();
		ImGui::SetCursorPosX(76);
		ImGui::SetNextItemWidth(90);
		if (ImGui::InputFloat("##NumInstances", &s_fRadius, 1.f, 1.f, "%.0f"))
			pMapToolObject->Set_Radius(s_fRadius);

		ImGui::End();
	}
}

void CMapToolHelper::Menu_CamLerpInfo(CMapToolObject* _pMapToolObject)
{
	s_iCamType = _pMapToolObject->Get_CamType();
	s_fRadius = _pMapToolObject->Get_Radius();

	ImGui::Text("RADIUS");
	ImGui::SameLine();
	ImGui::SetCursorPosX(60);
	ImGui::SetNextItemWidth(90);
	if (ImGui::InputFloat("##Radius", &s_fRadius, 1.f, 1.f, "%.3f"))
		_pMapToolObject->Set_Radius(s_fRadius);
}

void CMapToolHelper::Menu_MapShaderInfo()
{
	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	if (nullptr == pModel)
		return;

	string strModelName = pModel->Get_ModelInfo().strModelName;
	_int iIndex = Compute_MapIndex(strModelName);

	if (-1 == iIndex)
		return;

	_uint iNumMesh = pModel->Get_NumMeshes();

	if (s_vecPassIndices[s_iMapIndex].empty())
		s_vecPassIndices[s_iMapIndex].resize(iNumMesh);
	if (s_vecSamplingFactors[s_iMapIndex].empty()) {
		s_vecSamplingFactors[s_iMapIndex].resize(iNumMesh);
		fill(s_vecSamplingFactors[s_iMapIndex].begin(), s_vecSamplingFactors[s_iMapIndex].end(), 1.f);
	}

	vector<string> vecMeshNames(iNumMesh);
	for (_uint i = 0; i < iNumMesh; ++i)
		vecMeshNames[i] = pModel->Get_MeshName(i);

	vector<const _char*> vecMapMeshNames(iNumMesh);
	for (_uint i = 0; i < iNumMesh; ++i)
		vecMapMeshNames[i] = vecMeshNames[i].c_str();

	CBasicMap* pBasicMap = dynamic_cast<CBasicMap*>(m_pPickedObject);
	if (nullptr == pBasicMap)
		return;

	string strMapInfo = strModelName + "_ShaderInfo";
	ImGui::Begin(strMapInfo.c_str());

	_float fButtonWidth = 80;
	if (ImGui::Button("Reset", ImVec2(fButtonWidth, 30)))
		Reset_MapShaderInfo();
	ImGui::SameLine();

	ImVec2 vWindowSize = ImGui::GetWindowSize();
	_float fButtonPosX = (vWindowSize.x - fButtonWidth) * 0.5f;
	ImGui::SetCursorPosX(fButtonPosX - fButtonWidth * 0.6f);
	if (ImGui::Button("Save", ImVec2(fButtonWidth, 30)))
		Save_MapShaderInfo();
	ImGui::SameLine();
	
	ImGui::SetCursorPosX(fButtonPosX + fButtonWidth * 0.6f);
	if (ImGui::Button("Load", ImVec2(fButtonWidth, 30)))
		Load_MapShaderInfo();
	ImGui::SameLine();

	ImGui::SetCursorPosX(fButtonPosX + fButtonWidth * 3.1f);
	if (ImGui::Button("SaveOctree", ImVec2(fButtonWidth, 30)))
		Save_Octree();

	for (_uint j = 0; j < iNumMesh; j++)
	{
		ImGui::PushID(j);
		ImGui::Text("%d", j);
		ImGui::SameLine(25); // 다음 메뉴 위치를 25에서부터 시작하도록 지정
		if (ImGui::Selectable(vecMapMeshNames[j], s_iSelectedMeshIndex == j, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(320,0)))
		{
			s_iSelectedMeshIndex = j;
			pBasicMap->Reset_Time(j);
		}
		ImGui::SameLine();
		//------------------------------
		ImGui::SetNextItemWidth(150);
		if (ImGui::Combo("##PassIndex", &s_vecPassIndices[s_iMapIndex][j], s_ShaderPasses, IM_ARRAYSIZE(s_ShaderPasses)))
		{
			pBasicMap->Set_PassIndex(j, s_vecPassIndices[s_iMapIndex][j]);
		}
		ImGui::SameLine();
		//------------------------------
		ImGui::SetNextItemWidth(100);
		if (ImGui::InputFloat("##SamplingFactor", &s_vecSamplingFactors[s_iMapIndex][j], 0.01f, 1.0f, "%.3f")) 
		{
			pBasicMap->Set_SamplingFactor(j, s_vecSamplingFactors[s_iMapIndex][j]);
		}
		ImGui::PopID();
	}
	ImGui::End();
}

void CMapToolHelper::Menu_MonsterInfo()
{
	if (false == IsMonster(m_strCurModel))
		return;

	CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
	if (nullptr == pMapToolObject)
		return;

	s_iTriggerIdx = pMapToolObject->Get_TriggerIndex();

	string strMonsterName = m_strCurModel.substr(8);
	ImGui::Begin(strMonsterName.c_str());
	ImGui::SetCursorPosX(33);
	ImGui::Text("INDEX");
	ImGui::SetNextItemWidth(80);
	if (ImGui::Combo("##Index", &s_iTriggerIdx, s_triggerIndices, IM_ARRAYSIZE(s_triggerIndices)))
		pMapToolObject->Set_TriggerIndex(s_iTriggerIdx);

	ImGui::End();
}

void CMapToolHelper::Menu_RallyPointInfo()
{
	if (m_strCurModel != "RallyPoint")
		return;

	string strRallyInfo = m_strCurModel + " Info";
	ImGui::Begin(strRallyInfo.c_str());

	CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
	if (nullptr == pMapToolObject)
		return;

	string strConnectedMonster = pMapToolObject->Get_ConnectedMonster();
	s_iConnectedMonster = Compute_RallyingMonsterIndex(strConnectedMonster);

	vector<const _char*> vecRallyingMonsterNames;
	for (auto& monsterName : m_setRallyingMonsters)
		vecRallyingMonsterNames.push_back(monsterName.c_str());

	ImGui::SetNextItemWidth(150);
	if (ImGui::Combo("##ConnectedMonster", &s_iConnectedMonster, vecRallyingMonsterNames.data(), vecRallyingMonsterNames.size())) {
		string strMonsterName(vecRallyingMonsterNames[s_iConnectedMonster]);
		pMapToolObject->Set_ConnectedMonster(strMonsterName);
	}
		
	s_iTriggerIdx = pMapToolObject->Get_TriggerIndex();

	ImGui::Text("INDEX");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80);
	if (ImGui::Combo("##RallyPointIndex", &s_iTriggerIdx, s_triggerIndices, IM_ARRAYSIZE(s_triggerIndices)))
		pMapToolObject->Set_TriggerIndex(s_iTriggerIdx);

	ImGui::End();
}

void CMapToolHelper::Menu_BlendDecoInfo()
{
	if (false == IsBlendDeco(m_strCurModel))
		return;

	string strBlendDecoInfo = m_strCurModel + "_Info";
	ImGui::Begin(strBlendDecoInfo.c_str());

	CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
	if (nullptr == pMapToolObject)
		return;

	CModel* pModel = dynamic_cast<CModel*>(pMapToolObject->Get_Component(TEXT("Com_Model")));
	_uint iNumMeshes = pModel->Get_NumMeshes();

	_bool* bBlendDecoInfo = new _bool[iNumMeshes];
	for (_uint i = 0; i < iNumMeshes; i++)
		bBlendDecoInfo[i] = false;

	auto mapIter = m_mapBlendDecoInfos.find(m_strCurModel);
	if (m_mapBlendDecoInfos.end() != mapIter) {
		for (auto& blendMeshIndex : mapIter->second)
		{
			if (blendMeshIndex < iNumMeshes) // 배열 범위 검사
				bBlendDecoInfo[blendMeshIndex] = true;
		}
	}

	vector<string> vecMeshNames(iNumMeshes);
	for (_uint i = 0; i < iNumMeshes; ++i)
		vecMeshNames[i] = pModel->Get_MeshName(i);

	vector<const _char*> vecBlendDecoMeshNames(iNumMeshes);
	for (_uint i = 0; i < iNumMeshes; ++i)
		vecBlendDecoMeshNames[i] = vecMeshNames[i].c_str();
	
	for (_uint i = 0; i < iNumMeshes; i++) {
		ImGui::Text("%d", i);
		ImGui::SameLine(25); // 다음 메뉴 위치를 25에서부터 시작하도록 지정
		if (ImGui::Selectable(vecBlendDecoMeshNames[i], s_iSelectedMeshIndex == i, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(240, 0)))
		{
			s_iSelectedMeshIndex = i;
			pMapToolObject->Reset_Time(i);
		}
		ImGui::SameLine();

		string strLabel = "##BlendMeshIndex" + to_string(i);
		if (ImGui::Checkbox(strLabel.c_str(), &bBlendDecoInfo[i])) {

			unordered_set<_uint> setBlendMeshIndices;
			for (_uint j = 0; j < iNumMeshes; j++)
			{
				if (true == bBlendDecoInfo[j])
					setBlendMeshIndices.insert(j);
			}
			
			m_mapBlendDecoInfos.insert_or_assign(m_strCurModel, setBlendMeshIndices);
			pMapToolObject->Set_PassIndices(setBlendMeshIndices);
		}
	}
		
	Safe_Delete_Array(bBlendDecoInfo);

	ImGui::End();
}

void CMapToolHelper::Edit_Object()
{
	if (nullptr == m_pPickedObject)
		return;

	CTransform* pTransform = dynamic_cast<CTransform*>(m_pPickedObject->Get_Component(g_strTransformTag));
	if (nullptr == pTransform)
		return;

	Safe_AddRef(pTransform);

	ImGui::Begin(m_strCurModel.c_str());

	_uint iShaderVars = m_pPickedObject->Get_ShaderVars();
	_float fRimWidth = m_pPickedObject->Get_RimWidth();

	_bool bStencil = (iShaderVars >> 2) & 1;
	_bool bRimLight = (iShaderVars >> 1) & 1;
	_bool bMotionBlur = iShaderVars & 1;
	if (ImGui::Checkbox("Stencil", &bStencil))
		iShaderVars ^= 4;
	ImGui::SameLine();
	if (ImGui::Checkbox("RimLight", &bRimLight))
		iShaderVars ^= 2;
	ImGui::SameLine();
	if (ImGui::Checkbox("MotionBlur", &bMotionBlur))
		iShaderVars ^= 1;
	m_pPickedObject->Set_ShaderVars(iShaderVars);
	
	_float windowWidth = ImGui::GetContentRegionAvail().x;
	_float inputFloatWidth = 100; // 설정된 너비
	_float thicknessTextWidth = ImGui::CalcTextSize("Thickness ").x;
	_float totalWidth = thicknessTextWidth + inputFloatWidth + ImGui::GetStyle().ItemSpacing.x;
	_float offset = (windowWidth - totalWidth) * 0.5f;
	if (offset > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

	ImGui::Text("Thickness ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	if (ImGui::InputFloat("##fRimWidth", &fRimWidth, 0.01f, 1.0f, "%.3f"))
		m_pPickedObject->Set_RimWidth(fRimWidth);

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	if (nullptr == pModel)  // Shader_PosTex  패스 지정
	{
		/*ImGui::SetNextItemWidth(150);
		GetPassIndex();
		if (ImGui::Combo("##PosTexPassIndex", &s_iPassIndex, s_PosTexPasses, IM_ARRAYSIZE(s_PosTexPasses)))
			SetPassIndex(s_iPassIndex);*/
		
	}
	else  // Shader_Model 패스지정
	{
		CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
		if (pMapToolObject != nullptr) {
			ImGui::SetNextItemWidth(150);
			s_iPassIndex = pMapToolObject->Get_PassIndex();
			if (ImGui::Combo("##ModelPassIndex", &s_iPassIndex, s_ModelPassIndices, IM_ARRAYSIZE(s_ModelPassIndices)))
				pMapToolObject->Set_PassIndex(s_iPassIndex);
		}
	}
	
	_float4x4 tempMatrix = pTransform->Get_WorldFloat4x4();
	m_pGameInstance->EditTransform(tempMatrix); // 선택한 모델의 월드행렬을 수정 
	pTransform->Set_WorldMatrix(tempMatrix);

	Safe_Release(pTransform);

	ImGui::End();
}

void CMapToolHelper::OnLeftClick()
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		return;

	CGameObject* pPickedObject = Select_ModelByPicking();
	if (nullptr == pPickedObject)
		return;

	Safe_Release(m_pPickedObject);
	m_pPickedObject = nullptr;
	m_pPickedObject = pPickedObject;
	Safe_AddRef(m_pPickedObject);

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	m_strCurModel = pModel->Get_ModelInfo().strModelName;
	_uint iNumMeshes = pModel->Get_NumMeshes();

	_int iIndex = Compute_MapIndex(m_strCurModel);

	if (-1 == iIndex) // 피킹한 객체가 맵이 아님
		return;

	CBasicMap* pBasicMap = dynamic_cast<CBasicMap*>(m_pPickedObject);
	if (nullptr == pBasicMap)
		return;

	pBasicMap->Reset_Time(s_iPickedMeshIndex);
	s_iSelectedMeshIndex = s_iPickedMeshIndex;
	s_iMapIndex = iIndex;

	if (s_vecPassIndices[iIndex].empty() || s_vecSamplingFactors[iIndex].empty())
	{
		s_vecPassIndices[iIndex].resize(iNumMeshes);
		s_vecSamplingFactors[iIndex].resize(iNumMeshes);

		Load_MapShaderInfo();
	}
}

void CMapToolHelper::OnRightClick()
{
	if (false == IsAnythingSelected())
		return;

	_float2 vMouseViewPortPos = m_pGameInstance->Get_MouseViewPortPos();
	_vector vWorldPos = m_pGameInstance->Compute_WorldPos(vMouseViewPortPos, TEXT("Target_FieldDepth"));

	if (false == XMVector4Equal(vWorldPos, XMVectorZero())) 
		m_vPickPos = vWorldPos;
	else
	{
		CGameObject* pGrid = m_pGameInstance->Get_GameObject(LEVEL_TOOL_MAP, TEXT("Layer_Grid"), 0);
		if (nullptr == pGrid)
			return;

		CVIBuffer_Terrain* pGridBuffer = dynamic_cast<CVIBuffer_Terrain*>(pGrid->Get_Component(TEXT("Com_VIBuffer")));
		if (nullptr == pGridBuffer)
			return;

		const CTransform* pTransform = dynamic_cast<const CTransform*>(pGrid->Get_Component(g_strTransformTag));
		if (nullptr == pTransform)
			return;

		m_vPickPos = pGridBuffer->Get_PickPos(pTransform);
	}
		
	if (!::XMVector3Equal(::XMLoadFloat3(&m_vPickPos), ::XMVectorSet(0.f, 0.f, 0.f, 0.f)))
	{
		_float4 vTemp(m_vPickPos.x, m_vPickPos.y, m_vPickPos.z, 1.0f); // 위치
		CMapToolObject::MAPTOOLOBJECT_DESC tMapToolDesc = {};
		memcpy(&tMapToolDesc.matWorld.m[CTransform::STATE_POSITION], &vTemp, sizeof(_float4));
		tMapToolDesc.wstrModelName = CUtils::StrToWstr(m_strSelectedTxt);

		if (Compute_MapIndex(m_strSelectedTxt) == -1) // 맵이 아닐때
		{
			if (true == IsMap(m_strSelectedTxt)) { // BG0, BG1 
				if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"),
					TEXT("Prototype_GameObject_BG"), &tMapToolDesc)))
					return;
			}
			else
			{
				if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"),
					TEXT("Prototype_GameObject_MapToolObject"), &tMapToolDesc)))
					return;
			}
		}
		else
		{
			CGameObject::GAMEOBJECT_DESC tempDesc = {};
			tempDesc.matWorld = tMapToolDesc.matWorld;
			tempDesc.wstrModelName = tMapToolDesc.wstrModelName;

			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), 
				TEXT("Prototype_GameObject_BasicMap"), &tempDesc)))
				return;
		}

		list<CGameObject*>* pObjList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
		Safe_Release(m_pPickedObject);
		m_pPickedObject = nullptr;
		m_pPickedObject = pObjList->back();
		Safe_AddRef(m_pPickedObject);
		m_strCurModel = m_strSelectedTxt;

		if (m_setMapDecoTxts.end() != m_setMapDecoTxts.find(m_strCurModel))
			m_pPickedObject->Set_ShaderVars(2);

		_int i{};
		DisableOtherGroups(&i);
	}
}

void CMapToolHelper::On_DIK_Escape()
{
	_int i{};
	DisableOtherGroups(&i);
	Safe_Release(m_pPickedObject);
	m_pPickedObject = nullptr;
}

void CMapToolHelper::On_DIK_Delete()
{
	if (m_pPickedObject != nullptr) {
		Safe_Release(m_pPickedObject);
		m_pPickedObject->Set_Dead();
	}
		
	m_pPickedObject = nullptr;
}

void CMapToolHelper::Save_Level()
{
	if (s_iLevelIndex < 0)
		return;

	On_DIK_Escape();
	string strLevel = m_vecLevelName[s_iLevelIndex + LEVEL_INTRO];
	string tempFileName = "temp_" + strLevel + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return;
	}

	list<CGameObject*>* pObjectsList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
	if (nullptr == pObjectsList) {
		MSG_BOX(TEXT("Get_List() == nullptr."));
		return;
	}

	if (pObjectsList->empty()) {
		MSG_BOX(TEXT("List is empty."));
		return;
	}

	RegisterRallyPoints(pObjectsList);

	vector<CGameObject*> vecMap;
	vector<CGameObject*> vecTriggers;
	vector<CGameObject*> vecMonsters;
	vector<CGameObject*> vecRallyPoints;
	vector<CGameObject*> vecDecos;
	vector<CGameObject*> vecItems;
	vector<CGameObject*> vecKickables;

	for (auto& object : *pObjectsList)
	{
		if (nullptr == object)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(object->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		CTransform* pTransform = dynamic_cast<CTransform*>(object->Get_Component(g_strTransformTag));
		if (nullptr == pTransform)
			continue;

		string strModelName = pModel->Get_ModelInfo().strModelName;
		if (true == IsMap(strModelName)){
			vecMap.push_back(object);
			continue;
		}
		if (true == IsTrigger(strModelName))
		{
			vecTriggers.push_back(object);
			continue;
		}
		if (true == IsMonster(strModelName)) {
			vecMonsters.push_back(object);
			continue;
		}
		if (true == IsItem(strModelName)) {
			vecItems.push_back(object);
			continue;
		}
		if (true == IsKickable(strModelName)) {
			vecKickables.push_back(object);
			continue;
		}
		if (true == IsDeco(strModelName)) {
			vecDecos.push_back(object);
			continue;
		}
		if ("RallyPoint" == strModelName) {
			vecRallyPoints.push_back(object);
			continue;
		}
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();
		_uint iShaderVars = object->Get_ShaderVars();
		_float fRimWidth = object->Get_RimWidth();

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));
		outputFile.write(reinterpret_cast<const char*>(&iShaderVars), sizeof(iShaderVars));
		outputFile.write(reinterpret_cast<const char*>(&fRimWidth), sizeof(fRimWidth));
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return;
	}

	RenameFile(strLevel, tempFileName, string());

	wstring wstrSave;
	if (true == Save_Map(strLevel, vecMap))
		wstrSave += L"Map O\n";
	else
		wstrSave += L"Map X\n";

	if (true == Save_Triggers(strLevel, vecTriggers))
		wstrSave += L"Triggers O\n";
	else
		wstrSave += L"Triggers X\n";

	if (true == Save_Decos(strLevel, vecDecos))
		wstrSave += L"Decos O\n";
	else
		wstrSave += L"Decos X\n";

	if(true == Save_Monsters(strLevel, vecMonsters))
		wstrSave += L"Monsters O\n";
	else
		wstrSave += L"Monsters X\n";

	if (true == Save_Items(strLevel, vecItems))
		wstrSave += L"Items O\n";
	else
		wstrSave += L"Items X\n";

	if(true == Save_Kickables(strLevel, vecKickables))
		wstrSave += L"Kickables O\n";
	else
		wstrSave += L"Kickables X\n";

	if(true == Save_BlendDecoInfos())
		wstrSave += L"BlendDecoInfos O\n";
	else
		wstrSave += L"BlendDecoInfos X\n";

	MSG_BOX(wstrSave.c_str());
}

void CMapToolHelper::Load_Level()
{
	On_DIK_Escape();
	On_DIK_Delete();
	m_pGameInstance->Clear_Layer(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));

	string strLevel = m_vecLevelName[s_iLevelIndex + LEVEL_INTRO];
	string strFileName = "../../../objects_txt/" + strLevel + ".txt";

	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(strLevel) + TEXT(".txt");
		MSG_BOX(wstrError.c_str());
		return;
	}

	string strModelName;
	_float4x4 matWorld{};
	_int triggerType{};
	_int iTriggerIndex{};
	_int iCamType{};
	_float fRadius{};
	_float3 vMin{}, vMax{};
	_uint iShaderVars{};
	_float fRimWidth{};
	map<_uint, _float3> rallyPoints;
	string strConnectedMonster;

	while (!fileStream.eof()) 
	{
		_uint iStrLength{};
		fileStream.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileStream.read(&strModelName[0], iStrLength);
		fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));
		fileStream.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileStream.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		if (fileStream.eof())
			break;

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;
		wstring wstrGameObjectTag = TEXT("MapToolObject");

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileStream.close();
			return;
		}
	}

	fileStream.close();

	Load_Map(strLevel);
	Load_Decos(strLevel);
	Load_Triggers(strLevel);
	Load_Monsters(strLevel);
	//Load_RallyPoints(strLevel);
	Load_Items(strLevel);
	Load_Kickables(strLevel);

	MoveToCam();
}

void CMapToolHelper::Save_MapShaderInfo()
{
	string tempFileName = "temp_" + m_vecMapModelNames[s_iMapIndex] + "_ShaderInfo.txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return;
	}

	if (s_vecPassIndices[s_iMapIndex].empty() || s_vecSamplingFactors[s_iMapIndex].empty())
		return;

	for (_int i = 0; i < s_vecPassIndices[s_iMapIndex].size(); i++)
	{
		outputFile.write(reinterpret_cast<const char*>(&s_vecPassIndices[s_iMapIndex][i]), sizeof(s_vecPassIndices[s_iMapIndex][i]));
		outputFile.write(reinterpret_cast<const char*>(&s_vecSamplingFactors[s_iMapIndex][i]), sizeof(s_vecSamplingFactors[s_iMapIndex][i]));
	}
	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return;
	}

	// 현재시간 받아오기
	auto now = chrono::system_clock::now();
	time_t currentTime = chrono::system_clock::to_time_t(now);

	struct tm timeinfo;
	localtime_s(&timeinfo, &currentTime);

	// 현재 시간을 문자열로 변환
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);

	string fileName_Time = "../../../objects_txt/" + string(buffer) + "_" + m_vecMapModelNames[s_iMapIndex] + "_ShaderInfo.txt";
	string fileName = "../../../objects_txt/" + m_vecMapModelNames[s_iMapIndex] + "_ShaderInfo.txt";
	if (rename(fileName.c_str(), fileName_Time.c_str()) != 0)
	{
		MSG_BOX(TEXT("Failed to rename original file."));
		return;
	}

	if (rename(tempFileName.c_str(), fileName.c_str()) != 0) // 임시파일 이름을 level 이름으로 변경
	{
		wstring wstrError2 = TEXT("Failed to rename ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError2.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return;
	}

	wstring wstrSaveMsg = CUtils::StrToWstr(string(m_vecMapModelNames[s_iMapIndex] + "_ShaderInfo")) + TEXT(" Saved.");
	MSG_BOX(wstrSaveMsg.c_str());
}

void CMapToolHelper::Load_MapShaderInfo()
{
	if (s_vecPassIndices.empty() || s_vecSamplingFactors.empty())
		return;

	string strFileName = "../../../objects_txt/" + m_vecMapModelNames[s_iMapIndex] + "_ShaderInfo.txt";

	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(m_vecMapModelNames[s_iMapIndex]) + TEXT("_ShaderInfo.txt");
		MSG_BOX(wstrError.c_str());
		return;
	}

	CBasicMap* pBasicMap = dynamic_cast<CBasicMap*>(m_pPickedObject);

	_int iPassIndex{};
	_float fSamplingFactor{};
	_int iCount{};
	while (!fileStream.eof())
	{
		fileStream.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));
		fileStream.read(reinterpret_cast<char*>(&fSamplingFactor), sizeof(fSamplingFactor));

		if (fileStream.eof())
			break;

		s_vecPassIndices[s_iMapIndex][iCount] = iPassIndex;
		s_vecSamplingFactors[s_iMapIndex][iCount] = fSamplingFactor;
		pBasicMap->Set_PassIndex(iCount, iPassIndex);
		pBasicMap->Set_SamplingFactor(iCount, fSamplingFactor);
		iCount++;
	}

	fileStream.close();
}

CGameObject* CMapToolHelper::Select_ModelByPicking(const wstring& wstrLayerTag)
{
	vector<CGameObject*> vecPickedObjects;
	vector<_float4> vecPickPos;
	vector<_int> vecMeshIndex;

	list<CGameObject*>* pObjectList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, wstrLayerTag);
	if (nullptr == pObjectList)
		return nullptr;

	if (pObjectList->empty())
		return nullptr;

	_bool bCtrl = m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS);
	
	for (auto& object : *pObjectList)
	{
		if (nullptr == object)
			continue;

		if (true == object->Get_Hide())
			continue;

		CModel* pModel = dynamic_cast<CModel*>(object->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		CTransform* pTransform = dynamic_cast<CTransform*>(object->Get_Component(g_strTransformTag));
		if (nullptr == pTransform)
			continue;

		string strModelName = pModel->Get_ModelName();
		if (true == bCtrl && true == IsMap(strModelName))
			continue;

		_int iMeshIndex{};
		_float4 vPickPos = pModel->Check_Meshes(pTransform, iMeshIndex);
		if (vPickPos.w != 0) // 피킹 성공
		{
			vecPickedObjects.emplace_back(object);
			vecPickPos.emplace_back(vPickPos);
			vecMeshIndex.emplace_back(iMeshIndex);
		}
	}

	if (vecPickedObjects.empty())
		return nullptr;

	_float fShortest = { FLT_MAX };
	CGameObject* pResult = { nullptr };
	for (int i = 0; i < vecPickPos.size(); i++)
	{
		if (vecPickPos[i].w < fShortest) {
			fShortest = vecPickPos[i].w;
			pResult = vecPickedObjects[i];
			s_iPickedMeshIndex = vecMeshIndex[i];
		}
	}

	return pResult;
}

_int CMapToolHelper::Compute_MapIndex(const string& _strModelName)
{
	for (_int i = 0; i < m_vecMapModelNames.size(); i++)
	{
		if (_strModelName == m_vecMapModelNames[i])
			return i;
	}

	return -1;
}

_int CMapToolHelper::Compute_RallyingMonsterIndex(const string& _strModelName)
{
	_int iCount{};
	for (auto& monsterName : m_setRallyingMonsters) {
		if (monsterName == _strModelName)
			return iCount;
		iCount++;
	}

	return -1;
}

_bool CMapToolHelper::IsMap(const string& _strModelName)
{
	if (m_setMapNames.end() != m_setMapNames.find(_strModelName))
		return true;

	return _bool();
}

_bool CMapToolHelper::IsTrigger(const string& _strModelName)
{
	if (m_setTriggerNames.end() != m_setTriggerNames.find(_strModelName))
		return true;

	return _bool();
}

_bool CMapToolHelper::IsMonster(const string& _strModelName)
{
	if (m_setMonsterNames.end() != m_setMonsterNames.find(_strModelName))
		return true;
		
	return _bool();
}

_bool CMapToolHelper::IsDeco(const string& _strModelName)
{
	if (m_setMapDecoTxts.end() != m_setMapDecoTxts.find(_strModelName))
		return true;

	if (m_setTownDecoTxts.end() != m_setTownDecoTxts.find(_strModelName))
		return true;

	if (m_setLabDecoTxts.end() != m_setLabDecoTxts.find(_strModelName))
		return true;

	return _bool();
}

_bool CMapToolHelper::IsItem(const string& _strModelName)
{
	if (m_setItemTxts.end() != m_setItemTxts.find(_strModelName))
		return true;

	return _bool();
}

_bool CMapToolHelper::IsRallyingMonster(const string& _strModelName)
{
	if (m_setRallyingMonsters.end() != m_setRallyingMonsters.find(_strModelName))
		return true;

	return false;
}

_bool CMapToolHelper::IsKickable(const string& _strModelName)
{
	if (m_setKickables.end() != m_setKickables.find(_strModelName))
		return true;

	return false;
}

_bool CMapToolHelper::IsTree(const string& _strModelName)
{
	if (m_setTrees.end() != m_setTrees.find(_strModelName))
		return true;

	return false;
}

_bool CMapToolHelper::IsBlendDeco(const string& _strModelName)
{
	if (m_setBlendDecos.end() != m_setBlendDecos.find(_strModelName))
		return true;

	return false;
}

_bool CMapToolHelper::RenameFile(const string& _strLevel, const string& _tempFileName, const string& _strCustom)
{
	// 현재시간 받아오기
	auto now = chrono::system_clock::now();
	time_t currentTime = chrono::system_clock::to_time_t(now);

	struct tm timeinfo;
	localtime_s(&timeinfo, &currentTime);

	// 현재 시간을 문자열로 변환
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);

	string fileName_Time = "../../../objects_txt/" + string(buffer) + "_" + _strLevel + _strCustom + ".txt";
	string fileName = "../../../objects_txt/" + _strLevel + _strCustom + ".txt";
	
	if (rename(fileName.c_str(), fileName_Time.c_str()) != 0)
	{
		string strFile = _strLevel + _strCustom + ".txt";
		wstring wstrRenameErr = L"Failed to Rename : " + CUtils::StrToWstr(strFile);
		MSG_BOX(wstrRenameErr.c_str());
		return false;
	}

	if (rename(_tempFileName.c_str(), fileName.c_str()) != 0) // 임시파일 이름을 level 이름으로 변경
	{
		wstring wstrError2 = TEXT("Failed to rename ") + CUtils::StrToWstr(_tempFileName);
		MSG_BOX(wstrError2.c_str());
		remove(_tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	return true;
}

void CMapToolHelper::HideTriggers(_bool bHideTriggers)
{
	list<CGameObject*>* pObjectList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
	if (pObjectList == nullptr)
		return;

	if (pObjectList->empty())
		return;

	for (auto& obj : *pObjectList)
	{
		if (nullptr == obj)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		string strModelName = pModel->Get_ModelInfo().strModelName;
		if ("Trigger" == strModelName || "Dummy" == strModelName)
			obj->Set_Hide(bHideTriggers);
	}
}

void CMapToolHelper::HideGrid(_bool bHideGrid)
{
	CGameObject* pGrid = m_pGameInstance->Get_GameObject(LEVEL_TOOL_MAP, TEXT("Layer_Grid"));
	if (nullptr == pGrid)
		return;

	pGrid->Set_Hide(bHideGrid);
}

void CMapToolHelper::HideDecos(_bool bHideDecos)
{
	list<CGameObject*>* pObjectList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
	if (pObjectList == nullptr)
		return;

	if (pObjectList->empty())
		return;

	for (auto& obj : *pObjectList)
	{
		if (nullptr == obj)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		string strModelName = pModel->Get_ModelInfo().strModelName;
		if(true == IsDeco(strModelName))
			obj->Set_Hide(bHideDecos);
	}
}

void CMapToolHelper::HideWalls(_bool bHideWalls)
{
	list<CGameObject*>* pObjectList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
	if (pObjectList == nullptr)
		return;

	if (pObjectList->empty())
		return;

	for (auto& obj : *pObjectList)
	{
		if (nullptr == obj)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		string strModelName = pModel->Get_ModelInfo().strModelName;
		if ("NonRenderWall" == strModelName)
			obj->Set_Hide(bHideWalls);
	}
}

_bool CMapToolHelper::ExcludeModel(string& _strModelName)
{
	if (_strModelName.size() < 4)
		return false;

	if (_strModelName.substr(0, 4) == "Test" || _strModelName.substr(0, 4) == "Dash" || _strModelName.substr(0, 5) == "Smoke" ||
		_strModelName.substr(0, 9) == "SkySphere" || _strModelName.substr(_strModelName.size() - 5) == "Blend"
		|| "Tornado" == _strModelName || _strModelName.substr(0, 6) == "Vacuum" || _strModelName.substr(0, 5) == "Sword"
		|| _strModelName.substr(_strModelName.size() - 5) == "Sword" || _strModelName.substr(0, 5) == "Kirby")
		return true;

	return _bool();
}

void CMapToolHelper::MoveToCam()
{
	list<CGameObject*>* pObjList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
	if (nullptr == pObjList)
		return;
	if (pObjList->empty())
		return;
	CGameObject* pCamera = m_pGameInstance->Get_GameObject(LEVEL_TOOL_MAP, TEXT("Layer_Camera"));
	if (nullptr == pCamera)
		return;
	CTransform* pCamTransform = pCamera->Get_TransformCom();
	if (nullptr == pCamTransform)
		return;
	CMapToolObject* pMapToolObject = { nullptr };

	for (auto& obj : *pObjList)
	{
		if (nullptr == obj)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		string strModelName = pModel->Get_ModelName();
		if (strModelName == "Camera")
		{
			pMapToolObject = dynamic_cast<CMapToolObject*>(obj);
			break;
		}
	}

	if (nullptr == pMapToolObject)
		return;

	CTransform* pTransform = pMapToolObject->Get_TransformCom();
	if (nullptr == pTransform)
		return;

	_vector vPos = pTransform->Get_State(CTransform::STATE_POSITION);
	_vector vLook = XMVector3Normalize(pTransform->Get_State(CTransform::STATE_LOOK));
	_vector vUp = XMVector3Normalize(pTransform->Get_State(CTransform::STATE_UP));

	_vector vNewPos = vPos - vLook * 35.f + vUp * 30.f;

	pCamTransform->Set_State(CTransform::STATE_POSITION, vNewPos);
	pCamTransform->Look_At(vPos + vUp * 15.f);
}

void CMapToolHelper::DisableOtherGroups(_int* _pCurTxtGroup)
{
	for (auto& txtIndex : m_vecTxtIndices)
	{
		if (_pCurTxtGroup == txtIndex)
			continue;

		*txtIndex = -1;
	}
}

_bool CMapToolHelper::IsAnythingSelected()
{
	for (auto& txtIndex : m_vecTxtIndices)
	{
		if (*txtIndex != -1)
			return true;
	}

	return _bool();
}

void CMapToolHelper::ClearSearchFilter(_char* _filterBuf, _bool& bWasOpen)
{
	if (false == bWasOpen)
		return;

	_filterBuf[0] = '\0';
	bWasOpen = false;
}

void CMapToolHelper::FilterListBoxStrings(const _char* _filterBuf, vector<const _char*>& _vecNames, vector<string>& _vecTxts)
{
	if (_filterBuf[0] == '\0')
	{
		_vecNames.resize(_vecTxts.size());
		for (_int i = 0; i < _vecTxts.size(); ++i)
			_vecNames[i] = _vecTxts[i].c_str();
	}
	else
	{
		for (auto& objTxt : _vecTxts)
		{
			string strLower = string(objTxt);
			transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
			string strFilter = string(_filterBuf);
			transform(strFilter.begin(), strFilter.end(), strFilter.begin(), ::tolower);
			if (strLower.find(strFilter) != string::npos)
				_vecNames.push_back(objTxt.c_str());
		}
	}
}

void CMapToolHelper::Reset_MapShaderInfo()
{
	if (nullptr == m_pPickedObject)
		return;

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	if (nullptr == pModel)
		return;

	_int iNumMesh = pModel->Get_NumMeshes();
	s_vecPassIndices[s_iMapIndex].resize(iNumMesh);
	s_vecSamplingFactors[s_iMapIndex].resize(iNumMesh);
	fill(s_vecSamplingFactors[s_iMapIndex].begin(), s_vecSamplingFactors[s_iMapIndex].end(), 1.f);
}

void CMapToolHelper::Save_Octree()
{
	CBasicMap* pBasicMap = dynamic_cast<CBasicMap*>(m_pPickedObject);
	pBasicMap->Save_OctreeData(m_vecLevelName[s_iLevelIndex + LEVEL_INTRO]);
}

_bool CMapToolHelper::Save_Map(const string& _strLevel, vector<CGameObject*>& _vecMap)
{
	string strCustom = "_Map";
	string tempFileName = "temp_" + _strLevel + strCustom + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return false;
	}

	_uint iNumObjects = _vecMap.size();
	outputFile.write(reinterpret_cast<const char*>(&iNumObjects), sizeof(iNumObjects));

	for (auto& map : _vecMap)
	{
		CModel* pModel = dynamic_cast<CModel*>(map->Get_Component(TEXT("Com_Model")));
		CTransform* pTransform = dynamic_cast<CTransform*>(map->Get_Component(g_strTransformTag));

		string strModelName = pModel->Get_ModelInfo().strModelName;
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();

		_float3 vMin{ FLT_MAX, FLT_MAX, FLT_MAX }, vMax{ -FLT_MAX, -FLT_MAX , -FLT_MAX };
		pModel->Find_MinMax(vMin, vMax);

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));

		outputFile.write(reinterpret_cast<const char*>(&vMin), sizeof(vMin));
		outputFile.write(reinterpret_cast<const char*>(&vMax), sizeof(vMax));
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	if(false == RenameFile(_strLevel, tempFileName, strCustom))
		return false;
	
	return true;
}

_bool CMapToolHelper::Save_Triggers(const string& _strLevel, vector<CGameObject*>& _vecTriggers)
{
	string strCustom = "_Triggers";
	string tempFileName = "temp_" + _strLevel + strCustom + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return false;
	}

	_uint iNumObjects = _vecTriggers.size();
	outputFile.write(reinterpret_cast<const char*>(&iNumObjects), sizeof(iNumObjects));

	for (auto& trigger : _vecTriggers)
	{
		CModel* pModel = dynamic_cast<CModel*>(trigger->Get_Component(TEXT("Com_Model")));
		CTransform* pTransform = dynamic_cast<CTransform*>(trigger->Get_Component(g_strTransformTag));

		string strModelName = pModel->Get_ModelInfo().strModelName;
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();
		_uint iShaderVars = trigger->Get_ShaderVars();
		_float fRimWidth = trigger->Get_RimWidth();

		CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(trigger);
		_int iTriggerIndex = pMapToolObject->Get_TriggerIndex();
		_int triggerType = pMapToolObject->Get_TriggerType();
		_int iCamType = pMapToolObject->Get_CamType();
		_float fRadius = pMapToolObject->Get_Radius();

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));
		outputFile.write(reinterpret_cast<const char*>(&iShaderVars), sizeof(iShaderVars));
		outputFile.write(reinterpret_cast<const char*>(&fRimWidth), sizeof(fRimWidth));

		outputFile.write(reinterpret_cast<const char*>(&iTriggerIndex), sizeof(iTriggerIndex));
		outputFile.write(reinterpret_cast<const char*>(&triggerType), sizeof(triggerType));
		outputFile.write(reinterpret_cast<const char*>(&iCamType), sizeof(iCamType));
		outputFile.write(reinterpret_cast<const char*>(&fRadius), sizeof(fRadius));
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	if (false == RenameFile(_strLevel, tempFileName, strCustom))
		return false;

	return true;
}

_bool CMapToolHelper::Save_Monsters(const string& _strLevel, vector<CGameObject*>& _vecMonsters)
{
	string strCustom = "_Monsters";
	string tempFileName = "temp_" + _strLevel + strCustom + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return false;
	}

	_uint iNumObjects = _vecMonsters.size();
	outputFile.write(reinterpret_cast<const char*>(&iNumObjects), sizeof(iNumObjects));

	for (auto& monster : _vecMonsters)
	{
		CModel* pModel = dynamic_cast<CModel*>(monster->Get_Component(TEXT("Com_Model")));
		CTransform* pTransform = dynamic_cast<CTransform*>(monster->Get_Component(g_strTransformTag));

		CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(monster);
		_int iTriggerIndex = pMapToolObject->Get_TriggerIndex(); // Monster Enum
		map<_uint, _float3> rallyPoints = pMapToolObject->Get_RallyPoints();
		_uint iNumRallyPoints = rallyPoints.size();

		string strModelName = pModel->Get_ModelInfo().strModelName;
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();
		_uint iShaderVars = monster->Get_ShaderVars();
		_float fRimWidth = monster->Get_RimWidth();

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));
		outputFile.write(reinterpret_cast<const char*>(&iShaderVars), sizeof(iShaderVars));
		outputFile.write(reinterpret_cast<const char*>(&fRimWidth), sizeof(fRimWidth));

		outputFile.write(reinterpret_cast<const char*>(&iTriggerIndex), sizeof(iTriggerIndex)); 
		outputFile.write(reinterpret_cast<const char*>(&iNumRallyPoints), sizeof(iNumRallyPoints));

		for (_uint iRallyPointIdx = 0; iRallyPointIdx < iNumRallyPoints; iRallyPointIdx++)
		{
			auto mapIter = rallyPoints.find(iRallyPointIdx);
			if (mapIter == rallyPoints.end())
				continue;

			_float3 vPos = mapIter->second;
			outputFile.write(reinterpret_cast<const char*>(&vPos), sizeof(vPos));
		}
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	if (false == RenameFile(_strLevel, tempFileName, strCustom))
		return false;

	return true;
}

_bool CMapToolHelper::Save_Decos(const string& _strLevel, vector<CGameObject*>& _vecDecos)
{
	string strCustom = "_DecoObjs";
	string tempFileName = "temp_" + _strLevel + strCustom + ".txt";

	ofstream outputFile(tempFileName, ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return false;
	}

	vector<pair<string, _float4x4>> vecAnimDecos;
	vector<pair<string, _float4x4>> vecNonAnimDecos;

	_uint iNumObjects = _vecDecos.size();
	outputFile.write(reinterpret_cast<const char*>(&iNumObjects), sizeof(iNumObjects));

	for (auto& obj : _vecDecos)
	{
		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		CTransform* pTransform = dynamic_cast<CTransform*>(obj->Get_Component(g_strTransformTag));
		string strModelName = pModel->Get_ModelInfo().strModelName;

		CMapToolObject* pMapToolObj = dynamic_cast<CMapToolObject*>(obj);
		_uint iMapObjType{};
		if (m_setAnimDecos.end() != m_setAnimDecos.find(strModelName))
			iMapObjType = CMapToolObject::MAPOBJ_ANIM;
		else if (m_setActorDecos.end() != m_setActorDecos.find(strModelName))
			iMapObjType = CMapToolObject::MAPOBJ_ACTOR;
		else
			iMapObjType = CMapToolObject::MAPOBJ_NONCOL;

		outputFile.write(reinterpret_cast<const char*>(&iMapObjType), sizeof(iMapObjType));

		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();
		_uint iShaderVars = obj->Get_ShaderVars();
		_float fRimWidth = obj->Get_RimWidth();
		_uint iPassIndex = static_cast<_uint>(pMapToolObj->Get_PassIndex());
		if (true == IsTree(strModelName))
			iPassIndex = MODEL_NEARCLIP;
		else
			iPassIndex = 0;

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));
		outputFile.write(reinterpret_cast<const char*>(&iShaderVars), sizeof(iShaderVars));
		outputFile.write(reinterpret_cast<const char*>(&fRimWidth), sizeof(fRimWidth));
		outputFile.write(reinterpret_cast<const char*>(&iPassIndex), sizeof(iPassIndex));
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	if (false == RenameFile(_strLevel, tempFileName, strCustom))
		return false;

	return true;
}

_bool CMapToolHelper::Save_Items(const string& _strLevel, vector<CGameObject*>& _vecItems)
{
	string strCustom = "_Items";
	string tempFileName = "temp_" + _strLevel + strCustom + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return false;
	}

	_uint iNumObjects = _vecItems.size();
	outputFile.write(reinterpret_cast<const char*>(&iNumObjects), sizeof(iNumObjects));

	for (auto& item : _vecItems)
	{
		CModel* pModel = dynamic_cast<CModel*>(item->Get_Component(TEXT("Com_Model")));
		CTransform* pTransform = dynamic_cast<CTransform*>(item->Get_Component(g_strTransformTag));

		string strModelName = pModel->Get_ModelInfo().strModelName;
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();
		_uint iShaderVars = item->Get_ShaderVars();
		_float fRimWidth = item->Get_RimWidth();

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));
		outputFile.write(reinterpret_cast<const char*>(&iShaderVars), sizeof(iShaderVars));
		outputFile.write(reinterpret_cast<const char*>(&fRimWidth), sizeof(fRimWidth));
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	if (false == RenameFile(_strLevel, tempFileName, strCustom))
		return false;

	return true;
}

_bool CMapToolHelper::Save_Kickables(const string& _strLevel, vector<CGameObject*>& _vecItems)
{
	string strCustom = "_Kickables";
	string tempFileName = "temp_" + _strLevel + strCustom + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return false;
	}

	_uint iNumObjects = _vecItems.size();
	outputFile.write(reinterpret_cast<const char*>(&iNumObjects), sizeof(iNumObjects));

	for (auto& item : _vecItems)
	{
		CModel* pModel = dynamic_cast<CModel*>(item->Get_Component(TEXT("Com_Model")));
		CTransform* pTransform = dynamic_cast<CTransform*>(item->Get_Component(g_strTransformTag));

		string strModelName = pModel->Get_ModelInfo().strModelName;
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();
		_uint iShaderVars = item->Get_ShaderVars();
		_float fRimWidth = item->Get_RimWidth();

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));
		outputFile.write(reinterpret_cast<const char*>(&iShaderVars), sizeof(iShaderVars));
		outputFile.write(reinterpret_cast<const char*>(&fRimWidth), sizeof(fRimWidth));
	}

	outputFile.close();

	if (!outputFile)
	{
		wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrError.c_str());
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	if (false == RenameFile(_strLevel, tempFileName, strCustom))
		return false;

	return true;
}

void CMapToolHelper::Load_Map(const string& _strLevel)
{
	string strFileName = "../../../objects_txt/" + _strLevel + "_Map.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strLevel) + TEXT("_Map.txt");
		MSG_BOX(wstrError.c_str());
		return;
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

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tMapDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return;
		}
	}

	fileInput.close();
}

void CMapToolHelper::Load_Triggers(const string& _strLevel)
{
	string strFileName = "../../../objects_txt/" + _strLevel + "_Triggers.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strLevel) + TEXT("_Triggers.txt");
		MSG_BOX(wstrError.c_str());
		return;
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

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		//matWorld._41 = matWorld._41 - 200.f;
		//matWorld._43 = matWorld._43 + 1200.f;
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		fileInput.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
		fileInput.read(reinterpret_cast<char*>(&triggerType), sizeof(triggerType));
		fileInput.read(reinterpret_cast<char*>(&iCamType), sizeof(iCamType));
		fileInput.read(reinterpret_cast<char*>(&fRadius), sizeof(fRadius));

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;
		tDesc.iTriggerIndex = iTriggerIndex;
		tDesc.iTriggerType = triggerType;
		tDesc.iCamType = iCamType;
		tDesc.fRadius = fRadius;

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return;
		}
	}

	fileInput.close();
}

void CMapToolHelper::Load_Monsters(const string& _strLevel)
{
	string strFileName = "../../../objects_txt/" + _strLevel + "_Monsters.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strLevel) + TEXT("_Monsters.txt");
		MSG_BOX(wstrError.c_str());
		return;
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

	wstring wstrGameObjectTag = TEXT("MapToolObject");

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

		_float3 vRallyPointPos{};

		for (_uint iRallyPointIdx = 0; iRallyPointIdx < iNumRallyPoints; iRallyPointIdx++)
		{
			fileInput.read(reinterpret_cast<char*>(&vRallyPointPos), sizeof(vRallyPointPos));

			CMapToolObject::MAPTOOLOBJECT_DESC tRallyPointDesc{};
			tRallyPointDesc.wstrModelName = TEXT("RallyPoint");
			tRallyPointDesc.matWorld._41 = vRallyPointPos.x;
			tRallyPointDesc.matWorld._42 = vRallyPointPos.y;
			tRallyPointDesc.matWorld._43 = vRallyPointPos.z;
			tRallyPointDesc.iTriggerIndex = iRallyPointIdx;
			tRallyPointDesc.strConnectedMonster = strModelName;
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tRallyPointDesc)))
			{
				MSG_BOX(TEXT("Failed to Create : RallyPoint"));
				fileInput.close();
				return;
			}
		}

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;
		tDesc.iTriggerIndex = iTriggerIndex; // Monster Enum

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return;
		}
	}

	fileInput.close();
}

void CMapToolHelper::Load_Decos(const string& _strLevel)
{
	string strFileName = "../../../objects_txt/" + _strLevel + "_DecoObjs.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strLevel) + TEXT("_DecoObjs.txt");
		MSG_BOX(wstrError.c_str());
		return;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	_uint iMapObjType{};
	string strModelName;
	_float4x4 matWorld{};
	_uint iStrLength{};
	_uint iShaderVars{};
	_float fRimWidth{};
	_uint iPassIndex{};
	wstring wstrGameObjectTag = TEXT("MapToolObject");

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iMapObjType), sizeof(iMapObjType));
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));
		fileInput.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));

		/*if ("GsRubbleA" == strModelName || "GsRubbleB" == strModelName || "GsRubbleC" == strModelName ||
			"GsTireAL" == strModelName || "GsTireBL" == strModelName || "GsTireCL" == strModelName)
			continue;*/

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.eMapObjType = CMapToolObject::TYPE_MAPOBJ(iMapObjType);
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;
		tDesc.iPassIndex = iPassIndex;

		auto mapIter = m_mapBlendDecoInfos.find(strModelName);
		if (mapIter != m_mapBlendDecoInfos.end())
		{
			tDesc.setBlendMeshIndices = mapIter->second;
		}

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return;
		}
	}

	fileInput.close();
}

void CMapToolHelper::Load_Items(const string& _strLevel)
{
	string strFileName = "../../../objects_txt/" + _strLevel + "_Items.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strLevel) + TEXT("_Items.txt");
		MSG_BOX(wstrError.c_str());
		return;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	string strModelName;
	_float4x4 matWorld{};
	_uint iStrLength{};
	_uint iShaderVars{};
	_float fRimWidth{};
	wstring wstrGameObjectTag = TEXT("MapToolObject");

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return;
		}
	}

	fileInput.close();
}

void CMapToolHelper::Load_Kickables(const string& _strLevel)
{
	string strFileName = "../../../objects_txt/" + _strLevel + "_Kickables.txt";

	ifstream fileInput(strFileName, ios::binary);
	if (fileInput.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strLevel) + TEXT("_Kickables.txt");
		MSG_BOX(wstrError.c_str());
		return;
	}

	_uint iNumObjects{};
	fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

	string strModelName;
	_float4x4 matWorld{};
	_uint iStrLength{};
	_uint iShaderVars{};
	_float fRimWidth{};
	wstring wstrGameObjectTag = TEXT("MapToolObject");

	for (_uint i = 0; i < iNumObjects; i++)
	{
		fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileInput.read(&strModelName[0], iStrLength);
		fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
		fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		tDesc.iShaderVars = iShaderVars;
		tDesc.fRimWidth = fRimWidth;

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
		{
			wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
			MSG_BOX(wstrErrorMsg.c_str());
			fileInput.close();
			return;
		}
	}

	fileInput.close();
}

void CMapToolHelper::RegisterRallyPoints(list<CGameObject*>* _pObjList)
{
	vector<CMapToolObject*> vecRallyingMonsterPtrs;
	vector<CMapToolObject*> vecRallyPoints;

	for (auto& obj : *_pObjList)
	{
		if (nullptr == obj)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		string strModelName = pModel->Get_ModelName();
		if (true == IsRallyingMonster(strModelName))
			vecRallyingMonsterPtrs.push_back(dynamic_cast<CMapToolObject*>(obj));
		else if ("RallyPoint" == strModelName)
			vecRallyPoints.push_back(dynamic_cast<CMapToolObject*>(obj));
	}

	if (vecRallyingMonsterPtrs.empty() || vecRallyPoints.empty())
		return;

	for (auto& rallyPoint : vecRallyPoints)
	{
		if (nullptr == rallyPoint)
			continue;

		_float fShortest = { FLT_MAX };
		CMapToolObject* pNearestMonster = { nullptr };

		CTransform* pTransform = rallyPoint->Get_TransformCom();
		_vector vPos = pTransform->Get_State_Vector(CTransform::STATE_POSITION);

		for (auto& monster : vecRallyingMonsterPtrs)
		{
			if (nullptr == monster)
				continue;

			CModel* pMonsterModel = dynamic_cast<CModel*>(monster->Get_Component(TEXT("Com_Model")));
			string strMonsterName = pMonsterModel->Get_ModelName();

			if (rallyPoint->Get_ConnectedMonster() != strMonsterName)
				continue;

			CTransform* pMonsterTransform = monster->Get_TransformCom();
			_vector vMonsterPos = pMonsterTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_float fDis = XMVectorGetX(XMVector3Length(vPos - vMonsterPos));

			if (fDis < fShortest)
			{
				fShortest = fDis;
				pNearestMonster = monster;
			}
		}

		if (nullptr == pNearestMonster)
			continue;

		_uint iRallyPointIndex = static_cast<_uint>(rallyPoint->Get_TriggerIndex());
		_float3 vFloatPos = vPos;

		pNearestMonster->Emplace_RallyPoint(iRallyPointIndex, vFloatPos);
	}
}

void CMapToolHelper::WriteLocalizedAnimMapDecos(vector<pair<string, _float4x4>>& _vecAnimDecos)
{
	if (_vecAnimDecos.empty())
		return;

	for (auto& Name_Matrix : _vecAnimDecos)
	{
		string strPath = "../../../model_txt/MapDeco/Anim/" + Name_Matrix.first + ".txt";
		ifstream inputFile(strPath, ios::binary);
		if (false == inputFile.is_open()) {
			MSG_BOX(TEXT("원본파일 열기실패"));
			return;
		}

		string strLocalized = "../../../model_txt/OptimizedMapDecos/Anim/" + Name_Matrix.first + ".txt";
		ofstream outputFile(strLocalized, ios::binary | ios::trunc); // 같은 이름을 가진 파일 있을 경우 삭제
		if (!outputFile.is_open()) {
			MSG_BOX(TEXT("새 파일 열기실패"));
			return;
		}

		//Bones

		_uint iNumBones = {};

		inputFile.read(reinterpret_cast<char*>(&iNumBones), sizeof(iNumBones));
		outputFile.write(reinterpret_cast<const char*>(&iNumBones), sizeof(iNumBones));

		for (_uint i = 0; i < iNumBones; i++)
		{
			constexpr _uint iBytesToCopy = sizeof(_int) + MAX_PATH + sizeof(_float4x4);
			_char copyBuf[iBytesToCopy];
			inputFile.read(copyBuf, iBytesToCopy);
			outputFile.write(copyBuf, iBytesToCopy);
		}

		_uint iNumMeshes{};
		inputFile.read(reinterpret_cast<char*>(&iNumMeshes), sizeof(iNumMeshes));
		outputFile.write(reinterpret_cast<const char*>(&iNumMeshes), sizeof(iNumMeshes));

		_matrix matWorld = XMLoadFloat4x4(&Name_Matrix.second);

		for (_uint i = 0; i < iNumMeshes; i++)
		{
			_char szName[MAX_PATH] = {};
			_uint iMaterialIndex{};
			_uint iNumVertices{};
			_uint iFaces{};
			inputFile.read(reinterpret_cast<char*>(&szName), sizeof(szName));
			inputFile.read(reinterpret_cast<char*>(&iMaterialIndex), sizeof(iMaterialIndex));
			inputFile.read(reinterpret_cast<char*>(&iNumVertices), sizeof(iNumVertices));
			inputFile.read(reinterpret_cast<char*>(&iFaces), sizeof(iFaces));
			outputFile.write(szName, MAX_PATH);
			outputFile.write(reinterpret_cast<const char*>(&iMaterialIndex), sizeof(iMaterialIndex));
			outputFile.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(iNumVertices));
			outputFile.write(reinterpret_cast<const char*>(&iFaces), sizeof(iFaces));

			_uint iNumBones{};
			inputFile.read(reinterpret_cast<char*>(&iNumBones), sizeof(iNumBones));
			outputFile.write(reinterpret_cast<const char*>(&iNumBones), sizeof(iNumBones));

			_float4x4 matTransformation{};
			for (_uint j = 0; j < iNumBones; j++)
			{
				constexpr _uint iBoneBytes = MAX_PATH;
				_char copyBuf0[iBoneBytes];
				inputFile.read(copyBuf0, iBoneBytes);
				outputFile.write(copyBuf0, iBoneBytes);

				inputFile.read(reinterpret_cast<char*>(&matTransformation), sizeof(_float4x4));

				_matrix transformationMatrix = XMLoadFloat4x4(&matTransformation);
				_matrix matNewTransformation = transformationMatrix * matWorld;
				_float4x4 matResult{};
				XMStoreFloat4x4(&matResult, matNewTransformation);

				outputFile.write(reinterpret_cast<const char*>(&matResult), sizeof(_float4x4));
			}

			for (_uint j = 0; j < iNumVertices; j++)
			{
				_float3 vPos{};
				inputFile.read(reinterpret_cast<char*>(&vPos), sizeof(vPos));

				_vector vTempPos = ::XMLoadFloat3(&vPos);
				_vector vResult = XMVector3TransformCoord(vTempPos, matWorld);
				::XMStoreFloat3(&vPos, vResult);
				outputFile.write(reinterpret_cast<const char*>(&vPos), sizeof(vPos));

				constexpr _uint iAnimMeshBytes = sizeof(_float3) + sizeof(_float2) + sizeof(_float3) + sizeof(XMUINT4) + sizeof(_float4) ;
				_char copyBuf1[iAnimMeshBytes];
				inputFile.read(copyBuf1, iAnimMeshBytes);
				outputFile.write(copyBuf1, iAnimMeshBytes);
			}

			for (_uint k = 0; k < iFaces; k++)
			{
				_char copyBuf2[sizeof(_uint) * 3];
				inputFile.read(copyBuf2, sizeof(_uint) * 3);
				outputFile.write(copyBuf2, sizeof(_uint) * 3);
			}
		}

		outputFile << inputFile.rdbuf();
		inputFile.close();
		outputFile.close();
	}
}

void CMapToolHelper::WriteLocalizedNonAnimMapDecos(vector<pair<string, _float4x4>>& _vecNonAnimDecos)
{
	if (_vecNonAnimDecos.empty())
		return;

	for (auto& nameWorldMat : _vecNonAnimDecos)
	{
		string strPath = "../../../model_txt/MapDeco/NonAnim/" + nameWorldMat.first + ".txt";

		ifstream inputFile(strPath, ios::binary);
		if (false == inputFile.is_open()) {
			MSG_BOX(TEXT("원본파일 열기실패"));
			return;
		}

		string strLocalized = "../../../model_txt/OptimizedMapDecos/NonAnim/" + nameWorldMat.first + ".txt";
		ofstream outputFile(strLocalized, ios::binary | ios::trunc);
		if (!outputFile.is_open()) {
			MSG_BOX(TEXT("새 파일 열기실패"));
			return;
		}

		_uint iNumMeshes{};
		inputFile.read(reinterpret_cast<char*>(&iNumMeshes), sizeof(iNumMeshes));
		outputFile.write(reinterpret_cast<const char*>(&iNumMeshes), sizeof(iNumMeshes));

		_matrix matWorld = XMLoadFloat4x4(&nameWorldMat.second);

		for (_uint i = 0; i < iNumMeshes; i++)
		{
			_char szName[MAX_PATH] = {};
			_uint iMaterialIndex{};
			_uint iNumVertices{};
			_uint iFaces{};
			inputFile.read(reinterpret_cast<char*>(&szName), sizeof(szName));
			inputFile.read(reinterpret_cast<char*>(&iMaterialIndex), sizeof(iMaterialIndex));
			inputFile.read(reinterpret_cast<char*>(&iNumVertices), sizeof(iNumVertices));
			inputFile.read(reinterpret_cast<char*>(&iFaces), sizeof(iFaces));
			outputFile.write(szName, MAX_PATH);
			outputFile.write(reinterpret_cast<const char*>(&iMaterialIndex), sizeof(iMaterialIndex));
			outputFile.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(iNumVertices));
			outputFile.write(reinterpret_cast<const char*>(&iFaces), sizeof(iFaces));

			for (_uint j = 0; j < iNumVertices; j++)
			{
				_float3 vPos{};
				inputFile.read(reinterpret_cast<char*>(&vPos), sizeof(vPos));

				_vector vTempPos = ::XMLoadFloat3(&vPos);
				_vector vResult = XMVector3TransformCoord(vTempPos, matWorld);
				::XMStoreFloat3(&vPos, vResult);
				outputFile.write(reinterpret_cast<const char*>(&vPos), sizeof(vPos));

				constexpr _uint iBytesToCopy = sizeof(_float3) + sizeof(_float2) + sizeof(_float3);
				_char copyBuf[iBytesToCopy];
				inputFile.read(copyBuf, iBytesToCopy);
				outputFile.write(copyBuf, iBytesToCopy);
			}

			for (_uint k = 0; k < iFaces; k++)
			{
				_char copyBuf[sizeof(_uint) * 3];
				inputFile.read(copyBuf, sizeof(_uint) * 3);
				outputFile.write(copyBuf, sizeof(_uint) * 3);
			}
		}

		outputFile << inputFile.rdbuf();
		inputFile.close();
		outputFile.close();
	}
}

_bool CMapToolHelper::Save_BlendDecoInfos()
{
	for (auto& blendDecoInfoPair : m_mapBlendDecoInfos)
	{
		string strModelName = blendDecoInfoPair.first;
		string strPath = "../../../objects_txt/BlendDecoInfo/" + strModelName + "_BlendMeshes.txt";
		ofstream outputFile(strPath, ios::binary | ios::trunc);
		if (!outputFile.is_open()) {
			MSG_BOX(TEXT("Failed to Open : BlendMeshes.txt"));
			return false;
		}

		_bool bStaticActor = false;
		if (m_setActorDecos.end() != m_setActorDecos.find(strModelName))
			bStaticActor = true;

		outputFile.write(reinterpret_cast<const char*>(&bStaticActor), sizeof(bStaticActor));

		_uint iNumBlendMeshes = blendDecoInfoPair.second.size();
		outputFile.write(reinterpret_cast<const char*>(&iNumBlendMeshes), sizeof(iNumBlendMeshes));

		for (auto& blendMeshIndex : blendDecoInfoPair.second)
		{
			_uint iBlendMeshIndex = blendMeshIndex;
			outputFile.write(reinterpret_cast<const char*>(&iBlendMeshIndex), sizeof(iBlendMeshIndex));
		}
		outputFile.close();
	}
	
	return true;
}

void CMapToolHelper::TraverseBlendDecoInfoTxts()
{
	string strPath = "../../../objects_txt/BlendDecoInfo/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();
			string strFileName = strFilePath.substr(0, strFilePath.length() - 4);

			Load_BlendDecoInfo(strPath, strFileName);
		}
		++dir_iter;
	}
}

void CMapToolHelper::Load_BlendDecoInfo(const string& _strFolderPath, const string& _strFileName)
{
	string strPath = _strFolderPath + _strFileName;

	string strModelName;
	string::size_type pos = _strFileName.find('_');
	if (pos != string::npos)
		strModelName = _strFileName.substr(0, pos);
	else
		return;

	strPath += ".txt";

	ifstream inputFile(strPath, ios::binary);
	if (inputFile.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strFileName) + TEXT(".txt");
		MSG_BOX(wstrError.c_str());
		return;
	}

	_bool bStaticActor{};
	_uint iNumBlendMeshes{}, iBlendMeshIndex{};
	unordered_set<_uint> setBlendMeshIndices;

	inputFile.read(reinterpret_cast<char*>(&bStaticActor), sizeof(bStaticActor));
	inputFile.read(reinterpret_cast<char*>(&iNumBlendMeshes), sizeof(iNumBlendMeshes));

	for (_uint i = 0; i < iNumBlendMeshes; i++)
	{
		inputFile.read(reinterpret_cast<char*>(&iBlendMeshIndex), sizeof(iBlendMeshIndex));
		setBlendMeshIndices.insert(iBlendMeshIndex);
	}

	inputFile.close();

	m_mapBlendDecoInfos.insert_or_assign(strModelName, setBlendMeshIndices);
}

CMapToolHelper* CMapToolHelper::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapToolHelper* pInstance = new CMapToolHelper(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMapToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CMapToolHelper::Clone(void* pArg)
{
	CMapToolHelper* pInstance = new CMapToolHelper(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CMapToolHelper"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMapToolHelper::Free()
{
	__super::Free();

	Safe_Release(m_pPickedObject);
}

