#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Ladder.h"
#include "Base.h"


BEGIN(Client)

class CCollisionCenter :
    public CBase
{
    DECLARE_SINGLETON(CCollisionCenter)
public:
    enum CONTENT_TYPE {
        CONTENT_BODY,		    // 캐릭터와 몬스터가 몸끼리 박음

        CONTENT_ATTACK,         // 플레이어의 히트박스 X 몬스터
        CONTENT_DAMAGE,         // 몬스터와의 히트박스 X 플레이어

        CONTENT_VACUUMOBJECT,   // 커비가 흡수 후 날리는 것들과의 충돌

        CONTENT_INTERACT,	    
        CONTENT_ITEM,	        // 아이템 습득관련
        CONTENT_NONEVENT,	    
        CONTENT_END
    };

private:
    CCollisionCenter() {};
    virtual ~CCollisionCenter() = default;

public:
    void Initialize();

public:
    void Collision_Tick(_float fTimeDelta);
    void Add_Ladder(CLadder* pLadder);

private:
    CONTENT_TYPE Find_ColliderType(class CPhysXObject* pSrc, class CPhysXObject* pDst);
    class CPhysXObject* Find_TypePtr(COLLISION_TYPE eType, CPhysXObject* pSrc, CPhysXObject* pDst);
    void Collision_Collider(CONTENT_TYPE eType, class CPhysXObject* pSrc, class CPhysXObject* pDst);
    void Ladder_Collider();

    set< pair<CGameObject*, CGameObject*> > m_WaitingList;
    set< class CTrigger* > m_Hitboxes;
    list<CLadder*>                          m_Ladders;


    // 히트박스 관련
#pragma region KNOCKBACK
    _bool Small_KnockBack(_uint uKirbyState); 
    _bool Normal_KnockBack(_uint uKirbyState);
    _bool Up_KnockBack(_uint uKirbyState);
    _bool FlyAway_KnockBack(_uint uKirbyState);

    void HitStop_Rogic(class CKirby* pKirby);

    void Damage_To_Monster(class CKirby* pKirby, CPhysXObject* pMonster);
#pragma endregion


    // 기능들
private:
    void Camera_Shaking(_float fPower = 1.f, _float fTime = 0.5f, _float2 vDir = { 0.f, -1.f });
    void Camera_Zooming(_float fZoom);
    _bool Kirby_Dodge_SlowMotionSystem(CPhysXObject* pPlayer);

    void Player_Monster_Knock_back(CPhysXObject* pPlayer, CPhysXObject* pMonster);
    void Fly_DeadAway(CPhysXObject* pSrc, CPhysXObject* pDst);
    void Knock_back(CPhysXObject* pObject, _float3 vKnockbackDir, _float fPower);

    void Compute_Damage(CPhysXObject* pPlayer, CPhysXObject* pMonster);
    void Compute_Heal(CPhysXObject* pPlayer, CPhysXObject* pItem);
    void Compute_Coin(CPhysXObject* pPlayer, CPhysXObject* pItem);
    void Compute_SuperPower(CPhysXObject* pPlayer, CPhysXObject* pItem);

    void Timer_System(_float fTimeDelta);
    _bool  m_bCheckTimer = { false };
    _float m_fTimeDeltaResetTime = { 0.f };


private:
    CONTENT_TYPE m_eColliderType[COLLISION_END][COLLISION_END];
    virtual void Free() override;

};

END
