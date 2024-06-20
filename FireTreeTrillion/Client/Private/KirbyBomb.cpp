#include "stdafx.h"
#include "KirbyBomb.h"
#include "MultiEffect.h"
#include "HitBox.h"

CKirbyBomb::CKirbyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CRigidObject{ pDevice, pContext }
{
}

CKirbyBomb::CKirbyBomb(const CKirbyBomb& rhs)
    : CRigidObject{ rhs }
{
}

HRESULT CKirbyBomb::Initialize_Prototype()
{

    return S_OK;
}

HRESULT CKirbyBomb::Initialize(void* pArg)
{
    GAMEOBJECT_DESC		GameObjectDesc{};
    GameObjectDesc.fSpeedPerSec = 7.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    KIRBYBOMB_DESC desc = *(KIRBYBOMB_DESC*)pArg;

    m_pKirby = desc.pKirby;
    m_pKirbyHandsMatrix = desc.pKirbyHandsMatrix;
    m_pKirbyWorldMatrix = desc.pKirbyWorldMatrix;
    Safe_AddRef(m_pKirby);

    if (m_pKirby == nullptr)
        return E_FAIL;

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    _float4x4 vNewMatrix = *m_pKirbyHandsMatrix * *m_pKirbyWorldMatrix;
    m_pTransformCom->Set_WorldMatrix(vNewMatrix);
    _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
    vPos.y += 1.f;
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

    if (FAILED(Add_Components()))
        return E_FAIL;

    m_pModelCom->Set_Animation(5, 60.f, false, false);

    m_bStencil = true;
    m_bRimLight = true;
    m_bMotionBlur = true;

    m_fAttack = 10.f;

    return S_OK;
}

_int CKirbyBomb::Tick(_float fTimeDelta)
{
    if (true == m_bDead)
        return Ready_Dead();

    m_fTimeDelta = m_pGameInstance->Get_FirstTimer();
    m_fDontKicking += m_fTimeDelta;

    Compute_MotionBlur();
    __super::Tick(m_fTimeDelta);

    CKirby::KIRBY_INFODESC* desc = m_pKirby->Get_KirbyInfo();

    if (desc->m_bBombHold == true)
    {
        // 던져짐과 동시에 이것도 영원히 발동안함.
        if (m_bThrowTrigger == true)
        {
            _float4x4 vNewMatrix = *m_pKirbyHandsMatrix * *m_pKirbyWorldMatrix;
            m_pTransformCom->Set_WorldMatrix(vNewMatrix);
            _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
            vPos.y += 1.f;
            m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
        }
    }
    else
    {
        // 태생에 한번만 발동한다.
        Throwing(desc);

        // 근처에 플레이어가 있을 경우 발로 찬다.
        Kicking();

        // 근처에 몬스터 또는 폭탄이 있을 때 폭발한다.
        //SuddenBoom();

    }


    if (m_bThrowTrigger == false)
    {
        m_fBombTime += m_fTimeDelta;

        m_fBombingTime += m_fTimeDelta * (1.f / 8.f);

        if (m_fBombTime > 8.f)
            m_bDead = true;
    }

    // 등장 애님 끝나면.
    if (m_pModelCom->IsFinished())
    {
        m_pModelCom->Set_Animation(1, 60.f, true, false);
    }


    return OBJ_NOEVENT;
}

void CKirbyBomb::Late_Tick(_float fTimeDelta)
{
    __super::Late_Tick(m_fTimeDelta);
    m_pModelCom->Play_Animation(m_fTimeDelta * (1.f + pow(m_fBombingTime, 4.f)));


    if (m_pRigidBodyCom != nullptr)
    {
        m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
        if (RayCast_Terrain())
            m_pRigidBodyCom->Add_Torque(-1.1f);
    }


    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

}

HRESULT CKirbyBomb::Render()
{
    HRESULT hr;
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
        CHECK_FAILED(hr);
        hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
        CHECK_FAILED(hr);
        hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
        CHECK_FAILED(hr);
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;
        hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O);
        CHECK_FAILED(hr);

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CKirbyBomb::Render_LightDepth()
{
    if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

#ifdef _DEBUG

void CKirbyBomb::Render_IMGUI()
{
}

#endif

void CKirbyBomb::Kicking()
{
    if (m_fDontKicking < 1.f)
    {
        return;
    }
    CTransform* pTransform = m_pKirby->Get_TransformCom();
    _float4 vKirbyPos = pTransform->Get_State(CTransform::STATE_POSITION);
    _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
    _float4 vDir = vPos - vKirbyPos;

    _float fDistance = (vDir).Length();

    if (fDistance < 1.8f)
    {
        vDir.y = 0.f;
        m_pRigidBodyCom->Kick_RigidBody(XMVector3Normalize(vDir), 18.f);
        m_fDontKicking = 0.f;
    }
}

void CKirbyBomb::Throwing(CKirby::KIRBY_INFODESC* desc)
{
    // 손에 들고 있는 상황이 아님이 감지되었다.
    // 이 폭탄은 평생에 한번 발동한다.
    if (m_bThrowTrigger == true)
    {
        Add_Rigid();
        m_pRigidBodyCom->Kick_RigidBody((_float3)desc->m_vBombThrowDir, desc->m_fBombPower);
        m_bThrowTrigger = false;
    }
}

HRESULT CKirbyBomb::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Model Prototype_Component_Model_KirbyBombDefault*/
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyBombDefault"),
        TEXT("Com_Model"), (CComponent**)&m_pModelCom);
    CHECK_FAILED(hr);

    CHitBox::HITBOX_DESC HitBox{};
    HitBox.pOwner = this;
    HitBox.pDesc = &m_tColliderDesc[BODY];
    HitBox.pCollisionType = PLAYERBULLET;
    if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
        return E_FAIL;
    Set_BodyCollider(COLLIDER_SPHERE, 0.f, 0.f, 1.f);

    return S_OK;
}

HRESULT CKirbyBomb::Add_Rigid()
{
    HRESULT hr;

    /* For.Com_RigidBody */
    CRigidBody::RIGIDBODY_DESC rigidDesc {};
    rigidDesc.bTrigger = false;
    rigidDesc.bDynamic = true;
    rigidDesc.bKinematic = false;
    rigidDesc.eShapeType = RIGID_SPHERE;
    rigidDesc.fOffsetSize = { 1.3f, 1.3f, 1.3f };
    rigidDesc.vMaterial = _float3(0.5f, 0.5f, 0.f);
    rigidDesc.fDensity = 1.f;
    rigidDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
    hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
        TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
    CHECK_FAILED(hr);
    //m_pRigidBodyCom->Set_Object(this);
    m_pRigidBodyCom->Activate(true);

    return S_OK;
}

HRESULT CKirbyBomb::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
        return E_FAIL;
    _float fWhiteColor = 0.f;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColor, sizeof(_float))))
        return E_FAIL;



    return S_OK;
}

void CKirbyBomb::Compute_MotionBlur()
{
    _vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
    _matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
    _float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
    _float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

    _float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

    m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
    m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
    m_vMotionVelocity.z = 1.f;//m_ePhyXState != PO_NORMAL ? 1.f : 0.f;

    m_vPreScreenPos = vCurScreenPos;
}

_bool CKirbyBomb::RayCast_Terrain()
{
    _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
    PxVec3 rayOrigin = PxVec3((_float)vPos.x, (_float)vPos.y, (_float)vPos.z);
    PxVec3 rayDirection = PxVec3(0.f, -1.f, 0.f);
    _float fMaxDistance = 1.5f;

    PxRaycastHit hit;
    PxRaycastBuffer hitBuffer;
    PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

    _bool isRayCast = m_pGameInstance->Get_Scene()->raycast(rayOrigin, rayDirection, fMaxDistance, hitBuffer, PxHitFlag::eNORMAL, filterData);

    _float4 vTerrainNormal;

    if (isRayCast == true)
    {
        hit = hitBuffer.block;
        vTerrainNormal = XMVectorSetW(CUtils::To_Vector(hit.normal), 0.f);

        if (acos((vTerrainNormal).Dot(XMVectorSet(0.f, 1.f, 0.f, 0.f))) < 0.01f)
            return true;
    }
    // 레이 쐈는데 터레인이 없었다.
    return false;
}

_int CKirbyBomb::Ready_Dead(_float fDeadScale)
{
    CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

    MultiFXDesc.vInitPos = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
    MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(CGameInstance::Get_Instance()->Get_CamLook());
    MultiFXDesc.vInitScale = { 3.f, 3.f, 3.f };
    if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_ExplodeSmoke"), &MultiFXDesc)))
        return OBJ_DEAD;

    //MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(CGameInstance::Get_Instance()->Get_CamLook());
    _float fScale = CUtils::Make_RandomFloat(0.95f, 2.5f);
    MultiFXDesc.vInitScale = { fScale, fScale, fScale };
    MultiFXDesc.fStartDelay = .05f;

    MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(static_cast<_float3>(CUtils::Make_RandomAngle_Vector(60.f, _float4{ 0.f, 1.f, 0.f, 0.f })));
    if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BombParticle_v1"), &MultiFXDesc)))
        return OBJ_DEAD;


    fScale = CUtils::Make_RandomFloat(0.95f, 2.5f);
    MultiFXDesc.vInitScale = { fScale, fScale, fScale };
    MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(static_cast<_float3>(CUtils::Make_RandomAngle_Vector(60.f, _float4{ 0.f, 1.f, 0.f, 0.f })));
    if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BombParticle_v1"), &MultiFXDesc)))
        return OBJ_DEAD;

    fScale = CUtils::Make_RandomFloat(0.95f, 2.5f);
    MultiFXDesc.vInitScale = { fScale, fScale, fScale };

    MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(static_cast<_float3>(CUtils::Make_RandomAngle_Vector(60.f, _float4{ 0.f, 1.f, 0.f, 0.f })));
    if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BombParticle_v1"), &MultiFXDesc)))
        return OBJ_DEAD;

    return OBJ_DEAD;
}

CKirbyBomb* CKirbyBomb::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirbyBomb* pInstance = new CKirbyBomb(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CKirby"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirbyBomb::Clone(void* pArg)
{
    CKirbyBomb* pInstance = new CKirbyBomb(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CKirbyBomb"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirbyBomb::Free()
{
    __super::Free();

    Safe_Release(m_pModelCom);
    Safe_Release(m_pRigidBodyCom);
    Safe_Release(m_pKirby);
}
