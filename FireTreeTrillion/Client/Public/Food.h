#pragma once
#include "ItemObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CFood final : public CItemObject
{
private:
	CFood(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFood(const CFood& rhs);
	virtual ~CFood() = default;

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
	HRESULT Add_Components(wstring strPrototag);
	HRESULT Bind_ShaderResources();

	CGameObject*	 m_pPlayer = { nullptr };
	CModel*			 m_pModelCom = { nullptr };

	_float		 m_fDrinkTime = { 0.f };

public:
	static CFood* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END