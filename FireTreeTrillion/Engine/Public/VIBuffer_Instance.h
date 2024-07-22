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
	//공전 스피드 계산
	virtual _float Compute_RandOrbitSpeed() = 0;
	//공전 스피드 계산
	virtual _float Compute_RandTurnSpeed() = 0;
	//Color + Alpha 계산
	virtual _float4 Compute_RandColor() = 0;
	virtual _float4 Compute_RandTargetColor() = 0;
	// 범위 기준 랜덤 선택일 때, 랜덤으로 포지셔닝이 된다.
	virtual _float4 Compute_RandRangePosition() = 0;



	_float4 Compute_RectanglePos(_uint iIndex);

	virtual VTXMATRIX* Map();
	virtual void Unmap();

	void Compute_AllLifeTime( _float fTimeDelta);
	void Apply_Velocity(_float fTimeDelta, VTXMATRIX* pVertices);

	//virtual void Assemble(_float fTimeDelta, VTXMATRIX* pVertices);

	virtual void SimpleMove(_float fTimeDelta, VTXMATRIX* pVertices);

	virtual void Drop(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void Spread(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void Appear(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void Disappear(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void Wiggle(_float fTimeDelta, VTXMATRIX* pVertices);

	//꼬리에 꼬리를 문다.
	virtual void Tail(_float fTimeDelta, VTXMATRIX* pVertices);

	//중력을 적용한다.(velocity에 중력 값을 부여한다)
	virtual void Gravity(_float fTimeDelta, VTXMATRIX* pVertices);

	//중점 주위를 돈다.
	virtual void Orbit(_float fTimeDelta, VTXMATRIX* pVertices);

	//중점으로 모인다.
	virtual void Assemble(_float fTimeDelta, VTXMATRIX* pVertices);

	//감속, 가속한다.
	virtual void Acceleration(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void Decelerate(_float fTimeDelta, VTXMATRIX* pVertices);
	//공전 감속, 가속한다.
	virtual void OrbitAcceleration(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void OrbitDecelerate(_float fTimeDelta, VTXMATRIX* pVertices);

	//자전 감속, 가속한다.
	virtual void TurnAcceleration(_float fTimeDelta, VTXMATRIX* pVertices);
	virtual void TurnDecelerate(_float fTimeDelta, VTXMATRIX* pVertices);

	// 색상이 보간된다
	virtual void Color_Interpolate(_float fTimeDelta, VTXMATRIX* pVertices);

	// 자전한다.
	virtual void Turn(_float fTimeDelta, VTXMATRIX* pVertices);
	// 이동 방향으로 회전한다.
	virtual void Turn_MoveDirection(_float fTimeDelta, VTXMATRIX* pVertices, const _float4x4* pSocketMatrix);


	virtual void Save_PrePos(VTXMATRIX* pVertices, const _float4x4* pSocketMatrix);


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

	_float						m_fRemainedDuration = { 0.f };

	//파싱 변수들
	_float2*					m_pLifeTimes = { nullptr };

	_float3*					m_pDirections = { nullptr };
	_float*						m_pSpeeds = { nullptr };

	_float3*					m_pColors = { nullptr };
	_float3*					m_pTargetColors = { nullptr };

	_float*						m_pAlphas = { nullptr };

	// Orbit 도는 속도 (Degree값) 랜덤 필요함
	_float*						m_pOrbitSpeed = { nullptr };
	_float*						m_pTurnSpeed = { nullptr };

	_float*						m_pStartDelays = { nullptr };

	_float*						m_pAccSupplyAmount = { nullptr };
	_float*						m_pOrbitSupplyAmount = { nullptr };
	_float*						m_pTurnSupplyAmount = { nullptr };


	//나머지 계산용 변수들
	_float3*					m_pInitialScales = { nullptr };
	_float*						m_pInitialSpeeds = { nullptr };

	// 로컬을 위한 위치
	_float3*					m_pPrePositions = { nullptr };
	_float3*					m_pPreWorldPositions = { nullptr };

	_float3*					m_pVelocities = { nullptr };

	_float3*					m_pPreAxis = { nullptr };

	_float*						m_fGravity = { nullptr };
	

	//void	Update_Buffer(_uint _iNumInstance);


public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END