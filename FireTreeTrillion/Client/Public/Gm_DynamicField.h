#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

#include "Gm_ParkSolarPanelOnce.h"
#include "Gm_ParkSolarPanelCharge.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGm_DynamicField final : public CPhysXObject
{
public: 
	enum DYNAMICFILED_TYPE { DFMOVE_UPDOWN, DFMOVE_LEFT, DFMOVE_RIGHT, DFMOVE_FRONTBACK, DFMOVE_NONE };
	enum GIMMICK_TYPE { GIMMICK_SPONCE, GIMMICK_SPCHARGE, GIMMICK_SURPRISE, GIMMICK_NONE };

public:
	void Set_SolarPanelOnce(class CGm_ParkSolarPanelOnce* _pSolarPanel);
	void Set_SolarPanelCharge(class CGm_ParkSolarPanelCharge* _pSolarPanel);
	void Set_SurpriseBoard(class CSurprisedBoard* _pSurpriseBoard); 

	_uint Get_GimmickIndex() { return  m_iGimmickIndex; }
	
	_bool IsActivated() { return m_bIsInteraction; }
	void Set_Interaction(_bool bInteraction) { m_bIsInteraction = bInteraction; }
	void RegisterToActorToKirby();

private:
	CGm_DynamicField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGm_DynamicField(const CGm_DynamicField& rhs);
	virtual ~CGm_DynamicField() = default;

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
	HRESULT				Add_Components(const wstring& _wstrModelTag);
	HRESULT				Bind_ShaderResources();
	HRESULT				SetUp_ShaderInfo(const wstring& _wstrModelTag);
	void				Apply_Quake(_float _fTimeDelta, _float _fQuakeDuration, _float _fShakeIntensity);
	_int				Movement_Field(_float _fTimeDelta);
	

	unordered_set<_uint>	m_setUpdateMeshs;
	vector<_uint>			m_vecPassIndices;
	vector<_float>			m_vecSamplingFactors;

	CModel*					m_pModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };

	PxRigidStatic*				m_pStaticActor = { nullptr };
	PxRigidDynamic*				m_pDynamicActor = { nullptr };
	CGm_ParkSolarPanelOnce*		m_pSolarPanelOnce = { nullptr };
	CGm_ParkSolarPanelCharge*	m_pSolarPanelCharge = { nullptr };
	CSurprisedBoard*			m_pSurpriseBoard = { nullptr };

	DYNAMICFILED_TYPE		m_eDFieldType = { DFMOVE_NONE };
	GIMMICK_TYPE			m_eGimmickType = { GIMMICK_NONE };
	CGm_ParkSolarPanelOnce::PANELONCE_STATE m_eSPOnceState = {};
	CGm_ParkSolarPanelCharge::PANELCHARGE_STATE m_eSPChargeState = {};

	_float					m_fTime = { 0.f };
	_float					m_fQuakeTime = { 0.f };
	
	_bool					m_bIsInteraction = { FALSE };
	_bool					m_bIsQuake = { FALSE };

	_uint					m_iGimmickIndex = {};

public:
	static CGm_DynamicField* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
