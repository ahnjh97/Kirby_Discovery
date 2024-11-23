#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "FinaleRoadGrouper.h"
BEGIN(Engine)
END

BEGIN(Client)


// 캐릭터에게 지속적으로 운석을 날리는 무형의 객체이다.
class CDisaster_Master final : public CGameObject
{
private:
	CDisaster_Master(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDisaster_Master(const CDisaster_Master& rhs);
	virtual ~CDisaster_Master() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;

private:
	void			Make_MissBaum();
	void			Make_OnTerrainBaum(_float4 vTargetPos, _bool bBaum);
	void			Make_AirParticle();
	void			Make_FinaleRoad(ROADTYPE eType, MOVETYPE eMoveType, _float3 vTargetPos, _float3 vLookDir, _float3 vDestPos, _float3 vDestDir, _float fDestZAngle = -1.f);

	void			Make_CutAirParticle();
	_bool			m_bCutInitializeParticle = { true };

	void			Moving_FinaleRoad(_float fKirbyX);
	_bool			m_bRoadTrigger[10] = { true, true, true, true, true, true, true, true, true, true };
	void			Moving_TargetBaum(_float fKirbyX);
	_bool			m_bBaumTrigger[10] = { true, true, true, true, true, true, true, true, true, true };

	class CFinaleKirby*	m_pKirby = { nullptr };
	class CFinalePartical_Maker* m_pMaker = { nullptr };
	_float				m_fMakeBaumDelay = { 0.f };
	class CLight*		m_pLight = { nullptr };

	_float				m_fAirParticleDelay = { 0.f };
	_float				m_fBuildingParticleDelay = { 0.f };

	_bool				m_bKirbyCutSceneStart = { false };


public:
	static CDisaster_Master* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END