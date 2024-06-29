#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CTunnel final : public CGameObject
{
public:
	enum TEX_TUNNEL { TEX_BASECOLOR, TEX_NORMAL, TEX_MRA, TEX_END };

private:
	CTunnel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTunnel(const CTunnel& rhs);
	virtual ~CTunnel() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr, nullptr, nullptr };
	vector<CGameObject*> m_vecTunnelRocks;

	vector<_int> m_vecPassIndices;

	_bool m_bCollsion = { false };

	_float m_fSamplingFactor = { 1.f };
	_float m_fZero = {};

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CTunnel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END
