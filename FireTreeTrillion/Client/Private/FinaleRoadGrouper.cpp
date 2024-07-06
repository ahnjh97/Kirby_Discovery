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

HRESULT CFinaleRoadGrouper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleRoadGrouper::Initialize(void* pArg)
{
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
