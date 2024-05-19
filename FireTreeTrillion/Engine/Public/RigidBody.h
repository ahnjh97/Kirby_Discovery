#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CRigidBody : public CComponent
{
public:
	enum SHAPE { TYPE_BOX, TYPE_SPHERE, TYPE_CAPSULE, TYPE_END };

public:
	struct RIGIDBODY_DESC
	{
		SHAPE eShape;
	};

private:
	CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRigidBody(const CRigidBody& rhs);
	virtual ~CRigidBody() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	//virtual void	Update(CTransform* pTransform);

public:
	void			Create_Actor();
	void			SetUp_Actor();
	void			Release_Actor();

public:
	//_float4x4					Get_OriginMatrix();
	physx::PxTransform			Get_PxTransform();
	_bool						IsOnPhysX();

protected:
	physx::PxRigidDynamic*	m_pActor				= nullptr;
	physx::PxShape*			m_pShape				= nullptr;

	// 물리엔진 on/off
	_bool					m_bKinematic			= false;
	// trigger에 대한 on/off
	_bool					m_bTrigger				= false;
	
	// 현 RigidBody의 형태
	SHAPE					m_eShapeType			= TYPE_SPHERE;
	_float4x4_sm			m_OriginTransformMatrix;

public:
	static CRigidBody*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg);
	virtual void		Free() override;


};

END
