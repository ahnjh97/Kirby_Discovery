#pragma once

#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance abstract : public CVIBuffer
{
public:


protected:
	CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instance(const CVIBuffer_Instance& rhs);
	virtual ~CVIBuffer_Instance() = default;

public:
	virtual _float	Compute_RandLifetime() = 0;
	virtual _float	Compute_RandStartDelay() = 0;

	virtual _float3 Compute_RandScale() = 0;
	virtual _float3 Compute_RandRotation() = 0;
	virtual _float4 Compute_RandPosition() = 0;

	//Dir(float3) + Speed(float1) 계산
	virtual _float4 Compute_RandDirection() = 0;
	//Color + Alpha 계산
	virtual _float4 Compute_RandColor() = 0;


	virtual void Drop(_float fTimeDelta);
	virtual void Spread(_float fTimeDelta);
	virtual void Decelerate(_float fTimeDelta);

	void Compute_AllLifeTime( _float fTimeDelta);


	void Compute_LifeTime(VTXMATRIX* pVertices, _uint iInstanceIndex, _float fTimeDelta);

	void Change_InstanceInfo(VTXMATRIX* pVertices, _uint iInstanceIndex);
	void Update_InstanceDesc(const INSTANCE_DESC& InstanceDesc);
	void Revive();

public:
	virtual HRESULT Initialize_Prototype(const INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize_Prototype(_uint _iNumInstance);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

	

protected:
	ID3D11Buffer*			m_pVBInstance = { nullptr };
	_uint					m_iInstanceStride = { 0 };
	_uint					m_iNumInstance = { 0 };
	_uint					m_iIndexCountPerInstance = { 0 };
	VTXMATRIX*				m_pInstanceVertices = { nullptr };
	INSTANCE_DESC			m_InstanceDesc{};

protected:
	D3D11_BUFFER_DESC		m_InstanceBufferDesc{};
	D3D11_SUBRESOURCE_DATA	m_InstanceSubResourceData{};

protected:
	random_device				m_RandomDevice;
	mt19937_64					m_RandomNumber;


	_float2*					m_pLifeTimes = { nullptr };
	_float*						m_pStartDelays = { nullptr };
	_float3*					m_pDirections = { nullptr };
	_float3*					m_pInitialScales = { nullptr };
	_float*						m_pInitialSpeeds = { nullptr };
	_float*						m_pSpeeds = { nullptr };
	_float3*					m_pColors = { nullptr };
	_float*						m_pAlphas = { nullptr };

	

	//void	Update_Buffer(_uint _iNumInstance);


public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END