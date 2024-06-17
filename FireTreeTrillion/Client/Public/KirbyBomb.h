#pragma once

#include "Client_Defines.h"
#include "RigidObject.h"
#include "Kirby.h"

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
		CKirby* pKirby = { nullptr };
		_float4x4* pKirbyHandsMatrix = { nullptr };
		const _float4x4* pKirbyWorldMatrix = { nullptr };
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

	void			Kicking();
	void			Throwing(CKirby::KIRBY_INFODESC* desc);
	_bool			m_bThrowTrigger = { true };

	void			SuddenBoom();

private:
	HRESULT			Add_Components();
	HRESULT			Add_Rigid();
	HRESULT			Bind_ShaderResources();
	void			Compute_MotionBlur();
	_bool			RayCast_Terrain();
	
	_int			Boom_Dead();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	_float			m_fBombTime = { 0.f };

	_float			m_fBombingTime = { 0.f };
	_float			m_fDontKicking = { 0.f };



private:
	CModel* m_pModelCom = { nullptr };
	CRigidBody* m_pRigidBodyCom = { nullptr };

	CKirby* m_pKirby = { nullptr };
	_float4x4* m_pKirbyHandsMatrix = { nullptr };
	const _float4x4* m_pKirbyWorldMatrix = { nullptr };

public:
	static CKirbyBomb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END