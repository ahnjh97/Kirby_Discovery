#pragma once
#include "ItemObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

class CCoin final : public CItemObject
{
private:
	CCoin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCoin(const CCoin& rhs);
	virtual ~CCoin() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

private:
	CModel*			m_pModelCom = { nullptr };
	_float			m_fCoinTime = { 0.f };

	_float4			m_vTargetPos = { 0.f, 0.f, 0.f, 0.f };

public:
	static CCoin* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

