#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "FinalePartical.h"

BEGIN(Engine)
END

BEGIN(Client)

class CFinalePartical_Maker final : public CGameObject
{
private:
	CFinalePartical_Maker(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalePartical_Maker(const CFinalePartical_Maker& rhs);
	virtual ~CFinalePartical_Maker() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

	void Make_Partical(_int iNum, 
		_float4 vPos, _float fPosOffset, 
		_float vScale, _float fScaleOffset, 
		_float4 vDir, _float fRandomAngle, _float fPower,
		_bool bNoGravity = false);


private:
	vector<CFinalePartical*> m_FinaleParticals;
	_int					m_iCount = { 0 };


public:
	static CFinalePartical_Maker* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END