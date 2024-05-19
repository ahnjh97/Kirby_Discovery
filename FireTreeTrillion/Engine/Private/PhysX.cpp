#include "PhysX.h"
#include "Utils.h"

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
    mToleranceScale.length = 1;   // typical length of an object
    mToleranceScale.speed = 9.8;  // typical speed of an object, gravity*1s is a reasonable choice

    m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, mToleranceScale, true, m_pPvd);

    PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_pDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    m_pScene = m_pPhysics->createScene(sceneDesc);

    m_pPvdSceneClient = m_pScene->getScenePvdClient();
    if (m_pPvdSceneClient)
    {
        m_pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        m_pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        m_pPvdSceneClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // create simulation
    //m_pMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.6f);
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
    //}

    return S_OK;
}

void CPhysX::Tick()
{
    m_pScene->simulate(1.0f / 60.0f);
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

void CPhysX::Update(_fmatrix matrix)
{
    PxVec3 pos = CUtils::To_Float4x4(matrix).getPosition();
    PxTransform newPose(pos);
    m_pRigidDynamic->setGlobalPose(newPose);
}


//
//// DX의 행렬(_float4x4)을 PhysX의 행렬으로 변경한다.
//physx::PxMat44 CPhysX::To_Float4x4(const _float4x4& mat)
//{
//    physx::PxMat44 out;
//    memcpy(&out.column0, &mat.m[0], sizeof(_float4));
//    memcpy(&out.column1, &mat.m[1], sizeof(_float4));
//    memcpy(&out.column2, &mat.m[2], sizeof(_float4));
//    memcpy(&out.column3, &mat.m[3], sizeof(_float4));
//    return out;
//}
//
//// PhysX의 행렬(_float4x4)을 DX의 행렬로 변경한다.
//_float4x4 CPhysX::To_Float4x4(const physx::PxMat44& mat)
//{
//    _float4x4 out;
//    memcpy(&out.m[0], &mat.column0, sizeof(_float4));
//    memcpy(&out.m[1], &mat.column1, sizeof(_float4));
//    memcpy(&out.m[2], &mat.column2, sizeof(_float4));
//    memcpy(&out.m[3], &mat.column3, sizeof(_float4));
//    return out;
//}


//physx::PxMaterial* CPhysX::FindMaterial(const string& strMtrlTag)
//{
//    //auto itr = m_pMaterials.find(strMtrlTag);
//    //Assert(itr != m_Materials.end());
//    //return itr->second;
//}

// PhysX에서 사용되는 Actor의 data를 가져온다.
CComponent* CPhysX::Get_Component(physx::PxActor* pActor)
{
    if (pActor == nullptr || pActor->userData == nullptr)
        return nullptr;
    return static_cast<CComponent*>(pActor->userData);
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

    m_pShape->release();

    // 2. Scene 해제
    m_pScene->release();

    // 3. Material 해제
    m_pMaterial->release();

    // 4. Dispatcher 해제
    m_pDispatcher->release();

    // 5. Pvd Transport 해제
    m_pPvdTransport->release();

    // 6. Physics 해제
    m_pPhysics->release();

    // 7. Pvd 해제 (선택적, Pvd 사용 시)
    m_pPvd->release();

    // 8. Foundation 해제
    m_pFoundation->release();
}

