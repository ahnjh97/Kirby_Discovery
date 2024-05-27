#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CRigidBody : public CComponent
{
public:
	struct RIGIDBODY_DESC
	{
		RIGID_SHAPE			eShapeType;
		_float4x4			matWorld;
		_bool				bTrigger;
		_float				fOffsetSize;
		_float3				vMaterial = {0.5f, 0.5f, 0.5f};
		class CGameObject*	pObj = nullptr;
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
	void			Set_PhysXObject(class CGameObject* _pObj) { m_pActorObject = _pObj; }
	
	void			Create_Actor();
	void			SetUp_Actor();
	void			Release_Actor();

	void			Activate(_bool _bActive);

	// 날리기
	void			Add_Force(_float3 vForce);

public:
	physx::PxTransform	Get_PxTransform();

	void				Set_PxWorldMatrix(const _float4x4& worldMatrix);
	_float4x4			Get_PxWorldMatrix();

	_bool				Is_Activated();

protected:
	class CGameObject*			m_pActorObject		 = nullptr;
	physx::PxRigidDynamic*		m_pActor			 = nullptr;
	physx::PxShape*				m_pShape			 = nullptr;
	physx::PxController*		m_pCapsuleController = nullptr;

	_bool						m_bTrigger = false;

	// 물체의 질량
	_float						m_fDensity		= 10.f;

	// 현 RigidBody의 형태
	RIGID_SHAPE					m_eShapeType	= RIGID_CAPSULE;
	_float4x4					m_OriginTransformMatrix = _float4x4::Identity;
	_float3						m_vMaterial = { 0.5f, 0.5f, 0.6f };
	_float						m_fOffsetSize = 1.f;

public:
	static CRigidBody*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg);
	virtual void			Free() override;


};

END
