#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
class CTexture;
END

BEGIN(Client)

class CBG final : public CGameObject
{
public:
	struct MAP_DESC : public GAMEOBJECT_DESC
	{
		_float3 vMin{};
		_float3 vMax{};
	};

private:
	CBG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBG(const CBG& rhs);
	virtual ~CBG() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	class CQuadTree* m_pQuadTree = { nullptr };
	_uint m_iRenderAll{}, m_iRenderMyMesh{};
	_float m_fSamplingFactor = { 1.f };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CBG* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END

