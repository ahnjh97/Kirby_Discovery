#include "GameInstance.h"
#include "OcTree.h"

COcTree::COcTree()
{
}

HRESULT COcTree::Initialize(const vector<_float3> _vecEdges, const vector<_float3*>& _vecMeshVerticesPtrs, const vector<_uint>& _vecMeshNumVertices
	, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices)
{
	if (_vecEdges.empty() || _vecIndicesPtrs.empty() || _vecNumIndices.empty()) {
		MSG_BOX(TEXT("OcTree: vector empty."));
		return E_FAIL;
	}
		
	m_vecEdges.resize(OC_END);
	m_vecEdges = _vecEdges;

	m_vecChildren.resize(OC_END);
	m_vecMeshFaces.resize(_vecIndicesPtrs.size()); // 메쉬 개수만큼

	_float fX{}, fY{}, fZ{};
	for (_int i = 0; i < OC_END; i++) {
		fX += m_vecEdges[i].x;
		fY += m_vecEdges[i].y;
		fZ += m_vecEdges[i].z;
	}
		
	m_vCenter = _float3(fX * 0.125f, fY * 0.125f, fZ * 0.125f);

	_uint iTotal{};
	for (auto& numIndices : _vecNumIndices)
		iTotal += numIndices;

	// 분열 중지 조건
	if (iTotal / _vecNumIndices.size() < 30 || XMVectorGetX(XMVector3Length(m_vecEdges[OC_XYZ] - m_vecEdges[OC_xyz])) < 32)
		return S_OK;

	_uint iRightFront{}, iRightBottom{}, iRightBack{}, iRightTop{}, iRightCenter{};
	_uint iFrontCenter{}, iBottomCenter{}, iBackCenter{}, iTopCenter{};
	_uint iFrontBottomCenter{}, iBottomBackCenter{}, iBackTopCenter{}, iTopFrontCenter{};
	_uint iLeftFront{}, iLeftBottom{}, iLeftBack{}, iLeftTop{}, iLeftCenter{};

	IdentifyOctant();



	return S_OK;
}

void COcTree::Culling(CGameInstance* pGameInstance, const vector<_float3*>& _vecMeshVerticesPtrse)
{

}

_bool COcTree::IsDrawable(CGameInstance* pGameInstance, const vector<_float3*>& _vecMeshVerticesPtrs)
{
	_vector vCamPos = pGameInstance->Get_CamPosition();
	_float fCenterDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vCenter) - vCamPos));
	_float	fWidth = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vecEdges[OC_XYZ]) - XMLoadFloat3(&m_vecEdges[OC_xyz])));

	if (fCenterDistance * 0.2f > fWidth)
		return true;

	return false;
}

void COcTree::IdentifyOctant()
{
	_vector vPlanes[6] = {}; // Right, Back, Left, Front, Top, Bottom;

	vPlanes[0] = XMPlaneFromPoints(m_vecEdges[OC_XYZ], m_vecEdges[OC_XyZ], m_vecEdges[OC_Xyz]); // Right
	vPlanes[1] = XMPlaneFromPoints(m_vecEdges[OC_XYz], m_vecEdges[OC_Xyz], m_vecEdges[OC_xyz]); // Back
	vPlanes[2] = XMPlaneFromPoints(m_vecEdges[OC_xYz], m_vecEdges[OC_xyz], m_vecEdges[OC_xyZ]); // Left
	vPlanes[3] = XMPlaneFromPoints(m_vecEdges[OC_xYZ], m_vecEdges[OC_XYZ], m_vecEdges[OC_XyZ]); // Front
	vPlanes[4] = XMPlaneFromPoints(m_vecEdges[OC_xYZ], m_vecEdges[OC_XYZ], m_vecEdges[OC_XYz]); // Top
	vPlanes[5] = XMPlaneFromPoints(m_vecEdges[OC_xyZ], m_vecEdges[OC_XyZ], m_vecEdges[OC_Xyz]); // Bottom

	for (_int i = 0; i < 6; i++)
	{
		//_float fDis = XMPlaneDotCoord(vPlanes[i], )
	}
}

COcTree* COcTree::Create(const vector<_float3> _vecEdges, const vector<_float3*>& _vecMeshVerticesPtrs, const vector<_uint>& _vecMeshNumVertices
	, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices)
{
	COcTree* pInstance = new COcTree();

	if (FAILED(pInstance->Initialize(_vecEdges, _vecMeshVerticesPtrs, _vecMeshNumVertices, _vecIndicesPtrs, _vecNumIndices)))
	{
		MSG_BOX(TEXT("Failed to Create : COcTree"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COcTree::Free()
{
	__super::Free();

	for (_int i = 0; i < OC_END; i++)
		Safe_Release(m_vecChildren[i]);
}
