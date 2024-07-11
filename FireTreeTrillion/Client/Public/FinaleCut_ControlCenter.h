#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CFinaleCut_ControlCenter final : public CGameObject
{
private:
	CFinaleCut_ControlCenter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleCut_ControlCenter(const CFinaleCut_ControlCenter& rhs);
	virtual ~CFinaleCut_ControlCenter() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

	_uint			Get_CutScene() { return m_iCutScene; }
	void			Set_CutScene(_uint iScene);

private:
	_uint					m_iCutScene = { 0 };
	_float					m_fTimeDelta = { 0.f };

public:
	static CFinaleCut_ControlCenter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END