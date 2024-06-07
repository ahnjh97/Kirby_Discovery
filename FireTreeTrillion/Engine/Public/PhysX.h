#pragma once
#include "Base.h"
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CPhysX final : public CBase
{
private:
    CPhysX();
    virtual ~CPhysX() = default;

public:
    HRESULT Initialize();
    HRESULT Ready_CollisionContents();
    void    Tick(_float fTimeDelta);

    // For Animation
    void    Ready_TestGround();

    // test 
    void CheckPvdConnection(PxPvd* pvd);
    void Test();
    _float4x4 Update(_fmatrix matrix);

    // Actor 관리
    void AddActor(physx::PxActor& pActor);
    void RemoveActor(physx::PxActor& pActor);

    //EventCallBack 함수들
    void Register_Player(PxActor* pPlayerActor);
    void Register_Trigger(PxActor* pTriggerActor, _int iTriggerType, _int iTriggerIndex);
    void Emplace_TriggerFunc(_int iTriggerType, function<void(_int)> func);
    void Emplace_ExitFunc(_int iTriggerType, function<void(void)> exitFunc);
    void Clear_EventCallBack();

    PxPhysics*                          Get_Physics() { return m_pPhysics; }
    PxMaterial*                         Get_Material() { return m_pMaterial; }
    PxScene*                            Get_Scene() { return m_pScene; }
    PxControllerManager*                Get_ControllerManager() { return m_pControllerManager; }
    
    _uint                               Get_CollisionContent(COLLISION_TYPE eMeType, COLLISION_TYPE eOtherType) 
    { 
        return arrCollisionContents[eMeType][eOtherType];
    }

    // NOT YET
    //PxMaterial*                       FindMaterial(const string& strMtrlTag);
    //PxCooking*                        GetCooking() { return m_pCooking; }
    //class CComponent*                 Get_Component(physx::PxActor* pActor);

public:
    PxRigidDynamic* CreateDynamicActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);
    PxRigidStatic*  CreateStaticActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);

   

private:
    PxDefaultAllocator          mDefaultAllocatorCallback;
    PxDefaultErrorCallback      mDefaultErrorCallback;
    PxTolerancesScale           mToleranceScale;

    PxControllerManager*        m_pControllerManager = nullptr;
    
    PxDefaultCpuDispatcher*     m_pDispatcher = nullptr;
    PxPvdTransport*             m_pPvdTransport = nullptr;
    PxFoundation*               m_pFoundation = nullptr;
    PxPhysics*                  m_pPhysics = nullptr;
    PxShape*                    m_pShape = nullptr;
    
    PxMaterial*                 m_pMaterial = nullptr;
    
    PxPvd*                      m_pPvd = nullptr;
    PxScene*                    m_pScene = nullptr;
    PxPvdSceneClient*           m_pPvdSceneClient = nullptr;

    //PxCooking*                m_pCooking = nullptr;
    PxRigidDynamic*             m_pRigidDynamic = nullptr;

    class CEventCallBack*       m_pEventCallBack = nullptr;

    map<string, PxMaterial*>    m_mapMaterials;

private:
    // 충돌처리할 그룹들의 결과컨텐츠를 2차원 배열로 저장합니다.
    _uint arrCollisionContents[COLLISION_END][COLLISION_END];

public:

    static CPhysX*  Create();
    virtual void    Free() override;

};


// PxSimulationEventCallback : RigidBody의 충돌처리에 대한 콜백 결과를 받아오는 클래스
class ENGINE_DLL CSimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
    virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    // 트리거 이벤트 처리
    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
};


// PxControllerBehaviorCallback : Controller 끼리의 충돌처리에 대한 콜백 결과를 받아오는 클래스
// 해당 클래스를 상속받아 이벤트 결과를 활용한다.
class ENGINE_DLL CControllerBehaviorCallback : public PxControllerBehaviorCallback
{
public:
    // 컨트롤러가 다른 객체와 충돌했을 때 호출되는 함수
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) override;

    // 컨트롤러가 다른 컨트롤러와 충돌했을 때 호출되는 함수
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) override;

    // 사용하지 않지만 순수가상함수로 상속받아야하는 함수. 절대 지우지 말 것!
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle&) override;
    
};

// PxUserControllerHitReport : Controller의 모든 충돌 정보를 가지고옴.
class CUserControllerHitReport : public physx::PxUserControllerHitReport 
{
private:
    void Initialize();

public:
    // 캐릭터 컨트롤러의 충돌 이벤트 처리
    virtual void onShapeHit(const physx::PxControllerShapeHit& hit) override;
    
    virtual void onControllerHit(const PxControllersHit& hit) override;

    // NOT YET
    //virtual void onControllerShapeHit(const PxControllerShapeHit& hit) override {}
    
    // for 순수가상함수
    virtual void onObstacleHit(const PxControllerObstacleHit& hit) override {}

    void    CollsionEvent(class CGameObject* pObj, class CGameObject* pOtherObj/*, COLLISION_TYPE eOwnCollsionGroup, COLLISION_TYPE eOtherCollsionGroup*/);

//private:
    // CollisionGroup끼리 부딪혔을 때, 충돌 컨텐츠 지정. Initialize에서 내용물 채워주기.
    //_uint arrCollisionContents[COLLISION_END][COLLISION_END];

};


END


static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

