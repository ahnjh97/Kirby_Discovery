#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGm_ParkFhEntranceAlien final : public CPhysXObject
{
public:
	enum ANIM_STATE { STATE_LOOP, STATE_NONE };
private:
	CGm_ParkFhEntranceAlien(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGm_ParkFhEntranceAlien(const CGm_ParkFhEntranceAlien& rhs);
	virtual ~CGm_ParkFhEntranceAlien() = default;

public:
	virtual HRESULT		Initialize_Prototype()						override;
	virtual HRESULT		Initialize(void* pArg)						override;
	virtual _int		Tick(_float fTimeDelta)						override;
	virtual void		Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT		Render()									override;
	virtual HRESULT		Render_LightDepth()							override;
#ifdef _DEBUG
	virtual void		Render_IMGUI()								override;
#endif

	virtual void		Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	HRESULT				Add_Components();
	HRESULT				Bind_ShaderResources();

	unordered_set<_uint>	m_setBeforeHideMeshs;
	unordered_set<_uint>	m_setAfterHideMeshs;

	CModel*					m_pModelCom = { nullptr };
	CModel*					m_pNonAnimModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	PxRigidStatic*			m_pStaticActor = { nullptr };

	ANIM_STATE				m_eAnimState = { STATE_NONE };

	_float					m_fBreakAnimTime = { 0.f };
	_float					m_fWhiteColorDiffuse = { 0.f };
	_float4					m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };
	//_float				m_fHitPower = {};
	
	_bool					m_bStartAnim = { FALSE };

public:
	static CGm_ParkFhEntranceAlien* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
