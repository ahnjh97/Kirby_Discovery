#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CLaser final : public CGameObject
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
	virtual	void Render_IMGUI()			override;
#endif

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };

	_vector		m_vPosition = {};
	_float		m_fTimeDelta = { 0.f };
	_float		m_fLifeTime = { 0.f };

	_bool		m_bEnd = { false };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

public:
	static CLaser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END