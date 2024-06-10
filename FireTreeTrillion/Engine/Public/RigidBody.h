#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CRigidBody : public CComponent
{
public:
	PxActor* Get_Actor() { if (m_bDynamic) return m_pActor; else return m_pStaticActor; };

public:
	struct RIGIDBODY_DESC
	{
		RIGID_SHAPE	eShapeType;
		_float4x4	matWorld;
		_bool		bTrigger;
		_bool		bDynamic;
		_bool		bKinematic;
		_float		fOffsetSize;
		_float3		vMaterial = {0.5f, 0.5f, 0.5f};
		CGameObject* pObj = { nullptr };
		RIGIDBODY_DESC
		(RIGID_SHAPE _eShapeType = { RIGID_END },
			_float4x4 _matWorld = {},
			_bool _bTrigger = { false },
			_bool _bDynamic = { true },
			_bool _bKinematic = { false },
			_float _fOffsetSize = {},
			_float3 _vMaterial = { 0.5f, 0.5f, 0.5f },
			CGameObject* _pObj = {}
		) : eShapeType(_eShapeType), matWorld(_matWorld), bTrigger(_bTrigger), bDynamic(_bDynamic), bKinematic(_bKinematic),fOffsetSize(_fOffsetSize), vMaterial(_vMaterial), pObj(_pObj) {}
	};

private:
	CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRigidBody(const CRigidBody& rhs);
	virtual ~CRigidBody() = default;

public:
	virtual HRESULT Initialize_Prototype()  override;
	virtual HRESULT Initialize(void* pArg)  override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()			override;
#endif

	void			Update(_fmatrix matrix);
	void			Update(_float4 vPos);
	void			Update_PhysX(class CTransform* pTransform);

public:
	void			Create_Actor();
	void			SetUp_Actor();
	void			Release_Actor();

	void			Activate(_bool _bActive);
	void			SetUp_TriggerType(_int iTriggerType) { m_iTriggerType = iTriggerType; }
	void			SetUp_TriggerIndex(_int iTriggerIndex) { m_iTriggerIndex = iTriggerIndex; }

	// 날리기
	void			Add_Force(_float3 vForce);
	void			Add_Torque(_float3 vTorque);
	void			Add_Velocity(_float3 vVelocity);

	void			Overlap_Hitbox(CGameObject* pGameObject, _float4 vPos, _float fRadius = 1.f);

public:
	PxTransform			Get_PxTransform();

	void				Set_PxWorldMatrix(const _float4x4& worldMatrix);
	_float4x4			Get_PxWorldMatrix();

	void				Set_ActorName(string _name) { m_pActor->setName(_name.c_str()); }
	_bool				Is_Activated();

protected:
	PxRigidDynamic*			m_pActor				= { nullptr };
	PxRigidStatic*			m_pStaticActor			= { nullptr };

	PxShape*				m_pShape				= { nullptr };
	PxController*			m_pCapsuleController	= { nullptr };
	class CGameObject*		m_pActorObject			= { nullptr };

	_bool					m_bTrigger				= { false };
	// 물체의 질량
	_float					m_fDensity				= { 10.f };

	// 현 RigidBody의 형태
	RIGID_SHAPE			m_eShapeType = { RIGID_CAPSULE };
	_float4x4			m_OriginTransformMatrix = { _float4x4::Identity };
	_float3				m_vMaterial = { 0.5f, 0.5f, 0.6f };
	_float				m_fOffsetSize = { 1.f };
	_bool				m_bDynamic = { true };
	_bool				m_bKinematic = { false };

	_int				m_iTriggerType = { -1 };
	_int				m_iTriggerIndex = { -1 };

public:
	static CRigidBody*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg);
	virtual void			Free() override;


};

END
