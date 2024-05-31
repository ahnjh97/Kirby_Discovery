#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CFSM;
//class CCharacterController;
END

BEGIN(Client)

class CRigidObject abstract : public CPhysXObject
{
protected:
	CRigidObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRigidObject(const CRigidObject& rhs);
	virtual ~CRigidObject() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
	virtual void	Render_IMGUI()					override;

protected:
	//CCharacterController*	m_pControllerCom	= { nullptr };
	CShader*				m_pShaderCom = { nullptr };
//	CFSM*					m_pFSM = { nullptr };
//
//protected:
//	// For_PhysX
//	_float			m_fOffsetTurn = { 7.f };
//	_float4			m_vOriginUp = { 0.f, 1.f, 0.f, 0.f };
//		
//protected:
//	void			SetOn_Slope(_float fTimeDelta);
//	void			Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END