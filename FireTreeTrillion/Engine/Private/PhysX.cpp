#include "PhysX.h"
#include "Utils.h"
#include "EventCallBack.h"
#include "GameObject.h"
#include "GameInstance.h"

CPhysX::CPhysX()
{
}

#define OVERLAP_MAX 8

PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
    // 기본 충돌 처리
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;
    return PxFilterFlag::eDEFAULT;
}

HRESULT CPhysX::Initialize()
{
    // init physx
    m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
    if (!m_pFoundation) throw("PxCreateFoundation failed!");
    m_pPvd = PxCreatePvd(*m_pFoundation);
    m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_pPvd->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eALL);

    //mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(),true, mPvd);
    mToleranceScale.length = 1;        // typical length of an object
    mToleranceScale.speed = 0.1f;      // typical speed of an object, gravity * 1s is a reasonable choice

    m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, mToleranceScale, true, m_pPvd);
    m_pEventCallBack = new CEventCallBack();
    PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    m_pDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_pDispatcher;
    sceneDesc.simulationEventCallback = m_pEventCallBack;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
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
    //for (physx::PxU32 i = 0; i < size; i++) 
    //{
    //    for (physx::PxU32 j = 0; j < size - i; j++) 
    //    {
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

    return S_OK;
}

void CPhysX::Tick(_float fTimeDelta)
{
    m_pScene->simulate(fTimeDelta);
    m_pScene->fetchResults(true);

    //CheckPvdConnection(m_pPvd);
}

void CPhysX::CheckPvdConnection(PxPvd* pvd) 
{
    if (pvd->isConnected()) 
    {
        MSG_BOX(L"PVD is connected.");
    }
    else 
    {
        MSG_BOX(L"PVD is NOT connected.");
    }
}


/// physX에 영향을 받는 테스트용 Ground를 만들어줍니다.
void CPhysX::Ready_TestGround()
{
    PxRigidStatic* groundPlane = PxCreatePlane(*m_pPhysics, PxPlane(0, 1, 0, 0), *m_pMaterial);
    m_pScene->addActor(*groundPlane);
}

void CPhysX::Test()
{
    // create simulation
    m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    PxRigidStatic* groundPlane = PxCreatePlane(*m_pPhysics, PxPlane(0, 1, 0, 0), *m_pMaterial);
    m_pScene->addActor(*groundPlane);

    float halfExtent = .5f;
    m_pShape = m_pPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_pMaterial);
    PxU32 size = 30;
    PxTransform t(PxVec3(0));

    PxTransform localTm(PxVec3(0, 0, 0) * halfExtent);
    m_pRigidDynamic = m_pPhysics->createRigidDynamic(t.transform(localTm));
    m_pRigidDynamic->attachShape(*m_pShape);
    PxRigidBodyExt::updateMassAndInertia(*m_pRigidDynamic, 10.0f);
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
    if (nullptr == m_pScene)
        return;

    m_pScene->addActor(pActor);
}

void CPhysX::RemoveActor(physx::PxActor& pActor)
{
    if (nullptr == m_pScene)
        return;

    m_pScene->removeActor(pActor);
}

void CPhysX::Register_Player(PxActor* pPlayerActor)
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Register_Player(pPlayerActor);
}

void CPhysX::Register_Controller(PxActor* pActor, PxController* pController)
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Register_Controller(pActor, pController);
}

void CPhysX::Register_Trigger(PxActor* pTriggerActor, _int iTriggerType, _int iTriggerIndex)
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Register_Trigger(pTriggerActor, iTriggerType, iTriggerIndex);
}

void CPhysX::Emplace_TriggerFunc(_int iTriggerType, function<void(_int)> func)
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Emplace_TriggerFunc(iTriggerType, func);
}

void CPhysX::Emplace_ExitFunc(_int iTriggerType, function<void(void)> exitFunc)
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Emplace_ExitFunc(iTriggerType, exitFunc);
}

void CPhysX::Clear_EventCallBack()
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Clear_EventCallBack();
}

//physx::PxMaterial* CPhysX::FindMaterial(const string& strMtrlTag)
//{
//    //auto itr = m_pMaterials.find(strMtrlTag);
//    //Assert(itr != m_Materials.end());
//    //return itr->second;
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
    m_pRigidDynamic = pDynamicActor;
    return pDynamicActor;
}

void CPhysX::Kick_DynamicActor(_float3 _kickDirection, _float impulseMagnitude)
{
    PxVec3 kickDirection(_kickDirection.x, _kickDirection.y, _kickDirection.z);
    PxVec3 impulse = kickDirection * impulseMagnitude;
    m_pRigidDynamic->addForce(impulse, PxForceMode::eIMPULSE);
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


// 함수 내에 해당 코드를 포함한다고 가정
//void CPhysX::Overlap_Hitbox(CGameObject* pGameObject, _float4 vPos, _float fRadius)
//{
//    // 겹침을 검사할 구체의 기하학적 모양 생성
//    PxSphereGeometry overlapShape = PxSphereGeometry(fRadius);
//    // 초기 위치와 회전을 설정하는 PxTransform 객체 생성
//    PxTransform pxTransform(PxVec3(vPos.x, vPos.y, vPos.z));
//
//    // Overlap 결과를 저장할 배열 동적 할당
//    PxOverlapHit* hitOverlap = new PxOverlapHit[OVERLAP_MAX]; 	//const int maxHits = 8; //= 4096;
//    PxScene* myScene = CGameInstance::Get_Instance()->Get_Scene();
//    _int howMany = PxSceneQueryExt::overlapMultiple(*myScene, overlapShape, pxTransform, hitOverlap, OVERLAP_MAX, PxQueryFilterData(PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC | PxQueryFlag::eNO_BLOCK));
//
//    CGameObject* pPlayer = nullptr;
//    for (_int i = 0; i < howMany; ++i)
//    {
//        PxOverlapHit& hit = hitOverlap[i];
//        PxRigidActor* actor = hit.actor;  // 충돌된 객체의 액터
//        // FOR TEST
//        if (howMany > 2)
//            _int b = 3;
//        if (actor->userData == "RigidMesh")
//            continue;// _int a = 3;
//
//        const char* actorName = actor->getName();
//        CComponent* pComponent = static_cast<CComponent*>(actor->userData);
//        if (pComponent == nullptr) continue;
//
//        // ======================================== FOR TEST : 임시 ========================================
//        CGameObject* pActorObject = pComponent->Get_Object();
//        if (pActorObject == nullptr) continue;
//        if (pActorObject->Get_PrototypeTag() != pPlayer->Get_PrototypeTag())
//            pPlayer->Collision_Overlap(pActorObject); // actorObject가 플레이어가 아닐경우 collision_overlap 실행
//        // =================================================================================================
//    }
//
//    Safe_Delete_Array(hitOverlap);
//}


CPhysX* CPhysX::Create()
{
    CPhysX* pInstance = new CPhysX();

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX(TEXT("Failed to Create : CPhysX"));
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

    if (m_pRigidDynamic != nullptr)
        m_pRigidDynamic->release();

    Safe_Delete(m_pEventCallBack);

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
        /*if (pComponentDst->Get_Object()->Get_PrototypeTag() == L"Prototype_GameObject_Kirby")
            int a = 3;
        if (pComponentSrc->Get_Object()->Get_PrototypeTag() == L"Prototype_GameObject_Kirby")
            int b = 3;*/
	}
}

void CSimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    _int a = 3;
}

// ====================================================================================================================

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


void CUserControllerHitReport::onShapeHit(const PxControllerShapeHit& hit)
{
}

void CUserControllerHitReport::onControllerHit(const PxControllersHit& hit)
{
	PxController* MeController = hit.controller;
	PxController* otherController = hit.other;
    // physX에서 사라지게 한 Actor 예외처리
    if (MeController->getActor() == nullptr || otherController->getActor() == nullptr)
        return;

	CComponent* pComponentSrc = static_cast<CComponent*>(MeController->getUserData());
	CComponent* pComponentDst = static_cast<CComponent*>(otherController->getUserData());
	if (pComponentSrc != nullptr && pComponentDst != nullptr)
	{
		CGameObject* pActorObjectSrc = pComponentSrc->Get_Object();
		CGameObject* pActorObjectDst = pComponentDst->Get_Object();

        CGameInstance::Get_Instance()->Add_CollisionObjects(pActorObjectSrc, pActorObjectDst);
    }
}

/// <summary> 히트박스와 콜라이더의 충돌을 어떻게 처리할 것인지 정의하는 PhysX의 콜백함수입니다. </summary>
_bool CControllerFilterCallback::filter(const PxController& pObj, const PxController& pOtherObj)
{
    if (pObj.getActor() != nullptr && pOtherObj.getActor() != nullptr)
    {
        CComponent* pComponentObj    = static_cast<CComponent*>(pObj.getUserData());
        CComponent* pComponentOther  = static_cast<CComponent*>(pOtherObj.getUserData());

        if (pComponentObj != nullptr && pComponentOther != nullptr)
        {
            CGameObject* pActorObject = pComponentObj->Get_Object();
            CGameObject* pActorOther  = pComponentOther->Get_Object();
            // 둘중에 하나가 true라면 (둘 중에 하나가 히트박스 또는 아이템 등인 것이다) return false 하여 물리적 충돌을 피한다.
            if ((CGameInstance::Get_Instance()->Is_PassingGroup(pActorObject) 
                || CGameInstance::Get_Instance()->Is_PassingGroup(pActorOther)) == true)
            {
                return false;
            }
        }
    }
    return true;
}

