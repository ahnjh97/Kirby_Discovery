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
	virtual void	Render_IMGUI()						override;

private:
	void	SetUpTxtVectors(TYPE _eType);
	
	// ImGui
	void	Menu_Level();
	void	Menu_NonAnimModels();
	void	Menu_TriggerInfo();
	void	Menu_CamLerpInfo(class CMapToolObject* _pMapToolObject);
	void	Menu_MapShaderInfo();
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

	// Picking
	CGameObject* Select_ModelByPicking(const wstring& wstrLayerTag = TEXT("Layer_Parse"));

	_int Compute_MapIndex(const string& strModelName);

private:
	vector<string>	m_vecLevelName; 
	vector<string>	m_vecMapModelNames;

	vector<string> m_vecAnimTxts;
	vector<string> m_vecNonAnimTxts;
	string m_strCurModel;
	CGameObject* m_pPickedObject = { nullptr };
	_float3 m_vPickPos = {};

public:
	static	CMapToolHelper* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void			Free() override;

};

END



