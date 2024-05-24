#pragma once
#include "Client_Defines.h"
#include "Effect.h"

BEGIN(Client)

class CParticle final : public CEffect
{
public:
	typedef struct : public FX_DESC
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
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr };
	CShader* m_pShaderCom = { nullptr };
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


