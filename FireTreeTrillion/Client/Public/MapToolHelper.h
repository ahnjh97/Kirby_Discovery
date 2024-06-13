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
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg = nullptr)	override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()						override;
#endif

private:
	void	SetUpTxtVectors();
	
	// ImGui
	void	Menu_Level();
	void	Menu_NonAnimModels();
	void	Menu_TriggerInfo();
	void	Menu_CamLerpInfo(class CMapToolObject* _pMapToolObject);
	void	Menu_MapShaderInfo();
	void	Menu_MonsterInfo();
	void	Menu_RallyPointInfo();
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

	void	RegisterRallyPoints(list<CGameObject*>* _pObjList);
	void	SaveMapDecoObjects(vector<CGameObject*>& _vecDecoObjs);
	void	LoadMapDecoObjects();
	void	WriteLocalizedAnimMapDecos(vector<pair<string, _float4x4>>& _vecAnimDecos);
	void	WriteLocalizedNonAnimMapDecos(vector<pair<string, _float4x4>>& _vecNonAnimDecos);

	// Picking
	CGameObject* Select_ModelByPicking(const wstring& wstrLayerTag = TEXT("Layer_Parse"));
	_int Compute_MapIndex(const string& _strModelName);
	_int Compute_RallyingMonsterIndex(const string& _strModelName);

	_bool IsMap(const string& _strModelName);
	_bool IsTrigger(const string& _strModelName);
	_bool IsMonster(const string& _strModelName);
	_bool IsRallyingMonster(const string& _strModelName);

	// Options
	void HideTriggers(_bool bHideTriggers);
	void HideGrid(_bool bHideGrid);
	_bool ExcludeModel(string& _strModelName);

private:
	vector<string>	m_vecLevelName; 
	vector<string>	m_vecMapModelNames;

	// For Menu Separation
	unordered_set<string>	m_setMapNames;
	unordered_set<string>	m_setMonsterNames; 
	unordered_set<string>	m_setTriggerNames;
	unordered_set<string>	m_setRallyingMonsters;
	unordered_set<string>	m_setObjectTxts;

	unordered_set<string>	m_setNonColDecos;
	unordered_set<string>	m_setAnimDecos;
	unordered_set<string>	m_setActorDecos;

	vector<string>	m_vecMapTxts;
	vector<string>	m_vecTriggerTxts;
	vector<string>	m_vecMonsterTxts;
	vector<string>	m_vecObjectTxts;

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



