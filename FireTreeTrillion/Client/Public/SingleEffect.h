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

protected:
	FX_DESC m_FXDesc = {};

public:
	static CSingleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CSingleEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, FX_DESC EditEffectDesc);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END