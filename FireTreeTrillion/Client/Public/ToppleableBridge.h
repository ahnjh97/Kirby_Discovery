#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CToppleableBridge final : public CPhysXObject
{
private:
	CToppleableBridge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToppleableBridge(const CToppleableBridge& rhs);
	virtual ~CToppleableBridge() = default;

public:
	void OnCollision();

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
	HRESULT			Add_Components(const wstring& _wstrModelName);
	HRESULT			Bind_ShaderResources();
	void			Make_Particles();

	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	CGameObject*	m_pAnimBridge = { nullptr };
	PxRigidStatic*	m_pStaticActor = { nullptr };
	PxRigidStatic*	m_pFakeWall = { nullptr };

	wstring			m_wstrModelName;

	_float			m_fHitPower = {};
	_float			m_fWhiteColorDiffuse = {};

	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };
	
	_bool			m_bCollision = { false };
	_bool			m_bActorCreated = { false };
	_float			m_fHitTime = {};
	_bool			m_bSoundSignal = false;

public:
	static CToppleableBridge* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

