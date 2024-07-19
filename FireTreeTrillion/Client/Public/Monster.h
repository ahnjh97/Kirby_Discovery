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
	enum MONSTER_STATE {
		MON_CIRCLE, MON_PATROL, MON_WAIT, MON_SLEEP, MON_END
	};

	struct MONSTER_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		_bool bRealDead = { false };
		_uint eMonState = { 0 };
		_float fAngle = { 0.f };
		vector<_float4> vecRallyPoints;
	};

protected:
	CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster(const CMonster& rhs);
	virtual ~CMonster() = default;

public:
	void Set_MonState(MONSTER_STATE eMonState) { m_eMonState = eMonState; }

	_float Get_AnimRatio() { return m_pModelCom->Get_AnimRatio(); }
	MONSTER_STATE Get_MonState() { return m_eMonState; }

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

	void			Damage_Delay(_float fTimeDelta);
	void			Set_Damage_Delay(_float fMaxTime) { m_fMonsterOverPowerMaxTime = fMaxTime; }
	_bool			Get_MonsterOverPower() { return m_bMonsterOverPower; }


protected:
	// JSPark : 우선 몬스터들은 modelCom을 하나만 가지고 있다는 전제하에
	CModel*					m_pModelCom	 = { nullptr };

	MONSTER_STATE			m_eMonState = { MON_END };


	// 무적상태 제어
	_bool					m_bMonsterOverPower = { false };
	_float					m_fMonsterOverPowerTime = { 0.f };
	_float					m_fMonsterOverPowerMaxTime = { 0.f };
	_float					m_fPreHp = { 0.f };
	_float					m_fY = { 0.f };

	// 납작해졌다가 죽을 시간
	_float	m_fPressedTime = { 0.f };


public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END