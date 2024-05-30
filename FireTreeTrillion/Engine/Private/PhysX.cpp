#include "PhysX.h"
#include "Utils.h"
#include "EventCallBack.h"
#include "GameObject.h"
#include "GameInstance.h"

CPhysX::CPhysX()
{
}

HRESULT CPhysX::Initialize()
{
    // init physx
    m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
    if (!m_pFoundation) throw("PxCreateFoundation failed!");
    m_pPvd = PxCreatePvd(*m_pFoundation);
    m_pPvdTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_pPvd->connect(*m_pPvdTransport, physx::PxPvdInstrumentationFlag::eALL);

    //mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(),true, mPvd);
    mToleranceScale.length = 1;        // typical length of an object
    mToleranceScale.speed = 0.1f;      // typical speed of an object, gravity * 1s is a reasonable choice

    m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, mToleranceScale, true, m_pPvd);
    CEventCallBack* pEventCallBack = new CEventCallBack();
    PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_pDispatcher;
    sceneDesc.simulationEventCallback = pEventCallBack;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    sceneDesc.broadPhaseType = PxBroadPhaseType::eSAP; // 또는 eMBP

    m_pScene = m_pPhysics->createScene(sceneDesc);
    
    m_pPvdSceneClient = m_pScene->getScenePvdClient();
    if (m_pPvdSceneClient)
    {
        m_pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS,   true);
        m_pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS,      true);
        m_pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES,  true);
    }

    // create simulation
    m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    m_pControllerManager = PxCreateControllerManager(*m_pScene);

    //physx::PxRigidStatic* groundPlane = PxCreatePlane(*m_pPhysics, physx::PxPlane(0, 1, 0, 0), *m_pMaterial);
    //m_pScene->addActor(*groundPlane);

    //float halfExtent = .5f;
    //m_pShape = m_pPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_pMaterial);
    //physx::PxU32 size = 30;
    //physx::PxTransform t(physx::PxVec3(0));
    //for (physx::PxU32 i = 0; i < size; i++) {
    //    for (physx::PxU32 j = 0; j < size - i; j++) {
    //        physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
    //        physx::PxRigidDynamic* body = m_pPhysics->createRigidDynamic(t.transform(localTm));
    //        body->attachShape(*m_pShape);
    //        physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
    //        m_pScene->addActor(*body);
    //    }
    //        physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
    //        body->attachShape(*shape);
    //        physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
    //        mScene->addActor(*body);
    //    }
    //}

    // 충돌처리할 그룹들을 나누어 충돌결과를 관리한다.
    Ready_CollisionContents();

    return S_OK;
}

/// <summary> Initialize 'COLLISION_CONTENT' </summary>
HRESULT CPhysX::Ready_CollisionContents()
{
    // BLOCK EVENT
    arrCollisionContents[PLAYER][MONSTER]   = CONTENT_ATTACK;
    arrCollisionContents[MONSTER][PLAYER]   = CONTENT_ATTACK;

    //arrCollisionContents[MONSTER][PLAYER]	= CONTENT_ATTACK;
    arrCollisionContents[PLAYER][INTERACT]  = CONTENT_INTERACT;
    arrCollisionContents[PLAYER][ITEM]      = CONTENT_ACQUIRE;

    return S_OK;
}

void CPhysX::Tick(_float fTimeDelta)
{
    m_pScene->simulate(fTimeDelta);
    m_pScene->fetchResults(true);
}

void CPhysX::Test()
{
    // create simulation
    m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    physx::PxRigidStatic* groundPlane = PxCreatePlane(*m_pPhysics, physx::PxPlane(0, 1, 0, 0), *m_pMaterial);
    m_pScene->addActor(*groundPlane);

    float halfExtent = .5f;
    m_pShape = m_pPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_pMaterial);
    physx::PxU32 size = 30;
    physx::PxTransform t(physx::PxVec3(0));

    physx::PxTransform localTm(physx::PxVec3(0, 0, 0) * halfExtent);
    m_pRigidDynamic = m_pPhysics->createRigidDynamic(t.transform(localTm));
    m_pRigidDynamic->attachShape(*m_pShape);
    physx::PxRigidBodyExt::updateMassAndInertia(*m_pRigidDynamic, 10.0f);
    m_pScene->addActor(*m_pRigidDynamic);
}

_float4x4 CPhysX::Update(_fmatrix matrix)
{
    //PxVec3 pos = CUtils::To_Float4x4(matrix).getPosition();
    //PxTransform newPose(pos);
    //m_pRigidDynamic->setGlobalPose(newPose);
    PxTransform trans = m_pRigidDynamic->getGlobalPose();
    _float4x4 matPos = CUtils::To_Float4x4(trans);
    return matPos;
}

void CPhysX::AddActor(physx::PxActor& pActor)
{
    m_pScene->addActor(pActor);
}

void CPhysX::RemoveActor(physx::PxActor& pActor)
{
    m_pScene->removeActor(pActor);
}

void CPhysX::Register_Player(PxActor* pPlayerActor)
{
    if (nullptr == m_pScene)
        return;
    CEventCallBack* pEventCallBack = dynamic_cast<CEventCallBack*>(m_pScene->getSimulationEventCallback());
    if (nullptr == pEventCallBack)
        return;
    pEventCallBack->Register_Player(pPlayerActor);
}

void CPhysX::Register_Trigger(PxActor* pTriggerActor, _int iTriggerType, _int iTriggerIndex)
{
    if (nullptr == m_pScene)
        return;
    CEventCallBack* pEventCallBack = dynamic_cast<CEventCallBack*>(m_pScene->getSimulationEventCallback());
    if (nullptr == pEventCallBack)
        return;
    pEventCallBack->Register_Trigger(pTriggerActor, iTriggerType, iTriggerIndex);
}

void CPhysX::SetUp_TriggerFunc(_int iTriggerType, function<void(_int)> func)
{
    if (nullptr == m_pScene)
        return;
    CEventCallBack* pEventCallBack = dynamic_cast<CEventCallBack*>(m_pScene->getSimulationEventCallback());
    if (nullptr == pEventCallBack)
        return;
    pEventCallBack->SetUp_TriggerFunc(iTriggerType, func);
}

void CPhysX::Clear_EventCallBack()
{
    if (nullptr == m_pScene)
        return;
    CEventCallBack* pEventCallBack = dynamic_cast<CEventCallBack*>(m_pScene->getSimulationEventCallback());
    if (nullptr == pEventCallBack)
        return;
    pEventCallBack->Clear_EventCallBack();
}

//physx::PxMaterial* CPhysX::FindMaterial(const string& strMtrlTag)
//{
//    //auto itr = m_pMaterials.find(strMtrlTag);
//    //Assert(itr != m_Materials.end());
//    //return itr->second;
//}

// Actor에 넣어둔 data를 가져온다. >> Not Yet >> 이 부분은 사용하려는 것의 정보를 가져오게 설정 바꿀것.
//CComponent* CPhysX::Get_Component(physx::PxActor* pActor)
//{
//    if (pActor == nullptr || pActor->userData == nullptr)
//        return nullptr;
//    return static_cast<CComponent*>(pActor->userData);
//}

PxRigidDynamic* CPhysX::CreateDynamicActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial)
{
    PxCookingParams tParams(mToleranceScale);

    PxConvexMeshDesc meshDesc;
    meshDesc.points.count = iNumVertices;
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = pVerticesPos;
    meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* pMesh = PxCreateConvexMesh(tParams, meshDesc);
    if (nullptr == pMesh) {
        MSG_BOX(TEXT("Failed to Create ConvexMesh."));
        return nullptr;
    }

    PxMeshScale meshScale(PxVec3(1.0f, 1.0f, 1.0f));
    PxMeshGeometryFlags meshFlags = PxMeshGeometryFlags();
    PxConvexMeshGeometry meshGeometry(pMesh);

    PxTransform pxTransform(PxVec3(vPos.x, vPos.y, vPos.z));
    PxRigidDynamic* pDynamicActor = m_pPhysics->createRigidDynamic(pxTransform);
    if (nullptr == pDynamicActor) {
        MSG_BOX(TEXT("Failed to Create RigidDynamic."));
        return nullptr;
    }

    PxShape* pShape = { nullptr };
    if(nullptr == pMaterial)
        pShape = m_pPhysics->createShape(meshGeometry, *m_pMaterial);
    else
        pShape = m_pPhysics->createShape(meshGeometry, *pMaterial);

    if (nullptr == pShape) {
        MSG_BOX(TEXT("Failed to Create Shape."));
        pDynamicActor->release();
        return nullptr;
    }

    pDynamicActor->attachShape(*pShape);
    m_pScene->addActor(*pDynamicActor);
    pMesh->release();
    pShape->release(); 

    return pDynamicActor;
}

PxRigidStatic* CPhysX::CreateStaticActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial)
{
    PxCookingParams tParams(mToleranceScale);
    tParams.buildTriangleAdjacencies = true;

    PxTriangleMeshDesc triDesc;
    triDesc.points.count = iNumVertices;
    triDesc.points.stride = sizeof(PxVec3);
    triDesc.points.data = pVerticesPos;
    triDesc.triangles.count = iNumIndices / 3;  // 삼각형 개수
    triDesc.triangles.stride = 3 * sizeof(PxU32);
    triDesc.triangles.data = pIndices;
    PxTriangleMesh* pTriMesh = PxCreateTriangleMesh(tParams, triDesc);
    PxMeshScale meshScale(PxVec3(1.0f, 1.0f, 1.0f));
    PxMeshGeometryFlags meshFlags = PxMeshGeometryFlags();
    PxTriangleMeshGeometry triGeom(pTriMesh, meshScale, meshFlags);

    PxTransform pxTransform(PxVec3(vPos.x, vPos.y, vPos.z));
    PxRigidStatic* pStaticActor = m_pPhysics->createRigidStatic(pxTransform);
    if (nullptr == pStaticActor) {
        MSG_BOX(TEXT("Failed to Create RigidStatic."));
        return nullptr;
    }

    PxShape* pShape = { nullptr };
    if (nullptr == pMaterial)
    {
        PxMaterial* pMtrl = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
        pShape = m_pPhysics->createShape(triGeom, *pMtrl);

    }
    else
        pShape = m_pPhysics->createShape(triGeom, *pMaterial);

    pStaticActor->attachShape(*pShape);
    m_pScene->addActor(*pStaticActor);
    pTriMesh->release();
    pShape->release();

    return pStaticActor;
}

CPhysX* CPhysX::Create()
{
    CPhysX* pInstance = new CPhysX();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX(TEXT("Failed to Created : CPhysX"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CPhysX::Free()
{
    __super::Free();

    m_pPvd->disconnect();

    if(nullptr != m_pShape)
        m_pShape->release();

    m_pControllerManager->release();

    // 2. Scene 해제
    if (m_pScene != nullptr)
        m_pScene->release();

    // 3. Material 해제
    if (m_pMaterial != nullptr)
        m_pMaterial->release();

    // 4. Dispatcher 해제
    if (m_pDispatcher != nullptr)
        m_pDispatcher->release();

    // 5. Pvd Transport 해제
    if (m_pPvdTransport != nullptr)
        m_pPvdTransport->release();

    // 6. Physics 해제
    if (m_pPhysics != nullptr)
        m_pPhysics->release();

    // 7. Pvd 해제 (선택적, Pvd 사용 시)
    if (m_pPvd != nullptr)
        m_pPvd->release();

    // 8. Foundation 해제
    if (m_pFoundation != nullptr)
        m_pFoundation->release();
}


// =========================================== 충돌 이벤트들을 던져주는 클래스 ===========================================
//eNOTIFY_TOUCH_FOUND    : 두 물체가 서로 접촉을 시작했을 때 이벤트를 발생시킵니다. (동적 객체와 정적 객체 모두에 적용 가능)
//eNOTIFY_TOUCH_LOST     : 두 물체가 서로의 접촉을 끝냈을 때 이벤트를 발생시킵니다. (동적 객체와 정적 객체 모두에 적용 가능)
//eNOTIFY_TOUCH_PERSISTS : 두 물체가 접촉을 유지하는 동안 이벤트를 지속적으로 발생시킵니다. (동적 객체와 정적 객체 모두에 적용 가능)
//eNOTIFY_TOUCH_FORCE_THRESHOLD : 접촉하는 물체의 힘이 일정 임계값 이상일 때 이벤트를 발생시킵니다. (동적 객체와 정적 객체 모두에 적용 가능)

// 충돌처리함수
void CSimulationEventCallback::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
	CComponent* pComponentDst = static_cast<CComponent*>(pairHeader.actors[0]->userData);
	CComponent* pComponentSrc = static_cast<CComponent*>(pairHeader.actors[1]->userData);
	if (pComponentDst != nullptr && pComponentSrc != nullptr)
	{
		
	}
}

void CSimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
}

// ====================================================================================================================

// RIGIDBODY 충돌을 여기서 상세히 기록할 것 
PxControllerBehaviorFlags CControllerBehaviorCallback::getBehaviorFlags(const PxShape& shape, const PxActor& actor)
{
    // 특정 조건에 따라 행동을 정의
    //if (actor.is<PxRigidStatic>())
    CComponent* pComponent = static_cast<CComponent*>(actor.userData);
    if (pComponent != nullptr)
    {
        CGameObject* pObj = pComponent->Get_Object();
        //MSG_BOX(TEXT("뚜뚱 어떠한 것과 충 돌"));
    }
    if (actor.is<PxRigidStatic>())  // PxController
    {
        //MSG_BOX(TEXT("floor collision"));
        return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
    }
    return PxControllerBehaviorFlag::eCCT_SLIDE;
}


// 컨트롤러에 대한 충돌을 나눠야하는데 
PxControllerBehaviorFlags CControllerBehaviorCallback::getBehaviorFlags(const PxController& controller)
{
    // controller의 속성에 따라 행동을 커스터마이징
    // 예를 들어, 충돌 시 미끄러지도록 설정                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
    CComponent* pComponent = static_cast<CComponent*>(controller.getUserData());
    if (pComponent != nullptr)
    {
    }

    return PxControllerBehaviorFlag::eCCT_SLIDE;
}

PxControllerBehaviorFlags CControllerBehaviorCallback::getBehaviorFlags(const PxObstacle&)
{
    return PxControllerBehaviorFlag::eCCT_SLIDE;
}


void CUserControllerHitReport::onShapeHit(const physx::PxControllerShapeHit& hit)
{
}

void CUserControllerHitReport::onControllerHit(const PxControllersHit& hit)
{
	PxController* MeController = hit.controller;
	PxController* otherController = hit.other;

    // wi
	CComponent* pComponentDst = static_cast<CComponent*>(MeController->getUserData());
	CComponent* pComponentSrc = static_cast<CComponent*>(otherController->getUserData());

	if (pComponentDst != nullptr && pComponentSrc != nullptr)
	{
		CGameObject* pActorObjectDst = pComponentDst->Get_Object();
		CGameObject* pActorObjectSrc = pComponentSrc->Get_Object();

        CollsionEvent(pActorObjectDst, pActorObjectSrc);
	}
}

void CUserControllerHitReport::CollsionEvent(CGameObject* pObj, CGameObject* pOtherObj/*, COLLISION_TYPE eOwnCollsionGroup, COLLISION_TYPE eOtherCollsionGroup*/)
{
    COLLISION_TYPE ObjGroup = pObj->Get_CollisionGroup();
    COLLISION_TYPE OtherGroup = pOtherObj->Get_CollisionGroup();
    
    _uint iCollisionContent = CGameInstance::Get_Instance()->Get_CollisionContent(ObjGroup, OtherGroup);
    switch (iCollisionContent)
    {
    // 공-피격, 상호작용
    case CONTENT_ATTACK:
    {
        pOtherObj->Collision_Attack(pObj);
        pObj->Collision_Attack(pOtherObj);
    }
    break;
    // 트리거, NPC 충돌
    case CONTENT_INTERACT:
    {
    }
    break;
    case CONTENT_ACQUIRE:
    {
        //pObj->Collision_Acquire(pOtherObj);
        //pOtherObj->Collision_Acquire(pObj);
    }
    break;
    case CONTENT_NONEVENT:
    {
        //pObj->Collision_BlockEvent();
        //pOtherObj->
    }
    break;
    }
}

//
//void CUserControllerHitReport::onControllerHit(const PxControllersHit& hit)
//{
//    PxController* MeController = hit.controller;
//    PxController* otherController = hit.other;
//
//    // wi
//    CComponent* pComponentDst = static_cast<CComponent*>(MeController->getUserData());
//    CComponent* pComponentSrc = static_cast<CComponent*>(otherController->getUserData());
//
//    if (pComponentDst != nullptr && pComponentSrc != nullptr)
//    {
//        CGameObject* pActorObjectDst = pComponentDst->Get_Object();
//        COLLISION_TYPE objectTypeDst = pActorObjectDst->Get_CollisionGroup();
//
//        CGameObject* pActorObjectSrc = pComponentSrc->Get_Object();
//        COLLISION_TYPE objectTypeSrc = pActorObjectSrc->Get_CollisionGroup();
//
//
//        switch (objectTypeDst)
//        {
//        case COLLISION_TYPE::PLAYER:
//            pActorObjectDst->Collision_Attack(pActorObjectSrc);
//            //handlePlayerCollision(static_cast<PxRigidDynamic*>(pairHeader.actors[0]), static_cast<PxRigidDynamic*>(pairHeader.actors[1]));
//            break;
//        case COLLISION_TYPE::MONSTER:
//            pActorObjectDst->Collision_Attack();
//            //handleEnemyCollision(static_cast<PxRigidDynamic*>(pairHeader.actors[0]), static_cast<PxRigidDynamic*>(pairHeader.actors[1]));
//            break;
//        case COLLISION_TYPE::INTERACT:
//            //MSG_BOX(TEXT("충돌 주체가 FRIEND"));
//            //handleObstacleCollision(static_cast<PxRigidDynamic*>(pairHeader.actors[0]), static_cast<PxRigidDynamic*>(pairHeader.actors[1]));
//            break;
//        }
//
//        switch (objectTypeSrc)
//        {
//        case COLLISION_TYPE::PLAYER:
//            pActorObjectSrc->Collision_Attack();
//            //MSG_BOX(TEXT("충돌 대상자가 PLAYER"));
//            //handlePlayerCollision(static_cast<PxRigidDynamic*>(pairHeader.actors[1]), static_cast<PxRigidDynamic*>(pairHeader.actors[0]));
//            break;
//        case COLLISION_TYPE::MONSTER:
//            pActorObjectSrc->Collision_Attack();
//            //MSG_BOX(TEXT("충돌 대상자가 MONSTER"));
//            //handleEnemyCollision(static_cast<PxRigidDynamic*>(pairHeader.actors[1]), static_cast<PxRigidDynamic*>(pairHeader.actors[0]));
//            break;
//        case COLLISION_TYPE::INTERACT:
//            //handleObstacleCollision(static_cast<PxRigidDynamic*>(pairHeader.actors[1]), static_cast<PxRigidDynamic*>(pairHeader.actors[0]));
//            break;
//        }
//    }
//}
