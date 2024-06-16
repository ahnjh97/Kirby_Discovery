#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CTrigger final : public CPhysXObject
{
public:
	enum TRIGGER {  TRIGGER_CAM, TRIGGER_SHADER, TRIGGER_STAR, 
					TRIGGER_ITEM = 50, TRIGGER_HITBOX, TRIGGER_MAPOBJ, TRIGGER_END };


	typedef struct : public GAMEOBJECT_DESC
	{
		_uint		iTriggerType;
		_int		iTriggerIndex = -1;
		_uint		eCollisionGroup;
		_float3		vTriggerSize;
		_float4x4   vInitialPos;
	}TRIGGER_DESC;

private:
	CTrigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrigger(const CTrigger& rhs);
	virtual ~CTrigger() = default;

public:
	virtual HRESULT Initialize_Prototype()								override;
	virtual HRESULT Initialize(void* pArg)								override;
	virtual _int	Tick(_float fTimeDelta)								override;
	virtual void	Late_Tick(_float fTimeDelta)						override;
	virtual HRESULT Render()											override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()										override;
#endif
	CPhysXObject*	Get_Owner() const { return m_pOwner; }
	void			Set_Owner(class CPhysXObject* pObj);
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;
	virtual void	Collision_Hitbox(CPhysXObject* pGameObject)			override;
	_bool			Is_Alive() { return m_bAlive; }
	void			Check_Collision();
	void			Close_Collision();

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };	
	CRigidBody*		m_pRigidBodyCom = { nullptr };

	// for ITEM, HITBOX
	CPhysXObject*	m_pOwner = nullptr;
	CTransform*		m_pOwnerTransform = nullptr;
	_bool			m_bAlive = false;
	_float3			m_vSize = _float3(1.f, 1.5f, 1.f);
	_float4x4		m_vInitialPos = _float4x4::Identity;

private:
	TRIGGER			m_eTriggerType = { TRIGGER_END };
	_int			m_iTriggerIndex = { -1 };

public:
	static CTrigger*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

END

