#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CDimensionClaw final : public CGameObject
{
private:
	CDimensionClaw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDimensionClaw(const CDimensionClaw& rhs);
	virtual ~CDimensionClaw() = default;

public:
	void MoveToStrangePos();
	_bool CheckCollsion(_float4 vPos);

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual HRESULT Render()						override;

private:
	_float m_fKirbyOffsetY = {};
	_float m_fKirbyRadius = {};

	_float m_fGradient = {};
	_float m_fOffsetY = {};

public:
	static CDimensionClaw* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


