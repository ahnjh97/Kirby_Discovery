#include "GameInstance.h"
#include "OcTree.h"

COcTree::COcTree()
{
}

HRESULT COcTree::Initialize(_float3 vXYZ, _float3 vXyZ, _float3 vXyz, _float3 vXYz,
	_float3 vxYZ, _float3 vxyZ, _float3 vxyz, _float3 vxYz, const vector<FACE>& _vecFaces)
{
	m_vOctants[OC_XYZ] = vXYZ;
	m_vOctants[OC_XyZ] = vXyZ;
	m_vOctants[OC_Xyz] = vXyz;
	m_vOctants[OC_XYz] = vXYz;
	m_vOctants[OC_xYZ] = vxYZ;
	m_vOctants[OC_xyZ] = vxyZ;
	m_vOctants[OC_xyz] = vxyz;
	m_vOctants[OC_xYz] = vxYz;

	_float fX{}, fY{}, fZ{};
	for (_int i = 0; i < OC_END; i++) {
		fX += m_vOctants[i].x;
		fY += m_vOctants[i].y;
		fZ += m_vOctants[i].z;
	}
		
	m_vCenter = _float3(fX * 0.125f, fY * 0.125f, fZ * 0.125f);

	if (_vecFaces.empty())
		return S_OK;

	if (_vecFaces.size() < 32)
	{
		for (auto& face : _vecFaces)
		{

		}
	}

	return S_OK;
}

HRESULT COcTree::SetUp_Neighbors()
{

    return S_OK;
}

void COcTree::Culling(CGameInstance* pGameInstance, const _float3* pVerticesPos, _uint* pIndices, _uint* pNumIndices)
{

}

_bool COcTree::IsDrawable(CGameInstance* pGameInstance, const _float3* pVerticesPos)
{
	_vector vCamPos = pGameInstance->Get_CamPosition();
	_float fCenterDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vCenter) - vCamPos));
	_float	fWidth = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iOctants[OC_XYZ]]) - XMLoadFloat3(&pVerticesPos[m_iOctants[OC_xyz]])));

	if (fCenterDistance * 0.2f > fWidth)
		return true;

	return false;
}

COcTree* COcTree::Create(_uint iXYZ, _uint iXyZ, _uint iXyz, _uint iXYz, _uint ixYZ, _uint ixyZ, _uint ixyz, _uint ixYz)
{
	COcTree* pInstance = new COcTree();

	if (FAILED(pInstance->Initialize(iXYZ, iXyZ, iXyz, iXYz, ixYZ, ixyZ, ixyz, ixYz)))
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
		Safe_Release(m_pChildren[i]);
}
