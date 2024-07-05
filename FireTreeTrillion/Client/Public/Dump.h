#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CDump :
    public CPhysXObject
{
private:
	CDump(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDump(const CDump& rhs);
	virtual ~CDump() = default;

	enum ANIMINDEX { DUMP_CUTDUMP1, DUMP_CUTDUMP2, DUMP_END };


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

	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };

	ANIMINDEX		m_eCurAnim = { DUMP_END };

public:
	static CDump* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END