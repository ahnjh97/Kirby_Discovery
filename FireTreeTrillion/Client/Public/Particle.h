#pragma once
#include "Client_Defines.h"
#include "Effect.h"

BEGIN(Client)

class CParticle final : public CEffect
{
#ifdef _DEBUG
	friend class CFXToolDirector;
#endif

public:
	typedef struct : public FX_DESC
	{
		_uint iNumInstance = { 0 };
		//_float fLifeTime = { 1.f };
		//_float fLifetimeRandomOffset = { 0.f };

		//_float fStartDelay = { 0.f };
		//_float fStarDelayRandomOffset = { 0.f };


		

	}PARTICLE_DESC;

private:
	CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle(const CParticle& rhs);
	virtual ~CParticle() = default;

public:
	virtual _bool	IsEnded() override;

	void			Update_InstanceInfo(INSTANCE_DESC* _InstanceDesc = nullptr);
	virtual void	Fill_SaveData(_Out_ PARTICLE_DATA* pFXData) override;

	virtual void	Reset_Duration() override
	{
		m_fDuration.first = 0.f;
		m_pVIBufferCom->Revive();
	}

	HRESULT Initialize_Prototype();
	HRESULT Initialize_Prototype(PARTICLE_DESC FXDesc);
	virtual HRESULT Initialize(void* pArg) override;

	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CVIBuffer_Instance_Point*	m_pVIBufferCom = { nullptr };
	CModel*						m_pModelCom = { nullptr };
	CTexture*					m_pTextureCom[TEX_END] = { nullptr };
	CShader*					m_pShaderCom = { nullptr };
	PARTICLE_DESC				m_FXDesc = {};
	INSTANCE_DESC				m_InstanceDesc = {};

	HRESULT			Add_Components(PARTICLE_DESC& FXDesc);
	HRESULT			Bind_ShaderResources(_int iTexIdx = 0, _int iMaskTexIdx = 0);

public:
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PARTICLE_DESC FXDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


