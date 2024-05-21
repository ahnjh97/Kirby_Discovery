#pragma once
#include "Client_Defines.h"
#include "Effect.h"

BEGIN(Client)

class CMultiEffect final :  public CEffect
{
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

protected:
	MULTI_FX_DESC m_FXDesc = {};

public:
	static CMultiEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CMultiEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MULTI_FX_DESC EditEffectDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END