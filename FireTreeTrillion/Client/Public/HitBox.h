#pragma once
#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

// Ä¿ºñÀÇ ¹î»ì
class CHitBox final : public CCharacter
{
public:
	struct HITBOX_DESC
	{
		CGameObject* pOwner;
	};
private:
	CHitBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHitBox(const CHitBox& rhs);
	virtual ~CHitBox() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	_int			Check_Collision(_float fTimeDelta);

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	class CGameObject*	m_pOwner			= nullptr;
	class CTransform*	m_pOwnerTransform	= nullptr;

public:
	static CHitBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END