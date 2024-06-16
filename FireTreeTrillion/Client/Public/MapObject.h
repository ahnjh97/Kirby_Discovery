#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CMapObject : public CPhysXObject
{
protected:
	CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapObject(const CMapObject& rhs);
	virtual ~CMapObject() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif
	//ITEMTYPE		Get_ItemType() { return m_eItemType; }

protected:
	CCharacterController*	m_pControllerCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };

	//ITEMTYPE				m_eItemType = { ITEM_END };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END