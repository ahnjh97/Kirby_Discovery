#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBox final : public CPhysXObject
{
private:
	CBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBox(const CBox& rhs);
	virtual ~CBox() = default;

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

	void				Break_From_Car();

private:
	HRESULT				Add_Components(wstring& wstrModelName);
	HRESULT				Bind_ShaderResources();

private:
	CShader*			m_pShaderCom = { nullptr };
	CModel*				m_pModelCom = { nullptr };
	CModel*				m_pNonAnimModelCom = { nullptr };
	PxRigidDynamic*		m_pDynamicActor = { nullptr };

	_bool				m_bPlayAnim = { false };
	_uint				m_iParticleMesh = {};
	_float				m_fWhiteColorDiffuse = {};

public:
	static CBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END

