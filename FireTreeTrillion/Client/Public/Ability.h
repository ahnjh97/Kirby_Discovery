#pragma once
#include "ItemObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CAbility final : public CItemObject
{
public:
	struct ABILITYITEM_DESC : public CGameObject::GAMEOBJECT_DESC {
		_float4 vPosition = {};
	};

private:
	CAbility(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAbility(const CAbility& rhs);
	virtual ~CAbility() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// Sphere 충돌 밀어내기
	void Sphere_Collision();
	_bool Check_Sphere(CTransform* pTransform, _float* fDist);

private:
	CModel*					m_pModelCom = { nullptr };

private:
	_float4		m_vPosition = {};

	_float		m_fJumpPower = { 0.f };
	_float		m_fPower = { 0.f };

public:
	static CAbility* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END