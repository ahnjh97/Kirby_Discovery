#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
class CTexture;
END

BEGIN(Client)

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


private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
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

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_BlendMap(const wstring& _wstrModelTag);

	void SetUpShaderInfo(const wstring& _wstrModelTag);

	_bool CheckIfBlendMapExists(const wstring& _wstrModelTag);

	void Save_OctreeData();
		
public:
	static CBasicMap* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
