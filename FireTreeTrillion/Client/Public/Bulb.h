#pragma once

#include "Client_Defines.h"
#include "Deform.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

class CBulb final : public CDeform
{
private:
	CBulb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBulb(const CBulb& rhs);
	virtual ~CBulb() = default;

	enum ANIMINDEX { BULB_FALL, BULB_LANDING, BULB_WAIT, BULB_END };

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;

	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	CModel* m_pModelCom = { nullptr };

	ANIMINDEX					m_eAnimIndex = { BULB_END };
	ANIMINDEX					m_ePreAnimIndex = { BULB_END };
	_float						m_fFallTime = { 0.f };

	void						Set_Animation();

public:
	static CBulb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

