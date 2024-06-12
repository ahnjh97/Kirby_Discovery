#include "GameInstance.h"
#include "Texture.h"
#include "OcTree.h"
#include "Shader.h"
#include "Model.h"
#include "Mesh.h"

COcTree::COcTree()
{
	CGameInstance::Get_Instance()->IncreaseIndex();
}

HRESULT COcTree::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
	, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
	, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
	, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices
	, const vector<CMesh*>& _vecMeshes, const vector<MESH_MATERIAL>& _vecMaterials
	, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames)
{
	if (false == _vecMeshes.empty())
		m_vecMeshes = _vecMeshes;

	m_iNumMeshes = _vecMeshes.size();
	m_vCenter = vCenter;
	m_vecEdges.resize(OC_END);
	m_vecChildren.resize(OC_END);
	m_vecMeshFaces.resize(m_iNumMeshes);
	m_vecChildrenFaces.resize(OC_END);
	for (auto& vecChildFaces : m_vecChildrenFaces)
		vecChildFaces.resize(m_iNumMeshes);

	m_vecMaterials.resize(m_iNumMeshes);
	for (auto& vecTex : m_vecMaterials)
		vecTex.resize(3); // Diffuse, Normal, MRA
	m_vecConstantNames = _vecConstantNames;

	SetUp_Edges(vCenter, vHalfExtents);


	// Save Original Mesh
	m_vecMeshes = _vecMeshes;
	for (auto& mesh : _vecMeshes)
		Safe_AddRef(mesh);
	m_vecPassIndices = _vecPassIndices;
	m_vecSamplingFactors = _vecSamplingFactors;

	_uint iMatIndex{};
	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++)
	{
		iMatIndex = _vecMeshes[iMeshIdx]->Get_MaterialIndex();
		m_vecMaterials[iMeshIdx][TEX_DIFFUSE] = _vecMaterials[iMatIndex].MaterialTextures[TextureType_DIFFUSE];
		m_vecMaterials[iMeshIdx][TEX_NORMAL] = _vecMaterials[iMatIndex].MaterialTextures[TextureType_NORMALS];
		m_vecMaterials[iMeshIdx][TEX_MRA] = _vecMaterials[iMatIndex].MaterialTextures[TextureType_METALNESS];
		Safe_AddRef(_vecMaterials[iMatIndex].MaterialTextures[TextureType_DIFFUSE]);
		Safe_AddRef(_vecMaterials[iMatIndex].MaterialTextures[TextureType_NORMALS]);
		Safe_AddRef(_vecMaterials[iMatIndex].MaterialTextures[TextureType_METALNESS]);
	}

	
	vector<vector<FACE>> vecMeshResultFaces(m_iNumMeshes);

	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++) {
		vecMeshResultFaces[iMeshIdx].resize(_vecNumIndices[iMeshIdx] / 3);
		memcpy(vecMeshResultFaces[iMeshIdx].data(), _vecIndicesPtrs[iMeshIdx], _vecNumIndices[iMeshIdx] * sizeof(_uint));
	}

	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++)
		IdentifyOctant(iMeshIdx, _vecVerticesPtrs[iMeshIdx], _vecNumVertices[iMeshIdx], vecMeshResultFaces[iMeshIdx]);

	// Create MyMesh
	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++) {
		if (m_vecMeshFaces[iMeshIdx].empty())
			continue;

		CMesh* pMyMesh = CMesh::Create(pDevice, pContext, _vecVerticesPtrs[iMeshIdx], _vecNumVertices[iMeshIdx], _vecNormalPtrs[iMeshIdx]
			, _vecTexCoordsPtrs[iMeshIdx], _vecTangentsPtrs[iMeshIdx], m_vecMeshFaces[iMeshIdx]);

		if (nullptr == pMyMesh)
			continue;

		m_vecMyMeshes.push_back(pMyMesh);
		m_vecMyMaterials.push_back(m_vecMaterials[iMeshIdx]);
		for (auto& tex : m_vecMaterials[iMeshIdx])
			Safe_AddRef(tex);
		m_vecMyPassIndices.push_back(m_vecPassIndices[iMeshIdx]);
		m_vecMySamplingFactors.push_back(m_vecSamplingFactors[iMeshIdx]);
	}
	

	// Create Children
	vector<_float3> vecChildrenCenters(OC_END);

	_float3 vQuarterExtents = _float3(vHalfExtents.x * 0.5f, vHalfExtents.y * 0.5f, vHalfExtents.z * 0.5f);
	SetUp_ChildrenCenter(vCenter, vQuarterExtents, vecChildrenCenters);

	for (_int j = 0; j < OC_END; j++) {
		m_vecChildren[j] = COcTree::Create(pDevice, pContext, vecChildrenCenters[j], vQuarterExtents
			, _vecVerticesPtrs, _vecNumVertices, _vecNormalPtrs, _vecTexCoordsPtrs, _vecTangentsPtrs
			,m_vecChildrenFaces[j], m_vecMaterials, _vecPassIndices, _vecSamplingFactors
			, _vecConstantNames);
	}

	return S_OK;
}

HRESULT COcTree::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
	, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
	, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
	, vector<vector<FACE>>& _vecMeshFaces, const vector<vector<CTexture*>>& _vecSortedMaterials
	, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames)
{
	m_iNumMeshes = _vecVerticesPtrs.size();
	m_vCenter = vCenter;
	m_vecEdges.resize(OC_END);
	m_vecChildren.resize(OC_END);
	m_vecMeshFaces.resize(m_iNumMeshes);
	m_vecChildrenFaces.resize(OC_END);
	for (auto& vecChildFaces : m_vecChildrenFaces)
		vecChildFaces.resize(m_iNumMeshes);
	m_vecConstantNames = _vecConstantNames;

	SetUp_Edges(vCenter, vHalfExtents);


	vector<vector<FACE>> vecMeshResultFaces(m_iNumMeshes);
	vecMeshResultFaces = _vecMeshFaces;

	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++)
		IdentifyOctant(iMeshIdx, _vecVerticesPtrs[iMeshIdx], _vecNumVertices[iMeshIdx], vecMeshResultFaces[iMeshIdx]);
	

	// Create Mesh
	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++) {
		if (_vecMeshFaces[iMeshIdx].empty())
			continue;

		CMesh* pMyMesh = CMesh::Create(pDevice, pContext, _vecVerticesPtrs[iMeshIdx], _vecNumVertices[iMeshIdx], _vecNormalPtrs[iMeshIdx]
			, _vecTexCoordsPtrs[iMeshIdx], _vecTangentsPtrs[iMeshIdx], _vecMeshFaces[iMeshIdx]);

		if (nullptr == pMyMesh)
			continue;

		m_vecMeshes.push_back(pMyMesh);
		m_vecMaterials.push_back(_vecSortedMaterials[iMeshIdx]);
		for (auto& tex : _vecSortedMaterials[iMeshIdx])
			Safe_AddRef(tex);
		m_vecPassIndices.push_back(_vecPassIndices[iMeshIdx]);
		m_vecSamplingFactors.push_back(_vecSamplingFactors[iMeshIdx]);
	}

	// Create MyMesh
	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++) {
		if (m_vecMeshFaces[iMeshIdx].empty())
			continue;

		CMesh* pMyMesh = CMesh::Create(pDevice, pContext, _vecVerticesPtrs[iMeshIdx], _vecNumVertices[iMeshIdx], _vecNormalPtrs[iMeshIdx]
			, _vecTexCoordsPtrs[iMeshIdx], _vecTangentsPtrs[iMeshIdx], m_vecMeshFaces[iMeshIdx]);

		if (nullptr == pMyMesh)
			continue;

		m_vecMyMeshes.push_back(pMyMesh);
		m_vecMyMaterials.push_back(_vecSortedMaterials[iMeshIdx]);
		for(auto& tex : _vecSortedMaterials[iMeshIdx])
			Safe_AddRef(tex);
		m_vecMyPassIndices.push_back(_vecPassIndices[iMeshIdx]);
		m_vecMySamplingFactors.push_back(_vecSamplingFactors[iMeshIdx]);
	}

	_uint iTotal{};
	for (auto& mesh : m_vecMeshes)
		iTotal += mesh->Get_NumIndices();

	if (m_vecMeshes.empty() || iTotal / m_vecMeshes.size() < 4096
		|| abs(XMVectorGetX(XMVector3Length(m_vecEdges[OC_XYZ] - m_vecEdges[OC_xyz]))) < 36)
		return S_OK;

	// Create Children
	vector<_float3> vecChildrenCenters(OC_END);

	_float3 vQuarterExtents = _float3(vHalfExtents.x * 0.5f, vHalfExtents.y * 0.5f, vHalfExtents.z * 0.5f);
	SetUp_ChildrenCenter(vCenter, vQuarterExtents, vecChildrenCenters);

	for (_int j = 0; j < OC_END; j++) {
		m_vecChildren[j] = COcTree::Create(pDevice, pContext, vecChildrenCenters[j], vQuarterExtents
			, _vecVerticesPtrs, _vecNumVertices, _vecNormalPtrs, _vecTexCoordsPtrs, _vecTangentsPtrs
			, m_vecChildrenFaces[j], _vecSortedMaterials, _vecPassIndices, _vecSamplingFactors, _vecConstantNames);
	}
	
	return S_OK;
}

HRESULT COcTree::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& octreeFile, vector<string>& _vecConstantNames)
{
	m_vecConstantNames = _vecConstantNames;
	m_vecChildren.resize(OC_END);
	Load_OctreeData(pDevice, pContext, octreeFile);

	string strStopOrRead;
	strStopOrRead.resize(4);
	octreeFile.read(&strStopOrRead[0], 4);
	if (strStopOrRead[0] == 'S')
		return S_OK;

	for (_uint i = 0; i < OC_END; i++)
		m_vecChildren[i] = COcTree::Create(pDevice, pContext, octreeFile, _vecConstantNames);

	return S_OK;
}

void COcTree::Culling(CGameInstance* pGameInstance, CShader* pShaderCom, _uint& iRenderAll, _uint& iRenderMyMesh)
{
	_float	fRadius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vecEdges[OC_XYZ]) - XMLoadFloat3(&m_vCenter)));

	_uint iInFrustum{};
	for (_uint iEdgeIdx = 0; iEdgeIdx < OC_END; iEdgeIdx++) {
		if (true == pGameInstance->isInFrustum_WorldSpace(m_vecEdges[iEdgeIdx], fRadius))
			iInFrustum++;
	}

	if (iInFrustum >= OC_END) {
		RenderAll(pShaderCom);
		iRenderAll++;
		return;
	}
		
	else if (iInFrustum > 0)
	{
		if (nullptr == m_vecChildren[OC_XYZ]) {
			RenderAll(pShaderCom);
			iRenderAll++;
			return;
		}
		
		RenderMyMesh(pShaderCom);
		iRenderMyMesh++;

		for (auto& child : m_vecChildren)
			child->Culling(pGameInstance, pShaderCom, iRenderAll, iRenderMyMesh);
	}
}

_bool COcTree::IsDrawable(CGameInstance* pGameInstance)
{
	_vector vCamPos = pGameInstance->Get_CamPosition();
	_float fCenterDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vCenter) - vCamPos));
	_float	fWidth = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vecEdges[OC_XYZ]) - XMLoadFloat3(&m_vecEdges[OC_xyz])));

	if (fCenterDistance > fWidth * 0.51f)
		return true;

	return false;
}

void COcTree::IdentifyOctant(_uint _iMeshIdx, const _float3* _pVerticesPos, const _uint _iNumVertices, const vector<FACE>& _vecMeshFaces)
{
	OCTANT eOctant = { OC_END };
	_int iCount{};

	for (auto& face : _vecMeshFaces)
	{
		_float3 vA = _pVerticesPos[face.iA];
		_float3 vB = _pVerticesPos[face.iB];
		_float3 vC = _pVerticesPos[face.iC];

		eOctant = FinalOctant(vA, vB, vC);

		if (eOctant == OC_END)
			m_vecMeshFaces[_iMeshIdx].emplace_back(face);
		else
			m_vecChildrenFaces[eOctant][_iMeshIdx].emplace_back(face);
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
	fileOutput.write(reinterpret_cast<const char*>(&m_vCenter), sizeof(m_vCenter));

	for(auto& edge : m_vecEdges)
		fileOutput.write(reinterpret_cast<const char*>(&edge), sizeof(edge));

	_uint iNumMeshes = m_vecMeshes.size();
	fileOutput.write(reinterpret_cast<const char*>(&iNumMeshes), sizeof(iNumMeshes));
	for (_uint iMeshIdx = 0; iMeshIdx < iNumMeshes; iMeshIdx++)
	{
		_uint iNumVertices = m_vecMeshes[iMeshIdx]->Get_NumVertices();
		fileOutput.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(iNumVertices));

		_float3* pVertices = m_vecMeshes[iMeshIdx]->Get_VerticesPtr();
		_float3* pNormals = m_vecMeshes[iMeshIdx]->Get_NormalsPtr();
		_float2* pTexCoords = m_vecMeshes[iMeshIdx]->Get_TexCoordsPtr();
		_float3* pTangents = m_vecMeshes[iMeshIdx]->Get_TangentsPtr();

		for (_uint iVertexIdx = 0; iVertexIdx < iNumVertices; iVertexIdx++) {
			fileOutput.write(reinterpret_cast<const char*>(&pVertices[iVertexIdx]), sizeof(pVertices[iVertexIdx]));
			fileOutput.write(reinterpret_cast<const char*>(&pNormals[iVertexIdx]), sizeof(pNormals[iVertexIdx]));
			fileOutput.write(reinterpret_cast<const char*>(&pTexCoords[iVertexIdx]), sizeof(pTexCoords[iVertexIdx]));
			fileOutput.write(reinterpret_cast<const char*>(&pTangents[iVertexIdx]), sizeof(pTangents[iVertexIdx]));
		}

		_uint iNumIndices = m_vecMeshes[iMeshIdx]->Get_NumIndices();
		fileOutput.write(reinterpret_cast<const char*>(&iNumIndices), sizeof(iNumIndices));
		
		_uint* pIndices = m_vecMeshes[iMeshIdx]->Get_IndicesPtr();
		for(_uint iIndex = 0; iIndex < iNumIndices; iIndex++)
			fileOutput.write(reinterpret_cast<const char*>(&pIndices[iIndex]), sizeof(pIndices[iIndex]));

		for (_uint iMatIdx = 0; iMatIdx < TEX_END; iMatIdx++) {
			if(nullptr == m_vecMaterials[iMeshIdx][iMatIdx])
				fileOutput.write(reinterpret_cast<const char*>(&m_iZero), sizeof(m_iZero));
			else
			{
				string strTexPath = CUtils::WstrToStr(m_vecMaterials[iMeshIdx][iMatIdx]->Get_TexturePath());
				_uint iStrLength = strTexPath.length();
				fileOutput.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
				fileOutput.write(strTexPath.c_str(), iStrLength);
			}
		}

		fileOutput.write(reinterpret_cast<const char*>(&m_vecPassIndices[iMeshIdx]), sizeof(m_vecPassIndices[iMeshIdx]));
		fileOutput.write(reinterpret_cast<const char*>(&m_vecSamplingFactors[iMeshIdx]), sizeof(m_vecSamplingFactors[iMeshIdx]));
	}

	_uint iNumMyMeshes = m_vecMyMeshes.size();
	fileOutput.write(reinterpret_cast<const char*>(&iNumMyMeshes), sizeof(iNumMyMeshes));
	for (_uint iMeshIdx = 0; iMeshIdx < iNumMyMeshes; iMeshIdx++)
	{
		_uint iNumVertices = m_vecMyMeshes[iMeshIdx]->Get_NumVertices();
		fileOutput.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(iNumVertices));

		_float3* pVertices = m_vecMyMeshes[iMeshIdx]->Get_VerticesPtr();
		_float3* pNormals = m_vecMyMeshes[iMeshIdx]->Get_NormalsPtr();
		_float2* pTexCoords = m_vecMyMeshes[iMeshIdx]->Get_TexCoordsPtr();
		_float3* pTangents = m_vecMyMeshes[iMeshIdx]->Get_TangentsPtr();

		for (_uint iVertexIdx = 0; iVertexIdx < iNumVertices; iVertexIdx++) {
			fileOutput.write(reinterpret_cast<const char*>(&pVertices[iVertexIdx]), sizeof(pVertices[iVertexIdx]));
			fileOutput.write(reinterpret_cast<const char*>(&pNormals[iVertexIdx]), sizeof(pNormals[iVertexIdx]));
			fileOutput.write(reinterpret_cast<const char*>(&pTexCoords[iVertexIdx]), sizeof(pTexCoords[iVertexIdx]));
			fileOutput.write(reinterpret_cast<const char*>(&pTangents[iVertexIdx]), sizeof(pTangents[iVertexIdx]));
		}

		_uint iNumIndices = m_vecMyMeshes[iMeshIdx]->Get_NumIndices();
		fileOutput.write(reinterpret_cast<const char*>(&iNumIndices), sizeof(iNumIndices));

		_uint* pIndices = m_vecMyMeshes[iMeshIdx]->Get_IndicesPtr();
		for (_uint iIndex = 0; iIndex < iNumIndices; iIndex++)
			fileOutput.write(reinterpret_cast<const char*>(&pIndices[iIndex]), sizeof(pIndices[iIndex]));

		for (_uint iMatIdx = 0; iMatIdx < TEX_END; iMatIdx++) {
			if (nullptr == m_vecMyMaterials[iMeshIdx][iMatIdx])
				fileOutput.write(reinterpret_cast<const char*>(&m_iZero), sizeof(m_iZero));
			else
			{
				string strTexPath = CUtils::WstrToStr(m_vecMyMaterials[iMeshIdx][iMatIdx]->Get_TexturePath());
				_uint iStrLength = strTexPath.length();
				fileOutput.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
				fileOutput.write(strTexPath.c_str(), iStrLength);
			}
		}

		fileOutput.write(reinterpret_cast<const char*>(&m_vecMyPassIndices[iMeshIdx]), sizeof(m_vecMyPassIndices[iMeshIdx]));
		fileOutput.write(reinterpret_cast<const char*>(&m_vecMySamplingFactors[iMeshIdx]), sizeof(m_vecMySamplingFactors[iMeshIdx]));
	}

	if (nullptr == m_vecChildren[OC_XYZ]) {
		string strStop = "Stop";
		fileOutput.write(strStop.c_str(), 4);
		return;
	}

	string strRead = "Read";
	fileOutput.write(strRead.c_str(), 4);

	for (auto& child : m_vecChildren)
		child->Save_OctreeData(fileOutput);
}

void COcTree::Load_OctreeData(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileInput)
{
	m_vecEdges.resize(OC_END);
	fileInput.read(reinterpret_cast<char*>(&m_vCenter), sizeof(m_vCenter));

	for (_int iEdgeIdx = 0; iEdgeIdx < OC_END; iEdgeIdx++)
		fileInput.read(reinterpret_cast<char*>(&m_vecEdges[iEdgeIdx]), sizeof(m_vecEdges[iEdgeIdx]));

	fileInput.read(reinterpret_cast<char*>(&m_iNumMeshes), sizeof(m_iNumMeshes));
	m_vecMeshes.reserve(m_iNumMeshes);
	m_vecPassIndices.resize(m_iNumMeshes);
	m_vecSamplingFactors.resize(m_iNumMeshes);
	m_vecMaterials.resize(m_iNumMeshes);
	for (auto& vecTex : m_vecMaterials)
		vecTex.resize(TEX_END);

	for (_uint iMeshIdx = 0; iMeshIdx < m_iNumMeshes; iMeshIdx++)
	{
		_uint iNumVertices{};
		fileInput.read(reinterpret_cast<char*>(&iNumVertices), sizeof(iNumVertices));

		_float3* pVertices = new _float3[iNumVertices];
		_float3* pNormals = new _float3[iNumVertices];
		_float2* pTexCoords = new _float2[iNumVertices];
		_float3* pTangents = new _float3[iNumVertices];

		for (_uint iVertexIdx = 0; iVertexIdx < iNumVertices; iVertexIdx++) {
			fileInput.read(reinterpret_cast<char*>(&pVertices[iVertexIdx]), sizeof(pVertices[iVertexIdx]));
			fileInput.read(reinterpret_cast<char*>(&pNormals[iVertexIdx]), sizeof(pNormals[iVertexIdx]));
			fileInput.read(reinterpret_cast<char*>(&pTexCoords[iVertexIdx]), sizeof(pTexCoords[iVertexIdx]));
			fileInput.read(reinterpret_cast<char*>(&pTangents[iVertexIdx]), sizeof(pTangents[iVertexIdx]));
		}

		_uint iNumIndices{};
		fileInput.read(reinterpret_cast<char*>(&iNumIndices), sizeof(iNumIndices));

		_uint* pIndices = new _uint[iNumIndices];
		for (_uint iIndex = 0; iIndex < iNumIndices; iIndex++)
			fileInput.read(reinterpret_cast<char*>(&pIndices[iIndex]), sizeof(pIndices[iIndex]));

		CMesh* pMesh = CMesh::Create(pDevice, pContext, pVertices, iNumVertices, pNormals
			, pTexCoords, pTangents, pIndices, iNumIndices);

		Safe_Delete_Array(pVertices);
		Safe_Delete_Array(pNormals);
		Safe_Delete_Array(pTexCoords);
		Safe_Delete_Array(pTangents);
		Safe_Delete_Array(pIndices);

		if (nullptr == pMesh)
			return;

		m_vecMeshes.push_back(pMesh);
		
		_uint iStrLength{};
		for (_uint iMatIdx = 0; iMatIdx < TEX_END; iMatIdx++) {
			fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));

			if (iStrLength == 0)
				continue;

			string strPath;
			strPath.resize(iStrLength);
			fileInput.read(&strPath[0], iStrLength);
			wstring wstrPath = CUtils::StrToWstr(strPath);
			CTexture* pTexture = CTexture::Create(pDevice, pContext, wstrPath);
			if (nullptr == pTexture)
				return;

			m_vecMaterials[iMeshIdx][iMatIdx] = pTexture;
		}

		fileInput.read(reinterpret_cast<char*>(&m_vecPassIndices[iMeshIdx]), sizeof(m_vecPassIndices[iMeshIdx]));
		fileInput.read(reinterpret_cast<char*>(&m_vecSamplingFactors[iMeshIdx]), sizeof(m_vecSamplingFactors[iMeshIdx]));
	}

	_uint iNumMyMeshes{};
	fileInput.read(reinterpret_cast<char*>(&iNumMyMeshes), sizeof(iNumMyMeshes));
	m_vecMyMeshes.reserve(iNumMyMeshes);
	m_vecMyPassIndices.resize(iNumMyMeshes);
	m_vecMySamplingFactors.resize(iNumMyMeshes);
	m_vecMyMaterials.resize(iNumMyMeshes);
	for (auto& vecTex : m_vecMyMaterials)
		vecTex.resize(TEX_END);

	for (_uint iMyMeshIdx = 0; iMyMeshIdx < iNumMyMeshes; iMyMeshIdx++)
	{
		_uint iNumVertices{};
		fileInput.read(reinterpret_cast<char*>(&iNumVertices), sizeof(iNumVertices));

		_float3* pVertices = new _float3[iNumVertices];
		_float3* pNormals = new _float3[iNumVertices];
		_float2* pTexCoords = new _float2[iNumVertices];
		_float3* pTangents = new _float3[iNumVertices];

		for (_uint iVertexIdx = 0; iVertexIdx < iNumVertices; iVertexIdx++) {
			fileInput.read(reinterpret_cast<char*>(&pVertices[iVertexIdx]), sizeof(pVertices[iVertexIdx]));
			fileInput.read(reinterpret_cast<char*>(&pNormals[iVertexIdx]), sizeof(pNormals[iVertexIdx]));
			fileInput.read(reinterpret_cast<char*>(&pTexCoords[iVertexIdx]), sizeof(pTexCoords[iVertexIdx]));
			fileInput.read(reinterpret_cast<char*>(&pTangents[iVertexIdx]), sizeof(pTangents[iVertexIdx]));
		}

		_uint iNumIndices{};
		fileInput.read(reinterpret_cast<char*>(&iNumIndices), sizeof(iNumIndices));

		_uint* pIndices = new _uint[iNumIndices];
		for (_uint iIndex = 0; iIndex < iNumIndices; iIndex++)
			fileInput.read(reinterpret_cast<char*>(&pIndices[iIndex]), sizeof(pIndices[iIndex]));

		CMesh* pMesh = CMesh::Create(pDevice, pContext, pVertices, iNumVertices, pNormals
			, pTexCoords, pTangents, pIndices, iNumIndices);

		Safe_Delete_Array(pVertices);
		Safe_Delete_Array(pNormals);
		Safe_Delete_Array(pTexCoords);
		Safe_Delete_Array(pTangents);
		Safe_Delete_Array(pIndices);

		if (nullptr == pMesh)
			return;

		m_vecMyMeshes.push_back(pMesh);

		_uint iStrLength{};
		for (_uint iMatIdx = 0; iMatIdx < TEX_END; iMatIdx++) {
			fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));

			if (iStrLength == 0)
				continue;
			string strPath;
			
			strPath.resize(iStrLength);
			fileInput.read(&strPath[0], iStrLength);
			wstring wstrPath = CUtils::StrToWstr(strPath);
			CTexture* pTexture = CTexture::Create(pDevice, pContext, wstrPath);
			if (nullptr == pTexture)
				return;

			m_vecMyMaterials[iMyMeshIdx][iMatIdx] = pTexture;
		}

		fileInput.read(reinterpret_cast<char*>(&m_vecMyPassIndices[iMyMeshIdx]), sizeof(m_vecMyPassIndices[iMyMeshIdx]));
		fileInput.read(reinterpret_cast<char*>(&m_vecMySamplingFactors[iMyMeshIdx]), sizeof(m_vecMySamplingFactors[iMyMeshIdx]));
	}
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

void COcTree::RenderAll(CShader* pShaderCom)
{
	for (_uint iMeshIdx = 0; iMeshIdx < m_vecMeshes.size(); iMeshIdx++)
	{
		m_vecMaterials[iMeshIdx][TEX_DIFFUSE]->Bind_ShaderResource(pShaderCom, "g_DiffuseTexture");
		if(nullptr != m_vecMaterials[iMeshIdx][TEX_NORMAL])
			m_vecMaterials[iMeshIdx][TEX_NORMAL]->Bind_ShaderResource(pShaderCom, "g_NormalTexture");
		if (nullptr != m_vecMaterials[iMeshIdx][TEX_MRA])
			m_vecMaterials[iMeshIdx][TEX_MRA]->Bind_ShaderResource(pShaderCom, "g_MRATexture");

		pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_vecSamplingFactors[iMeshIdx], sizeof(_float));
		pShaderCom->Begin(m_vecPassIndices[iMeshIdx]);
		m_vecMeshes[iMeshIdx]->Bind_Buffers();
		m_vecMeshes[iMeshIdx]->Render();
	}
}

void COcTree::RenderMyMesh(CShader* pShaderCom)
{
	for (_uint iMeshIdx = 0; iMeshIdx < m_vecMyMeshes.size(); iMeshIdx++)
	{
		m_vecMyMaterials[iMeshIdx][TEX_DIFFUSE]->Bind_ShaderResource(pShaderCom, "g_DiffuseTexture");
		if(nullptr != m_vecMyMaterials[iMeshIdx][TEX_NORMAL])
			m_vecMyMaterials[iMeshIdx][TEX_NORMAL]->Bind_ShaderResource(pShaderCom, "g_NormalTexture");
		if(nullptr != m_vecMyMaterials[iMeshIdx][TEX_MRA])
			m_vecMyMaterials[iMeshIdx][TEX_MRA]->Bind_ShaderResource(pShaderCom, "g_MRATexture");

		pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_vecMySamplingFactors[iMeshIdx], sizeof(_float));
		pShaderCom->Begin(m_vecMyPassIndices[iMeshIdx]);
		m_vecMyMeshes[iMeshIdx]->Bind_Buffers();
		m_vecMyMeshes[iMeshIdx]->Render();
	}

	if (nullptr == m_vecChildren[OC_XYZ])
		return;

	for (auto& pChild : m_vecChildren)
		pChild->RenderAll(pShaderCom);
}

COcTree* COcTree::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
	, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
	, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
	, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices
	, const vector<class CMesh*>& _vecMeshes, const vector<MESH_MATERIAL>& _vecMaterials
	, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames)
{
	COcTree* pInstance = new COcTree();

	if (FAILED(pInstance->Initialize(pDevice, pContext, vCenter, vHalfExtents, _vecVerticesPtrs, _vecNumVertices
		, _vecNormalPtrs, _vecTexCoordsPtrs, _vecTangentsPtrs,  _vecIndicesPtrs, _vecNumIndices, _vecMeshes
		, _vecMaterials, _vecPassIndices, _vecSamplingFactors, _vecConstantNames)))
	{
		MSG_BOX(TEXT("Failed to Create : COcTree"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

COcTree* COcTree::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
	, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
	, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
	, vector<vector<FACE>>& _vecMeshFaces, const vector<vector<CTexture*>>& _vecSortedMaterials
	, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames)
{
	COcTree* pInstance = new COcTree();

	if (FAILED(pInstance->Initialize(pDevice, pContext, vCenter, vHalfExtents, _vecVerticesPtrs, _vecNumVertices, _vecNormalPtrs
		, _vecTexCoordsPtrs, _vecTangentsPtrs, _vecMeshFaces, _vecSortedMaterials, _vecPassIndices, _vecSamplingFactors
		, _vecConstantNames)))
	{
		MSG_BOX(TEXT("Failed to Create : COcTree"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

COcTree* COcTree::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& octreeFile, vector<string>& _vecConstantNames)
{
	COcTree* pInstance = new COcTree();

	if (FAILED(pInstance->Initialize(pDevice, pContext, octreeFile, _vecConstantNames)))
	{
		MSG_BOX(TEXT("Failed to Create : COcTree"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COcTree::Free()
{
	__super::Free();

	for (auto& mesh : m_vecMeshes)
		Safe_Release(mesh);
	m_vecMeshes.clear();

	for (auto& myMesh : m_vecMyMeshes)
		Safe_Release(myMesh);
	m_vecMyMeshes.clear();

	for (auto& vecTex : m_vecMaterials)
	{
		for(auto& tex: vecTex)
			Safe_Release(tex);
		vecTex.clear();
	}
	m_vecMaterials.clear();

	for (auto& vecMyTex : m_vecMyMaterials)
	{
		for (auto& tex : vecMyTex)
			Safe_Release(tex);
		vecMyTex.clear();
	}
	m_vecMyMaterials.clear();

	for (auto& nonCol : m_vecNonCols)
		Safe_Release(nonCol);

	for (auto& colNonANim : m_vecColNonAnims)
		Safe_Release(colNonANim);

	for (auto& colAnim : m_vecColAnims)
		Safe_Release(colAnim);

	for (auto& myNonCol : m_vecMyNonCols)
		Safe_Release(myNonCol);

	for (auto& myColNonAnim : m_vecMyColNonAnims)
		Safe_Release(myColNonAnim);

	for (auto& myColAnim : m_vecMyColAnims)
		Safe_Release(myColAnim);

	for (_int i = 0; i < OC_END; i++)
		Safe_Release(m_vecChildren[i]);
}
