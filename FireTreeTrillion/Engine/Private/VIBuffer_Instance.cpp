#include "..\Public\VIBuffer_Instance.h"
#include "GameInstance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer{pDevice, pContext }
{
	
}

CVIBuffer_Instance::CVIBuffer_Instance(const CVIBuffer_Instance & rhs)
	: CVIBuffer{ rhs }
	/*, m_pVBInstance{ rhs.m_pVBInstance }*/
	/*, m_iInstanceStride{ rhs.m_iInstanceStride }*/
	, m_iNumInstance{ rhs.m_iNumInstance }
	/*, m_iIndexCountPerInstance{rhs.m_iIndexCountPerInstance }
	, m_pInstanceVertices{ rhs.m_pInstanceVertices }
	, m_InstanceBufferDesc{ rhs.m_InstanceBufferDesc }
	, m_InstanceSubResourceData { rhs.m_InstanceSubResourceData }	
	, m_InstanceDesc { rhs.m_InstanceDesc }*/
{
	
}

HRESULT CVIBuffer_Instance::Initialize_Prototype(const INSTANCE_DESC& InstanceDesc)
{
	m_RandomNumber = mt19937_64(m_RandomDevice());

	m_InstanceDesc = InstanceDesc;

	return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize_Prototype(_uint _iNumInstance)
{
	m_iNumInstance = _iNumInstance;
	return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize(void * pArg)
{

	m_iInstanceStride = sizeof(VTXMATRIX);
	m_iIndexCountPerInstance = 1;

	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOS);

	m_iNumIndices = m_iIndexCountPerInstance * m_iNumInstance;
	m_iIndexStride = sizeof(_ushort);
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점 버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

	//pVertices[0].vPosition = _float3(0.f, 0.f, 0.f);

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



	_ushort* pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion


#pragma region INSTANCE_BUFFER

	ZeroMemory(&m_InstanceBufferDesc, sizeof m_InstanceBufferDesc);

	/* 인덱스 버퍼의 byte크기 */
	m_InstanceBufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance;
	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_InstanceBufferDesc.MiscFlags = 0;
	m_InstanceBufferDesc.StructureByteStride = m_iInstanceStride;

	m_pInstanceVertices = new VTXMATRIX[m_iNumInstance];
	ZeroMemory(m_pInstanceVertices, sizeof(VTXMATRIX) * m_iNumInstance);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		m_pInstanceVertices[i].vRight = _float4{ 1.f, 0.f, 0.f, 0.f };
		m_pInstanceVertices[i].vUp = _float4{ 0.f, 1.f, 0.f, 0.f };
		m_pInstanceVertices[i].vLook = _float4{ 0.f, 0.f, 1.f, 0.f };
		m_pInstanceVertices[i].vPosition = _float4{ 0.f, 0.f, 0.f, 1.f };
		m_pInstanceVertices[i].bAlive = false;
	}

	ZeroMemory(&m_InstanceSubResourceData, sizeof m_InstanceSubResourceData);
	m_InstanceSubResourceData.pSysMem = m_pInstanceVertices;

#pragma endregion


	if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceSubResourceData, &m_pVBInstance)))
		return E_FAIL;


	m_pLifeTimes = new _float2[m_iNumInstance];
	ZeroMemory(m_pLifeTimes, sizeof(_float2) * m_iNumInstance);

	m_pStartDelays = new _float[m_iNumInstance];
	ZeroMemory(m_pStartDelays, sizeof(_float) * m_iNumInstance);

	m_pDirections = new _float3[m_iNumInstance];
	ZeroMemory(m_pDirections, sizeof(_float3) * m_iNumInstance);

	m_pSpeeds = new _float[m_iNumInstance];
	ZeroMemory(m_pSpeeds, sizeof(_float) * m_iNumInstance);

	m_pColors = new _float3[m_iNumInstance];
	ZeroMemory(m_pColors, sizeof(_float3) * m_iNumInstance);

	m_pAlphas = new _float[m_iNumInstance];
	ZeroMemory(m_pAlphas, sizeof(_float) * m_iNumInstance);

	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Buffers()
{
	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance, 
	};

	_uint				iVertexStrides[] = {
		m_iVertexStride, 
		m_iInstanceStride
	};

	_uint				iOffsets[] = {
		0, 
		0
	};

	/* 정점버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);

	/* 인덱스버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	/* 그릴때 어떤 형태로 정점들을 이어 그릴건지. */
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology); 

	return S_OK;
}

HRESULT CVIBuffer_Instance::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	/* 인덱스버퍼의 값을 이용하여 정점 버퍼의 정점들을 그려낸다. */
	m_pContext->DrawIndexedInstanced(m_iIndexCountPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

void CVIBuffer_Instance::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX*		pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		
		if ( !pVertices[i].bAlive || 0.f < m_pStartDelays[i])
		{
			continue;
		}

		pVertices[i].vPosition.y -= m_pSpeeds[i] * fTimeDelta;

		//Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Spread(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX*		pVertices = ((VTXMATRIX*)SubResource.pData);

	

	for (size_t i = 0; i < m_iNumInstance; i++)
	{

		if (!pVertices[i].bAlive || 0.f < m_pStartDelays[i])
		{
			continue;
		}

		_float4		vDir = Dir(pVertices[i].vPosition - Pos(m_InstanceDesc.vPivot));
		
		pVertices[i].vPosition += vDir * m_pSpeeds[i] * fTimeDelta;


		//Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

//Disappear 느낌으로 스케일도 줄여보기
void CVIBuffer_Instance::Decelerate(_float fTimeDelta)
{

	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);
	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);


	for (size_t i = 0; i < m_iNumInstance; i++)
	{

		m_pSpeeds[i] -= fTimeDelta;
		if (m_pSpeeds[i] < 0.f)
			m_pSpeeds[i] = 0.f;

		/*
		if (m_pLifeTimes[i].x / m_pLifeTimes[i].y < .7f)
			continue;


		//m_pSpeeds[i] -= fTimeRatio;

		m_pSpeeds[i] -= fTimeDelta;
		if (m_pSpeeds[i] < 0.f)
			m_pSpeeds[i] = 0.f;
*/

		_float fTimeRatio = (m_pLifeTimes[i].x / m_pLifeTimes[i].y < .7f) ? 1.f :  ((m_pLifeTimes[i].x / m_pLifeTimes[i].y) - .7f) / .3f;
		
		_float4x4 InstanceMat = { _float4x4::Identity};


		InstanceMat.Right(*(_float3*)&pVertices[i].vRight);
		InstanceMat.Up(*(_float3*)&pVertices[i].vUp);
		InstanceMat.Forward(*(_float3*)&pVertices[i].vLook);
		InstanceMat.Translation(*(_float3*)& pVertices[i].vPosition);

		_float3 vScale = CUtils::Get_Scaled_Matrix(InstanceMat);

		vScale *= fTimeRatio;

		CUtils::Set_Scaled_Matrix(InstanceMat, vScale.x, vScale.y, vScale.z);

		pVertices[i].vRight = Dir(InstanceMat.Right());
		pVertices[i].vUp = Dir(InstanceMat.Up());
		pVertices[i].vLook = Dir(InstanceMat.Forward());
		pVertices[i].vPosition = Pos(InstanceMat.Translation());
		
	}

	m_pContext->Unmap(m_pVBInstance, 0); 
}

void CVIBuffer_Instance::Compute_AllLifeTime(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Compute_LifeTime(VTXMATRIX* pVertices, _uint iInstanceIndex, _float fTimeDelta)
{
	if (0.f < m_pStartDelays[iInstanceIndex])
	{
		m_pStartDelays[iInstanceIndex] -= fTimeDelta;
		//pVertices[iInstanceIndex].bAlive = false;

		if (m_pStartDelays[iInstanceIndex] <= 0.f)
		{
			pVertices[iInstanceIndex].bAlive = true;
			m_pStartDelays[iInstanceIndex] = 0.f;
		}
		return;
	}

	m_pLifeTimes[iInstanceIndex].x += fTimeDelta;

	if (m_pLifeTimes[iInstanceIndex].x > m_pLifeTimes[iInstanceIndex].y)
	{
		//루프가 아니였다면 죽어!!
		if(!m_InstanceDesc.bIsLoop)
			pVertices[iInstanceIndex].bAlive = false;
		//아니라면 다시 초기화~
		else
		{
			Change_InstanceInfo(pVertices, iInstanceIndex);
		}
	}

}

void CVIBuffer_Instance::Update_InstanceDesc(const INSTANCE_DESC& _InstanceDesc)
{
	m_InstanceDesc = _InstanceDesc;


	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		Change_InstanceInfo(pVertices, i);
	}

	m_pContext->Unmap(m_pVBInstance, 0);



	//_float3 vScaleRandomOffset = m_InstanceDesc.vScaleRandomOffset;
	//

	////준 desc 대로 값 모두 채워줘야해~~
	//for (_uint i = 0; i < m_iNumInstance; ++i)
	//{
	//	m_pInstanceVertices[i].vRight = Vector4::UnitX * (m_InstanceDesc.vScale.x + CUtils::Make_RandomFloat(-vScaleRandomOffset.x, vScaleRandomOffset.x));
	//	m_pInstanceVertices[i].vUp = Vector4::UnitY * (m_InstanceDesc.vScale.y + CUtils::Make_RandomFloat(-vScaleRandomOffset.y, vScaleRandomOffset.y));
	//	m_pInstanceVertices[i].vLook = Vector4::UnitZ * (m_InstanceDesc.vScale.y + CUtils::Make_RandomFloat(-vScaleRandomOffset.z, vScaleRandomOffset.z));
	//}

}

void CVIBuffer_Instance::Revive()
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].bAlive = false;
		Change_InstanceInfo(pVertices, i);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Change_InstanceInfo(VTXMATRIX* pVertices, _uint iInstanceIndex)
{

	m_pLifeTimes[iInstanceIndex].x = 0.f;

	m_pLifeTimes[iInstanceIndex].y = Compute_RandLifetime();
	m_pStartDelays[iInstanceIndex] = Compute_RandStartDelay();


	_float4x4 instanceMat = _float4x4::Identity;

	_float3 vScale = Compute_RandScale();
	instanceMat.Right() *= vScale.x;
	instanceMat.Up() *= vScale.y;
	instanceMat.Forward() *= vScale.z;

	_float3 vRot = Compute_RandRotation();
	vRot = { vRot.x, vRot.y, vRot.z };
	//vRot = 
	Quaternion vResultQuat = Quaternion::CreateFromYawPitchRoll(vRot);
	instanceMat.Transform(instanceMat, vResultQuat, instanceMat);


	_float3 vPosition = Compute_RandPosition();
	instanceMat.Translation(vPosition);

	pVertices[iInstanceIndex].vRight = Dir(instanceMat.Right());
	pVertices[iInstanceIndex].vUp = Dir(instanceMat.Up());
	pVertices[iInstanceIndex].vLook = Dir(instanceMat.Forward());
	pVertices[iInstanceIndex].vPosition = Pos(instanceMat.Translation());
	pVertices[iInstanceIndex].bAlive = false;

	_float4 vDirection = Compute_RandDirection();
	m_pDirections[iInstanceIndex] = _float3{ vDirection.x, vDirection.y, vDirection.z };
	m_pSpeeds[iInstanceIndex] = vDirection.w;


	_float4 vColor = Compute_RandColor();
	m_pColors[iInstanceIndex] = _float3{ vColor.x, vColor.y, vColor.z };
	m_pAlphas[iInstanceIndex] = vColor.w;

}

//void CVIBuffer_Instance::Update_Buffer(_uint _iNumInstance)
//{
//}

void CVIBuffer_Instance::Free()
{
	__super::Free();


	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pSpeeds);
		Safe_Delete_Array(m_pInstanceVertices);
	}

	Safe_Delete_Array(m_pLifeTimes);
	
	Safe_Release(m_pVBInstance);
}
