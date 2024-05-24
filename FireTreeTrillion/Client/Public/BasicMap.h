#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CBasicMap final : public CGameObject
{
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

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CGameObject* m_pBlendMap = { nullptr }; // 

	vector<_uint> m_vecPassIndices;
	vector<_float> m_vecSamplingFactors;
	CRenderer::RENDERGROUP m_eRenderGroup = { CRenderer::RENDER_NONBLEND };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_BlendMap(const wstring& _wstrModelTag);

	void SetUpShaderInfo(const wstring& _wstrModelTag);
		
public:
	static CBasicMap* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
