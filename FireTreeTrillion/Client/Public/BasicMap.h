#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
class CTexture;
END

BEGIN(Client)

// 옥토트리와 셰이더 UV조절용 맵오브젝트
class CBasicMap final : public CGameObject
{
public:
	struct MAP_DESC : public GAMEOBJECT_DESC
	{
		_float3 vMin{};
		_float3 vMax{};
	};

public:
	void Set_PassIndex(_uint iIndex, _uint iPassIndex) { m_vecPassIndices[iIndex] = iPassIndex; }
	void Set_SamplingFactor(_uint iIndex, _float fSamplingFactor) { m_vecSamplingFactors[iIndex] = fSamplingFactor; }
	void Reset_Time(_uint iIndex) { m_iMeshIndex = iIndex; m_fTime = 0;  }

private:
	CBasicMap(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBasicMap(const CBasicMap& rhs);
	virtual ~CBasicMap() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

public:
	void Save_OctreeData(const string& strLevel);

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CShader* m_pNonAnimShaderCom = { nullptr };
	CShader* m_pAnimShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CGameObject* m_pBlendMap = { nullptr }; 

	vector<_uint> m_vecPassIndices;
	vector<_float> m_vecSamplingFactors;
	CRenderer::RENDERGROUP m_eRenderGroup = { CRenderer::RENDER_NONBLEND };
	_float m_fTime = { };
	_uint m_iMeshIndex = {};
	_float m_fNonMatchTime = {};
	_bool m_bCull = { false };
	_bool m_bBlendMap = { false };

	class COcTree* m_pOcTree = { nullptr };
	_uint m_iRenderAll{}, m_iRenderMyMesh{};
	unordered_map<string, _float> m_ModelShapeRadiiMap;
	unordered_map<string, pair<_uint, _float>> m_ModelActionAnimMap;
	unordered_map<string, pair<_uint, _float>> m_ModelIdleAnimMap;
	vector<PxRigidStatic*> m_vecAnimDecoTriggersActors;
	vector<PxShape*> m_vecShapes;

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_BlendMap(const wstring& _wstrModelTag);

	void SetUpShaderInfo(const wstring& _wstrModelTag);

	_bool CheckIfBlendMapExists(const wstring& _wstrModelTag);
	void InsertMapDecos();
	PxRigidStatic* AddTriggerActorForAnimDeco(const string& _strModelName, _float4x4& _matWorld);
	void SetUpAnimDecoInfo(const string& _strModelName, _float _fTriggerRadius, _uint iIdleIndex, _float fIdleAnimSpeed
		, _uint iActionIndex, _float fActionAnimSpeed);
		
public:
	static CBasicMap* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
