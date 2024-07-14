#pragma once
#include "Component.h"

BEGIN(Engine)
class CTransform;

class ENGINE_DLL CCharacterController : public CComponent
{
public:
	enum TYPE { CAPSULE, BOX, TYPE_END };

public:
	struct CAPSULE_SHAPE
	{
		_float	fRadius = 0.5f;
		_float	fHeight = 1.f;
	};
	struct BOX_SHAPE
	{
		_float	fHalfForwardExtent	= 0.5f;
		_float	fHalfHeight			= 0.5f;
		_float	fHalfSideExtent		= 0.5f;
	};
	struct CONTROLLER_DESC
	{
		_float4			vInitialPos;
		_uint			uCollisionType;
		string			strProtoObjName;
		_float			fOffset;

		TYPE			eType = CAPSULE;
		CAPSULE_SHAPE	tCapsuleShape;
		BOX_SHAPE		tBoxShape;
	};

protected:
	CCharacterController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCharacterController(const CCharacterController& rhs);
	virtual ~CCharacterController() = default;

public:
	// 갑자기 위치값이 변화되는 경우 사용하시오.(ex. 텔레포트 등)
	void			Set_Position(class CTransform* pTransform, const _float4& vPos);
	//이것은 트랜스폼을 움직이게 하지 않습니다. 주의!!!!!!!!!!!!!
	void			Set_CapsulePosition(const _float4& vPos);

	// 발 위치값 지정
	void			Set_FootPosition(const _float4& vPos);
	void			Set_FallVelocity(_float fFallVelocity) { m_fFallVelocity = fFallVelocity; }

	_float4			Get_Position();
	_float4			Get_FootPosition();

	_float			Get_Radius() const { return m_tControllerCapsuleDesc.radius; }
	void			RegisterAsPlayer();
	PxRigidActor*	Get_MostRecentActor() { return m_pMostRecentActor; }

public:
	virtual HRESULT Initialize(void* pArg)	override;
	virtual void	Start_Tick()			override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()			override;
#endif

public:
	// 이동에 대한 함수
	void			Move(class CTransform* pTransform, _fvector vPosition, _float fTimeDelta);
	void			MoveUpAndDown(class CTransform* pTransform, _fvector vPosition, _float fTimeDelta);
	void			Move_Dir(class CTransform* pTransform, _fvector fDelta, _float fTimeDelta, _float fOffsetY = 0.f);			// 방향 벡터로 움직임
	_bool			Jump(CTransform* pTransform, _float fFallVelocity, _float fTimeDelta);				// 점프
	_bool			Jump_Parabola(CTransform* pTransform, _fvector vGoPos, _float fTimeDelta);			// 목표 지점으로 점프
	void			FreeFall(CTransform* pTransform, _float fTimeDelta, _float fGravityOffset = 6.f, _float fHeightOffset  = 0.f);	// 자유 낙하
	void			Reset_FallVelocity() { m_fFallVelocity = 0.f; }										// 자유 낙하 중력값 초기화
	PxVec3			Compute_Slope(CTransform* pTransform);												// 경사면의 노말벡터 계산
	PxVec3			Compute_Slope_DynamicActor(CTransform* pTransform);									// 경사면의 노말벡터 계산

	PxVec3			Compute_PureSlope();																// 경사면의 노말벡터 계산
	_float			Compute_Height(_fvector vAxis = XMVectorSet(0.f, 0.f, 0.f, 0.f));					// 경사면의 높이 계산

	_float			Compute_Wall(_fvector vLook, _float fOffSet = 1.f);									// StaticActor에 Raycast
	_float			Compute_Wall(_fvector vLook, _float3 vOffset);									// StaticActor에 Raycast

	_float			RayCastToStaticActor(_fvector vDir, _float fRayDistance = 25.f, _float3 vOffset = _float3()); // StaticActor에 Raycast
	_float			RayCastToDynamicActor(_fvector vLook, _float3 vOffset = _float3());
	PxVec3			Compute_TerrainPosition();
	_vector			Compute_TerrainPosition_Vector();
	PxVec3			TerrainRayCast_Collision(PxVec3 _rayOrigin, PxVec3 _rayDirection, _float _fMaxDistance);
	PxVec3			TerrainRayCast_Collision_Dynamic(PxVec3 _rayOrigin, PxVec3 _rayDirection, _float _fMaxDistance);


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

	_bool	Is_Wall() {
		PxControllerState m_pPxState;
		m_pController->getState(m_pPxState);

		if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_SIDES)
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
	class CGameObject*					m_pObject = nullptr;

	PxController*				m_pController = nullptr;
	PxMaterial*					m_ControllerMaterial = nullptr;
	_float3								m_vMaterialOptions = _float3(0.5f, 0.5f, 0.5f);

	//PxControllerDesc를 상속시켜서 사용하고 싶었으나 안되었음
	PxCapsuleControllerDesc		m_tControllerCapsuleDesc;
	PxBoxControllerDesc 			m_tControllerBoxDesc;

	PxControllerFilters			m_ControllerFilters;
	PxFilterData					m_tFilterDesc;
	
	class CControllerBehaviorCallback*	m_pControllerCallBack = nullptr;
	class CUserControllerHitReport*		m_pControllerHitReport = nullptr;
	class CControllerFilterCallback*	m_pControllerFilterCallback = nullptr;
	_float								m_fSlopeLimitDegree = 45.f;
	_float								m_fFallVelocity = { 0.f };
	_float								m_fFallAcceleration = { 0.f };

	_float								m_fHeightOffset = { 1.f };
	TYPE								m_eType = TYPE::CAPSULE;
	//CAPSULE_SHAPE						m_tCapsuleShape;
	//BOX_SHAPE							m_tBoxShape;

	_bool								m_isCollision = { false };
	string								m_strObjectName = "";

	PxRigidActor*						m_pMostRecentActor = { nullptr };


public:
	static	CCharacterController*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*				Clone(void* pArg = nullptr) override;
	virtual void					Free()						override;

};

END