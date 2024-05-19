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
    mToleranceScale.length = 1;        // typical length of an object
    mToleranceScale.speed = 0.1f;         // typical speed of an object, gravity * 1s is a reasonable choice

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


    //// create simulation
    mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    physx::PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, physx::PxPlane(0, 1, 0, 0), *mMaterial);
    mScene->addActor(*groundPlane);

    //float halfExtent = .5f;
    //shape = mPhysics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *mMaterial);
    //physx::PxU32 size = 30;
    //physx::PxTransform t(physx::PxVec3(0));
    //for (physx::PxU32 i = 0; i < size; i++) {
    //    for (physx::PxU32 j = 0; j < size - i; j++) {
    //        physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
    //        physx::PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
    //        body->attachShape(*shape);
    //        physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
    //        mScene->addActor(*body);
    //    }
    //}


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
    PxRigidDynamic* pDynamicActor = mPhysics->createRigidDynamic(pxTransform);
    if (nullptr == pDynamicActor) {
        MSG_BOX(TEXT("Failed to Create RigidDynamic."));
        return nullptr;
    }

    PxShape* pShape = { nullptr };
    if(nullptr == pMaterial)
        pShape = mPhysics->createShape(meshGeometry, *mMaterial);
    else
        pShape = mPhysics->createShape(meshGeometry, *pMaterial);

    if (nullptr == pShape) {
        MSG_BOX(TEXT("Failed to Create Shape."));
        pDynamicActor->release();
        return nullptr;
    }

    pDynamicActor->attachShape(*pShape);
    mScene->addActor(*pDynamicActor);
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
    PxRigidStatic* pStaticActor = mPhysics->createRigidStatic(pxTransform);
    if (nullptr == pStaticActor) {
        MSG_BOX(TEXT("Failed to Create RigidStatic."));
        return nullptr;
    }

    PxShape* pShape = { nullptr };
    if (nullptr == pMaterial)
        pShape = mPhysics->createShape(triGeom, *mMaterial);
    else
        pShape = mPhysics->createShape(triGeom, *pMaterial);

    pStaticActor->attachShape(*pShape);
    mScene->addActor(*pStaticActor);
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

    mPvd->disconnect();

    if(nullptr != shape)
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
