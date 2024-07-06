#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CMapToolHelper final : public CGameObject
{
private:
	CMapToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapToolHelper(const CMapToolHelper& rhs);
	virtual ~CMapToolHelper() = default;

public:
	void Set_PickedObject(CGameObject* pGameObject, const string& strModelName)
	{
		Safe_Release(m_pPickedObject);
		m_pPickedObject = pGameObject;
		m_strCurModel = strModelName;
		Safe_AddRef(m_pPickedObject);
	}

public:
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg = nullptr)	override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()						override {};
#endif

private:
	void	ReadMapDecoTxts();
	void	ReadMapObjTxts();
	void	ReadMonsterTxts();
	
	//마을 오브젝트를 로드함.
	void	ReadTownDecoTxts();
	void	ReadLabDecoTxts();
	void	ReadParkDecoTxts();

	// ImGui
	void	Menu_Level();
	void	Menu_NonAnimModels();
	void	Menu_TriggerInfo();
	void	Menu_CamLerpInfo(class CMapToolObject* _pMapToolObject);
	void	Menu_MapShaderInfo();
	void	Menu_MonsterInfo();
	void	Menu_RallyPointInfo();
	void	Menu_BlendDecoInfo();
	void	Edit_Object();

	// On Key/Mouse Input 
	void	OnLeftClick();
	void	OnRightClick();
	void	On_DIK_Escape();
	void	On_DIK_Delete();

	// File I/O
	void	Save_Level();
	void	Load_Level();
	void	Save_MapShaderInfo();
	void	Load_MapShaderInfo();
	void	Reset_MapShaderInfo();
	void	Save_Octree();

	_bool	Save_Map(const string& _strLevel, vector<CGameObject*>& _vecMap);
	_bool	Save_Triggers(const string& _strLevel, vector<CGameObject*>& _vecTriggers);
	_bool	Save_Monsters(const string& _strLevel, vector<CGameObject*>& _vecMonsters);
	_bool	Save_Decos(const string& _strLevel, vector<CGameObject*>& _vecDecos);
	_bool	Save_Items(const string& _strLevel, vector<CGameObject*>& _vecItems);
	_bool	Save_Kickables(const string& _strLevel, vector<CGameObject*>& _vecItems);
	
	void	Load_Map(const string& _strLevel);
	void	Load_Triggers(const string& _strLevel);
	void	Load_Monsters(const string& _strLevel);
	void	Load_Decos(const string& _strLevel);
	void	Load_Items(const string& _strLevel);
	void	Load_Kickables(const string& _strLevel);
	
	void	RegisterRallyPoints(list<CGameObject*>* _pObjList);
	void	WriteLocalizedAnimMapDecos(vector<pair<string, _float4x4>>& _vecAnimDecos);
	void	WriteLocalizedNonAnimMapDecos(vector<pair<string, _float4x4>>& _vecNonAnimDecos);

	_bool	Save_BlendDecoInfos();
	void	TraverseBlendDecoInfoTxts();
	void	Load_BlendDecoInfo(const string& _strFolderPath, const string& _strFileName);
	
	// Picking
	CGameObject* Select_ModelByPicking(const wstring& wstrLayerTag = TEXT("Layer_Parse"));
	_int Compute_MapIndex(const string& _strModelName);
	_int Compute_RallyingMonsterIndex(const string& _strModelName);

	_bool IsMap(const string& _strModelName);
	_bool IsTrigger(const string& _strModelName);
	_bool IsMonster(const string& _strModelName);
	_bool IsDeco(const string& _strModelName);
	_bool IsItem(const string& _strModelName);
	_bool IsRallyingMonster(const string& _strModelName);
	_bool IsKickable(const string& _strModelName);
	_bool IsTree(const string& _strModelName);
	_bool IsBlendDeco(const string& _strModelName);

	_bool RenameFile(const string& _strLevel, const string& _tempFileName, const string& _strCustom);

	// Options
	void HideTriggers(_bool bHideTriggers);
	void HideGrid(_bool bHideGrid);
	void HideDecos(_bool bHideDecos);
	void HideWalls(_bool bHideWalls);

	_bool ExcludeModel(string& _strModelName);
	void MoveToCam();
	void DisableOtherGroups(_int* _pCurTxtGroup);
	_bool IsAnythingSelected();

	void ClearSearchFilter(_char* _filterBuf, _bool& bWasOpen);
	void FilterListBoxStrings(const _char* _filterBuf, vector<const _char*>& _vecNames, vector<string>& _vecTxts);

private:
	vector<string>	m_vecLevelName; 
	vector<string>	m_vecMapModelNames;

	// For Menu Separation
	unordered_set<string>	m_setMapNames;
	unordered_set<string>	m_setMonsterNames; 
	unordered_set<string>	m_setTriggerNames;
	unordered_set<string>	m_setRallyingMonsters;
	unordered_set<string>	m_setObjectTxts;
	unordered_set<string>	m_setMapDecoTxts;
	unordered_set<string>	m_setItemTxts;

	unordered_set<string>	m_setNonColDecos;
	unordered_set<string>	m_setAnimDecos;
	unordered_set<string>	m_setActorDecos;
	unordered_set<string>	m_setKickables;
	unordered_set<string>	m_setTrees;
	unordered_set<string>	m_setBlendDecos; //투명도 적용이 필요한 데코오브젝트

	unordered_set<string>	m_setTownDecoTxts;
	unordered_set<string>	m_setLabDecoTxts;
	unordered_set<string>	m_setParkDecoTxts;

	vector<string>	m_vecMapTxts;
	vector<string>	m_vecTriggerTxts;
	vector<string>	m_vecMonsterTxts;
	vector<string>	m_vecObjectTxts;
	vector<string>	m_vecMapDecoTxts;
	vector<string>	m_vecItemTxts;
	vector<string>	m_vecKickableTxts;
	
	vector<_int*>	m_vecTxtIndices;

	//마을을 위한 txts
	vector<string>	m_vecTownDecoTxts;
	vector<string>	m_vecLabDecoTxts;
	vector<string>	m_vecParkDecoTxts;

	unordered_map<string, unordered_set<_uint>> m_mapBlendDecoInfos;

	string m_strSelectedTxt;
	string m_strCurModel;
	CGameObject* m_pPickedObject = { nullptr };
	_float3 m_vPickPos = {};

public:
	static	CMapToolHelper*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

END



