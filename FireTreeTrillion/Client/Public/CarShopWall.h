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
	void			SwitchAfterBefore();

	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };

	_float			m_fHitPower = {};
	_float			m_fWhiteColorDiffuse = {};
	
	_bool			m_bStartAnimation = { false };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };

	unordered_set<_uint>	m_setNonRenderMeshes;
	unordered_set<_uint>	m_setBeforeMeshIndices;
	unordered_set<_uint>	m_setNormalXMesh;

public:
	static CCarShopWall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
