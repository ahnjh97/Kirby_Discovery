#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CCar final : public CPhysXObject
{
private:
	CCar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCar(const CCar& rhs);
	virtual ~CCar() = default;

	enum ANIMINDEX { CAR_FALL, CAR_LANDING, CAR_SHAKE, CAR_WAIT, CAR_END };

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

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	CModel*						m_pModelCom = { nullptr };
	CCharacterController*		m_pControllerCom = { nullptr };
	CShader*					m_pShaderCom = { nullptr };

	ANIMINDEX					m_eAnimIndex = { CAR_END };
	ANIMINDEX					m_ePreAnimIndex = { CAR_END };
	_float						m_fFallTime = { 0.f };

	void						Set_Animation();

public:
	static CCar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END