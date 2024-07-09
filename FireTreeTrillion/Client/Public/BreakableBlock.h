#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

class CBreakableBlock final : public CPhysXObject
{
private:
	CBreakableBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBreakableBlock(const CBreakableBlock& rhs);
	virtual ~CBreakableBlock() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth()	override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_bool	m_bHit = { false };

	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
public:
	static CBreakableBlock* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

