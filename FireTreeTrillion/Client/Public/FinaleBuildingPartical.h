#pragma once
#include "Client_Defines.h"
#include "GameObject.h"


BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CFinaleBuildingPartical final : public CGameObject
{
private:
	CFinaleBuildingPartical(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleBuildingPartical(const CFinaleBuildingPartical& rhs);
	virtual ~CFinaleBuildingPartical() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

	void			Set_BuildingPartical(_float4 vPos);

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();
	CShader*		m_pShaderCom = { nullptr };
	enum BUILDINGTYPE { A, B, C, D , BUILDINGEND };
	CModel* m_pModelCom[BUILDINGEND] = { nullptr, nullptr, nullptr, nullptr };
	BUILDINGTYPE m_eType = { BUILDINGEND };

	_float4			m_vDir = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fSpeed = { 0.f };
	_bool			m_bActive = { false };
	_float			m_fActiveTime = { 0.f };

	_float			m_fTimeDelta = { 0.f };

	_float			m_fTurn = { 0.f };
	_float4			m_fTurnAxis = { 0.f, 0.f, 0.f, 0.f };

public:
	static CFinaleBuildingPartical* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END