#include "..\Public\Object_Manager.h"
#include "GameObject.h"
#include "Layer.h"
#include "Utils.h"
#include "Model.h"

CObject_Manager::CObject_Manager()
{

}

const CComponent * CObject_Manager::Get_Component(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strComTag, _uint iIndex)
{
	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);

	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_Component(strComTag, iIndex);	
}

CGameObject* CObject_Manager::Get_GameObject(_uint iLevelIndex, const wstring& strLayerTag, _uint iIndex)
{
	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_GameObject(iIndex);
}

CGameObject* CObject_Manager::Get_GameObject(_uint iLevelIndex, const wstring& wstrLayerTag)
{
	CLayer* pLayer = Find_Layer(iLevelIndex, wstrLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_GameObject();
}

CGameObject* CObject_Manager::Get_GameObject_ByTag(_uint iLevelIndex, const wstring& strLayerTag, wstring _tag)
{
	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_GameObject_ByTag(_tag);
}

_uint CObject_Manager::Get_GameObject_Num(_uint _iLevelIndex, const wstring& _strLayerTag)
{
	//레이어 검색 후 오브젝트 개수 리턴
	CLayer* pLayer = Find_Layer(_iLevelIndex, _strLayerTag);
	if (nullptr == pLayer)
		return 0;

	return pLayer->Get_GameObject_Num();
}

HRESULT CObject_Manager::Initialize(_uint iNumLevels)
{
	m_iNumLevels = iNumLevels;

	m_pLayers = new map<const wstring, class CLayer*>[iNumLevels];

	return S_OK;
}

HRESULT CObject_Manager::Add_Prototype(const wstring & strPrototypeTag, CGameObject * pPrototype)
{
	if (nullptr != Find_Prototype(strPrototypeTag))
		return E_FAIL;

	pPrototype->Set_PrototypeTag(strPrototypeTag);

	m_Prototypes.emplace(strPrototypeTag, pPrototype);

	return S_OK;
}

HRESULT CObject_Manager::Add_Clone(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strPrototypeTag, void* pArg)
{
	/* 복제해야할 원형객체를 검색한다. */
	CGameObject*	pPrototype = Find_Prototype(strPrototypeTag);
	CHECK_NULLPTR(pPrototype);

	CGameObject*	pGameObject = pPrototype->Clone(pArg);
	CHECK_NULLPTR(pGameObject);

	/* 복제한 사본객체를 추가해야할 레이어를 찾는다.*/
	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);

	/* 레이어가 없었다면 만들어서 객체를 추가하고 만든 레이어를 다시 맵에 추가해준다. */
	if (nullptr == pLayer)
	{
		pLayer = CLayer::Create();
		CHECK_NULLPTR(pLayer);
		pLayer->Add_GameObject(pGameObject);

		m_pLayers[iLevelIndex].emplace(strLayerTag, pLayer);
	}
	/* 추가하려고하느 ㄴ레이어가 이미 있었다.*/
	else	
		pLayer->Add_GameObject(pGameObject);

	m_mapCloneObjs.emplace(pGameObject, strPrototypeTag);

	return S_OK;
}

CGameObject * CObject_Manager::Clone_GameObject(const wstring & strPrototypeTag, void * pArg)
{
	/* 복제해야할 원형객체를 검색한다. */
	CGameObject*	pPrototype = Find_Prototype(strPrototypeTag);
	CHECK_NULLPTR(pPrototype);

	CGameObject*	pGameObject = pPrototype->Clone(pArg);
	CHECK_NULLPTR(pGameObject);

	return pGameObject; 
}

void CObject_Manager::Tick(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			/* 필요한 위치의 갱신작어블 수행한다. */
			Pair.second->Tick(fTimeDelta);
		}		
	}
}

void CObject_Manager::Late_Tick(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			/* 갱신된 위치를 활용하여 추가적인 기능을 수행하낟. */
			Pair.second->Late_Tick(fTimeDelta);
		}
	}

#ifdef _DEBUG
	IMGUI_Tick();
#endif
}

void CObject_Manager::Clear(_uint iLevelIndex)
{
	for (auto& Pair : m_pLayers[iLevelIndex])
	{
		Safe_Release(Pair.second);
	}
	m_pLayers[iLevelIndex].clear();
}

void CObject_Manager::Clear_Layer(_uint iLevelIndex, const wstring& strLayerTag)
{
	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (pLayer == nullptr)
		return;
	list<CGameObject*> objList = pLayer->Get_GameObjectList();
	if (objList.empty())
		return;
	for (auto& obj : objList)
	{
		if (obj == nullptr)
			continue;
		obj->Set_Dead();
	}
}

list<CGameObject*>*		CObject_Manager::Get_List(_uint iLevelIndex, const wstring & strLayerTag)
{
	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);

	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_list();
}

static _char filter[MAX_PATH] = "";
#ifdef _DEBUG
/// <summary> 객체를 Clone할 당시에 자동으로 추가되어 관리되는 IMGUI 함수 </summary>
void CObject_Manager::IMGUI_Tick()
{
	// LEVEL_LOGO까지의 LEVEL에서는 IMGUI_Tick을 돌리지 않는다.
	if (m_iCurrentLevel <= 2) return;

	ImGui::Begin("MainImGuiEditor");
	ImGui::Text("FPS : "); ImGui::SameLine();
	ImGui::Text("%.2f", ImGui::GetIO().Framerate);
	ImGui::Separator();  ImGui::NewLine();

	if (m_iCurrentLevel != 7) // MapTool
	{
		for (auto& map : m_pLayers[m_iCurrentLevel])
		{
			auto ObjList = map.second->Get_GameObjectList();

			string LayerName = CUtils::WstrToStr(map.first);

			if (ImGui::TreeNode(LayerName.c_str()))
			{
				int index = 0;
				for (auto& obj : ObjList)
				{
					auto iter = m_mapCloneObjs.find(obj);
					if (iter == m_mapCloneObjs.end())
					{
						MSG_BOX(TEXT("오브젝트 매니저에서 IMGUI 못찾고있음!!"));
						continue;
					}

					const string& ProtoName = CUtils::WstrToStr(iter->second) + to_string(index);
					if (ImGui::TreeNode(ProtoName.c_str()))
					{
						obj->Render_IMGUI();
						ImGui::TreePop();
					}

					++index;
				}

				ImGui::TreePop();
			}
		}
	}
	else
	{
		for (auto& map : m_pLayers[m_iCurrentLevel])
		{
			auto ObjList = map.second->Get_GameObjectList();

			string LayerName = CUtils::WstrToStr(map.first);
			if(LayerName == "Layer_Parse")
				ImGui::InputText("##Filter", filter, IM_ARRAYSIZE(filter)); // 필터 입력받기

			if (ImGui::TreeNode(LayerName.c_str()))
			{
				_int index = 0;
				for (auto& obj : ObjList)
				{
					if (nullptr == obj)
						continue;

					auto iter = m_mapCloneObjs.find(obj);
					if (iter == m_mapCloneObjs.end())
					{
						MSG_BOX(TEXT("오브젝트 매니저에서 IMGUI 못찾고있음!!"));
						continue;
					}

					CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
					if (nullptr == pModel)
						continue;

					string strModelName = pModel->Get_ModelName() + "_" + to_string(index);
					string strLower = strModelName;
					transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);

					string strFilter = string(filter);
					transform(strFilter.begin(), strFilter.end(), strFilter.begin(), ::tolower);
					if (strLower.find(strFilter) != string::npos) {
						if (ImGui::TreeNode(strModelName.c_str())) {
							obj->Render_IMGUI();
							ImGui::TreePop();
						}
					}

					++index;
				}

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();

}
#endif

CGameObject * CObject_Manager::Find_Prototype(const wstring & strPrototypeTag)
{
	auto		iter = m_Prototypes.find(strPrototypeTag);

	if (iter == m_Prototypes.end())
		return nullptr;

	return iter->second;	
}

CLayer * CObject_Manager::Find_Layer(_uint iLevelIndex, const wstring & strLayerTag)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	auto		iter = m_pLayers[iLevelIndex].find(strLayerTag);

	if (iter == m_pLayers[iLevelIndex].end())
		return nullptr;

	return iter->second;
}

CObject_Manager * CObject_Manager::Create(_uint iNumLevels)
{
	CObject_Manager*		pInstance = new CObject_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX(TEXT("Failed To Created : CObject_Manager"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObject_Manager::Free()
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Safe_Release(Pair.second);
		m_pLayers[i].clear();		
	}

	Safe_Delete_Array(m_pLayers);

	for (auto& Pair : m_Prototypes)
		Safe_Release(Pair.second);

	m_Prototypes.clear();
}
