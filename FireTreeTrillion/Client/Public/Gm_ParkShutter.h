#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"
#include "Gm_ParkSolarPanelOnce.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGm_ParkShutter final : public CPhysXObject
{
public:
	enum ANIM_STATE { STATE_OPENIDLE, STATE_CLOSEIDLE, STATE_TOCLOSE, STATE_TOOPEN, STATE_NONE };
private:
	CGm_ParkShutter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGm_ParkShutter(const CGm_ParkShutter& rhs);
	virtual ~CGm_ParkShutter() = default;

public:
	void Set_ParkShutter(CGm_ParkSolarPanelOnce* _pSPOnce) {
		m_pGimmickSPOnce = _pSPOnce;
		Safe_AddRef(m_pGimmickSPOnce);
	}
	void Set_Interaction(_bool _bIsInteraction) { m_bIsInteraction = _bIsInteraction; }

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
	CGm_ParkSolarPanelOnce* m_pGimmickSPOnce = { nullptr };

	ANIM_STATE				m_eAnimState = { STATE_NONE };

	_float					m_fWhiteColorDiffuse = { 0.f };
	_float4					m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };
	//_float				m_fHitPower = {};
	
	_bool					m_bStartAnim = { FALSE };
	_bool					m_bIsInteraction = { FALSE };

public:
	static CGm_ParkShutter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
