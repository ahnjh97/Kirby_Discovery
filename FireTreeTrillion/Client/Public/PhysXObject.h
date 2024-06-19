#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "CollisionCenter.h"
#include "Effect.h"

BEGIN(Client)

/// <summary> 
/// ABILITY 속성값을 부여하기 위한 상위 클래스입니다.
/// 1. physX 영향을 받는 대부분의 객체에 해당됩니다.
/// 2. 해당 클래스를 상속받지 않는 클래스는 다음과 같습니다.
///		- Effect, Camera, UI_Object
/// </summary>
class CPhysXObject abstract : public CGameObject
{
protected:
	CPhysXObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPhysXObject(const CPhysXObject& rhs);
	virtual ~CPhysXObject() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;

	// 충돌처리 함수 ( 피직스를 사용하는 것들 끼리 충돌했을 때 발생하는 함수이다. )
	virtual void		Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) {}
	virtual void		Collision_Hitbox(CPhysXObject* pGameObject) {}

#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif

	ABILITYTYPE	Get_AbilityType() { return m_eAbilityType; }
	void Set_AbilityType(ABILITYTYPE eAbilityType) { m_eAbilityType = eAbilityType; }

	VACUUMSIZE Get_VacuumSize() { return m_eVacuumSize; }


	// 넉백력을 정의해준다.
	void	Set_DamageMoving(_float3 vDamgeDir, _float DamageJumpPower) {
		m_vDamegeDir = vDamgeDir;
		m_fDamageJumpPower = DamageJumpPower;
	}
	_float3 Get_DamegeDir() { return m_vDamegeDir; }
	_float	Get_DamageJumpPower() { return m_fDamageJumpPower; }
	void	Set_DamageJumpPower(_float fDamageJumpPower) { m_fDamageJumpPower = fDamageJumpPower; }

	void				Set_PhyXState(PHYXOBJECT_CURSTATE eState);
	PHYXOBJECT_CURSTATE Get_PhyXState() { return m_ePhyXState; }
	_float	Get_ObjTimeDelta() const { return m_fTimeDelta; }



	//이펙트를 자신의 리스트에 추가한다.
	void	Add_Effect(CEffect* pEffect);
	void	Delete_AllEffect();
	void	Delete_Effect(string strTag);

	// 어택만 Character에서 빼 왔다. 공격력은 총알에도 있을 수 있기 때문임.
	_float	Get_Attack() { return m_fAttack; }
	_float	m_fAttack = { 0.f };


public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	virtual _int Ready_Dead(_float fDeadScale = 1.8f);

	// 현재 이 객체의 타입
	ABILITYTYPE m_eAbilityType = { ABILITY_END };

	// 작은놈인지, 큰놈인지를 이것으로 분류한다.
	VACUUMSIZE m_eVacuumSize = { SIZE_END };

	// 충돌 시, 날아갈 방향을 정의한다. (세기도 조절 가능하다)
	_float3	m_vDamegeDir = { 0.f, 0.f, 0.f };

	// 충돌 시, 공중으로 뜨는 힘을 정의한다.
	_float	m_fDamageJumpPower = { 0.f };
	
	// 모든 객체들이 가지는 시간값
	_float	m_fTimeDelta = { 0.f };

	// 피직스 오브젝트들의 현재 큰 상태를 의미한다.
	PHYXOBJECT_CURSTATE m_ePhyXState = { PO_NORMAL };

	//피직스 오브젝트들에게 귀속되어 움직이는 이펙트들
	list<CEffect*>	m_FXList;
	_float4x4		m_EffectSocket;
};

END