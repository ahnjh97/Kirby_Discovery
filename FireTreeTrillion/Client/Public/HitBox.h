#pragma once
#include "Client_Defines.h"
#include "GameObject.h"


BEGIN(Client)

// Ä¿ºñÀÇ ¹î»ì
class CHitBox final : public CGameObject
{
public:
	struct HITBOX_DESC
	{
		CGameObject* pOwner;
		COLLISION_DESC* pDesc;
		_uint		 pCollisionType;
	};
private:
	CHitBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHitBox(const CHitBox& rhs);
	virtual ~CHitBox() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
#ifdef _DEBUG
	void	Render_IMGUI();
#endif
	_bool			Is_Alive() const { return m_pOwnerCollisionDesc->bAlive; }
	void			Set_Alive(_bool bAlive) { m_pOwnerCollisionDesc->bAlive = bAlive; }

	class CGameObject* Get_Owner() { return m_pOwner; }
	COLLISION_DESC*	   Get_CollisionDesc() { return m_pOwnerCollisionDesc; }

	void			Restore_Logic(_float fTimeDelta);
	
private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	class CGameObject*	m_pOwner			= nullptr;
	class CTransform*	m_pOwnerTransform	= nullptr;

	_float				m_fCollisionTime = { 0.f };

	COLLISION_DESC*		m_pOwnerCollisionDesc = { nullptr };
	COLLISION_VALUE     m_eValue = { VALUE_END };


public:
	static CHitBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END