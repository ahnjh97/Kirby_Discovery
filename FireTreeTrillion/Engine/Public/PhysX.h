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

    // Actor °ü¸®
    void AddActor(physx::PxActor& pActor);
    void RemoveActor(physx::PxActor& pActor);

    PxPhysics*                          Get_Physics() { return m_pPhysics; }
    PxMaterial*                         Get_Material() { return m_pMaterial; }

    // not yet
    //PxMaterial*                         FindMaterial(const string& strMtrlTag);
    PxScene*                            Get_Scene() { return m_pScene; }
    PxControllerManager*                Get_ControllerManager() { return m_pControllerManager; }
    //PxCooking*                        GetCooking() { return m_pCooking; }

    //physx::PxMat44                    To_Float4x4(const _float4x4& mat);
    //_float4x4                         To_Float4x4(const physx::PxMat44& mat);
    class CComponent*                   Get_Component(physx::PxActor* pActor);

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

END