#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CItemObject : public CPhysXObject
{
protected:
	CItemObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CItemObject(const CItemObject& rhs);
	virtual ~CItemObject() = default;

public:
	enum ITEMTYPE {
		ITEM_COIN, ITEM_FOOD, ITEM_SUPERPOWER,
		ITEM_END
	};


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

	ITEMTYPE		Get_ItemType() { return m_eItemType; }
	_int			Get_ItemPoint() { return m_iItemPoint; }
	_bool			Get_ItemCollisionComplete() { return m_bCollisionComplete; }

protected:
	CCharacterController* m_pControllerCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	ITEMTYPE m_eItemType = { ITEM_END };
	// 아이템을 먹었을 때, 해당 포인트로 작동한다.
	// EX) FOOD타입이 30이라면 30회복, COIN타입이 1이라면 코인 1회복, SUPERPOWER타입 5라면 5초간 무적이다.
	// 아무튼 이 값은 충돌 처리시, 타입에 따라서 알맞게 정해줄 예정임
	_int	 m_iItemPoint = { 0 };

	// 충돌되면 true로 바뀌고, 더 이상 충돌되지 않는다.
	_bool	 m_bCollisionComplete = { false };

	_float4x4* m_ItemSocketMatrix = { nullptr };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END