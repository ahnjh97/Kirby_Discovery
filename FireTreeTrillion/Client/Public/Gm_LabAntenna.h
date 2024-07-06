#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGm_LabAntenna final : public CPhysXObject
{
public:
	enum ANIM_STATE { STATE_WAIT, STATE_BREAK, STATE_NONE };
private:
	CGm_LabAntenna(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGm_LabAntenna(const CGm_LabAntenna& rhs);
	virtual ~CGm_LabAntenna() = default;

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
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	unordered_set<_uint>	m_setDebrisMeshs;

	CModel*					m_pModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };

	ANIM_STATE				m_eAnimState = { STATE_NONE };

	_float					m_fHitPower = {};
	_float					m_fWhiteColorDiffuse = {};
	
	_bool					m_bStartAnimation = { false };
	_float4					m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

public:
	static CGm_LabAntenna* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
