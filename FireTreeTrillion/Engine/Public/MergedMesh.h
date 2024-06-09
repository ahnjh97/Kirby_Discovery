#pragma once
#include "VIBuffer.h"
#include "Model.h"

BEGIN(Engine)

class CMergedMesh final : public CVIBuffer
{
private:
	CMergedMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMergedMesh(const CMergedMesh& rhs);
	virtual ~CMergedMesh() = default;

public:
	virtual HRESULT Initialize_Prototype(const vector<_float3*>& _vecVertices, const vector<_uint>& _vecNumVertices
		, const vector<_float3*>& _vecNormals, const vector<_uint*>& _vecIndices, const vector<_uint>& _vecNumIndices
		, const vector<_uint>& _vecMaterialIndices , const string& strModelName, _fmatrix TransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT CreateStaticActor(_float4 vPos);
	_float4 Get_PickPos(const class CTransform* pTransform) const;
	void Find_MinMax(_float3& vMin, _float3& vMax);
	void SetUpShaderInfo(const string& strModelName, vector<_uint>& _vecPassIndices, vector<_float>& _vecSamplingFactors);

private:
	_char					m_szName[MAX_PATH] = { "" };

	class PxTriangleMeshGeometry m_TriangleMeshGeometry;
	class PxTriangleMesh* m_pTriangleMesh = { nullptr };
	class PxRigidActor* m_pActor = { nullptr };

	_uint m_iOriginalNumIndices = {};

public:
	static CMergedMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext
		, const vector<_float3*>& _vecVertices, const vector<_uint>& _vecNumVertices, const vector<_float3*>& _vecNormals
		, const vector<_uint*>& _vecIndices, const vector<_uint>& _vecNumIndices
		, const vector<_uint>& _vecMaterialIndices, const string& strModelName, _fmatrix TransformMatrix);
	virtual CMergedMesh* Clone(void* pArg);
	virtual void Free() override;
};

END

