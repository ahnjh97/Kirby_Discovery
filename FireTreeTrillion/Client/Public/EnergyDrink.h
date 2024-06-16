#pragma once
#include "ItemObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CEnergyDrink final : public CItemObject
{
private:
	CEnergyDrink(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnergyDrink(const CEnergyDrink& rhs);
	virtual ~CEnergyDrink() = default;

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

	CGameObject*	 m_pPlayer = { nullptr };
	CModel*			 m_pModelCom = { nullptr };
	class CTrigger*	 m_pTrigger = nullptr;

	_float		 m_fDrinkTime = { 0.f };

public:
	static CEnergyDrink* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END