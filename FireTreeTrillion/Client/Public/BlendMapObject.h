#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBlendMapObject : public CGameObject
{
public:
	struct BLENDMAPOBJ_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		MODEL tModel = {};
		unordered_set<_uint> setBlendMeshIndices;
		_uint iPassIndex = { MAP_ALPHABLEND };
	};
private:
	CBlendMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBlendMapObject(const CBlendMapObject& rhs);
	virtual ~CBlendMapObject() = default;

public:
	void SetUp_BlendMeshes(unordered_set<_uint>& _setBlendMeshIndices) { m_setBlendMeshIndices = _setBlendMeshIndices; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override { return OBJ_NOEVENT; }
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	PxRigidStatic* m_pStaticActor = { nullptr };

	_float m_fWhiteColorDiffuse = {};

	unordered_set<_uint> m_setBlendMeshIndices;
	vector<_uint> m_vecPassIndices;
	
	unordered_set<string> m_setNearClipAlphaBlend;
	_uint m_iPassIndex = {};

private:
	HRESULT Add_Components(MODEL tModel);
	HRESULT Bind_ShaderResources();

public:
	static CBlendMapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

