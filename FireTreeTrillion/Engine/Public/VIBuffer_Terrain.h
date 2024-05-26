#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
public:
	_uint Get_NumVerticesX() { return m_iNumVerticesX; }
	_uint Get_NumVerticesZ() { return m_iNumVerticesZ; }
	_float Get_Scale() { return m_fScale; }

private:
	CVIBuffer_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Terrain(const CVIBuffer_Terrain& rhs);
	virtual ~CVIBuffer_Terrain() = default;

public:
	virtual HRESULT Initialize_Prototype(const wstring& strHeightMapFilePath);
	virtual HRESULT Initialize_Prototype(_uint iX, _uint iZ);
	virtual HRESULT Initialize(void* pArg);

public:
	void Culling(_fmatrix WorldMatrixInv);
	_float3 Get_PickPos(const class CTransform* pTransform) const;
private:
	_uint				m_iNumVerticesX = { 0 };
	_uint				m_iNumVerticesZ = { 0 };
	_float				m_fScale = { 10.f };

	class CGameInstance*		m_pGameInstance = { nullptr };
	class CQuadTree*			m_pQuadTree = { nullptr };

public:
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strHeightMapFilePath);
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iX, _uint iZ);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
