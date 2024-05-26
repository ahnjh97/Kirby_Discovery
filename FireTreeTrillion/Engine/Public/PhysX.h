#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CPhysX final : public CBase
{
private:
    CPhysX();
    virtual ~CPhysX() = default;

public:
    HRESULT Initialize();
    void Tick(_float fTimeDelta);
    
    // test 
    void Test();
    _float4x4 Update(_fmatrix matrix);

    // Actor 관리
    void AddActor(physx::PxActor& pActor);
    void RemoveActor(physx::PxActor& pActor);

    PxPhysics*                          Get_Physics() { return m_pPhysics; }
    PxMaterial*                         Get_Material() { return m_pMaterial; }

    // NOT YET
    //PxMaterial*                         FindMaterial(const string& strMtrlTag);
    PxScene*                            Get_Scene() { return m_pScene; }
    PxControllerManager*                Get_ControllerManager() { return m_pControllerManager; }
    //PxCooking*                        GetCooking() { return m_pCooking; }

    // NOT YET
    //class CComponent*                   Get_Component(physx::PxActor* pActor);

public:
    PxRigidDynamic* CreateDynamicActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);
    PxRigidStatic* CreateStaticActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);

private:
    PxDefaultAllocator                  mDefaultAllocatorCallback;
    PxDefaultErrorCallback              mDefaultErrorCallback;
    PxTolerancesScale                   mToleranceScale;

    PxControllerManager*                m_pControllerManager = nullptr;
    
    PxDefaultCpuDispatcher*             m_pDispatcher = nullptr;
    PxPvdTransport*                     m_pPvdTransport = nullptr;
    PxFoundation*                       m_pFoundation = nullptr;
    PxPhysics*                          m_pPhysics = nullptr;
    PxShape*                            m_pShape = nullptr;
    
    PxMaterial*                         m_pMaterial = nullptr;
    
    PxPvd*                              m_pPvd = nullptr;
    PxScene*                            m_pScene = nullptr;
    PxPvdSceneClient*                   m_pPvdSceneClient = nullptr;

    //PxCooking*                        m_pCooking = nullptr;
    physx::PxRigidDynamic*              m_pRigidDynamic = nullptr;

    map<string, physx::PxMaterial*>     m_mapMaterials;

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


// PxControllerBehaviorCallback : Controller의 충돌처리에 대한 콜백 결과를 받아오는 클래스
// 해당 클래스를 상속받아 이벤트 결과를 활용한다.
class ENGINE_DLL CControllerBehaviorCallback : public PxControllerBehaviorCallback
{
public:
    // 컨트롤러가 다른 객체와 충돌했을 때 호출되는 함수
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) override;

    // 컨트롤러가 다른 컨트롤러와 충돌했을 때 호출되는 함수
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) override;

};

END