#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CCharacterController : public CComponent
{
public:
	struct CONTROLLER_DESC
	{
		_float4 vInitialPos;
	};

protected:
	CCharacterController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCharacterController(const CCharacterController& rhs);
	virtual ~CCharacterController() = default;

public:
	void			Set_PhysXObject(class CGameObject* _pObj) { m_pObject = _pObj; }
	// 갑자기 위치값이 변화되는 경우 사용하시오.(ex. 텔레포트 등)
	void			Set_Position(const _float4& vPos);
	// 발 위치값 지정
	void			Set_FootPosition(const _float4& vPos);

	_float4			Get_Position();
	_float4			Get_FootPosition();

	//PxControllerCollisionFlag getCollisionFlags()
	//{
	//	return collisionFlags;
	//}
	void			Get_ShapeInfo(physx::PxCapsuleGeometry& CapsuleGeo, physx::PxTransform& pxTransform);
	_float			Get_Radius() const { return m_tControllerDesc.radius; }

public:
	virtual HRESULT Initialize(void* pArg)	override;
	virtual void	Start_Tick()			override;
	virtual void	Render_IMGUI()			override;

public:
	// 이동에 대한 함수
	void			Move(class CTransform* pTransform, _float fSpeed, _float fTimeDelta);	// look방향으로 움직임
	void			Move_Dir(class CTransform* pTransform, _fvector fDelta, _float fTimeDelta);
	_bool			Jump(CTransform* pTransform, _float fFallVelocity, _float fTimeDelta);	// 점프
	void			FreeFall(CTransform* pTransform, _float fTimeDelta, _float fOffset = 1.f);					// 자유 낙하
	PxVec3			Compute_Slope(CTransform* pTransform);									// 경사면의 노말벡터 계산
	_float			Compute_Height();														// 경사면의 노말벡터 계산
	PxVec3			Compute_TerrainPosition();
	_vector			Compute_TerrainPosition_Vector();
	PxVec3			TerrainRayCast_Collision(PxVec3 _rayOrigin, PxVec3 _rayDirection, _float _fMaxDistance);

	// 이동용, 기본 중력없기 때문에 이 함수로 중력 만들어 줄 것
	/*physx::PxControllerCollisionFlags Move(_float3 vVelocity, _float fTimeDelta, _float minDist = 0.001f);
	physx::PxControllerCollisionFlags MoveDisp(_float3 vPosDelta, _float fTimeDelta, _float minDist = 0.001f);*/

	_bool	Is_Activated();
	void	Activate(_bool bActive);

	_bool	Is_Terrain() {
		PxControllerState m_pPxState;
		m_pController->getState(m_pPxState);

		if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN)
			return true;
		else
			return false;
	}

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

	_float							m_fOffset = { 1.f };

public:
	static	CCharacterController*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*				Clone(void* pArg = nullptr) override;
	virtual void					Free()						override;

};

END