#pragma once
#include "Client_Defines.h"
#include "Effect.h"

BEGIN(Client)
class CSingleEffect final : public CEffect
{

public:
	//typedef struct  : public FX_DESC
	//{
	//	//이펙트 만들어지는 기준점(월드)

	//	_bool bIsNonLighting = { true };
	//	_bool bIsBlooming = { false };

	//}EDITEFFECT_DESC;

private:
	CSingleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSingleEffect(const CSingleEffect& rhs);
	virtual ~CSingleEffect() = default;

public:
	virtual void	Reset_Duration() override
	{
		m_fDuration.first = 0.f;

		for (size_t i = KF_POS; i < KF_END; ++i)
		{
			m_iCurKeyframeIdxs[i] = 0;
		}

		m_fLifetime.second -= m_fLifetime.first;
		m_fLifetime.first -= m_fLifetime.first;
	}

	HRESULT Initialize_Prototype();
	HRESULT Initialize_Prototype(FX_DESC FXDesc);
	virtual HRESULT Initialize(void* pArg) override;

	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:

	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };
	CModel*				m_pModelCom = { nullptr };
	CTexture*			m_pTextureCom[TEX_END] = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	FX_DESC				m_FXDesc = {};

	_bool				m_bNoRender = { false };
	HRESULT			Add_Components(FX_DESC& FXDesc);
	HRESULT			Bind_ShaderResources(_int iTexIdx = 0, _int iMaskTexIdx = 0);

public:
	static CSingleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CSingleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, FX_DESC FXDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END