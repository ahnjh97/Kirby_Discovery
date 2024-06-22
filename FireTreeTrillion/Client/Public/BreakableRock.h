#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBreakableRock final : public CPhysXObject
{
private:
	CBreakableRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBreakableRock(const CBreakableRock& rhs);
	virtual ~CBreakableRock() = default;

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
	HRESULT			Add_Components(wstring strPrototag);
	HRESULT			Bind_ShaderResources();

	_int			Make_Partical();

	CModel*						m_pModelCom = { nullptr };
	CCharacterController*		m_pControllerCom = { nullptr };
	CShader*					m_pShaderCom = { nullptr };

public:
	static CBreakableRock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END