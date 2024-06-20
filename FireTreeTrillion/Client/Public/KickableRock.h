#pragma once
#include "Client_Defines.h"
#include "RigidObject.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)

class CKickableRock final : public CRigidObject
{
private:
	CKickableRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKickableRock(const CKickableRock& rhs);
	virtual ~CKickableRock() = default;

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
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	HRESULT			Add_Components(const wstring& wstrModelName);
	HRESULT			Bind_ShaderResources();

private:
	CModel*			m_pModelCom = { nullptr };

	_float			m_fLifeTime = _float();

	_float			m_fFlyTime = { 0.f };
	_float			m_fDeadTime = { 0.f };

	_bool			m_bLockCollision = { false };

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };


public:
	static CKickableRock*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END