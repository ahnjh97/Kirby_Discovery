#pragma once

#include "VIBuffer_Instance.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Rect final : public CVIBuffer_Instance
{
	
private:
	CVIBuffer_Instance_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instance_Rect(const CVIBuffer_Instance_Rect& rhs);
	virtual ~CVIBuffer_Instance_Rect() = default;

public:
	virtual _float Compute_RandLifetime() override;
	virtual _float Compute_RandStartDelay() override;

	virtual _float3 Compute_RandScale() override;
	virtual _float3 Compute_RandRotation() override;
	virtual _float4 Compute_RandPosition() override;

	virtual _float4 Compute_RandDirection() override;
	virtual _float Compute_RandOrbitSpeed() override;

	virtual _float4 Compute_RandColor() override;
	virtual _float4 Compute_RandTargetColor() override;

	virtual _float4 Compute_RandRangePosition() override;


	//virtual HRESULT Initialize_Prototype(const INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;



public:
	//static CVIBuffer_Instance_Rect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC& InstanceDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END