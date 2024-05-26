#include "PhysX.h"
#include "Utils.h"
#include "EventCallBack.h"

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
    mToleranceScale.speed = 0.1f;         // typical speed of an object, gravity * 1s is a reasonable choice

    m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, mToleranceScale, true, m_pPvd);
    CEventCallBack* pEventCallBack = new CEventCallBack();
    PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_pDispatcher;
    sceneDesc.simulationEventCallback = pEventCallBack;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

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
    //m_pRigidDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
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

void CPhysX::Register_Trigger(PxActor* pTriggerActor,  _int iCamIndex)
{
    if (nullptr == m_pScene)
        return;
    CEventCallBack* pEventCallBack = dynamic_cast<CEventCallBack*>(m_pScene->getSimulationEventCallback());
    if (nullptr == pEventCallBack)
        return;
    pEventCallBack->Register_Trigger(pTriggerActor, iCamIndex);
}

void CPhysX::SetUp_CamSetIndexFunc(function<void(_int)> func)
{
    if (nullptr == m_pScene)
        return;
    CEventCallBack* pEventCallBack = dynamic_cast<CEventCallBack*>(m_pScene->getSimulationEventCallback());
    if (nullptr == pEventCallBack)
        return;
    pEventCallBack->SetUp_CamSetIndexFunc(func);
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

