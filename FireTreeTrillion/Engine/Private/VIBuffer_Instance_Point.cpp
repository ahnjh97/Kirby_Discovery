#include "VIBuffer_Instance_Point.h"
#include "GameInstance.h"

CVIBuffer_Instance_Point::CVIBuffer_Instance_Point(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer_Instance{ pDevice, pContext }
{
}

CVIBuffer_Instance_Point::CVIBuffer_Instance_Point(const CVIBuffer_Instance_Point & rhs)
	: CVIBuffer_Instance{ rhs }
{
}



//
//HRESULT CVIBuffer_Instance_Point::Initialize_Prototype(const INSTANCE_DESC& InstanceDesc)
//{
//	//인스턴스 디스크립션 저장
//	if (FAILED(__super::Initialize_Prototype(InstanceDesc)))
//		return E_FAIL;
//
//	m_iNumInstance = InstanceDesc.iNumInstance;
//	m_iInstanceStride = sizeof(VTXMATRIX);
//	m_iIndexCountPerInstance = 1;
//
//	m_iNumVertices = 1;
//	m_iVertexStride = sizeof(VTXPOS);
//	
//	m_iNumIndices = m_iIndexCountPerInstance * m_iNumInstance;
//	m_iIndexStride = sizeof(_ushort);
//	m_iNumVertexBuffers = 2;
//	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
//	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
//
//#pragma region VERTEX_BUFFER
//	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
//
//	/* 정점버퍼의 byte크기 */
//	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
//	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	m_BufferDesc.CPUAccessFlags = 0;
//	m_BufferDesc.MiscFlags = 0;
//	m_BufferDesc.StructureByteStride = m_iVertexStride;
//
//	VTXPOS*		pVertices = new VTXPOS[m_iNumVertices];
//	ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);
//
//	pVertices[0].vPosition = _float3(0.0f, 0.0f, 0.f);
//
//	ZeroMemory(&m_InitialData, sizeof m_InitialData);
//	m_InitialData.pSysMem = pVertices;
//
//	if (FAILED(__super::Create_Buffer(&m_pVB)))
//		return E_FAIL;
//
//	Safe_Delete_Array(pVertices);
//#pragma endregion
//
//
//#pragma region INDEX_BUFFER
//
//	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);
//
//	/* 인덱스 버퍼의 byte크기 */
//	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
//	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	m_BufferDesc.CPUAccessFlags = 0;
//	m_BufferDesc.MiscFlags = 0;
//	m_BufferDesc.StructureByteStride = 0;
//
//
//
//	_ushort*		pIndices = new _ushort[m_iNumIndices];
//	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);
//
//	ZeroMemory(&m_InitialData, sizeof m_InitialData);
//	m_InitialData.pSysMem = pIndices;
//
//	if (FAILED(__super::Create_Buffer(&m_pIB)))
//		return E_FAIL;
//
//	Safe_Delete_Array(pIndices);
//
//#pragma endregion
//
//#pragma region INSTANCE_BUFFER
//	
//	ZeroMemory(&m_InstanceBufferDesc, sizeof m_InstanceBufferDesc);
//
//	uniform_real_distribution<float>	ScaleX(InstanceDesc.vMinScale.x, InstanceDesc.vMaxScale.x);
//	uniform_real_distribution<float>	ScaleY(InstanceDesc.vMinScale.y, InstanceDesc.vMaxScale.y);
//	uniform_real_distribution<float>	ScaleZ(InstanceDesc.vMinScale.z, InstanceDesc.vMaxScale.z);
//
//	uniform_real_distribution<float>	LifeTime(InstanceDesc.vLifetime.x, InstanceDesc.vLifetime.y);
//	uniform_real_distribution<float>	Speed(InstanceDesc.vSpeed.x, InstanceDesc.vSpeed.y);
//
//
//	/* 인덱스 버퍼의 byte크기 */
//	m_InstanceBufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance;
//	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
//	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//	m_InstanceBufferDesc.MiscFlags = 0;
//	m_InstanceBufferDesc.StructureByteStride = m_iInstanceStride;
//
//	m_pInstanceVertices = new VTXMATRIX[m_iNumInstance];
//	ZeroMemory(m_pInstanceVertices, sizeof(VTXMATRIX) * m_iNumInstance);
//
//	m_pLifeTimes = new _float2[m_iNumInstance];
//	ZeroMemory(m_pLifeTimes, sizeof(_float2) * m_iNumInstance);
//
//	m_pSpeeds = new _float[m_iNumInstance];
//	ZeroMemory(m_pSpeeds, sizeof(_float) * m_iNumInstance);
//	
//	//for (size_t i = 0; i < m_iNumInstance; i++)
//	//{
//	//	XMStoreFloat4(&m_pInstanceVertices[i].vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f) * ScaleX(m_RandomNumber));
//	//	XMStoreFloat4(&m_pInstanceVertices[i].vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f) * ScaleY(m_RandomNumber));
//	//	XMStoreFloat4(&m_pInstanceVertices[i].vLook, XMVectorSet(0.f, 0.f, 1.f, 0.f) * ScaleZ(m_RandomNumber));
//	//	m_pInstanceVertices[i].vPosition = Compute_RandPosition();
//
//	//	m_pInstanceVertices[i].bAlive = true;
//
//
//	//	/* 0.f 현재 라이프타임 */
//	//	/* LifeTime(m_RandomNumber) 인스턴스마다 랜덤하게 설정된 각각의 라이프타임디다. */
//	//	m_pLifeTimes[i] = _float2(0.f, LifeTime(m_RandomNumber));
//	//	m_pSpeeds[i] = Speed(m_RandomNumber);
//	//}
//
//	ZeroMemory(&m_InstanceSubResourceData, sizeof m_InstanceSubResourceData);
//	m_InstanceSubResourceData.pSysMem = m_pInstanceVertices;
//
//#pragma endregion
//
//	return S_OK;
//}

HRESULT CVIBuffer_Instance_Point::Initialize_Prototype(_uint iNumInstance)
{
	//갯수 저장
	/*if (FAILED(__super::Initialize_Prototype(iNumInstance)))
		return E_FAIL;*/


	return S_OK;
}

HRESULT CVIBuffer_Instance_Point::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer_Instance_Point::Initialize(void * pArg)
{
	INSTANCE_POINT_DESC instanceDesc = {};
	if (nullptr != pArg)
		instanceDesc = *(INSTANCE_POINT_DESC*)pArg;
	else
		return E_FAIL;

	m_iNumInstance = instanceDesc.iNumInstance;
	m_InstanceDesc = instanceDesc;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}


_float CVIBuffer_Instance_Point::Compute_RandLifetime()
{
	return m_InstanceDesc.fLifetime + CUtils::Make_RandomFloat(-m_InstanceDesc.fLifetimeRandomOffset, m_InstanceDesc.fLifetimeRandomOffset);
}

_float CVIBuffer_Instance_Point::Compute_RandStartDelay()
{
	return m_InstanceDesc.fStartDelay + CUtils::Make_RandomFloat(-m_InstanceDesc.fStarDelayRandomOffset, m_InstanceDesc.fStarDelayRandomOffset);
}

_float3 CVIBuffer_Instance_Point::Compute_RandScale()
{
	_float3 vScale = { m_InstanceDesc.vScale.x + CUtils::Make_RandomFloat(-m_InstanceDesc.vScaleRandomOffset.x, m_InstanceDesc.vScaleRandomOffset.x),
					 m_InstanceDesc.vScale.y + CUtils::Make_RandomFloat(-m_InstanceDesc.vScaleRandomOffset.y, m_InstanceDesc.vScaleRandomOffset.y),
					 m_InstanceDesc.vScale.z + CUtils::Make_RandomFloat(-m_InstanceDesc.vScaleRandomOffset.z, m_InstanceDesc.vScaleRandomOffset.z) };


	return vScale * m_InstanceDesc.vInitScale;
}

_float3 CVIBuffer_Instance_Point::Compute_RandRotation()
{
	return { m_InstanceDesc.vRotation.x + CUtils::Make_RandomFloat(-m_InstanceDesc.vRotationRandomOffset.x, m_InstanceDesc.vRotationRandomOffset.x),
		m_InstanceDesc.vRotation.y + CUtils::Make_RandomFloat(-m_InstanceDesc.vRotationRandomOffset.y, m_InstanceDesc.vRotationRandomOffset.y),
		m_InstanceDesc.vRotation.z + CUtils::Make_RandomFloat(-m_InstanceDesc.vRotationRandomOffset.z, m_InstanceDesc.vRotationRandomOffset.z) };
}


_float4 CVIBuffer_Instance_Point::Compute_RandPosition()
{
	_float3 vPosition = {	m_InstanceDesc.vCenter.x + CUtils::Make_RandomFloat(-m_InstanceDesc.vRange.x, m_InstanceDesc.vRange.x) * m_InstanceDesc.vInitScale.x,
							m_InstanceDesc.vCenter.y + CUtils::Make_RandomFloat(-m_InstanceDesc.vRange.y, m_InstanceDesc.vRange.y) * m_InstanceDesc.vInitScale.y,
							m_InstanceDesc.vCenter.z + CUtils::Make_RandomFloat(-m_InstanceDesc.vRange.z, m_InstanceDesc.vRange.z) * m_InstanceDesc.vInitScale.z };


	return Pos(vPosition);
}

_float4 CVIBuffer_Instance_Point::Compute_RandDirection()
{
	return {	m_InstanceDesc.vDir.x + CUtils::Make_RandomFloat(-m_InstanceDesc.vDirRandomOffset.x, m_InstanceDesc.vDirRandomOffset.x),
				m_InstanceDesc.vDir.y + CUtils::Make_RandomFloat(-m_InstanceDesc.vDirRandomOffset.y, m_InstanceDesc.vDirRandomOffset.y),
				m_InstanceDesc.vDir.z + CUtils::Make_RandomFloat(-m_InstanceDesc.vDirRandomOffset.z, m_InstanceDesc.vDirRandomOffset.z),
				m_InstanceDesc.fSpeed + CUtils::Make_RandomFloat(-m_InstanceDesc.fSpeedRandomOffset,m_InstanceDesc.fSpeedRandomOffset)};
}

_float CVIBuffer_Instance_Point::Compute_RandOrbitSpeed()
{
	return {	m_InstanceDesc.fOrbitSpeed + CUtils::Make_RandomFloat(-m_InstanceDesc.fOrbitSpeedRandomOffset, m_InstanceDesc.fOrbitSpeedRandomOffset), };
}

_float4 CVIBuffer_Instance_Point::Compute_RandColor()
{
	return { /*SATURATE*/(m_InstanceDesc.vColor.x + CUtils::Make_RandomFloat(-m_InstanceDesc.vColorRandomOffset.x, m_InstanceDesc.vColorRandomOffset.x)),
				/*SATURATE*/(m_InstanceDesc.vColor.y + CUtils::Make_RandomFloat(-m_InstanceDesc.vColorRandomOffset.y, m_InstanceDesc.vColorRandomOffset.y)),
				/*SATURATE*/(m_InstanceDesc.vColor.z + CUtils::Make_RandomFloat(-m_InstanceDesc.vColorRandomOffset.z, m_InstanceDesc.vColorRandomOffset.z)),
				/*SATURATE*/(m_InstanceDesc.fAlpha + CUtils::Make_RandomFloat(-m_InstanceDesc.fAlphaRandomOffset, m_InstanceDesc.fAlphaRandomOffset)) };
}

_float4 CVIBuffer_Instance_Point::Compute_RandRangePosition()
{
	if (m_InstanceDesc.fMinRange >= m_InstanceDesc.fMaxRange)
		return _float4();

	_float3 vCenter = m_InstanceDesc.vCenter;

	_float4 vPos = vCenter + (_float3)CUtils::Make_Random_Vector(CUtils::Make_RandomFloat(m_InstanceDesc.fMinRange, m_InstanceDesc.fMaxRange));

	_float fMinX = vCenter.x - m_InstanceDesc.vRange.x;
	_float fMaxX = vCenter.x + m_InstanceDesc.vRange.x;
	_float fMinY = vCenter.y - m_InstanceDesc.vRange.y;
	_float fMaxY = vCenter.y + m_InstanceDesc.vRange.y;
	_float fMinZ = vCenter.z - m_InstanceDesc.vRange.z;
	_float fMaxZ = vCenter.z + m_InstanceDesc.vRange.z;

	if (vPos.x < fMinX || vPos.x > fMaxX)
		vPos.x = MAPVALUE(vPos.x, vCenter.x - m_InstanceDesc.fMaxRange, vCenter.x + m_InstanceDesc.fMaxRange, fMinX, fMaxX);
	if (vPos.y < fMinY || vPos.y > fMaxY)
		vPos.y = MAPVALUE(vPos.y, vCenter.y - m_InstanceDesc.fMaxRange, vCenter.y + m_InstanceDesc.fMaxRange, fMinY, fMaxY);
	if (vPos.z < fMinZ || vPos.z > fMaxZ)
		vPos.z = MAPVALUE(vPos.z, vCenter.z - m_InstanceDesc.fMaxRange, vCenter.z + m_InstanceDesc.fMaxRange, fMinZ, fMaxZ);

	return vPos;
}


//CVIBuffer_Instance_Point * CVIBuffer_Instance_Point::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const INSTANCE_DESC& InstanceDesc)
//{
//	CVIBuffer_Instance_Point*		pInstance = new CVIBuffer_Instance_Point(pDevice, pContext);
//
//	if (FAILED(pInstance->Initialize_Prototype(InstanceDesc)))
//	{
//		MSG_BOX(TEXT("Failed To Create : CVIBuffer_Instance_Point"));
//
//		Safe_Release(pInstance);
//	}
//
//	return pInstance;
//
//}

CVIBuffer_Instance_Point* CVIBuffer_Instance_Point::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumInstance)
{
	CVIBuffer_Instance_Point* pInstance = new CVIBuffer_Instance_Point(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(iNumInstance)))
	{
		MSG_BOX(TEXT("Failed To Create : CVIBuffer_Instance_Point"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CVIBuffer_Instance_Point* CVIBuffer_Instance_Point::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Instance_Point* pInstance = new CVIBuffer_Instance_Point(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CVIBuffer_Instance_Point"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Instance_Point::Clone(void * pArg)
{
	CComponent*		pInstance = new CVIBuffer_Instance_Point(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CVIBuffer_Instance_Point"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Instance_Point::Free()
{
	__super::Free();

}
