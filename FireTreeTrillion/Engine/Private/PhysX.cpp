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
    // 모든 충돌을 감지하도록 설정
    pairFlags = PxPairFlag::eCONTACT_DEFAULT;
    pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
    pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
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
    sceneDesc.gravity = PxVec3(0.0f, -9.81f * 3.f, 0.0f);
    m_pDispatcher = PxDefaultCpuDispatcherCreate(1);
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
//    // create simulation
//    m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
//    PxRigidStatic* groundPlane = PxCreatePlane(*m_pPhysics, PxPlane(0, 1, 0, 0), *m_pMaterial);
//    m_pScene->addActor(*groundPlane);
//
//    float halfExtent = .5f;
//    m_pShape = m_pPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_pMaterial);
//    PxU32 size = 30;
//    PxTransform t(PxVec3(0));
//
//    PxTransform localTm(PxVec3(0, 0, 0) * halfExtent);
//    m_pRigidDynamic = m_pPhysics->createRigidDynamic(t.transform(localTm));
//    m_pRigidDynamic->attachShape(*m_pShape);
//    PxRigidBodyExt::updateMassAndInertia(*m_pRigidDynamic, 10.0f);
//    m_pScene->addActor(*m_pRigidDynamic);
}

_float4x4 CPhysX::Update(_fmatrix matrix)
{
    //PxTransform trans = m_pRigidDynamic->getGlobalPose();
    //_float4x4 matPos = CUtils::To_Float4x4(trans);
    //return matPos;
    return _float4x4();
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

void CPhysX::Emplace_MapDecoTrigger(PxActor* pTriggerActor, CModel* pMapDecoModel, _uint iAnimIdx, _float fTickPerSec)
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Emplace_MapDecoTrigger(pTriggerActor, pMapDecoModel, iAnimIdx, fTickPerSec);
}

void CPhysX::Clear_EventCallBack()
{
    if (nullptr == m_pEventCallBack)
        return;

    m_pEventCallBack->Clear_EventCallBack();
}

void CPhysX::ResetScene()
{
    //// 기존 Scene의 모든 Actor를 제거
    //PxU32 actorCount = m_pScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
    //PxActor** actors = new PxActor * [actorCount];
    //m_pScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, actors, actorCount);
    //for (PxU32 i = 0; i < actorCount; ++i) {
    //    m_pScene->removeActor(*actors[i]);
    //    actors[i]->release();
    //}
    //delete[] actors;
    //Safe_Delete(m_pEventCallBack);

    //// 새로운 Scene 생성
    //m_pScene->release();
    //PxSceneDesc sceneDesc(mToleranceScale);
    //sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    //sceneDesc.cpuDispatcher = m_pDispatcher;
    //sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    //sceneDesc.broadPhaseType = PxBroadPhaseType::eSAP; // 또는 eMBP
    //m_pEventCallBack = new CEventCallBack();
    //sceneDesc.simulationEventCallback = m_pEventCallBack;
    //m_pScene = m_pPhysics->createScene(sceneDesc);
}

//physx::PxMaterial* CPhysX::FindMaterial(const string& strMtrlTag)
//{
//    //auto itr = m_pMaterials.find(strMtrlTag);
//    //Assert(itr != m_Materials.end());
//    //return itr->second;
//}


PxRigidDynamic* CPhysX::CreateDynamicActor(_float4x4& matWorld, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial)
{
    PxCookingParams tParams(mToleranceScale);

    PxConvexMeshDesc meshDesc;
    meshDesc.points.count = iNumVertices;
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = pVerticesPos;
    meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eSHIFT_VERTICES;
       
    PxConvexMesh* pMesh = PxCreateConvexMesh(tParams, meshDesc);
    if (nullptr == pMesh) {
        MSG_BOX(TEXT("Failed to Create ConvexMesh."));
        return nullptr;
    }

    _float3 vScale{};
    _float4 vQuaternion{};
    _vector vScaleVector, vRotQuat, vTrans;
    ::XMMatrixDecompose(&vScaleVector, &vRotQuat, &vTrans, XMLoadFloat4x4(&matWorld));
    XMStoreFloat3(&vScale, vScaleVector);
    XMStoreFloat4(&vQuaternion, vRotQuat);

    PxMeshScale meshScale(PxVec3(vScale.x, vScale.y, vScale.z));
    PxConvexMeshGeometryFlags meshFlags = PxConvexMeshGeometryFlags();
    PxConvexMeshGeometry meshGeometry(pMesh, meshScale, meshFlags);

    PxTransform pxTransform(PxVec3(matWorld._41, matWorld._42, matWorld._43), PxQuat(vQuaternion.x, vQuaternion.y, vQuaternion.z, vQuaternion.w));

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
    //pShape->setSimulationFilterData(PxFilterData(0, 0, 0, 0));

    pDynamicActor->attachShape(*pShape);
    m_pScene->addActor(*pDynamicActor);
    //physx::PxRigidBodyExt::updateMassAndInertia(*pDynamicActor, 0.1f);

    pMesh->release();
    pShape->release(); 
    m_pRigidDynamic = pDynamicActor;
    return pDynamicActor;
}

void CPhysX::Add_Force(_float3 vForce)
{
    if (m_pRigidDynamic == nullptr) return;

    PxVec3 PxForce = physx::PxVec3(vForce.x, vForce.y, vForce.z);
    m_pRigidDynamic->addForce(PxForce, physx::PxForceMode::eFORCE);
}

void CPhysX::Add_Force(PxRigidDynamic* pDynamicActor, _float3 vForce)
{
    PxVec3 PxForce = physx::PxVec3(vForce.x, vForce.y, vForce.z);
    pDynamicActor->addForce(PxForce, physx::PxForceMode::eFORCE);
}

void CPhysX::Kick_DynamicActor(_float3 _kickDirection, _float impulseMagnitude)
{
    PxVec3 kickDirection(_kickDirection.x, _kickDirection.y, _kickDirection.z);
    PxVec3 impulse = kickDirection * impulseMagnitude;
    m_pRigidDynamic->addForce(impulse, PxForceMode::eIMPULSE);
}

PxRigidStatic* CPhysX::CreateStaticActor(_float4x4& matWorld, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial)
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
    
    _float3 vScale{};
    _float4 vQuaternion{};
    _vector vScaleVector, vRotQuat, vTrans;
    ::XMMatrixDecompose(&vScaleVector, &vRotQuat, &vTrans, XMLoadFloat4x4(&matWorld));
    XMStoreFloat3(&vScale, vScaleVector);
    XMStoreFloat4(&vQuaternion, vRotQuat);
    PxTriangleMesh* pTriMesh = PxCreateTriangleMesh(tParams, triDesc);
    
    PxMeshScale meshScale(PxVec3(vScale.x, vScale.y, vScale.z));
    PxMeshGeometryFlags meshFlags = PxMeshGeometryFlags();
    PxTriangleMeshGeometry triGeom(pTriMesh, meshScale, meshFlags);

    PxTransform pxTransform(PxVec3(matWorld._41, matWorld._42, matWorld._43), PxQuat(vQuaternion.x, vQuaternion.y, vQuaternion.z, vQuaternion.w));

    PxRigidStatic* pStaticActor = m_pPhysics->createRigidStatic(pxTransform);
    if (nullptr == pStaticActor) {
        MSG_BOX(TEXT("Failed to Create RigidStatic."));
        return nullptr;
    }

    PxShape* pShape = { nullptr };
    if (nullptr == pMaterial)
    {
\
        PxMaterial* pMtrl = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
        //PxMaterial* pMtrl = m_pPhysics->createMaterial(0.f, 0.f, 0.6f);
\
        pShape = m_pPhysics->createShape(triGeom, *pMtrl);
    }
    else
        pShape = m_pPhysics->createShape(triGeom, *pMaterial);
    pShape->setSimulationFilterData(PxFilterData(0, 0, 0, 0));

    pStaticActor->attachShape(*pShape);
    m_pScene->addActor(*pStaticActor);
    pTriMesh->release();
    pShape->release();

    return pStaticActor;
}

PxConvexMesh* CPhysX::CreateConvexMesh(_float3* pVerticesPos, _uint iNumVertices, PxMaterial* pMaterial)
{
    PxCookingParams tParams(mToleranceScale);

    PxConvexMeshDesc meshDesc;
    meshDesc.points.count = iNumVertices;
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = pVerticesPos;
    meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eSHIFT_VERTICES;

    return PxCreateConvexMesh(tParams, meshDesc);;
}


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

    if(m_pControllerManager != nullptr)
        m_pControllerManager->release();

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

// ====================================================================================================================
PxControllerBehaviorFlags CControllerBehaviorCallback::getBehaviorFlags(const PxController& controller)
{
    //// controller의 속성에 따라 행동을 커스터마이징
    //// 예를 들어, 충돌 시 미끄러지도록 설정                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
    //CComponent* pComponent = static_cast<CComponent*>(controller.getUserData());
    //if (pComponent != nullptr)
    //{
    //    //CGameObject* pActorObject = pComponent->Get_Object();
    //    if (pActorObject->Get_PrototypeTag() == L"Prototype_GameObject_StarBlock")
    //    {
    //        return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
    //    }
    //}
    return PxControllerBehaviorFlag::eCCT_SLIDE;
}

