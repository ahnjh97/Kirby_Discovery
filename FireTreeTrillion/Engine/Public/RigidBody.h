#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CRigidBody : public CComponent
{
public:
	enum SHAPE { TYPE_BOX, TYPE_SPHERE, TYPE_CAPSULE, TYPE_END };

private:
	CRigidBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRigidBody(const CRigidBody& rhs);
	virtual ~CRigidBody() = default;

public:
	virtual HRESULT Initialize_Prototype()  override;
	virtual HRESULT Initialize(void* pArg)  override;
	virtual void	Start_Tick()			override;
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

	// 날리기
	void			Add_Force(_float3 vForce);

public:
	physx::PxTransform	Get_PxTransform();

	void				Set_PxWorldMatrix(const _float4x4& worldMatrix);
	_float4x4			Get_PxWorldMatrix();

	_bool				Is_Activated();

protected:
	class CGameObject*		m_pObject		= nullptr;
	physx::PxRigidDynamic*	m_pActor		= nullptr;
	physx::PxShape*			m_pShape		= nullptr;

	// ========== IMGUI에서 제어할 수 있도록 붙이기 ==========
	// 물리엔진 on/off
	// - 움직임을 제어하는 객체인가? true == isKinectic
	// - 내가 직접 움직임을 제어할 것은 아니지만 물리 영향을 받았으면 좋겠어. false == isKinectic
	_bool					m_bKinematic	= false;
	// 이 RigidBody를 들고 있는 객체가 trigger인가
	_bool					m_bTrigger		= false;
	// 물체의 질량
	_float					m_fDensity		= 10.f;

	// 현 RigidBody의 형태
	SHAPE					m_eShapeType	= TYPE_SPHERE;
	_float4x4				m_OriginTransformMatrix;

public:
	static CRigidBody*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg);
	virtual void			Free() override;


};

END
