#include "..\Public\VIBuffer_Instance.h"
#include "GameInstance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{

}

CVIBuffer_Instance::CVIBuffer_Instance(const CVIBuffer_Instance& rhs)
	: CVIBuffer{ rhs }
	, m_iNumInstance{ rhs.m_iNumInstance }
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

HRESULT CVIBuffer_Instance::Initialize(void* pArg)
{
	INSTANCE_DESC tInstanceDesc{};
	if (nullptr != pArg)
		tInstanceDesc = *(INSTANCE_DESC*)(pArg);

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

	if (INSTANCE_SHAPE_END == tInstanceDesc.eInstanceShape)
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			m_pInstanceVertices[i].vRight = _float4{ 1.f, 0.f, 0.f, 0.f };
			m_pInstanceVertices[i].vUp = _float4{ 0.f, 1.f, 0.f, 0.f };
			m_pInstanceVertices[i].vLook = _float4{ 0.f, 0.f, 1.f, 0.f };
			m_pInstanceVertices[i].vPosition = _float4{ 0.f, 0.f, 0.f, 1.f };
			m_pInstanceVertices[i].bAlive = false;
		}
	}
	else if (INSTANCE_SHAPE_RECTANGLE == tInstanceDesc.eInstanceShape)
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			m_pInstanceVertices[i].vRight = _float4{ tInstanceDesc.vScale.x, 0.f, 0.f, 0.f };
			m_pInstanceVertices[i].vUp = _float4{ 0.f, tInstanceDesc.vScale.y, 0.f, 0.f };
			m_pInstanceVertices[i].vLook = _float4{ 0.f, 0.f, tInstanceDesc.vScale.z, 0.f };
			m_pInstanceVertices[i].vPosition = Compute_RectanglePos(i);
			m_pInstanceVertices[i].bAlive = true;
		}
	}
	else if (INSTANCE_SHAPE_SPHERE == tInstanceDesc.eInstanceShape)
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			m_pInstanceVertices[i].vRight = _float4{ tInstanceDesc.vScale.x, 0.f, 0.f, 0.f };
			m_pInstanceVertices[i].vUp = _float4{ 0.f, tInstanceDesc.vScale.y, 0.f, 0.f };
			m_pInstanceVertices[i].vLook = _float4{ 0.f, 0.f, tInstanceDesc.vScale.z, 0.f };
			m_pInstanceVertices[i].vPosition = Compute_RandPosition();
			m_pInstanceVertices[i].bAlive = true;
		}
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

	m_pVelocities = new _float3[m_iNumInstance];
	ZeroMemory(m_pVelocities, sizeof(_float3) * m_iNumInstance);

	m_pInitialSpeeds = new _float[m_iNumInstance];
	ZeroMemory(m_pInitialSpeeds, sizeof(_float) * m_iNumInstance);

	m_pInitialScales = new _float3[m_iNumInstance];
	ZeroMemory(m_pInitialScales, sizeof(_float3) * m_iNumInstance);

	m_pPrePositions = new _float3[m_iNumInstance];
	ZeroMemory(m_pPrePositions, sizeof(_float3) * m_iNumInstance);

	m_pPreWorldPositions = new _float3[m_iNumInstance];
	ZeroMemory(m_pPreWorldPositions, sizeof(_float3) * m_iNumInstance);


	m_pColors = new _float3[m_iNumInstance];
	ZeroMemory(m_pColors, sizeof(_float3) * m_iNumInstance);

	m_pTargetColors = new _float3[m_iNumInstance];
	ZeroMemory(m_pTargetColors, sizeof(_float3) * m_iNumInstance);

	m_pAlphas = new _float[m_iNumInstance];
	ZeroMemory(m_pAlphas, sizeof(_float) * m_iNumInstance);


	// For Orbit
	m_pPreAxis = new _float3[m_iNumInstance];
	ZeroMemory(m_pPreAxis, sizeof(_float3) * m_iNumInstance);

	m_pOrbitSpeed = new _float[m_iNumInstance];
	ZeroMemory(m_pOrbitSpeed, sizeof(_float) * m_iNumInstance);

	m_pTurnSpeed = new _float[m_iNumInstance];
	ZeroMemory(m_pTurnSpeed, sizeof(_float) * m_iNumInstance);

	m_pAccSupplyAmount = new _float[m_iNumInstance];
	ZeroMemory(m_pAccSupplyAmount, sizeof(_float) * m_iNumInstance);

	m_pOrbitSupplyAmount = new _float[m_iNumInstance];
	ZeroMemory(m_pOrbitSupplyAmount, sizeof(_float) * m_iNumInstance);

	m_pTurnSupplyAmount = new _float[m_iNumInstance];
	ZeroMemory(m_pTurnSupplyAmount, sizeof(_float) * m_iNumInstance);
	// 개별적으로 작용하는 중력값
	m_fGravity = new _float[m_iNumInstance];
	ZeroMemory(m_fGravity, sizeof(_float) * m_iNumInstance);

	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Buffers()
{
	ID3D11Buffer* pVertexBuffers[] = {
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

_float4 CVIBuffer_Instance::Compute_RectanglePos(_uint iIndex)
{
	_uint iInstancePerRow = m_InstanceDesc.iNumInstance / m_InstanceDesc.iRows;
	_float fWidth = m_InstanceDesc.vRange.x * 2.f;
	_float fHeight = m_InstanceDesc.vRange.z * 2.f;

	_float fSpacingX = fWidth / (iInstancePerRow + 1);
	_float fSpacingZ = fHeight / (m_InstanceDesc.iRows + 1);

	_uint iRow = iIndex / iInstancePerRow;
	_uint iCol = iIndex % iInstancePerRow;

	_float fX = -m_InstanceDesc.vRange.x + (iCol + 1) * fSpacingX;
	if (iRow % 2 == 1)
		fX += fSpacingX * 0.5f;
	_float fZ = -m_InstanceDesc.vRange.z + (iRow + 1) * fSpacingZ;

	return _float4(fX, 0, fZ, 1);
}

VTXMATRIX* CVIBuffer_Instance::Map()
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	return static_cast<VTXMATRIX*>(SubResource.pData);
}

void CVIBuffer_Instance::Unmap()
{
	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::SimpleMove(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive/* || 0.f < m_pStartDelays[i]*/)
			continue;

		pVertices[i].vPosition += Dir(m_pDirections[i] * m_pSpeeds[i] * m_InstanceDesc.vInitScale *  fTimeDelta);
	}
}

void CVIBuffer_Instance::Drop(_float fTimeDelta, VTXMATRIX* pVertices)
{

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive/* || 0.f < m_pStartDelays[i]*/)
			continue;

		_float4 vPos = pVertices[i].vPosition;
		vPos.y -= m_pSpeeds[i] * fTimeDelta * m_InstanceDesc.vInitScale.y;
		pVertices[i].vPosition = vPos;

		//m_pVelocities[i].y -= m_pSpeeds[i] * fTimeDelta * m_InstanceDesc.vInitScale.y;

	}

}

void CVIBuffer_Instance::Spread(_float fTimeDelta, VTXMATRIX* pVertices)
{
	//D3D11_MAPPED_SUBRESOURCE		SubResource{};

	//m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	//VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);



	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive/* || 0.f < m_pStartDelays[i]*/)
			continue;
		//if (!pVertices[i].bAlive || 0.f < m_pStartDelays[i])
		//{
		//	continue;
		//}
		_float4 vPos = pVertices[i].vPosition;
		_float4	vDir = Dir(vPos - Pos(m_InstanceDesc.vPivot));
		vDir.Normalize();

		pVertices[i].vPosition = vPos + (_float3(vDir) * m_InstanceDesc.vInitScale * m_pSpeeds[i] * fTimeDelta);

		//m_pVelocities[i] += _float3(vDir) * m_InstanceDesc.vInitScale * m_pSpeeds[i] * fTimeDelta;
		//Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	//m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Acceleration(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		m_pSpeeds[i] += fTimeDelta * m_pAccSupplyAmount[i];
	}
}

void CVIBuffer_Instance::Decelerate(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		if (m_pSpeeds[i] == 0.f)
			continue;

		m_pSpeeds[i] -= fTimeDelta * m_pAccSupplyAmount[i];
		if (m_pSpeeds[i] < 0.f) m_pSpeeds[i] = 0.f;

		//if (m_pLifeTimes[i].x / m_pLifeTimes[i].y < .5f)
		//	continue;
		//_float fTimeRatio = ( 1.f - (m_pLifeTimes[i].x / m_pLifeTimes[i].y)) * 2.f;
		////fTimeRatio = EASE_OUT(fTimeRatio);
		//m_pSpeeds[i] = m_pInitialSpeeds[i] * fTimeRatio;
		//if (m_pSpeeds[i] < 0.f)
		//{
		//	m_pSpeeds[i] = 0.f;
		//	m_pLifeTimes[i].x = m_pLifeTimes[i].y;
		//}
	}
}

void CVIBuffer_Instance::OrbitAcceleration(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		m_pOrbitSpeed[i] += fTimeDelta * m_pOrbitSupplyAmount[i];
	}
}

void CVIBuffer_Instance::OrbitDecelerate(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		if (m_pOrbitSpeed[i] == 0.f) 
			continue;

		m_pOrbitSpeed[i] -= fTimeDelta * m_pOrbitSupplyAmount[i];
		if (m_pOrbitSpeed[i] < 0.f) m_pOrbitSpeed[i] = 0.f;
	}
}

void CVIBuffer_Instance::TurnAcceleration(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		m_pTurnSpeed[i] += fTimeDelta * m_pTurnSupplyAmount[i];
	}
}

void CVIBuffer_Instance::TurnDecelerate(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		if (m_pTurnSpeed[i] == 0.f)
			continue;

		m_pTurnSpeed[i] -= fTimeDelta * m_pTurnSupplyAmount[i];
		if (m_pTurnSpeed[i] < 0.f) m_pTurnSpeed[i] = 0.f;
	}
}

void CVIBuffer_Instance::Color_Interpolate(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		_float fColorRatio = SATURATE(m_pLifeTimes[i].x / m_pLifeTimes[i].y);

		pVertices[i].vColor = XMVectorSetW(_float3::Lerp(m_pColors[i], m_pTargetColors[i], fColorRatio), 0.f);

		if (m_pLifeTimes[i].x >= m_pLifeTimes[i].y)
			pVertices[i].vColor = XMVectorSetW(m_pTargetColors[i], 0.f);
	}
}

void CVIBuffer_Instance::Turn(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		pVertices[i].fAngleZ += fTimeDelta * m_pTurnSpeed[i];
	}
}

void CVIBuffer_Instance::Turn_MoveDirection(_float fTimeDelta, VTXMATRIX* pVertices, const _float4x4* pSocketMatrix)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;
		
		_vector vPos = {0.f, 0.f, 0.f, 0.f};

		// 월드상의 포지션으로 만드는 과정
		if (pSocketMatrix == nullptr)
			vPos = pVertices[i].vPosition;
		else
			vPos = _float4::Transform(pVertices[i].vPosition, *pSocketMatrix);

		_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ);
		_vector ScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
		_vector ScreenPrePos = { 0.f, 0.f, 0.f, 0.f };
		if (pSocketMatrix == nullptr)
			ScreenPrePos = XMVector3TransformCoord(m_pPrePositions[i], ViewProjectionMatrix);
		else
			ScreenPrePos = XMVector3TransformCoord(m_pPreWorldPositions[i], ViewProjectionMatrix);

		_float fCenterX = XMVectorGetX(ScreenPos);
		_float fCenterY = XMVectorGetY(ScreenPos);
		_float fAfterX = XMVectorGetX(ScreenPrePos);
		_float fAfterY = XMVectorGetY(ScreenPrePos);

		_float fAngle = (atan2f(fAfterY - fCenterY, fAfterX - fCenterX)) * 180.f / 3.141592f;
		if (fAngle < 0.f)
			fAngle += 360.f;

		pVertices[i].fAngleZ = (fAngle);
	}
}

void CVIBuffer_Instance::Save_PrePos(VTXMATRIX* pVertices, const _float4x4* pSocketMatrix)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		if (_float3::Distance(m_pPrePositions[i], F4toF3(pVertices[i].vPosition)) < .05f)
			continue;


		m_pPrePositions[i] = static_cast<_float3>(pVertices[i].vPosition);

		if (pSocketMatrix != nullptr)
		{
			m_pPreWorldPositions[i] = (_float3)_float4::Transform(pVertices[i].vPosition, *pSocketMatrix);
		}
	}
}


void CVIBuffer_Instance::Appear(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		_float fTimeRatio = clamp((m_pLifeTimes[i].x / m_pLifeTimes[i].y) * 5.f, .01f, 1.f);
		fTimeRatio = SATURATE(EASE_OUT(fTimeRatio));
		if (1.f < fTimeRatio)
			continue;

		_float4x4 InstanceMat = { _float4x4::Identity };

		InstanceMat.Right(*(_float3*)&pVertices[i].vRight);
		InstanceMat.Up(*(_float3*)&pVertices[i].vUp);
		InstanceMat.Forward(*(_float3*)&pVertices[i].vLook);
		InstanceMat.Translation(*(_float3*)&pVertices[i].vPosition);

		_float3 vScale = CUtils::Get_Scaled_Matrix(InstanceMat);

		vScale = m_pInitialScales[i] /** m_InstanceDesc.vInitScale*/ * fTimeRatio;

		CUtils::Set_Scaled_Matrix(InstanceMat, vScale.x, vScale.x, vScale.x);

		pVertices[i].vRight = Dir(InstanceMat.Right());
		pVertices[i].vUp = Dir(InstanceMat.Up());
		pVertices[i].vLook = Dir(InstanceMat.Forward());
		pVertices[i].vPosition = Pos(InstanceMat.Translation());
	}
}


void CVIBuffer_Instance::Disappear(_float fTimeDelta, VTXMATRIX* pVertices)
{

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		if ((m_pLifeTimes[i].x / m_pLifeTimes[i].y) < .5f)
			continue;

		_float fTimeRatio = 1.f - ((m_pLifeTimes[i].x / m_pLifeTimes[i].y) - .5f) * 2.f;
		fTimeRatio = SATURATE(EASE_OUT(fTimeRatio));

		_float4x4 InstanceMat = { _float4x4::Identity };

		InstanceMat.Right(*(_float3*)&pVertices[i].vRight);
		InstanceMat.Up(*(_float3*)&pVertices[i].vUp);
		InstanceMat.Forward(*(_float3*)&pVertices[i].vLook);
		InstanceMat.Translation(*(_float3*)&pVertices[i].vPosition);

		_float3 vScale = CUtils::Get_Scaled_Matrix(InstanceMat);

		vScale = m_pInitialScales[i] /** m_InstanceDesc.vInitScale*/ * fTimeRatio;

		CUtils::Set_Scaled_Matrix(InstanceMat, vScale.x, vScale.x, vScale.x);

		pVertices[i].vRight = Dir(InstanceMat.Right());
		pVertices[i].vUp = Dir(InstanceMat.Up());
		pVertices[i].vLook = Dir(InstanceMat.Forward());
		pVertices[i].vPosition = Pos(InstanceMat.Translation());
	}

}

void CVIBuffer_Instance::Wiggle(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		_matrix rotationMatrix = XMMatrixIdentity();
		rotationMatrix = XMMatrixRotationY(XMConvertToRadians(120.f * fTimeDelta));
		XMStoreFloat3(&m_pDirections[i], XMVector4Transform(XMLoadFloat3(&m_pDirections[i]), rotationMatrix));

		//m_pVelocities[i] += m_pDirections[i] * m_pSpeeds[i] * m_InstanceDesc.vInitScale * fTimeDelta;
		pVertices[i].vPosition += Dir(m_pDirections[i]) * m_pSpeeds[i] * fTimeDelta;
	}
}

void CVIBuffer_Instance::Tail(_float fTimeDelta, VTXMATRIX* pVertices)
{

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		_int iSoloVertice = m_iNumInstance % 10;

		if (i > m_iNumInstance - iSoloVertice)
		{
			pVertices[i].bAlive = false;
			return;
		}

		//꼬리
		if (i % 10 != 0)
		{

			_float4x4 PreInstanceMat = { _float4x4::Identity };

			PreInstanceMat.Right(*(_float3*)&pVertices[i - 1].vRight);
			PreInstanceMat.Up(*(_float3*)&pVertices[i - 1].vUp);
			PreInstanceMat.Forward(*(_float3*)&pVertices[i - 1].vLook);
			_float3 vScale = CUtils::Get_Scaled_Matrix(PreInstanceMat);
			vScale.x = clamp(vScale.x, .01f, 1.f);


			_float4x4 MyInstanceMat = { _float4x4::Identity };

			MyInstanceMat.Right(*(_float3*)&pVertices[i].vRight);
			MyInstanceMat.Up(*(_float3*)&pVertices[i].vUp);
			MyInstanceMat.Forward(*(_float3*)&pVertices[i].vLook);
			MyInstanceMat.Translation(*(_float3*)&pVertices[i].vPosition);

			CUtils::Set_Scaled_Matrix(MyInstanceMat, vScale.x - fTimeDelta * .5f, vScale.x - fTimeDelta * .5f, vScale.x - fTimeDelta * .5f);

			pVertices[i].vRight = Dir(MyInstanceMat.Right());
			pVertices[i].vUp = Dir(MyInstanceMat.Up());
			pVertices[i].vLook = Dir(MyInstanceMat.Forward());
			pVertices[i].vPosition = Pos(m_pPrePositions[i - 1]);

			m_pLifeTimes[i] = m_pLifeTimes[i - 1];
		}
	}

	//for (size_t i = 0; i < m_iNumInstance; i++)
	//{
	//	m_pPrePositions[i] = static_cast<_float3>(pVertices[i].vPosition);
	//}
}

void CVIBuffer_Instance::Gravity(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		//m_pDirections[i].y -= GRAVITY * fTimeDelta;
		_float4 vPos = pVertices[i].vPosition;
		m_fGravity[i] += GRAVITY * .14f * m_InstanceDesc.vInitScale.y * fTimeDelta;
		if (m_fGravity[i] > 13.f)
			m_fGravity[i] = 13.f;
		vPos.y -= m_fGravity[i];
		pVertices[i].vPosition = vPos;

		//m_pVelocities[i].y -= GRAVITY * 2.5f * m_InstanceDesc.vInitScale.y * fTimeDelta;
	}
}

void CVIBuffer_Instance::Orbit(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		_float3 vPos = (_float3)pVertices[i].vPosition;
		_float3 vDistance = vPos - m_InstanceDesc.vPivot;
		_float fOrbitSpeed = m_pOrbitSpeed[i];

		_float4x4 RotMatrix = _float4x4::Identity;

		if (m_pPreAxis[i] != _float3(0.f, 0.f, 0.f))
		{
			CUtils::Turn_OtherMatrix(RotMatrix, m_pPreAxis[i], fTimeDelta, fOrbitSpeed);
		}

		//_float3 vEditDir = XMVector3Transform(vDistance, XMLoadFloat4x4(&RotMatrix));
		_float3 vEditDir = m_InstanceDesc.vPivot + (_float3)XMVector3Transform(vDistance, XMLoadFloat4x4(&RotMatrix));
		pVertices[i].vPosition = XMVectorSetW(vEditDir, 1.f);
	}
}

void CVIBuffer_Instance::Assemble(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!pVertices[i].bAlive)
			continue;

		_float3 vPos = (_float3)pVertices[i].vPosition;
		_float3 vDistance = m_InstanceDesc.vPivot - vPos;

		if (vDistance.Length() < 0.1f)
		{
			m_pLifeTimes[i].x = m_pLifeTimes[i].y;
			return;
		}

		vDistance.Normalize();
		_float fSpeed = m_pSpeeds[i] * m_InstanceDesc.vInitScale.x;

		pVertices[i].vPosition = XMVectorSetW(vPos + (vDistance * fTimeDelta * fSpeed), 1.f);
	}
}


void CVIBuffer_Instance::Compute_AllLifeTime(_float fTimeDelta)
{
	if (0.f < m_fRemainedDuration)
	{
		m_fRemainedDuration -= fTimeDelta;

		if (m_fRemainedDuration <= 0.f)
			m_fRemainedDuration = 0.f;
	}

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Apply_Velocity(_float fTimeDelta, VTXMATRIX* pVertices)
{
	for (size_t i = 0; i < m_iNumInstance; i++)
	{

		_float4x4 InstanceMat = { _float4x4::Identity };

		InstanceMat.Right(*(_float3*)&pVertices[i].vRight);
		InstanceMat.Up(*(_float3*)&pVertices[i].vUp);
		InstanceMat.Forward(*(_float3*)&pVertices[i].vLook);
		InstanceMat.Translation(*(_float3*)&pVertices[i].vPosition);

		_float3 vScale = CUtils::Get_Scaled_Matrix(InstanceMat);

		CUtils::Set_Scaled_Matrix(InstanceMat, vScale.x, vScale.x, vScale.x);

		CUtils::Set_State_Matrix(InstanceMat, CUtils::STATE_POSITION, pVertices[i].vPosition + m_pVelocities[i] * fTimeDelta);

		pVertices[i].vRight = Dir(InstanceMat.Right());
		pVertices[i].vUp = Dir(InstanceMat.Up());
		pVertices[i].vLook = Dir(InstanceMat.Forward());
		pVertices[i].vPosition = Pos(InstanceMat.Translation());

	}
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
	else if (0.f == m_pStartDelays[iInstanceIndex])
		pVertices[iInstanceIndex].bAlive = true;


	m_pLifeTimes[iInstanceIndex].x += fTimeDelta;

	if (m_pLifeTimes[iInstanceIndex].x > m_pLifeTimes[iInstanceIndex].y)
	{
		//루프가 아니였다면 죽어!!
		if (!m_InstanceDesc.bIsLoop)
		{
			pVertices[iInstanceIndex].bAlive = false;
			m_pLifeTimes[iInstanceIndex].x = m_pLifeTimes[iInstanceIndex].y;
		}
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
	m_fRemainedDuration = _InstanceDesc.fDuration;

	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		Change_InstanceInfo(pVertices, i);
	}

	m_pContext->Unmap(m_pVBInstance, 0);

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

	_float fDestLifeTime = Compute_RandLifetime();


	m_pLifeTimes[iInstanceIndex].x = 0.f;

	m_pLifeTimes[iInstanceIndex].y = fDestLifeTime;
	m_pStartDelays[iInstanceIndex] = Compute_RandStartDelay();


	_float4x4 instanceMat = _float4x4::Identity;

	_float3 vScale = Compute_RandScale();
	CUtils::Set_Scaled_Matrix(instanceMat, vScale.x, vScale.x, vScale.x);
	m_pInitialScales[iInstanceIndex] = _float3{ vScale.x, vScale.x, vScale.x };


	_float3 vRot = Compute_RandRotation();
	vRot = { vRot.x, vRot.y, vRot.z };
	pVertices[iInstanceIndex].fAngleZ = ToRadian(vRot.z);

	//Quaternion vResultQuat = Quaternion::CreateFromYawPitchRoll(vRot);
	//instanceMat.Transform(instanceMat, vResultQuat, instanceMat);

	_float3 vPosition = {};

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_SPHERERANDOM] == true)
		vPosition = (_float3)Compute_RandRangePosition();
	else
		vPosition = (_float3)Compute_RandPosition();


	instanceMat.Translation(vPosition);

	pVertices[iInstanceIndex].vRight = Dir(instanceMat.Right());
	pVertices[iInstanceIndex].vUp = Dir(instanceMat.Up());
	pVertices[iInstanceIndex].vLook = Dir(instanceMat.Forward());
	pVertices[iInstanceIndex].vPosition = Pos(instanceMat.Translation());
	pVertices[iInstanceIndex].bAlive = false;


	_float4 vDirection = Compute_RandDirection();

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_WIGGLE] == true)
	{
		m_pDirections[iInstanceIndex] = CUtils::Make_Random_Vector(1.f);
		m_pSpeeds[iInstanceIndex] = vDirection.w;
	}
	else
	{
		m_pDirections[iInstanceIndex] = F4toF3(vDirection);
		m_pSpeeds[iInstanceIndex] = vDirection.w;
	}

	m_pInitialSpeeds[iInstanceIndex] = m_pSpeeds[iInstanceIndex];

	m_pAccSupplyAmount[iInstanceIndex] = m_InstanceDesc.fAccSupplyAmount;
	m_pOrbitSupplyAmount[iInstanceIndex] = m_InstanceDesc.fOrbitSupplyAmount;
	m_pTurnSupplyAmount[iInstanceIndex] = m_InstanceDesc.fTurnSupplyAmount;

	//공전 시의 세팅
	if (m_InstanceDesc.vecMoveCommands[INSTANCE_ORBIT] == true)
	{
		if (m_InstanceDesc.vecMoveCommands[INSTANCE_CUSTOMORBITAXIS] == true)
		{
			m_pPreAxis[iInstanceIndex] = m_InstanceDesc.vRotationAxis;
		}
		else
		{
			_float3 vPos = pVertices[iInstanceIndex].vPosition;
			_float3 vDistance = vPos - m_InstanceDesc.vPivot;
			_float3 vDir = XMVector3Normalize(vDistance);
			if (vDistance == _float3(0.f, 0.f, 0.f))
				return;

			_float3 vDirection = m_pDirections[iInstanceIndex];
			vDirection.Normalize();

			_float fSpeed = m_pSpeeds[iInstanceIndex];
			m_pPreAxis[iInstanceIndex] = vDirection.Cross(vDir);
		}
		m_pOrbitSpeed[iInstanceIndex] = Compute_RandOrbitSpeed();
	}

	//자전 시의 세팅
	if (m_InstanceDesc.vecMoveCommands[INSTANCE_TURN] == true)
	{
		m_pTurnSpeed[iInstanceIndex] = Compute_RandTurnSpeed();
	}

	_float4 vColor = Compute_RandColor();

	pVertices[iInstanceIndex].vColor = vColor;

	m_pColors[iInstanceIndex] = _float3{ vColor.x, vColor.y, vColor.z };
	m_pAlphas[iInstanceIndex] = vColor.w;

	m_pTargetColors[iInstanceIndex] = F4toF3(Compute_RandTargetColor());

	m_pVelocities[iInstanceIndex] = _float3();

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_SIMPLEMOVE] == true)
		m_pVelocities[iInstanceIndex] = m_pDirections[iInstanceIndex] * m_pSpeeds[iInstanceIndex];

	m_pPrePositions[iInstanceIndex] = vPosition;
	m_fGravity[iInstanceIndex] = 0.f;
}
void CVIBuffer_Instance::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pLifeTimes);

	Safe_Delete_Array(m_pDirections);
	Safe_Delete_Array(m_pSpeeds);

	Safe_Delete_Array(m_pColors);
	Safe_Delete_Array(m_pTargetColors);

	Safe_Delete_Array(m_pAlphas);

	Safe_Delete_Array(m_pOrbitSpeed);
	Safe_Delete_Array(m_pTurnSpeed);

	Safe_Delete_Array(m_pStartDelays);

	Safe_Delete_Array(m_pAccSupplyAmount);
	Safe_Delete_Array(m_pOrbitSupplyAmount);
	Safe_Delete_Array(m_pTurnSupplyAmount);

	Safe_Delete_Array(m_pInitialSpeeds);
	Safe_Delete_Array(m_pInitialScales);

	Safe_Delete_Array(m_pPrePositions);
	Safe_Delete_Array(m_pPreWorldPositions);
	Safe_Delete_Array(m_pVelocities);

	Safe_Delete_Array(m_pPreAxis);

	Safe_Delete_Array(m_fGravity);

	Safe_Delete_Array(m_pInstanceVertices);
	Safe_Release(m_pVBInstance);
}
