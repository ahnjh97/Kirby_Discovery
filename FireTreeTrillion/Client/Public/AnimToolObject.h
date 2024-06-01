#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#define	INFO(state) m_tKirbyInfo.state

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CFSM;
class CCharacterController;
END

BEGIN(Client)


// CAnimToolObject은 TEST용 모델이어서 GameObject 바로 상속받았습니다.
class CAnimToolObject : public CGameObject
{
private:
	CAnimToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimToolObject(const CAnimToolObject& rhs);
	virtual ~CAnimToolObject() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual _int			Tick(_float fTimeDelta) override;
	virtual void			Late_Tick(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual HRESULT			Render_LightDepth() override;
#ifdef _DEBUG
	virtual void			Render_IMGUI() override;
#endif

	void					Change_ModelCom(wstring _protoTag);

	// Animtion 제어	
	void					Set_Animation(_int iAnimIndex);
	_bool					isAnimFinish();

private:
	HRESULT					Add_Components();
	HRESULT					Bind_ShaderResources();
	void					SetOn_Slope(_float fTimeDelta);
	void					Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta);

protected:
	// For_PhysX
	_float					m_fOffsetTurn = { 7.f };
	_float4					m_vOriginUp = { 0.f, 1.f, 0.f, 0.f };

private:
	CModel*					m_pModelCom = {nullptr};
	CShader*				m_pShaderCom = { nullptr };
	class CCamera_Free*		m_pCamera = { nullptr };
	CCharacterController*	m_pControllerCom = { nullptr };

	wstring					m_wstrModelName = L"Prototype_Component_Model_KirbyDefault";
	_int					m_iTestAnim = { 0 };

public:
	static CAnimToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END