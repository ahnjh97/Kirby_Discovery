#pragma once
#include "Client_Defines.h"
#include "MapObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCharacterController;
END

BEGIN(Client)

class CStarBlock final : public CMapObject
{
public:
	enum SIZE_TYPE { SMALL, MEDIUM, LARGE, SIZE_END };
	
	struct STARBLOCK_DESC : public CGameObject::GAMEOBJECT_DESC 
	{
		SIZE_TYPE	eSize;
	};

private:
	CStarBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStarBlock(const CStarBlock& rhs);
	virtual ~CStarBlock() = default;

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;
	virtual HRESULT		Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void		Render_IMGUI()					override;
#endif
	virtual void		Collision_Hitbox(CPhysXObject* pGameObject) override;

private:
	HRESULT				Add_Components();
	HRESULT				Bind_ShaderResources();

private:
	CModel*					m_pModelCom = nullptr;
	CCharacterController*	m_pControllerCom = { nullptr };

	SIZE_TYPE				m_eSize = SIZE_END;
	_int	m_iHP = 10;
	
public:
	static CStarBlock*	 Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void		 Free() override;

};

END