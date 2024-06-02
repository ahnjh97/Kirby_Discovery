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
	HRESULT Initialize(const vector<_float3> _vecEdges, const vector<_float3*>& _vecMeshVerticesPtrs, const vector<_uint>& _vecMeshNumVertices
		, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices);

	void Culling(class CGameInstance* pGameInstance, const vector<_float3*>& _vecMeshVerticesPtrs);
	_bool IsDrawable(class CGameInstance* pGameInstance, const vector<_float3*>& _vecMeshVerticesPtrs);

	void IdentifyOctant(); // 어떤 8분면에 속하는지를 검사

private:
	_float3					m_vCenter = {};
	vector<_float3>			m_vecEdges;	// 큐브의 꼭짓점들

	vector<COcTree*>		m_vecChildren;
	vector<vector<FACE>>	m_vecMeshFaces; // 각 메쉬의 인덱스 버퍼를 삼각형 단위(3개씩) 저장

public:
	static COcTree* Create(const vector<_float3> _vecEdges, const vector<_float3*>& _vecMeshVerticesPtrs, const vector<_uint>& _vecMeshNumVertices
		, const vector<_uint*>& _vecIndicesPtrs, const vector<_uint>& _vecNumIndices);
	virtual void Free() override;
};

END

