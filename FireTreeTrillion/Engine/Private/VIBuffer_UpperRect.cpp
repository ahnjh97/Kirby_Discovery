#include "..\Public\VIBuffer_UpperRect.h"

CVIBuffer_UpperRect::CVIBuffer_UpperRect(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer{ pDevice, pContext }
{

}

CVIBuffer_UpperRect::CVIBuffer_UpperRect(const CVIBuffer_UpperRect & rhs)
	: CVIBuffer{ rhs }
{

}

HRESULT CVIBuffer_UpperRect::Initialize_Prototype()
{
	m_iNumVertices = 4;
	m_iVertexStride = sizeof(VTXPOSTEX);
	m_iNumIndices = 6;
	m_iIndexStride = sizeof(_ushort);
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXPOSTEX*		pVertices = new VTXPOSTEX[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOSTEX) * m_iNumVertices);

	pVertices[0].vPosition = _float3(-0.5f, 1.f, 0.f);
	pVertices[0].vTexcoord = _float2(0.0f, 0.f);

	pVertices[1].vPosition = _float3(0.5f, 1.f, 0.f);
	pVertices[1].vTexcoord = _float2(1.0f, 0.f);

	pVertices[2].vPosition = _float3(0.5f, 0.f, 0.f);
	pVertices[2].vTexcoord = _float2(1.0f, 1.f);

	pVertices[3].vPosition = _float3(-0.5f, 0.f, 0.f);
	pVertices[3].vTexcoord = _float2(0.0f, 1.f);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion


#pragma region INDEX_BUFFER

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 인덱스 버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;

	_ushort*		pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_UpperRect::Initialize(void * pArg)
{
	return S_OK;
}

CVIBuffer_UpperRect * CVIBuffer_UpperRect::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CVIBuffer_UpperRect*		pInstance = new CVIBuffer_UpperRect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CVIBuffer_UpperRect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_UpperRect::Clone(void * pArg)
{
	CVIBuffer_UpperRect*		pInstance = new CVIBuffer_UpperRect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVIBuffer_UpperRect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_UpperRect::Free()
{
	__super::Free();



}
