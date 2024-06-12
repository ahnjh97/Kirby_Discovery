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
	// 발 위치값 지정
	void			Set_FootPosition(const _float4& vPos);

	_float4			Get_Position();
	_float4			Get_FootPosition();

	_float			Get_Radius() const { return m_tControllerCapsuleDesc.radius; }
	void			Clear_Collisions();
	_bool			Has_Collided();
	void			RegisterAsPlayer();

public:
	virtual HRESULT Initialize(void* pArg)	override;
	virtual void	Start_Tick()			override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()			override;
#endif

public:
	// 이동에 대한 함수
	void			Move(class CTransform* pTransform, _fvector vPosition, _float fTimeDelta);				// look방향으로 움직임
	void			Move_Dir(class CTransform* pTransform, _fvector fDelta, _float fTimeDelta);			// 방향 벡터로 움직임
	_bool			Jump(CTransform* pTransform, _float fFallVelocity, _float fTimeDelta);				// 점프
	_bool			Jump_Parabola(CTransform* pTransform, _fvector vGoPos, _float fTimeDelta);			// 목표 지점으로 점프
	void			FreeFall(CTransform* pTransform, _float fTimeDelta, _float fOffset = 1.f);			// 자유 낙하
	void			Reset_FallVelocity() { m_fFallVelocity = 0.f; }										// 자유 낙하 중력값 초기화
	PxVec3			Compute_Slope(CTransform* pTransform);												// 경사면의 노말벡터 계산
	_float			Compute_Height(_fvector vAxis = XMVectorSet(0.f, 0.f, 0.f, 0.f));																	// 경사면의 노말벡터 계산
	PxVec3			Compute_TerrainPosition();
	_vector			Compute_TerrainPosition_Vector();
	PxVec3			TerrainRayCast_Collision(PxVec3 _rayOrigin, PxVec3 _rayDirection, _float _fMaxDistance);


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
	class CGameObject*					m_pObject = nullptr;

	physx::PxController*				m_pController = nullptr;
	physx::PxMaterial*					m_ControllerMaterial = nullptr;
	_float3								m_vMaterialOptions = _float3(0.5f, 0.5f, 0.5f);

	//PxControllerDesc를 상속시켜서 사용하고 싶었으나 안되었음
	physx::PxCapsuleControllerDesc		m_tControllerCapsuleDesc;
	physx::PxBoxControllerDesc 			m_tControllerBoxDesc;

	physx::PxControllerFilters			m_ControllerFilters;
	physx::PxFilterData					m_tFilterDesc;
	
	class CControllerBehaviorCallback*	m_pControllerCallBack = nullptr;
	class CUserControllerHitReport*		m_pControllerHitReport = nullptr;
	class CControllerFilterCallback*	m_pControllerFilterCallback = nullptr;
	_float								m_fSlopeLimitDegree = 45.f;
	_float								m_fFallVelocity = { 0.f };
	_float								m_fFallAcceleration = { 0.f };

	_float								m_fOffset = { 1.f };
	TYPE								m_eType = TYPE::CAPSULE;
	//CAPSULE_SHAPE						m_tCapsuleShape;
	//BOX_SHAPE							m_tBoxShape;

	_bool								m_isCollision = { false };

public:
	static	CCharacterController*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*				Clone(void* pArg = nullptr) override;
	virtual void					Free()						override;

};

END