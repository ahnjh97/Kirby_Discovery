#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CMap_Stage1 final : public CGameObject
{
public:
	typedef struct : public GAMEOBJECT_DESC
	{
		vector<_uint> iPassIndices; // 메쉬마다의 쉐이더 Pass 인덱스 지정을 위한 vector (필요없을수도 있음)
	}MAP_DESC;

private:
	CMap_Stage1(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMap_Stage1(const CMap_Stage1& rhs);
	virtual ~CMap_Stage1() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CGameObject* m_pBlendMap = { nullptr }; // AlphaBlend로 그려야 하는 맵 메쉬들을 모아놓은 PartObject

	vector<_uint> m_vecPassIndices;
	CRenderer::RENDERGROUP m_eRenderGroup = { CRenderer::RENDER_NONBLEND };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_BlendMap(const wstring& _wstrModelTag);

public:
	static CMap_Stage1* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
