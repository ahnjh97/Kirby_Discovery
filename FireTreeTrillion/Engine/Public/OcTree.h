#pragma once
#include "Base.h"

BEGIN(Engine)

class COcTree final : public CBase
{
public:
	enum OCTANT { OC_XYZ, OC_XyZ, OC_Xyz, OC_XYz, OC_xYZ, OC_xyZ, OC_xyz, OC_xYz, OC_END };
	enum NEIGHBOR { NEIGHBOR_LEFT, NEIGHBOR_FRONT, NEIGHBOR_RIGHT, NEIGHBOR_BACK, NEIGHBOR_TOP, NEIGHBOR_BOTTOM, NEIGHBOR_END };

private:
	COcTree();
	virtual ~COcTree() = default;

public:
	HRESULT Initialize(_float3 vXYZ, _float3 vXyZ, _float3 vXyz, _float3 vXYz,
		_float3 vxYZ, _float3 vxyZ, _float3 vxyz, _float3 vxYz, const vector<FACE>& _vecFaces);
	HRESULT SetUp_Neighbors();
	void Culling(class CGameInstance* pGameInstance, const _float3* pVerticesPos, _uint* pIndices, _uint* pNumIndices);
	_bool IsOctantInFrustum(class CGameInstance* pGameInstance, const _float3* pVerticesPos);

private:
	_float3				m_vCenter = {};
	_float3				m_vOctants[OC_END] = {};

	class COcTree* m_pChildren[OC_END] = { nullptr, nullptr, nullptr , nullptr, nullptr, nullptr, nullptr , nullptr };
	class COcTree* m_pNeighbors[NEIGHBOR_END] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	vector<class CCell*> m_vecCells;

public:
	static COcTree* Create(_uint iXYZ, _uint iXyZ, _uint iXyz, _uint iXYz, _uint ixYZ, _uint ixyZ, _uint ixyz, _uint ixYz);
	virtual void Free() override;
};

END

