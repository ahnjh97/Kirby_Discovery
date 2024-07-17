#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGm_DynamicField final : public CPhysXObject
{
public: 
	enum DYNAMICFILED_TYPE { DFMOVE_UPDOWN, DFMOVE_LEFTRIGHT, DFMOVE_FRONTBACK, DFMOVE_NONE };

public:
	void Set_SolarPanelOnce(class CGm_ParkSolarPanelOnce* _pSolarPanel) { m_pSolarPanelOnce = _pSolarPanel; Safe_AddRef(m_pSolarPanelOnce); }
	void Set_SolarPanelCharge(class CGm_ParkSolarPanelCharge* _pSolarPanel) { m_pSolarPanelCharge = _pSolarPanel;  Safe_AddRef(m_pSolarPanelCharge); }
	void Set_SurpriseBoard(class CSurprisedBoard* _pSurpriseBoard) { m_pSurpriseBoard = _pSurpriseBoard; Safe_AddRef(m_pSurpriseBoard); }

	_uint Get_GimmickIndex() { return  m_iGimmickIndex; }

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

	unordered_set<_uint>	m_setUpdateMeshs;
	vector<_uint>			m_vecPassIndices;
	vector<_float>			m_vecSamplingFactors;

	CModel*					m_pModelCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };

	PxRigidStatic*			m_pStaticActor = { nullptr };
	PxRigidDynamic*			m_pDynamicActor = { nullptr };
	CGm_ParkSolarPanelOnce* m_pSolarPanelOnce = { nullptr };
	CGm_ParkSolarPanelCharge* m_pSolarPanelCharge = { nullptr };
	CSurprisedBoard*		m_pSurpriseBoard = { nullptr };

	DYNAMICFILED_TYPE		m_eDFieldType = { DFMOVE_NONE };

	_float					m_fTime = { 0.f };
	
	_bool					m_IsInteraction = { FALSE };
	_uint					m_iGimmickIndex = {};

public:
	static CGm_DynamicField* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
