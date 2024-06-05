#include "GameInstance.h"
#include "OcTree.h"
#include "Mesh.h"

COcTree::COcTree()
{
}

HRESULT COcTree::Initialize(_float3 vCenter, _float3 vHalfExtents, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices
	, const vector<_float3*>& _vecNormals, const vector<_float3*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices, ifstream& fileInput
	, vector<class CMesh*>& _vecMeshes)
{
	if (false == _vecMeshes.empty())
		m_vecMeshes = _vecMeshes;

	m_vCenter = vCenter;
	m_vecEdges.resize(OC_END);
	m_vecChildren.resize(OC_END);
	m_vecChildrenFaces.resize(OC_END);

	SetUp_Edges(vCenter, vHalfExtents);

	///*if(false == Load_OctreeData(fileInput))
	//	*/IdentifyOctant(_pVerticesPtrs, _iNumVertices, _vecMeshFaces);

	//if (_vecMeshFaces.size() < 30 || XMVectorGetX(XMVector3Length(m_vecEdges[OC_XYZ] - m_vecEdges[OC_xyz])) < 12)
	//	return S_OK;

	//vector<_float3> vecChildrenCenters(OC_END);

	//_float3 vQuarterExtents = _float3(vHalfExtents.x * 0.5f, vHalfExtents.y * 0.5f, vHalfExtents.z * 0.5f);
	//SetUp_ChildrenCenter(vCenter, vQuarterExtents, vecChildrenCenters);

	//for (_int j = 0; j < OC_END; j++) {
	//	m_vecChildren[j] = COcTree::Create(vecChildrenCenters[j], vQuarterExtents
	//		, _pVerticesPtrs, _iNumVertices, m_vecChildrenFaces[j], fileInput);
	//}

	return S_OK;
}

void COcTree::Culling(CGameInstance* pGameInstance, const _float3* _pVerticePoses, _uint _iNumVertices, vector<FACE>& _vecResultFaces)
{
	if (nullptr == m_vecChildren[OC_XYZ])
		return;
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

void COcTree::IdentifyOctant(const _float3* _VerticesPtr, const _uint _iNumVertices, const vector<FACE>& _vecMeshFaces)
{
	OCTANT eOctant = { OC_END };
	_int iCount{};

	for (auto& face : _vecMeshFaces)
	{
		_float3 vA = _VerticesPtr[face.iA];
		_float3 vB = _VerticesPtr[face.iB];
		_float3 vC = _VerticesPtr[face.iC];

		eOctant = FinalOctant(vA, vB, vC);

		if (eOctant == OC_END)
			m_vecMeshFaces.emplace_back(face);
		else
			m_vecChildrenFaces[eOctant].emplace_back(face);
	}
}

COcTree::OCTANT COcTree::CheckOctant(const _float3& vPoint)
{
	_int iBinaryNum{};
	if (vPoint.x > m_vCenter.x)
		iBinaryNum |= 4;
	if (vPoint.y > m_vCenter.y)
		iBinaryNum |= 2;
	if (vPoint.z > m_vCenter.z)
		iBinaryNum |= 1;

	switch (iBinaryNum)
	{
	case 0:
		return OC_xyz;
	case 1:
		return OC_xyZ;
	case 2:
		return OC_xYz;
	case 3:
		return OC_xYZ;
	case 4:
		return OC_Xyz;
	case 5:
		return OC_XyZ;
	case 6:
		return OC_XYz;
	case 7:
		return OC_XYZ;
	default:
		return OC_END;
	}
}

COcTree::OCTANT COcTree::FinalOctant(const _float3& _vA, const _float3& _vB, const _float3& _vC)
{
	OCTANT eOctantA = CheckOctant(_vA);
	OCTANT eOctantB = CheckOctant(_vB);

	if (eOctantA != eOctantB)
		return OC_END;

	OCTANT eOctantC = CheckOctant(_vC);

	if (eOctantA != eOctantC)
		return OC_END;

	return eOctantA;
}

void COcTree::Save_OctreeData(ofstream& fileOutput)
{
	if (m_vecMeshFaces.empty())
	{
		_uint iZero = 0;
		fileOutput.write(reinterpret_cast<const char*>(&iZero), sizeof(iZero));
	}

	_uint iNumFaces = m_vecMeshFaces.size();
	fileOutput.write(reinterpret_cast<const char*>(&iNumFaces), sizeof(iNumFaces));

	for (_uint iFaceIdx = 0; iFaceIdx < iNumFaces; iFaceIdx++)
		fileOutput.write(reinterpret_cast<const char*>(&m_vecMeshFaces[iFaceIdx]), sizeof(m_vecMeshFaces[iFaceIdx]));
	

	if (nullptr == m_vecChildren[OC_XYZ])
	{
		string strDelimiter = "Stop";
		_uint iSize = strDelimiter.size();
		fileOutput.write(reinterpret_cast<const char*>(&iSize), sizeof(iSize));
		fileOutput.write(strDelimiter.c_str(), iSize);
		return;
	}
	else
	{
		string strDelimiter = "Read";
		_uint iSize = strDelimiter.size();
		fileOutput.write(reinterpret_cast<const char*>(&iSize), sizeof(iSize));
		fileOutput.write(strDelimiter.c_str(), iSize);
	}

	for (_int iOctant = 0; iOctant < OC_END; iOctant++)
	{
		if (m_vecChildrenFaces[iOctant].empty())
		{
			_uint iZero = 0;
			fileOutput.write(reinterpret_cast<const char*>(&iZero), sizeof(iZero));
			continue;
		}

		_uint iNumFaces = m_vecChildrenFaces[iOctant].size();
		fileOutput.write(reinterpret_cast<const char*>(&iNumFaces), sizeof(iNumFaces));

		for (auto& childFace : m_vecChildrenFaces[iOctant])
			fileOutput.write(reinterpret_cast<const char*>(&childFace), sizeof(childFace));
		
	}

	if (nullptr == m_vecChildren[OC_XYZ])
		return;

	for (auto& child : m_vecChildren)
		child->Save_OctreeData(fileOutput);
}

_bool COcTree::Load_OctreeData(ifstream& fileInput)
{
	if (true == fileInput.fail())
		return false;

	if (fileInput.eof())
		return true;

	_uint iNumFaces{};
	fileInput.read(reinterpret_cast<char*>(&iNumFaces), sizeof(iNumFaces));

	FACE tTempFace = {};
	for (_uint iFaceIdx = 0; iFaceIdx < iNumFaces; iFaceIdx++)
	{
		fileInput.read(reinterpret_cast<char*>(&tTempFace), sizeof(tTempFace));
		m_vecMeshFaces.emplace_back(tTempFace);
	}
	
	_uint iSize{};
	string strDelimiter;
	fileInput.read(reinterpret_cast<char*>(&iSize), sizeof(iSize));
	strDelimiter.resize(iSize);
	fileInput.read(&strDelimiter[0], iSize);
	if ('S' == strDelimiter.front())
		return true;

	for (_int iOctant = 0; iOctant < OC_END; iOctant++)
	{
		_uint iNumFaces{};
		fileInput.read(reinterpret_cast<char*>(&iNumFaces), sizeof(iNumFaces));

		FACE tTempFace = {};
		for (_uint iFaceIdx = 0; iFaceIdx < iNumFaces; iFaceIdx++)
		{
			fileInput.read(reinterpret_cast<char*>(&tTempFace), sizeof(tTempFace));
			m_vecChildrenFaces[iOctant].emplace_back(tTempFace);
		}
	}

	return true;
}

void COcTree::SetUp_Edges(_float3 vCenter, _float3 vHalfExtents)
{
	m_vecEdges[OC_XYZ] = _float3(vCenter.x + vHalfExtents.x, vCenter.y + vHalfExtents.y, vCenter.z + vHalfExtents.z);
	m_vecEdges[OC_XyZ] = _float3(vCenter.x + vHalfExtents.x, vCenter.y - vHalfExtents.y, vCenter.z + vHalfExtents.z);
	m_vecEdges[OC_Xyz] = _float3(vCenter.x + vHalfExtents.x, vCenter.y - vHalfExtents.y, vCenter.z - vHalfExtents.z);
	m_vecEdges[OC_XYz] = _float3(vCenter.x + vHalfExtents.x, vCenter.y + vHalfExtents.y, vCenter.z - vHalfExtents.z);
	m_vecEdges[OC_xYZ] = _float3(vCenter.x - vHalfExtents.x, vCenter.y + vHalfExtents.y, vCenter.z + vHalfExtents.z);
	m_vecEdges[OC_xyZ] = _float3(vCenter.x - vHalfExtents.x, vCenter.y - vHalfExtents.y, vCenter.z + vHalfExtents.z);
	m_vecEdges[OC_xyz] = _float3(vCenter.x - vHalfExtents.x, vCenter.y - vHalfExtents.y, vCenter.z - vHalfExtents.z);
	m_vecEdges[OC_xYz] = _float3(vCenter.x - vHalfExtents.x, vCenter.y + vHalfExtents.y, vCenter.z - vHalfExtents.z);
}

void COcTree::SetUp_ChildrenCenter(_float3 vCenter, _float3 vQuarterExtents, vector<_float3>& _vecChildrenCenters)
{
	_vecChildrenCenters[OC_XYZ] = _float3(vCenter.x + vQuarterExtents.x, vCenter.y + vQuarterExtents.y, vCenter.z + vQuarterExtents.z);
	_vecChildrenCenters[OC_XyZ] = _float3(vCenter.x + vQuarterExtents.x, vCenter.y - vQuarterExtents.y, vCenter.z + vQuarterExtents.z);
	_vecChildrenCenters[OC_Xyz] = _float3(vCenter.x + vQuarterExtents.x, vCenter.y - vQuarterExtents.y, vCenter.z - vQuarterExtents.z);
	_vecChildrenCenters[OC_XYz] = _float3(vCenter.x + vQuarterExtents.x, vCenter.y + vQuarterExtents.y, vCenter.z - vQuarterExtents.z);
	_vecChildrenCenters[OC_xYZ] = _float3(vCenter.x - vQuarterExtents.x, vCenter.y + vQuarterExtents.y, vCenter.z + vQuarterExtents.z);
	_vecChildrenCenters[OC_xyZ] = _float3(vCenter.x - vQuarterExtents.x, vCenter.y - vQuarterExtents.y, vCenter.z + vQuarterExtents.z);
	_vecChildrenCenters[OC_xyz] = _float3(vCenter.x - vQuarterExtents.x, vCenter.y - vQuarterExtents.y, vCenter.z - vQuarterExtents.z);
	_vecChildrenCenters[OC_xYz] = _float3(vCenter.x - vQuarterExtents.x, vCenter.y + vQuarterExtents.y, vCenter.z - vQuarterExtents.z);
}

COcTree* COcTree::Create(_float3 vCenter, _float3 vHalfExtents, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices
	, const vector<_float3*>& _vecNormals, const vector<_float3*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices, ifstream& fileInput
	, vector<class CMesh*>& _vecMeshes)
{
	COcTree* pInstance = new COcTree();

	if (FAILED(pInstance->Initialize(vCenter, vHalfExtents, _vecVerticesPtrs, _vecNumVertices
		, _vecNormals, _vecIndicesPtrs, _vecNumIndices, fileInput, _vecMeshes)))
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
