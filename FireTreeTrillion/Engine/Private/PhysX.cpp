#include "PhysX.h"

CPhysX::CPhysX()
{
}

HRESULT CPhysX::Initialize()
{
    // init physx
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
    if (!mFoundation) throw("PxCreateFoundation failed!");
    mPvd = PxCreatePvd(*mFoundation);
    transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

    //mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(),true, mPvd);
    mToleranceScale.length = 100;        // typical length of an object
    mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);

    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    mScene = mPhysics->createScene(sceneDesc);

    pvdClient = mScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }


    // create simulation
    mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    physx::PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, physx::PxPlane(0, 1, 0, 0), *mMaterial);
    mScene->addActor(*groundPlane);

    float halfExtent = .5f;
    shape = mPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *mMaterial);
    physx::PxU32 size = 30;
    physx::PxTransform t(physx::PxVec3(0));
    for (physx::PxU32 i = 0; i < size; i++) {
        for (physx::PxU32 j = 0; j < size - i; j++) {
            physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
            physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);
            physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            mScene->addActor(*body);
        }
    }


    // run simulation
    //while (1) {
    //    mScene->simulate(1.0f / 60.0f);
    //    mScene->fetchResults(true);
    //}

    return S_OK;
}

void CPhysX::Tick()
{
    mScene->simulate(1.0f / 60.0f);
    mScene->fetchResults(true);
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

    mPvd->disconnect();

    shape->release();

    // 2. Scene 해제
    mScene->release();

    // 3. Material 해제
    mMaterial->release();

    // 4. Dispatcher 해제
    mDispatcher->release();

    // 5. Pvd Transport 해제
    transport->release();

    // 6. Physics 해제
    mPhysics->release();

    // 7. Pvd 해제 (선택적, Pvd 사용 시)
    mPvd->release();

    // 8. Foundation 해제
    mFoundation->release();
}
