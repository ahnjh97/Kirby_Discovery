#pragma once

#include "Client_Defines.h"
#include "RigidObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CRigidBody;
END

BEGIN(Client)

class CKirbyBomb final : public CRigidObject
{
public:
	typedef struct
	{
		_float4 vPos = { 0.f, 0.f, 0.f, 0.f };
		_float4 vDir = { 0.f, 0.f, 0.f, 0.f };
		_float	fPower = { 0.f };
	}KIRBYBOMB_DESC;

private:
	CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirbyBomb(const CKirbyBomb& rhs);
	virtual ~CKirbyBomb() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();
	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

private:
	CModel* m_pModelCom = { nullptr };
	CRigidBody* m_pRigidBodyCom = { nullptr };



public:
	static CKirbyBomb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END