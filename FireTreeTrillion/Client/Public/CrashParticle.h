#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CCrashParticle final : public CGameObject
{
public:
	struct CRASHPARTICLEDESC : public GAMEOBJECT_DESC
	{
		_bool	bGravity = { true };
		_float4 vDir = { 0.f, 0.f, 0.f, 0.f };
		_float	fSpeed = { 0.f };
		_float  fScale = { 0.f };
		_float4 vPos = { 0.f, 0.f, 0.f, 0.f };
	};

private:
	CCrashParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCrashParticle(const CCrashParticle& rhs);
	virtual ~CCrashParticle() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };


private:
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };

	_float			m_fRunTime = { 0.f };

	_float4			m_vDir = { 0.f, 0.f, 0.f, 0.f };
	_float			m_fSpeed = { 0.f };
	_float			m_fPreSpeed = { 0.f };
	_float			m_fTimeDelta = { 0.f };
	_float			m_fTurn = { 0.f };
	_float4			m_fTurnAxis = { 0.f, 0.f, 0.f, 0.f };

	_float			m_fScale = { 1.f };
	_float			m_fGravity = { 0.f };
	_bool			m_bGravity = { true };

public:
	static CCrashParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END