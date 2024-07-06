#include "stdafx.h"
#include "FinaleRoadGrouper.h"
#include "FinaleRoad.h"

CFinaleRoadGrouper::CFinaleRoadGrouper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CFinaleRoadGrouper::CFinaleRoadGrouper(const CFinaleRoadGrouper& rhs)
	:CGameObject{ rhs }
{
}

_bool CFinaleRoadGrouper::Make_CollideReaction(CFinaleRoad* pRoad)
{
	_bool bIsMyCollision{ false };

	for (auto& road : m_pRoads)
	{
		if (pRoad == road)
		{
			bIsMyCollision = true;
			break;
		}
	}

	if (bIsMyCollision)
	{ 
		for (auto& road : m_pRoads)
		{
			road->Make_CollisionEvent();
		}
	}
	return _bool();
}

HRESULT CFinaleRoadGrouper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleRoadGrouper::Initialize(void* pArg)
{
	ROADGROUPER_DESC RoadGroupDesc{};

	if (nullptr != pArg)
		RoadGroupDesc = *(ROADGROUPER_DESC*)pArg;

	RoadGroupDesc.fSpeedPerSec = 5.f;
	RoadGroupDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	switch (RoadGroupDesc.eRoadType)
	{
	case RTYPE_BUILDINGA:
	{

	}
	break;
	case RTYPE_BUILDINGB:
	{

	}
	break;
	case RTYPE_BUILDINGC:
	{

	}
	break;
	case RTYPE_BUILDINGD:
	{

	}
	break;
	case RTYPE_ROADA:
	{

	}
	break;
	case RTYPE_ROADB:
	{

	}
	break;

	default:
		break;
	}



	return S_OK;
}

_int CFinaleRoadGrouper::Tick(_float fTimeDelta)
{
	return _int();
}

void CFinaleRoadGrouper::Late_Tick(_float fTimeDelta)
{
}

HRESULT CFinaleRoadGrouper::Render()
{
	return S_OK;
}

#ifdef DEBUG
void CFinaleRoadGrouper::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif

CFinaleRoadGrouper* CFinaleRoadGrouper::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinaleRoadGrouper* pInstance = new CFinaleRoadGrouper(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinaleRoadGrouper"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinaleRoadGrouper::Clone(void* pArg)
{
	CFinaleRoadGrouper* pInstance = new CFinaleRoadGrouper(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinaleRoad"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinaleRoadGrouper::Free()
{
	for (auto* road : m_pRoads)
		Safe_Release(road);
	m_pRoads.clear();

	__super::Free();
}
