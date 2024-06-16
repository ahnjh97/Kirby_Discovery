#include "MergedMesh.h"
#include "GameInstance.h"

CMergedMesh::CMergedMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{pDevice, pContext}
{
}

CMergedMesh::CMergedMesh(const CMergedMesh& rhs)
	: CVIBuffer{ rhs }
{
}

HRESULT CMergedMesh::Initialize_Prototype(const vector<_float3*>& _vecVertices, const vector<_uint>& _vecNumVertices
	, const vector<_float3*>& _vecNormals, const vector<_uint*>& _vecIndices, const vector<_uint>& _vecNumIndices
	, const vector<_uint>& _vecMaterialIndices, const string& strModelName, _fmatrix TransformMatrix)
{
	_uint iNumMeshes = _vecVertices.size();

	vector<_uint> vecPassIndices(iNumMeshes);
	vector<_float> vecSamplingFactors(iNumMeshes);

	SetUpShaderInfo(strModelName, vecPassIndices, vecSamplingFactors);

	vector<_float> vecVerticeSamplingFactors;
	vector<_uint> vecTextureIndices;
	vector<_float3> vecMergedVerticePoses;
	vector<_float3> vecMergedNormals;
	vector<_uint> vecMergedIndices;
	_uint iVertexOffset{};

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		_float3* pVerticePoses = _vecVertices[i];
		_uint iNumVertices = _vecNumVertices[i];
		_float3* pNormals = _vecNormals[i];

		for (_uint j = 0; j < iNumVertices; j++) {
			vecTextureIndices.emplace_back(_vecMaterialIndices[i]);
			vecVerticeSamplingFactors.emplace_back(vecSamplingFactors[i]);
		}
			
		vecMergedVerticePoses.insert(vecMergedVerticePoses.end(), pVerticePoses, pVerticePoses + iNumVertices);
		vecMergedNormals.insert(vecMergedNormals.end(), pNormals, pNormals + iNumVertices);

		_uint* pIndices = _vecIndices[i];
		_uint iNumIndices = _vecNumIndices[i];
		for (_uint k = 0; k < iNumIndices; k++)
			vecMergedIndices.emplace_back(pIndices[k] + iVertexOffset);

		iVertexOffset += iNumVertices;
	}

	m_iNumVertices = vecMergedVerticePoses.size();
	m_iNumIndices = vecMergedIndices.size();

	m_pVerticesPos = new _float3[m_iNumVertices];

	m_iIndexStride = sizeof(_uint);
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_iVertexStride = sizeof(VTXMERGEDMESH);

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXMERGEDMESH* pVertices = new VTXMERGEDMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMERGEDMESH) * m_iNumVertices);
	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&vecMergedVerticePoses[i]), TransformMatrix));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&vecMergedNormals[i]), TransformMatrix));
		m_pVerticesPos[i] = pVertices[i].vPosition;
		pVertices[i].iTextureIndex = vecTextureIndices[i];
		pVertices[i].fSamplingFactor = vecVerticeSamplingFactors[i];
	}

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);


	//----------- Index Buffer --------------
	m_pIndices = new _uint[m_iNumIndices];
	for (_uint j = 0; j < m_iNumIndices; j++)
		m_pIndices[j] = vecMergedIndices[j];

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 인덱스 버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = m_pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMergedMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMergedMesh::CreateStaticActor(_float4x4& matWorld)
{
	m_pActor = m_pGameInstance->CreateStaticActor(matWorld, m_pVerticesPos, m_iNumVertices, m_pIndices, m_iNumIndices);
	if (m_pActor == nullptr)
		return E_FAIL;

	return S_OK;
}

_float4 CMergedMesh::Get_PickPos(const CTransform* pTransform) const
{
	vector<_float4> vecPickPos;

	_float3 vRayDir, vRayPos;
	m_pGameInstance->Transform_PickingToLocalSpace(pTransform, &vRayDir, &vRayPos);
	_vector vPickPos;
	_float fDis;

	_uint		iNumIndices = { 0 };
	for (size_t i = 0; i < m_iNumIndices / 3; i++)
	{
		_vector v0 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
		_vector v1 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
		_vector v2 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);

		_vector vecRayDir = ::XMVector3Normalize(::XMLoadFloat3(&vRayDir));
		_vector vecRayPos = ::XMLoadFloat3(&vRayPos);
		if (::TriangleTests::Intersects(vecRayPos, vecRayDir, v0, v1, v2, fDis))
		{
			vPickPos = vecRayPos + vecRayDir * fDis;

			_matrix WorldMatrix = XMLoadFloat4x4(&pTransform->Get_WorldMatrix());
			vPickPos = XMVector3TransformCoord(vPickPos, WorldMatrix);
			_float4 tempPos;
			::XMStoreFloat4(&tempPos, vPickPos);
			tempPos.w = fDis;
			if (tempPos.w != 0)
				vecPickPos.push_back(tempPos);
		}
	}

	if (vecPickPos.empty())
		return _float4();

	_float fShortest = { FLT_MAX };
	_float4 fResult = {};
	for (auto& iter : vecPickPos) // 카메라와 가장 거리가 가까운 위치 계산 
	{
		if (iter.w < fShortest) {
			fShortest = iter.w;
			fResult = iter;
		}
	}

	return fResult;
}

void CMergedMesh::Find_MinMax(_float3& vMin, _float3& vMax)
{
	for (_uint i = 0; i < m_iNumVertices; i++)
	{
		if (m_pVerticesPos[i].x < vMin.x)
			vMin.x = m_pVerticesPos[i].x;
		else if (m_pVerticesPos[i].x > vMax.x)
			vMax.x = m_pVerticesPos[i].x;

		if (m_pVerticesPos[i].y < vMin.y)
			vMin.y = m_pVerticesPos[i].y;
		else if (m_pVerticesPos[i].y > vMax.y)
			vMax.y = m_pVerticesPos[i].y;

		if (m_pVerticesPos[i].z < vMin.z)
			vMin.z = m_pVerticesPos[i].z;
		else if (m_pVerticesPos[i].z > vMax.z)
			vMax.z = m_pVerticesPos[i].z;
	}
}

void CMergedMesh::SetUpShaderInfo(const string& strModelName, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors)
{
	fill(_vecSamplingFactors.begin(), _vecSamplingFactors.end(), 1.f);

	string strFilePath = "../../../objects_txt/" + strModelName + "_ShaderInfo.txt";

	fstream fileStream(strFilePath, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to Open: ") + CUtils::StrToWstr(strModelName) + L"_ShaderInfo.txt";
		MSG_BOX(wstrError.c_str());
		return;
	}

	_uint iPassIndex{};
	_float fSamplingFactor{};
	_int iCount{};
	while (!fileStream.eof())
	{
		fileStream.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));
		fileStream.read(reinterpret_cast<char*>(&fSamplingFactor), sizeof(fSamplingFactor));

		if (fileStream.eof())
			break;

		_vecPassIndices[iCount] = iPassIndex;
		_vecSamplingFactors[iCount] = fSamplingFactor;
		iCount++;
	}

	fileStream.close();
}

CMergedMesh* CMergedMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext
	, const vector<_float3*>& _vecVertices, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormals
	, const vector<_uint*>& _vecIndices, const vector<_uint>& _vecNumIndices
	, const vector<_uint>& _vecMaterialIndices, const string& strModelName, _fmatrix TransformMatrix)
{
	CMergedMesh* pInstance = new CMergedMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(_vecVertices, _vecNumVertices, _vecNormals, _vecIndices, _vecNumIndices
		, _vecMaterialIndices, strModelName, TransformMatrix)))
	{
		MSG_BOX(TEXT("Failed To Create : CMergedMesh"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CMergedMesh* CMergedMesh::Clone(void* pArg)
{
	return nullptr;
}

void CMergedMesh::Free()
{
	__super::Free();

	if (false == m_isCloned)
	{
		if (nullptr != m_pActor) {
			PxScene* scene = m_pActor->getScene();
			if (nullptr != scene) {
				scene->removeActor(*m_pActor);
				m_pActor->release();
				m_pActor = nullptr;
			}
		}

		if (nullptr != m_pTriangleMesh) {
			m_pTriangleMesh->release();
			m_pActor = nullptr;
		}
	}
}
