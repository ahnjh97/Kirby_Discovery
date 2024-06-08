#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CShader;
END

BEGIN(Client)

class CRigidObject abstract : public CPhysXObject
{
protected:
	CRigidObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRigidObject(const CRigidObject& rhs);
	virtual ~CRigidObject() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif

protected:
	CShader*		m_pShaderCom = { nullptr };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END