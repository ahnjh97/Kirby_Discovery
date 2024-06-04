#pragma once
#include "Base.h"

BEGIN(Engine)

class COcTree final : public CBase
{
public:
	enum OCTANT { OC_XYZ, OC_XyZ, OC_Xyz, OC_XYz, OC_xYZ, OC_xyZ, OC_xyz, OC_xYz, OC_END };

private:
	COcTree();
	virtual ~COcTree() = default;

public:
	HRESULT Initialize(_float3 vCenter, _float3 vHalfExtents, const _float3* _pVerticesPtrs, const _uint _iNumVertices
		, const vector<FACE>& _vecMeshFaces, ifstream& fileInput);

	void Culling(class CGameInstance* pGameInstance, const _float3* _pVerticePoses, _uint _iNumVertices, vector<FACE>& _vecResultFaces);
	_bool IsDrawable(class CGameInstance* pGameInstance, const vector<_float3*>& _vecMeshVerticesPtrs);

	void IdentifyOctant(const _float3* _VerticesPtr, const _uint _iNumVertices, const vector<FACE>& _vecMeshFaces); // 어떤 8분면에 속하는지를 검사
	OCTANT CheckOctant(const _float3& vPoint);
	OCTANT FinalOctant(const _float3& _vA, const _float3& _vB, const _float3& _vC);

	void Save_OctreeData(ofstream& fileOutput);
	_bool Load_OctreeData(ifstream& fileInput);

	void SetUp_Edges(_float3 vCenter, _float3 vHalfExtents);
	void SetUp_ChildrenCenter(_float3 vCenter, _float3 vQuarterExtents, vector<_float3>& _vecChildrenCenters);

private:
	_uint					m_iNumMeshes = {};

	_float3					m_vCenter = {};
	vector<_float3>			m_vecEdges;	// 큐브의 꼭짓점들

	vector<COcTree*>		m_vecChildren;
	vector<FACE>			m_vecMeshFaces; // 각 메쉬의 인덱스 버퍼를 삼각형 단위(3개씩) 저장
	vector<vector<FACE>>	m_vecChidrenMeshFaces; 

public:
	static COcTree* Create(_float3 vCenter, _float3 vHalfExtents, const _float3* _pVerticesPtrs, const _uint _iNumVertices
		, const vector<FACE>& _vecMeshFaces, ifstream& fileInput);
	virtual void Free() override;
};

END

