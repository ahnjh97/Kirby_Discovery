#pragma once
#include "ItemObject.h"
#include "PartTimeHelper.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CPartTimeFood final : public CItemObject
{
private:
	CPartTimeFood(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPartTimeFood(const CPartTimeFood& rhs);
	virtual ~CPartTimeFood() = default;

public:
	virtual HRESULT		Initialize_Prototype()	override;
	virtual HRESULT		Initialize(void* pArg)	override;
	virtual _int		Tick(_float fTimeDelta) override;
	virtual void		Late_Tick(_float fTimeDelta) override;
	virtual HRESULT		Render()				override;
	virtual HRESULT		Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void		Render_IMGUI() override;
#endif
	virtual void		Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

private:
	HRESULT				Add_Components();
	HRESULT				Bind_ShaderResources();

private:
	array<CModel*, 4>	m_arrModelCom = { nullptr };
	//CGameObject*		m_pPlayer = { nullptr };
	PARTTIME_ITEM		m_eItem = { PARTTIME_ITEM::DRINK };

public:
	static CPartTimeFood*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

END