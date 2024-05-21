#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CCharacterController : public CComponent
{
protected:
	CCharacterController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCharacterController(const CCharacterController& rhs);
	virtual ~CCharacterController() = default;

public:
	virtual HRESULT Initialize(void* pArg)	override;
	virtual void	Start_Tick()			override;
	virtual void	Render_IMGUI()			override;

public:
	void			Set_PhysXObject(class CGameObject* _pObj) { m_pObject = _pObj; }

	// 갑자기 위치값이 변화되는 경우 사용하시오.(ex. 텔레포트 등)
	void			Set_Position(const _float4& vPos);
	// 발 위치값 지정
	void			Set_FootPosition(const _float4& vPos);

	_float4			Get_Position();
	_float4			Get_FootPosition();

	// 이동에 대한 함수
	void Test(class CTransform* pTransform, _float fSpeed, _float fTimeDelta);
	void Move(class CTransform* pTransform, _float fTimeDelta);

	// 이동용, 기본 중력없기 때문에 이 함수로 중력 만들어 줄 것
	physx::PxControllerCollisionFlags Move(_float3 vVelocity, _float fTimeDelta, _float minDist = 0.001f);
	physx::PxControllerCollisionFlags MoveDisp(_float3 vPosDelta, _float fTimeDelta, _float minDist = 0.001f);

	_bool	Is_Activated();
	void	Activate(_bool bActive);

	void	Get_ShapeInfo(physx::PxCapsuleGeometry& CapsuleGeo, physx::PxTransform& pxTransform);
	_float	Get_Radius() const { return m_tControllerDesc.radius; }

protected:
	void	Create_Controller();
	void	Release_Controller();
	// Controller값 디폴트 지정해주는 함수
	void	Set_DefaultValue();

protected:
	class CGameObject*				m_pObject = nullptr;

	physx::PxController*			m_pController = nullptr;
	physx::PxCapsuleControllerDesc	m_tControllerDesc;
	physx::PxControllerFilters		m_ControllerFilters;
	physx::PxFilterData				m_tFilterDesc;

	_float							m_fSlopeLimitDegree = 45.f;
	_float							m_fFallVelocity = { 0.f };
	_float							m_fFallAcceleration = { 0.f };

public:
	static	CCharacterController*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*				Clone(void* pArg = nullptr) override;
	virtual void					Free()						override;

};

END