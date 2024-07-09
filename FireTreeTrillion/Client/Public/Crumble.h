#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CCrumble final : public CPhysXObject
{
private:
	CCrumble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCrumble(const CCrumble& rhs);
	virtual ~CCrumble() = default;

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;
	virtual HRESULT		Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void		Render_IMGUI()					override;
#endif
	virtual void		Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	void				Add_Components(wstring& wstrModelName);
	HRESULT				Bind_ShaderResources();

private:
	CShader*			m_pShaderCom = { nullptr };
	CModel*				m_pModelCom = { nullptr };
	CModel*				m_pNonAnimModelCom = { nullptr }; // actor 생성 역할
	PxRigidDynamic*		m_pDynamicActor = { nullptr };
	
	_bool				m_bPlayAnim = { false };

public:
	static CCrumble*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END
