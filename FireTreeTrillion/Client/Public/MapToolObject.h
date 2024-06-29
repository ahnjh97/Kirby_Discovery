#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "OrbitingCamera.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPartObject;
END

BEGIN(Client)

class CMapToolObject : public CGameObject
{
public:
	enum TYPE_MAPOBJ { MAPOBJ_NONCOL, MAPOBJ_ANIM, MAPOBJ_ACTOR, MAPOBJ_END };

public:
	struct MAPTOOLOBJECT_DESC : public GAMEOBJECT_DESC
	{
		_int iTriggerType = { -1 };
		_int iTriggerIndex = { -1 };
		_float fRadius = { 0.f };
		_int iCamType = { -1 };
		_int iPassIndex = { 0 };
		map<_uint, _float3> RallyPoints;
		string strConnectedMonster;
		unordered_set<_uint> setBlendMeshIndices;
		TYPE_MAPOBJ eMapObjType = { MAPOBJ_END };
	};

public:
	_int Get_TriggerType() { return m_iTriggerType; }
	_int Get_TriggerIndex() { return m_iTriggerIndex; }
	_float Get_Radius() { return m_fRadius; }
	_int Get_CamType() { return m_iCamType; }
	string Get_ConnectedMonster() { return m_strConnectedMonster; }
	map<_uint, _float3>& Get_RallyPoints() { return m_RallyPoints; }
	TYPE_MAPOBJ Get_MapObjType() { return m_eMapObjType; }
	_int Get_PassIndex() { return m_iPassIndex; }

	virtual void Set_Hide(_bool bHide) { m_bHide = bHide; if (nullptr != m_pOrbitingCamera) m_pOrbitingCamera->Set_Hide(bHide); }

	void Set_TriggerType(_int iTriggerType) { m_iTriggerType = iTriggerType; }
	void Set_TriggerIndex(_int iTriggerIndex) { m_iTriggerIndex = iTriggerIndex; }
	void Set_CamType(_int iCamType) { m_iCamType = iCamType; }
	void Set_Radius(_float fRadius) { m_fRadius = fRadius; }
	void Set_ConnectedMonster(string& strConnectedMonster) { m_strConnectedMonster = strConnectedMonster; }
	void Set_MapObjType(TYPE_MAPOBJ eMapObjType) {}
	void Set_PassIndex(_int iPassIndex) { m_iPassIndex = iPassIndex; }
	void Set_PassIndices(unordered_set<_uint>& _setBlendMeshIndices);
	void Reset_Time(_uint iIndex) { m_iMeshIndex = iIndex; m_fTime = 0; }

private:
	CMapToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapToolObject(const CMapToolObject& rhs);
	virtual ~CMapToolObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

#ifdef _DEBUG
	virtual void Render_IMGUI() override;
#endif
public:
	void Emplace_RallyPoint(_uint iIndex, _float3 vPos) { m_RallyPoints.emplace(iIndex, vPos); }

private:
	_int m_iTriggerType = { -1 };
	_int m_iTriggerIndex = { -1 };
	_float m_fRadius = { 0.f };
	_int m_iCamType = { -1 };
	_int m_iPassIndex = { 0 };
	_float m_fWhiteColorDiffuse = {};
	_float m_fZero = {};
	_uint m_iMeshIndex = {};
	_float m_fTime = { 100.f };
	
	string m_strConnectedMonster;
	map<_uint, _float3> m_RallyPoints;
	TYPE_MAPOBJ m_eMapObjType = { MAPOBJ_ACTOR };

	vector<_uint> m_vecPassIndices;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	COrbitingCamera* m_pOrbitingCamera = { nullptr };
	class CMapToolHelper* m_pMapToolHelper = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_PartObject();

public:
	static CMapToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
