#pragma once

#include "VIBuffer_Instance.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Point final : public CVIBuffer_Instance
{
public:
	typedef struct : public INSTANCE_DESC
	{
		_uint iNumInstance = { 1 };

	}INSTANCE_POINT_DESC;

private:
	CVIBuffer_Instance_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instance_Point(const CVIBuffer_Instance_Point& rhs);
	virtual ~CVIBuffer_Instance_Point() = default;

public:
	virtual _float Compute_RandLifetime() override;
	virtual _float Compute_RandStartDelay() override;

	virtual _float3 Compute_RandScale() override;
	virtual _float3 Compute_RandRotation() override;
	virtual _float4 Compute_RandPosition() override;

	virtual _float4 Compute_RandDirection() override;
	virtual _float4 Compute_RandColor() override;




	//virtual HRESULT Initialize_Prototype(const INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize_Prototype(_uint iNumInstance);
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;



public:
	//static CVIBuffer_Instance_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC& InstanceDesc);
	static CVIBuffer_Instance_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumInstance);
	static CVIBuffer_Instance_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END