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
public:
	struct FOOD_DESC : public GAMEOBJECT_DESC
	{
		_float4x4*	pBoneMatrix = { nullptr };
		_bool		bRender;
		_uint		uItem;
	};

private:
	CPartTimeFood(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPartTimeFood(const CPartTimeFood& rhs);
	virtual ~CPartTimeFood() = default;

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;
	virtual HRESULT		Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void		Render_IMGUI() override;
#endif
	void				Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

	void				Update_Position(_float4 vPos);
	void				Set_Item(PARTTIME_ITEM _eItem) { m_eItem = _eItem; }
	void				Set_Render(_bool _bRender) { m_bRender = _bRender; }
private:
	HRESULT				Add_Components();
	HRESULT				Bind_ShaderResources();

private:
	CShader*			m_pShaderCom = { nullptr };
	_float4x4*			m_pBoneMatrix = { nullptr };
	array<CModel*, 4>	m_arrModelCom = { nullptr };
	PARTTIME_ITEM		m_eItem = { PARTTIME_ITEM::CAKE };
	_bool				m_bRender = false;

public:
	static CPartTimeFood*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};

END