#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGm_ParkSolarPanelOnce final : public CPhysXObject
{
public: //괄호의 숫자는 실제 인게임의 기믹 애님 순서. index는 아님
	enum ANIM_STATE {	STATE_ONWAITSTART, //충전 시작 (3)
						STATE_CHARGE, //충전 중 (2)
						STATE_ONWAIT, //충전 완료 (4)
						STATE_OFFWAIT, //충전 전 대기 (1)
						STATE_NONE };

	enum LAMP_TYPE { LAMP_GREEN, LAMP_RED, LAMP_YELLOW, LAMP_NONE };

public:
	ANIM_STATE Get_AnimState() { return m_eCurState; }

private:
	CGm_ParkSolarPanelOnce(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGm_ParkSolarPanelOnce(const CGm_ParkSolarPanelOnce& rhs);
	virtual ~CGm_ParkSolarPanelOnce() = default;

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

	unordered_set<_uint>	m_setUpdateMeshs;
	//unordered_set<_uint>	m_setAfterHideMeshs;

	CModel*					m_pModelCom = { nullptr };
	CModel*					m_pNonAnimModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };

	PxRigidStatic*			m_pStaticActor = { nullptr };

	ANIM_STATE				m_eCurState = { STATE_NONE };

	_float					m_fDecreasesTime = { 0.f };
	_float					m_fWhiteColorDiffuse = { 0.f };
	_float4					m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };
	//_float				m_fHitPower = {};
	
	_bool					m_IsInteraction = { FALSE };
	_bool					m_bStartCharge = { FALSE };

	vector<_uint>			m_vecPassIndices;

public:
	static CGm_ParkSolarPanelOnce* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
