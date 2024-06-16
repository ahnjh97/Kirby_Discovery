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
    void Register_Controller(PxActor* pActor, PxController* pController);
    void Register_Trigger(PxActor* pTriggerActor, _int iTriggerType, _int iTriggerIndex);
    void Emplace_TriggerFunc(_int iTriggerType, function<void(_int)> func);
    void Emplace_ExitFunc(_int iTriggerType, function<void(void)> exitFunc);
    void Emplace_MapDecoTrigger(PxActor* pTriggerActor, CModel* pMapDecoModel, _uint iAnimIdx, _float fTickPerSec);
    void Clear_EventCallBack();

    PxPhysics*                          Get_Physics() { return m_pPhysics; }
    PxMaterial*                         Get_Material() { return m_pMaterial; }
    PxScene*                            Get_Scene() { return m_pScene; }
    PxControllerManager*                Get_ControllerManager() { return m_pControllerManager; }
    
    // NOT YET
    //PxMaterial*                       FindMaterial(const string& strMtrlTag);
    //PxCooking*                        GetCooking() { return m_pCooking; }
    //class CComponent*                 Get_Component(physx::PxActor* pActor);

public:
    void            Kick_DynamicActor(_float3 _kickDirection, _float impulseMagnitude);
    void            Add_Force(_float3 _kickDirection);
    PxRigidDynamic* CreateDynamicActor(_float4x4& matWorld, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);
    PxRigidStatic*  CreateStaticActor(_float4x4& matWorld, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);

private:
    PxDefaultAllocator          mDefaultAllocatorCallback;
    PxDefaultErrorCallback      mDefaultErrorCallback;
    PxTolerancesScale           mToleranceScale;

    PxControllerManager*        m_pControllerManager = { nullptr };
    PxDefaultCpuDispatcher*     m_pDispatcher       = { nullptr };
    PxPvdTransport*             m_pPvdTransport     = { nullptr };
    PxFoundation*               m_pFoundation       = { nullptr };
    PxPhysics*                  m_pPhysics          = { nullptr };
    PxShape*                    m_pShape            = { nullptr };
    
    PxMaterial*                 m_pMaterial         = { nullptr };
    
    PxPvd*                      m_pPvd              = { nullptr };
    PxScene*                    m_pScene            = { nullptr };
    PxPvdSceneClient*           m_pPvdSceneClient   = { nullptr };

    //PxCooking*                m_pCooking          = nullptr;
    PxRigidDynamic*             m_pRigidDynamic     = { nullptr };

    class CEventCallBack*       m_pEventCallBack    = { nullptr };

    map<string, PxMaterial*>    m_mapMaterials;

public:
    static CPhysX*  Create();
    virtual void    Free() override;

};


// PxControllerBehaviorCallback : Controller 끼리의 충돌처리에 대한 콜백 결과를 받아오는 클래스
// 해당 클래스를 상속받아 이벤트 결과를 활용한다.
class ENGINE_DLL CControllerBehaviorCallback : public PxControllerBehaviorCallback
{
public:
    // 컨트롤러가 다른 객체와 충돌했을 때 호출되는 함수
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) override {
        return PxControllerBehaviorFlag::eCCT_SLIDE;
    }
    // 컨트롤러가 다른 컨트롤러와 충돌했을 때 호출되는 함수
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) override;

    // 사용하지 않지만 순수가상함수로 상속받아야하는 함수. 절대 지우지 말 것!
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle&) override {
        return PxControllerBehaviorFlag::eCCT_SLIDE;
    }
};

// PxUserControllerHitReport : Controller의 모든 충돌 정보를 가지고옴.
class ENGINE_DLL CUserControllerHitReport : public physx::PxUserControllerHitReport
{
public:
    // 캐릭터 컨트롤러의 충돌 이벤트 처리
    virtual void    onShapeHit(const physx::PxControllerShapeHit& hit) override;
    
    virtual void    onControllerHit(const PxControllersHit& hit) override;

    // for 순수가상함수
    virtual void    onObstacleHit(const PxControllerObstacleHit& hit) override {}
};

class CControllerFilterCallback : public PxControllerFilterCallback
{
public:
    virtual bool    filter(const PxController& pObj, const PxController& pOtherObj) override;
};

END

  