#include "stdafx.h"
#include "MapToolHelper.h"
#include "MapToolObject.h"
#include "BasicMap.h"
#include <filesystem>
#include "Utils.h"

using namespace filesystem;
static _int iNonAnimIdx = -1;
static _int iAnimIdx = -1;
static _int iLevelIndex = 0;

static const _char* triggerTypes[] = {"Camera", "Shader"};
static _int iTriggerType = -1;
static const _char* triggerIndices[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
									"11", "12", "13", "14", "15", "16", "17", "18", "19", "20" };
static _int iTriggerIdx = -1;

static _int iMapMeshIndex = -1;
static _int iSelectedMeshIndex = -1; 

static const _char* ShaderPasses[] = { "Blend X, NormalO", "Blend X, Normal X", "LightDepth", "Blend O, Normal O", "Blend O, Normal X"};
static vector<vector<_int>> vecPassIndices;
static vector<vector<_float>> vecSamplingFactors;
static _int iMapIndex = 0;
static _int iPickedMeshIndex = 0;

static _float fRadius = 0;

static const _char* camTypes[] = { "Front", "Rear"};
static _int iCamType = -1;

static _bool bHideTriggers = { false };

CMapToolHelper::CMapToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMapToolHelper::CMapToolHelper(const CMapToolHelper& rhs)
	: CGameObject(rhs)
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

	m_vecLevelName = { "Level_Static", "Level_Loading", "Level_Logo", // 두번째 줄에 실제 인게임 레벨 추가
			"Intro", "GamePlay",
			"Level_Tool_UI", "Level_Tool_FX", "Level_Tool_Anim", "Level_Tool_Map", "Level_End" };
	m_vecMapModelNames = { "Level0Stage1Step01", "Level1Stage1Step01"};
	vecPassIndices.resize(m_vecMapModelNames.size());
	vecSamplingFactors.resize(m_vecMapModelNames.size());

	SetUpTxtVectors(TYPE_ANIM);
	SetUpTxtVectors(TYPE_NONANIM);

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
	Menu_Level();
	Menu_NonAnimModels();
	Menu_TriggerInfo();
	Menu_MapShaderInfo();
	ImGui::End();

	if(m_pGameInstance->Get_KeyState(DIMKS_RBUTTON, KEY_DOWN))
		OnRightClick();

	if (!ImGui::IsAnyItemHovered() && m_pGameInstance->Get_KeyState(DIMKS_LBUTTON, KEY_DOWN) && !m_pGameInstance->Get_DIKeyState(DIK_LSHIFT, KEY_PRESS))
		OnLeftClick();

	if (m_pGameInstance->Get_DIKeyState(DIK_ESCAPE, KEY_DOWN))
		On_DIK_Escape();

	if (m_pGameInstance->Get_DIKeyState(DIK_DELETE, KEY_DOWN))
		On_DIK_Delete();

	Edit_Object();
}

HRESULT CMapToolHelper::Render()
{
	return S_OK;
}

void CMapToolHelper::Render_IMGUI()
{

}

void CMapToolHelper::SetUpTxtVectors(TYPE _eType)
{
	string strPath = "../../../model_txt/";
	if (TYPE_ANIM == _eType)
		strPath += "Anim/";
	else if (TYPE_NONANIM == _eType)
		strPath += "NonAnim/";

	directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
	directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

	while (dir_iter != end_iter) {
		if (is_regular_file(*dir_iter)) {
			string strFilePath = dir_iter->path().filename().string();

			if (TYPE_ANIM == _eType)
				m_vecAnimTxts.emplace_back(strFilePath.substr(0, strFilePath.length() - 4));
			else if (TYPE_NONANIM == _eType)
				m_vecNonAnimTxts.emplace_back(strFilePath.substr(0, strFilePath.length() - 4));
		}
		++dir_iter;
	}
}

void CMapToolHelper::Menu_Level()
{
	ImGui::SeparatorText("Level");
	for (_int i = LEVEL_INTRO; i <= LEVEL_GAMEPLAY; i++)
	{
		if (ImGui::RadioButton(m_vecLevelName[i].c_str(), iLevelIndex == i - LEVEL_INTRO)) {
			ImGui::OpenPopup("Example Popup");
			iLevelIndex = i - LEVEL_INTRO; 

			if (ImGui::BeginPopup("Example Popup"))
			{
				string strMsg = "Open " + m_vecLevelName[i] + "?";
				ImGui::Text(strMsg.c_str());
				if (ImGui::Button("Close"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		if (i % 2 == 1)
			ImGui::SameLine();
	}
	if (ImGui::Button("Save", ImVec2(100, 40)))
		Save_Level();
	ImGui::SameLine();
	if (ImGui::Button("Load", ImVec2(100, 40)))
		Load_Level();

	ImGui::SeparatorText("Options");
	if (ImGui::RadioButton("Hide Triggers", bHideTriggers)) {
		bHideTriggers = !bHideTriggers;
		HideTriggers(bHideTriggers);
	}
}

void CMapToolHelper::Menu_NonAnimModels()
{
	ImGui::SeparatorText("NonAnim");
	ImGui::SetNextItemWidth(200.0f);

	vector<const _char*> vecNonAnimModelNames(m_vecNonAnimTxts.size());
	for (_int i = 0; i < m_vecNonAnimTxts.size(); ++i)
		vecNonAnimModelNames[i] = m_vecNonAnimTxts[i].c_str();

	if (ImGui::ListBox("##NonAnim", &iNonAnimIdx, vecNonAnimModelNames.data(), m_vecNonAnimTxts.size(), 9))
	{
		m_strCurModel = m_vecNonAnimTxts[iNonAnimIdx];
		iAnimIdx = -1;
	}
}

void CMapToolHelper::Menu_TriggerInfo()
{
	if (nullptr == m_pPickedObject)
		return;

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	if (nullptr == pModel)
		return;

	string strModelName = pModel->Get_ModelInfo().strModelName;
	if (strModelName == "Camera" || strModelName == "Trigger" || strModelName == "Dummy")
	{
		CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(m_pPickedObject);
		iTriggerType = pMapToolObject->Get_TriggerType();
		iTriggerIdx = pMapToolObject->Get_TriggerIndex();

		ImGui::Begin(strModelName.c_str());
		if (strModelName == "Trigger" || strModelName == "Dummy") {
			ImGui::SetCursorPosX(37);
			ImGui::Text("TYPE");
			ImGui::SameLine();
			ImGui::SetCursorPosX(120);
		}
		else
			ImGui::SetCursorPosX(33);
		ImGui::Text("INDEX");


		if (strModelName == "Trigger")
		{
			ImGui::SetNextItemWidth(80);
			if (ImGui::Combo("##Type", &iTriggerType, triggerTypes, IM_ARRAYSIZE(triggerTypes)))
				pMapToolObject->Set_TriggerType(iTriggerType);
			ImGui::SameLine();
		}
		else if (strModelName == "Dummy")
		{
			ImGui::SetNextItemWidth(80);
			if (ImGui::Combo("##Type", &iCamType, camTypes, IM_ARRAYSIZE(camTypes)))
				pMapToolObject->Set_CamType(iCamType);
			ImGui::SameLine();
		}
		
		//-------------------------------------------
		ImGui::SetNextItemWidth(80);
		if (ImGui::Combo("##Index", &iTriggerIdx, triggerIndices, IM_ARRAYSIZE(triggerIndices))) {
			pMapToolObject->Set_TriggerIndex(iTriggerIdx);
		}

		if (strModelName == "Dummy")
			Menu_CamLerpInfo(pMapToolObject);

		ImGui::End();
	}
}

void CMapToolHelper::Menu_CamLerpInfo(CMapToolObject* _pMapToolObject)
{
	iCamType = _pMapToolObject->Get_CamType();
	fRadius = _pMapToolObject->Get_Radius();

	ImGui::Text("RADIUS");
	ImGui::SameLine();
	ImGui::SetCursorPosX(60);
	ImGui::SetNextItemWidth(90);
	if (ImGui::InputFloat("##Radius", &fRadius, 1.f, 1.f, "%.3f"))
		_pMapToolObject->Set_Radius(fRadius);
}

void CMapToolHelper::Menu_MapShaderInfo()
{
	if (nullptr == m_pPickedObject)
		return;

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	if (nullptr == pModel)
		return;

	string strModelName = pModel->Get_ModelInfo().strModelName;
	_int iIndex = Compute_MapIndex(strModelName);

	if (-1 == iIndex)
		return;

	_uint iNumMesh = pModel->Get_NumMeshes();

	if (vecPassIndices[iMapIndex].empty())
		vecPassIndices[iMapIndex].resize(iNumMesh);
	if (vecSamplingFactors[iMapIndex].empty()) {
		vecSamplingFactors[iMapIndex].resize(iNumMesh);
		fill(vecSamplingFactors[iMapIndex].begin(), vecSamplingFactors[iMapIndex].end(), 1.f);
	}

	vector<string> vecMeshNames(iNumMesh);
	for (_uint i = 0; i < iNumMesh; ++i)
		vecMeshNames[i] = pModel->Get_MeshName(i);

	vector<const _char*> vecMapMeshNames(iNumMesh);
	for (_uint i = 0; i < iNumMesh; ++i)
		vecMapMeshNames[i] = vecMeshNames[i].c_str();

	CBasicMap* pBasicMap = dynamic_cast<CBasicMap*>(m_pPickedObject);
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
		if (ImGui::Selectable(vecMapMeshNames[j], iSelectedMeshIndex == j, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(320,0)))
		{
			iSelectedMeshIndex = j;
			pBasicMap->Reset_Time(j);
		}
		ImGui::SameLine();
		//------------------------------
		ImGui::SetNextItemWidth(150);
		if (ImGui::Combo("##PassIndex", &vecPassIndices[iMapIndex][j], ShaderPasses, IM_ARRAYSIZE(ShaderPasses)))
		{
			pBasicMap->Set_PassIndex(j, vecPassIndices[iMapIndex][j]);
		}
		ImGui::SameLine();
		//------------------------------
		ImGui::SetNextItemWidth(100);
		if (ImGui::InputFloat("##SamplingFactor", &vecSamplingFactors[iMapIndex][j], 0.01f, 1.0f, "%.3f")) 
		{
			pBasicMap->Set_SamplingFactor(j, vecSamplingFactors[iMapIndex][j]);
		}
		ImGui::PopID();
	}
	ImGui::End();
}

void CMapToolHelper::Edit_Object()
{
	if (nullptr != m_pPickedObject)
	{
		CTransform* pTransform = dynamic_cast<CTransform*>(m_pPickedObject->Get_Component(g_strTransformTag));
		if (pTransform != nullptr)
		{
			ImGui::Begin("Guizmo");
			//Safe_AddRef(pTransform);
			_float4x4 tempMatrix = pTransform->Get_WorldFloat4x4();
			m_pGameInstance->EditTransform(tempMatrix); // 선택한 모델의 월드행렬을 수정 
			pTransform->Set_WorldMatrix(tempMatrix);
			//Safe_Release(pTransform);
			ImGui::End();
		}
	}
}

void CMapToolHelper::OnLeftClick()
{
	CGameObject* pPickedObject = Select_ModelByPicking();
	if (nullptr == pPickedObject)
		return;

	m_pPickedObject = pPickedObject;

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	string strModelName = pModel->Get_ModelInfo().strModelName;
	_uint iNumMeshes = pModel->Get_NumMeshes();

	_int iIndex = Compute_MapIndex(strModelName);

	if (-1 == iIndex) // 피킹한 객체가 맵이 아님
		return;

	CBasicMap* pBasicMap = dynamic_cast<CBasicMap*>(m_pPickedObject);
	pBasicMap->Reset_Time(iPickedMeshIndex);
	iSelectedMeshIndex = iPickedMeshIndex;
	iMapIndex = iIndex;

	if (vecPassIndices[iIndex].empty() || vecSamplingFactors[iIndex].empty())
	{
		vecPassIndices[iIndex].resize(iNumMeshes);
		vecSamplingFactors[iIndex].resize(iNumMeshes);

		Load_MapShaderInfo();
	}
}

void CMapToolHelper::OnRightClick()
{
	if (iAnimIdx == -1 && iNonAnimIdx == -1)
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
		tMapToolDesc.wstrModelName = CUtils::StrToWstr(m_vecNonAnimTxts[iNonAnimIdx]);

		if (Compute_MapIndex(m_vecNonAnimTxts[iNonAnimIdx]) == -1) // 맵이 아닐때
		{
			if (tMapToolDesc.wstrModelName == TEXT("BG1")) {
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

		iAnimIdx = iNonAnimIdx = -1;

		list<CGameObject*>* pObjList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));
		m_pPickedObject = pObjList->back();
	}
}

void CMapToolHelper::On_DIK_Escape()
{
	iAnimIdx = -1;
	iNonAnimIdx = -1;
	m_pPickedObject = nullptr;
}

void CMapToolHelper::On_DIK_Delete()
{
	if (m_pPickedObject != nullptr)
		m_pPickedObject->Set_Dead();
	m_pPickedObject = nullptr;
}

void CMapToolHelper::Save_Level()
{
	if (iLevelIndex < 0)
		return;

	string strLevel = m_vecLevelName[iLevelIndex + LEVEL_INTRO];
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
		_float4x4 matWorld = pTransform->Get_WorldMatrix();
		_uint iStrLength = strModelName.length();

		outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
		outputFile.write(strModelName.c_str(), iStrLength);
		outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));

		if ("Camera" == strModelName || "Trigger" == strModelName) {
			CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(object);

			if ("Trigger" == strModelName)
			{
				_int triggerType = pMapToolObject->Get_TriggerType();
				outputFile.write(reinterpret_cast<const char*>(&triggerType), sizeof(triggerType));
			}
			_int iTriggerIndex = pMapToolObject->Get_TriggerIndex();
			outputFile.write(reinterpret_cast<const char*>(&iTriggerIndex), sizeof(iTriggerIndex));
		}
		else if ("Dummy" == strModelName)
		{
			CMapToolObject* pMapToolObject = dynamic_cast<CMapToolObject*>(object);
			_int iTriggerIndex = pMapToolObject->Get_TriggerIndex();
			_int iCamType = pMapToolObject->Get_CamType();
			_float fRadius = pMapToolObject->Get_Radius();

			outputFile.write(reinterpret_cast<const char*>(&iTriggerIndex), sizeof(iTriggerIndex));
			outputFile.write(reinterpret_cast<const char*>(&iCamType), sizeof(iCamType));
			outputFile.write(reinterpret_cast<const char*>(&fRadius), sizeof(fRadius));
		}
		else if (-1 != Compute_MapIndex(strModelName)) // 맵인 경우
		{
			if (0 != strModelName.compare(strModelName.size() - 5, 5, "Blend")) // Blend맵이 아닌 경우
			{
				_float3 vMin{FLT_MAX, FLT_MAX, FLT_MAX}, vMax{-FLT_MAX, -FLT_MAX , -FLT_MAX };
				pModel->Find_MinMax(vMin, vMax);
				
				outputFile.write(reinterpret_cast<const char*>(&vMin), sizeof(vMin));
				outputFile.write(reinterpret_cast<const char*>(&vMax), sizeof(vMax));
			}
		}
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

	string fileName_Time = "../../../objects_txt/" + string(buffer) + "_" + strLevel + ".txt";
	string fileName = "../../../objects_txt/" + strLevel + ".txt";
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

	wstring wstrSaveMsg = CUtils::StrToWstr(strLevel) + TEXT(" Saved.");
	MSG_BOX(wstrSaveMsg.c_str());
}

void CMapToolHelper::Load_Level()
{
	On_DIK_Escape();
	On_DIK_Delete();
	m_pGameInstance->Clear_Layer(LEVEL_TOOL_MAP, TEXT("Layer_Parse"));

	string strLevel = m_vecLevelName[iLevelIndex + LEVEL_INTRO];
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

	while (!fileStream.eof()) 
	{
		_uint iStrLength;
		fileStream.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileStream.read(&strModelName[0], iStrLength);
		fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));

		if ("Camera" == strModelName || "Trigger" == strModelName) {
			if ("Trigger" == strModelName)
				fileStream.read(reinterpret_cast<char*>(&triggerType), sizeof(triggerType));

			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
		}
		else if ("Dummy" == strModelName)
		{
			fileStream.read(reinterpret_cast<char*>(&iTriggerIndex), sizeof(iTriggerIndex));
			fileStream.read(reinterpret_cast<char*>(&iCamType), sizeof(iCamType));
			fileStream.read(reinterpret_cast<char*>(&fRadius), sizeof(fRadius));
		}
		else if (-1 != Compute_MapIndex(strModelName)) // 맵이면
		{
			if (0 != strModelName.compare(strModelName.size() - 5, 5, "Blend")) // Blend맵이 아닌 경우
			{
				fileStream.read(reinterpret_cast<char*>(&vMin), sizeof(vMin));
				fileStream.read(reinterpret_cast<char*>(&vMax), sizeof(vMax));
			}
		}

		if (fileStream.eof())
			break;

		CMapToolObject::MAPTOOLOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;
		if ("Camera" == strModelName || "Trigger" == strModelName)
		{
			if("Trigger" == strModelName)
				tDesc.iTriggerType = triggerType;
			tDesc.iTriggerIndex = iTriggerIndex;
		}
		else if ("Dummy" == strModelName)
		{
			tDesc.iTriggerIndex = iTriggerIndex;
			tDesc.iCamType = iCamType;
			tDesc.fRadius = fRadius;
		}
		
		wstring wstrGameObjectTag;


		if (Compute_MapIndex(strModelName) != -1) // 맵인 경우
		{
			CBasicMap::MAP_DESC tMapDesc{};
			tMapDesc.wstrModelName = CUtils::StrToWstr(strModelName);
			tMapDesc.matWorld = matWorld;
			tMapDesc.vMin = vMin;
			tMapDesc.vMax = vMax;
			wstrGameObjectTag = TEXT("BasicMap");

			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tMapDesc)))
			{
				wstring wstrErrorMsg = TEXT("Failed to Clone MapToolObject") + wstrGameObjectTag;
				MSG_BOX(wstrErrorMsg.c_str());
				fileStream.close();
				return;
			}
		}	
		else
		{
			if ("BG1" == strModelName)
				wstrGameObjectTag = TEXT("BG");
			else
				wstrGameObjectTag = TEXT("MapToolObject");
			
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_") + wstrGameObjectTag, &tDesc)))
			{
				wstring wstrErrorMsg = TEXT("Failed to Clone: ") + wstrGameObjectTag;
				MSG_BOX(wstrErrorMsg.c_str());
				fileStream.close();
				return;
			}
		}
	}

	fileStream.close();
}

void CMapToolHelper::Save_MapShaderInfo()
{
	string tempFileName = "temp_" + m_vecMapModelNames[iMapIndex] + "_ShaderInfo.txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
		MSG_BOX(wstrErrorMsg.c_str());
		return;
	}

	if (vecPassIndices[iMapIndex].empty() || vecSamplingFactors[iMapIndex].empty())
		return;

	for (_int i = 0; i < vecPassIndices[iMapIndex].size(); i++)
	{
		outputFile.write(reinterpret_cast<const char*>(&vecPassIndices[iMapIndex][i]), sizeof(vecPassIndices[iMapIndex][i]));
		outputFile.write(reinterpret_cast<const char*>(&vecSamplingFactors[iMapIndex][i]), sizeof(vecSamplingFactors[iMapIndex][i]));
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

	string fileName_Time = "../../../objects_txt/" + string(buffer) + "_" + m_vecMapModelNames[iMapIndex] + "_ShaderInfo.txt";
	string fileName = "../../../objects_txt/" + m_vecMapModelNames[iMapIndex] + "_ShaderInfo.txt";
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

	wstring wstrSaveMsg = CUtils::StrToWstr(string(m_vecMapModelNames[iMapIndex] + "_ShaderInfo")) + TEXT(" Saved.");
	MSG_BOX(wstrSaveMsg.c_str());
}

void CMapToolHelper::Load_MapShaderInfo()
{
	if (vecPassIndices.empty() || vecSamplingFactors.empty())
		return;

	string strFileName = "../../../objects_txt/" + m_vecMapModelNames[iMapIndex] + "_ShaderInfo.txt";

	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(m_vecMapModelNames[iMapIndex]) + TEXT("_ShaderInfo.txt");
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

		vecPassIndices[iMapIndex][iCount] = iPassIndex;
		vecSamplingFactors[iMapIndex][iCount] = fSamplingFactor;
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
			iPickedMeshIndex = vecMeshIndex[i];
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

void CMapToolHelper::Reset_MapShaderInfo()
{
	if (nullptr == m_pPickedObject)
		return;

	CModel* pModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
	if (nullptr == pModel)
		return;

	_int iNumMesh = pModel->Get_NumMeshes();
	vecPassIndices[iMapIndex].resize(iNumMesh);
	vecSamplingFactors[iMapIndex].resize(iNumMesh);
	fill(vecSamplingFactors[iMapIndex].begin(), vecSamplingFactors[iMapIndex].end(), 1.f);
}

void CMapToolHelper::Save_Octree()
{

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

