#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CLaser final : public CPhysXObject
{
public:
	struct LASER_DESC : public CGameObject::GAMEOBJECT_DESC {
		_vector vPosition = {};
	};

private:
	CLaser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLaser(const CLaser& rhs);
	virtual ~CLaser() = default;

public:
	void Set_EndLaser(_bool bEnd) { m_bEnd = bEnd; }

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
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };

	_vector		m_vPosition = {};
	_float		m_fTimeDelta = { 0.f };
	_float		m_fLifeTime = { 0.f };

	_bool		m_bEnd = { false };

private:
	_float3 Compute_CollidingPoint(_float3 vLaserStart, _float3 vLaserDir, _float3 vLandCenter, _float3 vLandExtent);


	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

public:
	static CLaser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END