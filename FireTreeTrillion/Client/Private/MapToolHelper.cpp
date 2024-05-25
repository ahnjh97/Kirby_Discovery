#include "stdafx.h"
#include "MapToolHelper.h"
#include "Utils.h"
#include <iostream>
#include <filesystem>

using namespace filesystem;
static _int iNonAnimIdx = -1;
static _int iAnimIdx = -1;
static _int iLevelIndex = 0;

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
	Menu_Level();
	Menu_NonAnimModels();

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
	for (_int i = LEVEL_GAMEPLAY; i < LEVEL_GAMEPLAY + 1; i++)
	{
		if (ImGui::RadioButton(vecLevelName[i].c_str(), iLevelIndex == i - LEVEL_GAMEPLAY))
			iLevelIndex = i - LEVEL_GAMEPLAY; // 선택 시 실행할 로직 추가
	}
	if (ImGui::Button("Save", ImVec2(100, 40)))
		Save_Level();
	ImGui::SameLine();
	if (ImGui::Button("Load", ImVec2(100, 40)))
	{
		Load_Level();
		return;
	}
}

void CMapToolHelper::Menu_NonAnimModels()
{
	ImGui::SeparatorText("NonAnim");
	ImGui::SetNextItemWidth(200.0f);
	const _char** items2 = new const char* [m_vecNonAnimTxts.size()];
	for (size_t i = 0; i < m_vecNonAnimTxts.size(); ++i)
		items2[i] = m_vecNonAnimTxts[i].c_str();

	if (ImGui::ListBox("##NonAnim", &iNonAnimIdx, items2, static_cast<int>(m_vecNonAnimTxts.size()), 9))
	{
		m_strCurModel = m_vecNonAnimTxts[iNonAnimIdx];
		iAnimIdx = -1;
	}

	Safe_Delete_Array(items2);
}

void CMapToolHelper::Edit_Object()
{
	if (nullptr != m_pPickedObject)
	{
		CTransform* pTransform = dynamic_cast<CTransform*>(m_pPickedObject->Get_Component(g_strTransformTag));
		if (pTransform != nullptr)
		{
			ImGui::Begin("Guizmo");
			Safe_AddRef(pTransform);
			_float4x4 tempMatrix = pTransform->Get_WorldFloat4x4();
			m_pGameInstance->EditTransform(tempMatrix); // 선택한 모델의 월드행렬을 수정 
			pTransform->Set_WorldMatrix(tempMatrix);
			Safe_Release(pTransform);
			ImGui::End();
		}
	}
}

void CMapToolHelper::OnLeftClick()
{
	CGameObject* pPickedObject = Select_ModelByPicking();
	if (nullptr == pPickedObject)
		return;

	if (pPickedObject == m_pPickedObject)
		return;

	m_pPickedObject = pPickedObject;
}

void CMapToolHelper::OnRightClick()
{
	if (iAnimIdx == -1 && iNonAnimIdx == -1)
		return;

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

	if (!::XMVector3Equal(::XMLoadFloat3(&m_vPickPos), ::XMVectorSet(0.f, 0.f, 0.f, 0.f)))
	{
		_float4 vTemp(m_vPickPos.x, m_vPickPos.y, m_vPickPos.z, 1.0f); // 위치
		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		memcpy(&tempDesc.matWorld.m[CTransform::STATE_POSITION], &vTemp, sizeof(_float4));
		tempDesc.wstrModelName = CUtils::StrToWstr(m_vecNonAnimTxts[iNonAnimIdx]);

		wstring wstrPrototypeTag = TEXT("Prototype_GameObject_MapToolObject");
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), wstrPrototypeTag, &tempDesc)))
			return;
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

	string strLevel = vecLevelName[iLevelIndex + LEVEL_GAMEPLAY];
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

	string strLevel = vecLevelName[iLevelIndex + LEVEL_GAMEPLAY];
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
	while (!fileStream.eof()) 
	{
		_uint iStrLength;
		fileStream.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
		strModelName.resize(iStrLength);
		fileStream.read(&strModelName[0], iStrLength);
		fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4));

		if (fileStream.eof())
			break;

		GAMEOBJECT_DESC tDesc{};
		tDesc.wstrModelName = CUtils::StrToWstr(strModelName);
		tDesc.matWorld = matWorld;

		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_TOOL_MAP, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_MapToolObject"), &tDesc))) 
		{
			MSG_BOX(TEXT("Failed to Clone MapToolObject"));
			fileStream.close();
			return;
		}
	}
	fileStream.close();
}

CGameObject* CMapToolHelper::Select_ModelByPicking(const wstring& wstrLayerTag)
{
	vector<CGameObject*> vecPickedObjects;
	vector<_float4> vecPickPos;

	list<CGameObject*>* pObjectList = m_pGameInstance->Get_List(LEVEL_TOOL_MAP, wstrLayerTag);
	if (nullptr == pObjectList)
		return nullptr;

	if (pObjectList->empty())
		return nullptr;

	for (auto& object : *pObjectList)
	{
		if (nullptr == object)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(object->Get_Component(TEXT("Com_Model")));
		if (nullptr == pModel)
			continue;

		CTransform* pTransform = dynamic_cast<CTransform*>(object->Get_Component(g_strTransformTag));
		if (nullptr == pTransform)
			continue;

		_float4 vPickPos = pModel->Check_Meshes(pTransform);
		if (vPickPos.w != 0) // 피킹 성공
		{
			vecPickedObjects.push_back(object);
			vecPickPos.push_back(vPickPos);
		}
	}

	if (vecPickedObjects.empty())
		return nullptr;

	_float fShortest = { FLT_MAX };
	CGameObject* pResult = { nullptr };
	for (int i = 0; i < vecPickPos.size(); i++)
	{
		if (vecPickPos[i].w <= fShortest)
			pResult = vecPickedObjects[i];
	}

	return pResult;
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

