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
    void Tick();

public:
    PxRigidDynamic* CreateDynamicActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);
    PxRigidStatic* CreateStaticActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial);

private:
    PxDefaultAllocator          mDefaultAllocatorCallback;
    PxDefaultErrorCallback      mDefaultErrorCallback;
    PxTolerancesScale           mToleranceScale;
    PxDefaultCpuDispatcher* mDispatcher = NULL;
    PxPvdTransport* transport = NULL;
    PxFoundation* mFoundation = NULL;
    PxPhysics* mPhysics = NULL;
    PxShape* shape = NULL;
    PxScene* mScene = NULL;
    PxMaterial* mMaterial = NULL;
    PxPvd* mPvd = NULL;
    PxPvdSceneClient* pvdClient = NULL;

public:
    static CPhysX* Create();
    virtual void Free() override;
};

END