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
		_float		fOffsetSize;
		_float3		vMaterial = {0.5f, 0.5f, 0.5f};
		RIGIDBODY_DESC
		(RIGID_SHAPE _eShapeType = { RIGID_END },
			_float4x4 _matWorld = {},
			_bool _bTrigger = { false },
			_bool _bDynamic = { true },
			_float _fOffsetSize = {},
			_float3 _vMaterial = { 0.5f, 0.5f, 0.5f }
		) : eShapeType(_eShapeType), matWorld(_matWorld), bTrigger(_bTrigger), bDynamic(_bDynamic), fOffsetSize(_fOffsetSize), vMaterial(_vMaterial) {}
	};

private:
	CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRigidBody(const CRigidBody& rhs);
	virtual ~CRigidBody() = default;

public:
	virtual HRESULT Initialize_Prototype()  override;
	virtual HRESULT Initialize(void* pArg)  override;
	virtual void	Render_IMGUI()			override;

	void			Update(class CTransform* pTransform);
	void			Update(_fmatrix matrix);
	void			Update_PhysX(CTransform* pTransform);

public:
	void			Set_PhysXObject(class CGameObject* _pObj) { m_pObject = _pObj; }
	
	void			Create_Actor();
	void			SetUp_Actor();
	void			Release_Actor();

	void			Activate(_bool _bActive);
	void			SetUp_CamIndex(_int iCamIndex) { m_iCamIndex = iCamIndex; }

	// 날리기
	void			Add_Force(_float3 vForce);

public:
	PxTransform	Get_PxTransform();

	void				Set_PxWorldMatrix(const _float4x4& worldMatrix);
	_float4x4			Get_PxWorldMatrix();

	_bool				Is_Activated();

protected:
	class CGameObject*	m_pObject				= { nullptr };
	PxRigidDynamic*		m_pActor				= { nullptr };
	PxRigidStatic*		m_pStaticActor			= { nullptr };

	PxShape*			m_pShape				= { nullptr };
	PxController*		m_pCapsuleController	= { nullptr };

	_bool				m_bTrigger				= { false };

	// 물체의 질량
	_float				m_fDensity				= { 10.f };

	// 현 RigidBody의 형태
	RIGID_SHAPE			m_eShapeType = { RIGID_CAPSULE };
	_float4x4			m_OriginTransformMatrix = { _float4x4::Identity };
	_float3				m_vMaterial = { 0.5f, 0.5f, 0.6f };
	_float				m_fOffsetSize = { 1.f };
	_bool				m_bDynamic = { true };
	_int				m_iCamIndex = { -1 };

public:
	static CRigidBody*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg);
	virtual void			Free() override;


};

END
