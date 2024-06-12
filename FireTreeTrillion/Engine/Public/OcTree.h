#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL COcTree final : public CBase
{
public:
	enum OCTANT { OC_XYZ, OC_XyZ, OC_Xyz, OC_XYz, OC_xYZ, OC_xyZ, OC_xyz, OC_xYz, OC_END };
	enum OC_TEX { TEX_DIFFUSE, TEX_NORMAL, TEX_MRA, TEX_END };

private:
	COcTree();
	virtual ~COcTree() = default;

public:
	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
		, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
		, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
		, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices
		, const vector<class CMesh*>& _vecMeshes, const vector<MESH_MATERIAL>& _vecMaterials
		, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames);

	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
		, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
		, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
		, vector<vector<FACE>>& _vecMeshFaces, const vector<vector<class CTexture*>>& _vecSortedMaterials
		, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames);

	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& octreeFile, vector<string>& _vecConstantNames);

	void Culling(class CGameInstance* pGameInstance, class CShader* pShaderCom, _uint& iRenderAll, _uint& iRenderMyMesh);
	_bool IsDrawable(class CGameInstance* pGameInstance);

	void IdentifyOctant(_uint _iMeshIdx, const _float3* _pVerticesPos, const _uint _iNumVertices, const vector<FACE>& _vecMeshFaces); // 어떤 8분면에 속하는지를 검사
	OCTANT CheckOctant(const _float3& vPoint);
	OCTANT FinalOctant(const _float3& _vA, const _float3& _vB, const _float3& _vC);

	void Save_OctreeData(ofstream& fileOutput);
	void Load_OctreeData(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileInput);

	void SetUp_Edges(_float3 vCenter, _float3 vHalfExtents);
	void SetUp_ChildrenCenter(_float3 vCenter, _float3 vQuarterExtents, vector<_float3>& _vecChildrenCenters);

	void RenderAll(class CShader* pShaderCom);
	void RenderMyMesh(class CShader* pShaderCom);

	
	void InsertNonColModels(vector<class CModel*>& _vecNonColModels);
	void InsertColNonAnimModels(vector<class CModel*>& _vecColNonAnimModels);
	void InsertColAnimModels(vector<class CModel*>& _vecColAnimModels);

private:
	_uint					m_iNumMeshes = {};

	_float3					m_vCenter = {};
	vector<_float3>			m_vecEdges;	// 큐브의 꼭짓점들

	vector<COcTree*>		m_vecChildren;
	vector<vector<FACE>>	m_vecMeshFaces; // 각 메쉬의 인덱스 버퍼를 삼각형 단위(3개씩) 저장	
	vector<vector<vector<FACE>>> m_vecChildrenFaces;

	vector<class CMesh*>	m_vecMeshes;
	vector<vector<class CTexture*>>	m_vecMaterials;
	vector<_uint>			m_vecPassIndices;
	vector<_float>			m_vecSamplingFactors;

	vector<class CMesh*>	m_vecMyMeshes;
	vector<vector<class CTexture*>>	m_vecMyMaterials;
	vector<_uint>			m_vecMyPassIndices;
	vector<_float>			m_vecMySamplingFactors;

	vector<class CModel*>	m_vecNonCols;
	vector<class CModel*>	m_vecColNonAnims;
	vector<class CModel*>	m_vecColAnims;
	
	vector<class CModel*>	m_vecMyNonCols;
	vector<class CModel*>	m_vecMyColNonAnims;
	vector<class CModel*>	m_vecMyColAnims;

	vector<string>			m_vecConstantNames;

	_uint					m_iZero = {};
	
public:
	static COcTree* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
		, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
		, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
		, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices
		, const vector<class CMesh*>& _vecMeshes, const vector<MESH_MATERIAL>& _vecMaterials
		, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames);

	static COcTree* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float3 vCenter, _float3 vHalfExtents
		, const vector<_float3*>& _vecVerticesPtrs, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormalPtrs
		, const vector<_float2*>& _vecTexCoordsPtrs, const vector<_float3*>& _vecTangentsPtrs
		, vector<vector<FACE>>& _vecMeshFaces, const vector<vector<class CTexture*>>& _vecSortedMaterials
		, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors, vector<string>& _vecConstantNames);

	static COcTree* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& octreeFile, vector<string>& _vecConstantNames);

public:
	virtual void Free() override;
};

END

