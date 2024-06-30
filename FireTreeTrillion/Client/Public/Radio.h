#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CRadio final : public CGameObject
{
private:
	CRadio(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRadio(const CRadio& rhs);
	virtual ~CRadio() = default;

	enum ANIMINDEX { RADIO_BEAT, RADIO_JUMP, RADIO_END };

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
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float	m_fTimeDelta = { 0.f };

	void						JumpEvent(CGameObject* pObj);
	_bool						m_bJump = { false };
	_float						m_fJumpTime = { 0.f };
	_float						m_fSaveY = { 0.f };
	_float						m_fSaveAfterY = { 0.f };

	_bool						m_bEventStart = { false };
	_float						m_fEventTime = { 0.f };

	_uint						m_iSwitchEaseIn = { 0 };

public:
	static CRadio* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END