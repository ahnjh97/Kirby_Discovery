#pragma once

#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
class CFSM;
class CCharacterController;
END

BEGIN(Client)

class CMonster abstract : public CCharacter
{
public:
	struct MONSTER_DESC : public CGameObject::GAMEOBJECT_DESC
	{

	};

protected:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& rhs);
	virtual ~CMonster() = default;

public:
	_float Get_AnimRatio() { return m_pModelCom->Get_AnimRatio(); }

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
	virtual void	Add_AnimEvent()  override {}


protected:
	// JSPark : 우선 몬스터들은 modelCom을 하나만 가지고 있다는 전제하에
	CModel*					m_pModelCom	 = { nullptr };

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END