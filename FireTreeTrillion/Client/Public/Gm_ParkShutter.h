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
	enum SHUTTER_STATE { STATE_CLOSEIDLE, STATE_OPENIDLE, STATE_TOCLOSE, STATE_TOOPEN, STATE_NONE };

private:
	CGm_ParkShutter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGm_ParkShutter(const CGm_ParkShutter& rhs);
	virtual ~CGm_ParkShutter() = default;

public:
	void Set_ParkShutter(CGm_ParkSolarPanelOnce* _pSPOnce) {
		m_pGimmickSPOnce = _pSPOnce;
		Safe_AddRef(m_pGimmickSPOnce);
	}

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

	SHUTTER_STATE				m_eCurState = { STATE_NONE };

	_float					m_fWhiteColorDiffuse = { 0.f };
	_float4					m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };
	//_float				m_fHitPower = {};

public:
	static CGm_ParkShutter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
