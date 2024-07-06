#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGully final : public CPhysXObject
{
public:
	struct GULLY_DESC : public CGameObject::GAMEOBJECT_DESC {
		_vector vPosition = {};
	};

private:
	CGully(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGully(const CGully& rhs);
	virtual ~CGully() = default;

public:
	void Set_Gully(_fvector vPos, _float fLifeTime) {
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		m_fLifeTime = fLifeTime;
		m_fScale = 1.f;
		m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
		m_bPoolingDead = false;
	}

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual	void	Render_IMGUI()			override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_vector m_vPosition = {};
	
	_float	m_fTimeDelta = { 0.f };
	_float	m_fLifeTime = { 0.f };
	
	_float	m_fScale = { 0.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CGully* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END