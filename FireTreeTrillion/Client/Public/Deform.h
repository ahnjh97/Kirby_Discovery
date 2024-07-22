#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"
#include "Effect.h"


BEGIN(Engine)
class CShader;
class CCharacterController;
END

class CDeform : public CPhysXObject
{
public:
	enum DEFORMTYPE { DEFORM_CAR, DEFORM_BULB, DEFORM_END };

protected:
	CDeform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDeform(const CDeform& rhs);
	virtual ~CDeform() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

	DEFORMTYPE		Get_DeformType() { return m_eDeformType; }

protected:
	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	void			Compute_DeformRimLight(_float fTimeDelta);
	_int			m_iComputeMode = { 0 };
	_float			m_fDeformTime = { 0.f };
	_float3			m_vDeformRimColor = { 1.f, 0.f, 0.f };
	_int			m_iRenderCount = { 0 };
	_float			m_fUVOffsetTime = { 0.f };
	_float			m_fDissolveRatio = { 0.f };

	CCharacterController*	m_pControllerCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	DEFORMTYPE				m_eDeformType = { DEFORM_END };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

