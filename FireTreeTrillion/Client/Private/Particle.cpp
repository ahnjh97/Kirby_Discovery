#pragma once
#include "Client_Defines.h"
#include "Effect.h"

BEGIN(Client)
class CParticle final : public CEffect
{

public:
	typedef struct  : public FX_DESC
	{

	}PARTICLE_DESC;

private:
	CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle(const CParticle& rhs);
	virtual ~CParticle() = default;

public:
	HRESULT Initialize_Prototype();
	HRESULT Initialize_Prototype(PARTICLE_DESC FXDesc);
	virtual HRESULT Initialize(void* pArg) override;

	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:


	CVIBuffer_Rect*			m_pVIBufferCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CTexture*				m_pTextureCom[TEX_END] = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	PARTICLE_DESC			m_FXDesc = {};

	HRESULT			Add_Components(FX_DESC& FXDesc);
	HRESULT			Bind_ShaderResources(_int iTexIdx = 0, _int iMaskTexIdx = 0);

public:
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PARTICLE_DESC FXDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

CParticle::CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

CParticle::CParticle(const CParticle& rhs)
{
}

HRESULT CParticle::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CParticle::Initialize_Prototype(PARTICLE_DESC FXDesc)
{
	return E_NOTIMPL;
}

HRESULT CParticle::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

_int CParticle::Tick(_float fTimeDelta)
{
	return _int();
}

void CParticle::Late_Tick(_float fTimeDelta)
{
}

HRESULT CParticle::Render()
{
	return E_NOTIMPL;
}

HRESULT CParticle::Add_Components(FX_DESC& FXDesc)
{
	return E_NOTIMPL;
}

HRESULT CParticle::Bind_ShaderResources(_int iTexIdx, _int iMaskTexIdx)
{
	return E_NOTIMPL;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PARTICLE_DESC FXDesc)
{
	return nullptr;
}

CGameObject* CParticle::Clone(void* pArg)
{
	return nullptr;
}

void CParticle::Free()
{
}
