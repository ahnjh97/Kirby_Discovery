#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CCarShopWall final : public CPhysXObject
{
private:
	CCarShopWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCarShopWall(const CCarShopWall& rhs);
	virtual ~CCarShopWall() = default;

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

	_int			Make_Partical();
	_float			m_fHitPower = { 0.f };
	_float			m_fWhiteColorDiffuse = { 0.f };
	_uint			m_iNoNormalNoMRAMesh = {};

	CModel* m_pModelCom = { nullptr };
	CCharacterController* m_pControllerCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

public:
	static CCarShopWall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
