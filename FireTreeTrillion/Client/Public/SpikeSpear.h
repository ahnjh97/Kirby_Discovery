#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CSpikeSpear final : public CPhysXObject
{
public:
	struct SPIKESPEAR_DESC : public CGameObject::GAMEOBJECT_DESC {
		_vector vPosition = {};
	};

private:
	CSpikeSpear(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSpikeSpear(const CSpikeSpear& rhs);
	virtual ~CSpikeSpear() = default;

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

	_float	m_fTimeDelta = { 0.f };
	_float	m_fSpikeTime = { 0.f };
	_float	m_fLifeTime = { 0.f };

	_vector	m_vPosition = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

public:
	static CSpikeSpear* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END