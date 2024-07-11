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
        CONTENT_DEFORM,

        CONTENT_VACUUMOBJECT,   // 커비가 흡수 후 날리는 것들과의 충돌

        CONTENT_KICK,           // 플레이어 X 찰수있는 맵 오브젝트

        CONTENT_INTERACT,	    
        CONTENT_ITEM,	        // 아이템 습득관련
        CONTENT_TRIGGER,	    // 피직스 사용하지 않는 트리거용
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
    void Add_Collision(COLLISION_TYPE eCollType, class CGameObject* pGameObject);

private:
    void Collision_Collider(vector<CGameObject*> Dsts, vector<CGameObject*> Srcs, CCollisionCenter* pthis, 
        void(*func)(class CHitBox*, class CHitBox*, CCollisionCenter*));
    _bool Intersect(class CHitBox* Dst, class CHitBox* Src);
    void Ladder_Collider();

    void DeeDeeDee_Battle();
    void Simba_Battle();
    void FinalStage_Battle();
    void RealFinaleStage_Battle();
    void Body_To_Body_Collision();
    void Hitbox_Collision();

    vector<CLadder*>                          m_Ladders;
    vector<CGameObject*>                      m_GameObjects[COLLISION_END];


    // 히트박스 관련
#pragma region KNOCKBACK
    _bool Small_KnockBack(_uint uKirbyState); 
    _bool Normal_KnockBack(_uint uKirbyState);
    _bool Up_KnockBack(_uint uKirbyState);
    _bool FlyAway_KnockBack(_uint uKirbyState);

    void HitStop_Rogic(class CKirby* pKirby, _float fStopTime = 0.12f);

    void Damage_And_Effect_For_Monster(class CKirby* pKirby, class CPhysXObject* pMonster, _float fEffectOffSet = 0.5f);
#pragma endregion


    // 기능들
private:
    void Camera_Shaking(_float fPower = 1.f, _float fTime = 0.5f, _float2 vDir = { 0.f, -1.f });
    void Camera_Zooming(_float fZoom);
    _bool Kirby_Dodge_SlowMotionSystem(CPhysXObject* pPlayer);

    // 플레이어와 몬스터의 충돌로, 서로 가볍게넉백된다.
    void Player_Monster_Knock_back(CPhysXObject* pPlayer, CPhysXObject* pMonster);
    // 두개의 객체가 PO_FLYDEADAWAY 화 되어 날아간다. 서로 벡터를 계산하여 넉백까지 계산해준다.
    void Fly_DeadAway(CPhysXObject* pSrc, CPhysXObject* pDst);
    // 매개변수로 받는 개체가 해당방향과 파워로 날아간다.
    void Knock_back(CPhysXObject* pObject, _float3 vKnockbackDir, _float fPower);
    // 양쪽에 몸박했을 때, 서로 데미지를 받는다. 또한 데미지를 받는 적절한 상태라면 카메라 쉐이킹 포함이다.
    void Compute_Damage(CPhysXObject* pPlayer, CPhysXObject* pMonster);
    // 커비쪽이 데미지를 받는 로직. 또한 데미지를 받는 적절한 상태라면 카메라 쉐이킹 포함이다.
    void Compute_HitBoxDamage(CPhysXObject* pPlayer, CPhysXObject* pMonster);

    // 아이템 관련 로직들.
    void Compute_Heal(CPhysXObject* pPlayer, CPhysXObject* pItem);
    void Compute_Coin(CPhysXObject* pPlayer, CPhysXObject* pItem);
    void Compute_SuperPower(CPhysXObject* pPlayer, CPhysXObject* pItem);

    // 회피 무적 시간정지 전용
    void Dodge_Timer_System(_float fTimeDelta);
    _bool  m_bCheckTimer = { false };
    _float m_fTimeDeltaResetTime = { 0.f };

    void Hit_TimeStop(_float fTimeRatio, _float fTime);
    void Hit_Timer_System(_float fTimeDelta);
    _bool  m_bHitTimeStop = { false };
    _float m_fHitTimeDeltaResetTime = { 0.f };
    _float m_fHitTimeDeltaMaxResetTime = { 0.f };


private:
    CONTENT_TYPE m_eColliderType[COLLISION_END][COLLISION_END];
    virtual void Free() override;

};

END
