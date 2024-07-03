#pragma once
#include "Client_Defines.h"
#include "Effect.h"

BEGIN(Client)

class CMultiEffect final : public CEffect
{
#ifdef _DEBUG
	friend class CFXToolDirector;
#endif

public:
	typedef struct : public FX_DESC
	{
		_float fDuration = { 1.f };
		vector<string> FXs;

	}MULTI_FX_DESC;

private:
	CMultiEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMultiEffect(const CMultiEffect& rhs);
	virtual ~CMultiEffect() = default;

public:
	virtual void Add_Effect(CEffect* pEffect)
	{
		m_FXs.push_back(pEffect);
		//제일 긴 놈의 duration으로 정해진다.
		m_fDuration.second = pEffect->Get_BiggerDuration(m_fDuration.second);
	}
	virtual void Add_RenderGroup() override;

	virtual _int Get_Size() { return m_FXs.size(); }
	virtual void Reset_Duration() override
	{
		m_fDuration.first = 0.f;

		for (auto& fx : m_FXs)
			fx->Reset_Duration();
	}
	virtual void Fill_SaveData(_Out_ MULTI_FX_DATA* pFXData) override;

	HRESULT Initialize_Prototype();
	HRESULT Initialize_Prototype(MULTI_FX_DESC FXDesc);
	virtual HRESULT Initialize(void* pArg) override;

	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	vector<CEffect*> m_FXs;
	MULTI_FX_DESC m_FXDesc = {};

public:
	static CMultiEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CMultiEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MULTI_FX_DESC FXDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END