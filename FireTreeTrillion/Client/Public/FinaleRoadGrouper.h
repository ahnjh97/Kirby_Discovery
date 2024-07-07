#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CFinaleRoadGrouper final : public CGameObject
{
public:
	enum ROADTYPE
	{
		RTYPE_BUILDINGA,
		RTYPE_BUILDINGB,
		RTYPE_BUILDINGC,
		RTYPE_BUILDINGD,
		RTYPE_ROADA, RTYPE_ROADB, RTYPE_ROADC,
		RTYPE_END
	};

	enum MOVECMD
	{
		MOVECMD_STOP,
		MOVECMD_ROTATE,
		MOVECMD_FLY,
		MOVECMD_COLLIDE,
		MOVECMD_END
	};

	struct ROADGROUPER_DESC : public GAMEOBJECT_DESC
	{
		ROADTYPE	eRoadType = { RTYPE_END };
		MOVECMD		eMoveCommand = { MOVECMD_STOP };
		_bool		bIsAnimModel = { false };

		_float3		vDestPos = { -1.f, -1.f, -1.f };
		_float3		vDestDir = { -1.f, -1.f, -1.f };
	};

private:
	CFinaleRoadGrouper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleRoadGrouper(const CFinaleRoadGrouper& rhs);
	virtual ~CFinaleRoadGrouper() = default;

public:

	//충돌한 도로 놈이 Road Groupder의 함수를 호출하면, 자신이 가지고 있는 road의 주소를 확인하여 내 산하의 충돌인지 판별한다.
	//내 충돌이 맞다면, 가지고 있는 road들에게 작동 명령을 내린다.
	//자신도 유형에 따라 transform 이동을 한다.
	_bool			Make_CollideReaction(class CFinaleRoad* pRoad = nullptr);

	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

private:
	_bool		m_bStartCollideEvent = { false };
	MOVECMD		m_eCollideMove = { MOVECMD_END };
	_float		m_fCollideTime = { 1.f };

	_float3		m_vStartPos = { 0.f, 0.f, 0.f };
	_float3		m_vDestPos = { 0.f, 0.f, 0.f };


	_float3		m_vStartDir = { 0.f, 0.f, 0.f };
	_float3		m_vDestDir = { 0.f, 0.f, 0.f };


	vector<CFinaleRoad*> m_pRoads;

public:
	static CFinaleRoadGrouper* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END