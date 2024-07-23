#include "stdafx.h"
#include "FinaleKirby.h"

#include "FSM.h"
#include "Camera_Free.h"
#include "MultiEffect.h"
#include "Particle.h"

#include "Utils.h"
#include "Camera_Main.h"
#include "EventCenter.h"

#include "LevelChanger.h"

#include "KirbyDump_State.h"
#include "Hitbox.h"
#include "Bone.h"

#include "FinalePartical_Maker.h"

#include "Light.h"


CFinaleKirby::CFinaleKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCharacter{ pDevice, pContext }
{
}

CFinaleKirby::CFinaleKirby(const CFinaleKirby& rhs)
    : CCharacter{ rhs }
{
}

HRESULT CFinaleKirby::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFinaleKirby::Initialize(void* pArg)
{
    GAMEOBJECT_DESC		GameObjectDesc{};
    if (nullptr != pArg)
        GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

    GameObjectDesc.fSpeedPerSec = 7.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    if (FAILED(Kirby_SystemInitialize()))
        return E_FAIL;

    Kirby_StateInitialize();

    m_pControllerCom->RegisterAsPlayer();

    m_bSlope = false;

    // 마지막 스테이지에서 운석을 지속적으로 날려주는 기능을 가진 클래스를 생성한다.
    if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Disaster_Master"), TEXT("Prototype_GameObject_Disaster_Master"), this)))
        return E_FAIL;

    ////BDBY
    //CParticle::PARTICLE_DESC FXDesc{};
    //FXDesc.pSocketMatrix = &m_EffectSocket;
    //FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };

    //if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_bdby3"), &FXDesc)))
    //    return E_FAIL;

    m_fOffsetTurn = 2.f;
    m_bUpdate_FXSocketMatrix = false;

    return S_OK;
}

_int CFinaleKirby::Tick(_float fTimeDelta)
{
    if (m_bDead == true)
        return OBJ_DEAD;

    m_fTimeDelta = m_pGameInstance->Get_FirstTimer();

    HitStop_System(m_fTimeDelta);

    if (INFO(m_pLight1) == nullptr && INFO(m_pLight2) == nullptr && INFO(m_eBodyState) == BODY_DUMPDEFAULT)
    {
        Update_EffectSocket();
        _float4x4 KirbyEffectSocketMatrix = *Get_EffectSocket();
        _float4 vLook = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_LOOK);
        _float4 vUp = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_UP);
        _float4 vRight = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_RIGHT);
        _float4 vPos = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_POSITION);

        LIGHT_DESC			LightDesc{};
        LightDesc.eType = LIGHT_DESC::TYPE_HORONG;
        LightDesc.vPosition = (vPos - (vUp * 3.f) - (vLook * 11.f) + (vRight * 2.f));
        LightDesc.fRange = 7.f;
        LightDesc.vDiffuse = _float4(1.f, 0.6f, 0.2f, 1.f);
        LightDesc.vAmbient = _float4(.5f, .5f, .5f, 1.f);
        LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
        if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
            return OBJ_NOEVENT;
        INFO(m_pLight1) = CGameInstance::Get_Instance()->Get_LightLastAddress();
        Safe_AddRef(INFO(m_pLight1));

        LightDesc.vPosition = (vPos - (vUp * 3.f) - (vLook * 11.f) - (vRight * 2.f));
        if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
            return OBJ_NOEVENT;
        INFO(m_pLight2) = CGameInstance::Get_Instance()->Get_LightLastAddress();
        Safe_AddRef(INFO(m_pLight2));
    }

    if (INFO(m_pLight1) != nullptr)
    {
        _float4x4 KirbyEffectSocketMatrix = *Get_EffectSocket();
        _float4 vLook = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_LOOK);
        _float4 vUp = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_UP);
        _float4 vRight = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_RIGHT);
        _float4 vPos = CUtils::Get_State_Vector_Matrix(KirbyEffectSocketMatrix, CUtils::STATE_POSITION);

        INFO(m_pLight1)->Update_LightPos((vPos - (vUp * 3.f) - (vLook * 11.f) + (vRight * 2.f)));
        INFO(m_pLight2)->Update_LightPos((vPos - (vUp * 3.f) - (vLook * 11.f) - (vRight * 2.f)));
    }

    if (INFO(m_eBodyState) != BODY_DUMPCUT)
    {
        // 커비의 기본적인 축 보정, 밸런스 보정을 담당한다.
        Setting_KirbyBalance();

        // 특정 상황에서 뼈를 돌려준다.
        Bone_Rotation(m_fTimeDelta);

        Update_EffectSocket();
        // 유틸업데이트가 들어가있다. (FSM)
        __super::Tick(m_fTimeDelta);

        // 지면의 up벡터
        PxVec3 slope = m_pControllerCom->Compute_Slope_DynamicActor(m_pTransformCom);
        _vector vTerrainNormal = CUtils::To_Vector(slope);
        Lerp_UpVector(vTerrainNormal, 20.f, fTimeDelta);

        //카메라에 전달해줄 z 앵글을 구하고, 세팅한다.
        _float3 vUp = m_pTransformCom->Get_State(CTransform::STATE_UP);

        _float fZRotAngle = ToDegree(atan2(vUp.x, vUp.y)) /** .2f*/;
        static_cast<CCamera_Main*>(m_pCamera)->Set_ZAngle(fZRotAngle, .1f);

        Kirby_SystemTick(m_fTimeDelta);

        //확인을 위해 계속 갱신하겠슴_HS
        m_vBonePos = Compute_RootPos();
    }
    else if (INFO(m_eBodyState) == BODY_DUMPCUT)
    {
        //확인을 위해 계속 갱신하겠슴_HS
        m_vBonePos = Compute_RootPos();

        Update_EffectSocket();

        // 유틸업데이트가 들어가있다. (FSM)
        //__super::Tick(m_fTimeDelta);
        if (m_pFSM != nullptr)
            m_pFSM->Update(this, fTimeDelta);

        //캡슐의 위치만 업데이트한다.
        m_pControllerCom->Set_CapsulePosition(m_vBonePos);
    }




    return OBJ_NOEVENT;
}

void CFinaleKirby::Late_Tick(_float fTimeDelta)
{
    m_pModelCom[INFO(m_eBodyState)]->Play_Animation(m_fTimeDelta);

    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);

    if (INFO(m_bRimOnOff) == true)
    {
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
        m_iRenderCount = 1;
    }

}

HRESULT CFinaleKirby::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[INFO(m_eBodyState)]->Get_NumMeshes();

    if (INFO(m_bRimOnOff) == false)
    {
        for (size_t i = 0; i < iNumMeshes; i++)
        {
            if (Kirby_FaceCustom(INFO(m_eBodyState), i) == true)
                continue;

            if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
                return E_FAIL;

            if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
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
            if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
                return E_FAIL;
            if (FAILED(m_pShaderCom->Bind_RawValue("g_fOverPowerColor", &m_fOverPowerColor, sizeof(_float))))
                return E_FAIL;

            _bool   bBulb = false;
            if (FAILED(m_pShaderCom->Bind_RawValue("g_isBulb", &bBulb, sizeof(_bool))))
                return E_FAIL;


            /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

            if (INFO(m_eBodyState) == BODY_DUMPDEFAULT || Get_State() == DUMPTSTATE_CUT)
            {
                if (FAILED(m_pShaderCom->Begin(/*ANIMMODEL_KIRBY*//*14*/ANIMMODEL_FINALEKIRBY_WEAPONS)))
                    return E_FAIL;
            }
            else
            {

                if (FAILED(m_pShaderCom->Begin(ANIMMODEL_KIRBY)))
                    return E_FAIL;
            }

            m_pModelCom[INFO(m_eBodyState)]->Render(i);
        }
    }
    else if (INFO(m_bRimOnOff) == true)
    {
        if (m_iRenderCount == 1)
        {
            for (size_t i = 0; i < iNumMeshes; i++)
            {
                if (Kirby_FaceCustom(INFO(m_eBodyState), i) == true)
                    continue;

                if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
                    return E_FAIL;

                if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
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
                if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
                    return E_FAIL;
                if (FAILED(m_pShaderCom->Bind_RawValue("g_fOverPowerColor", &m_fOverPowerColor, sizeof(_float))))
                    return E_FAIL;

                _bool   bBulb = false;
                if (FAILED(m_pShaderCom->Bind_RawValue("g_isBulb", &bBulb, sizeof(_bool))))
                    return E_FAIL;


                /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

                if (INFO(m_eBodyState) == BODY_DUMPDEFAULT || Get_State() == DUMPTSTATE_CUT)
                {
                    if (FAILED(m_pShaderCom->Begin(/*ANIMMODEL_KIRBY*//*14*/ANIMMODEL_FINALEKIRBY_WEAPONS)))
                        return E_FAIL;
                }
                else
                {

                    if (FAILED(m_pShaderCom->Begin(ANIMMODEL_KIRBY)))
                        return E_FAIL;
                }

                m_pModelCom[INFO(m_eBodyState)]->Render(i);
            }

            m_iRenderCount = 0;
        }
        else if (m_iRenderCount == 0)
        {
            for (size_t i = 0; i < iNumMeshes; i++)
            {
                if (i == 7)
                    continue;

                if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
                    return E_FAIL;
                if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
                    return E_FAIL;
                if (FAILED(m_pShaderCom->Bind_RawValue("g_vDeformRimColor", &m_vDeformRimColor, sizeof(_float3))))
                    return E_FAIL;
                if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
                    return E_FAIL;
                if (FAILED(m_pMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture")))
                    return E_FAIL;
                if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveRatio", &m_fDissolveRatio, sizeof(_float))))
                    return E_FAIL;
                _float2 vTEXUV = { m_fUVOffsetTime , m_fUVOffsetTime };

                if (FAILED(m_pShaderCom->Bind_RawValue("g_vUVOffset", &vTEXUV, sizeof(_float2))))
                    return E_FAIL;

                /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
                if (FAILED(m_pShaderCom->Begin(ANIMMODEL_DEFORMRIM)))
                    return E_FAIL;

                m_pModelCom[INFO(m_eBodyState)]->Render(i);
            }
        }
    }

    return S_OK;
}

HRESULT CFinaleKirby::Render_LightDepth()
{
    if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom[INFO(m_eBodyState)])))
        return E_FAIL;

    return S_OK;
}


#ifdef _DEBUG
void CFinaleKirby::Render_IMGUI()
{

    _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
    _float4x4 vWorld = m_pTransformCom->Get_WorldFloat4x4();

    ImGui::Text("m_vPos.x : %.2f, m_vPos.y : %.2f m_vPos.z : %.2f", vPos.x, vPos.y, vPos.z);
    ImGui::Text("_11 : %.2f, _12 : %.2f, _13 : %.2f", vWorld._11, vWorld._12, vWorld._13);
    ImGui::Text("_21 : %.2f, _22 : %.2f, _23 : %.2f", vWorld._21, vWorld._22, vWorld._23);
    ImGui::Text("_31 : %.2f, _32 : %.2f, _33 : %.2f", vWorld._31, vWorld._32, vWorld._33);
    ImGui::Text("m_vRPos.x : %.2f, m_vRPos.y : %.2f m_vRPos.z : %.2f", m_vBonePos.x, m_vBonePos.y, m_vBonePos.z);

    //GetWindowDrawList()->AddCircleFilled(vCurPos, 6.0f, IM_COL32(255, 255, 100, 255));

    //ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImGui::GetForegroundDrawList()->AddCircleFilled(
        CUtils::WorldPosTo_ImguiProjPos((_float3)m_vBonePos), 10.f, IM_COL32(255, 255, 100, 255));
    __super::Render_IMGUI();

}
#endif

HRESULT CFinaleKirby::Render_DeferredInfo()
{
    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[INFO(m_eBodyState)]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {

        if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;

        if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(ANIMMODEL_DEFERREDINFO)))
            return E_FAIL;

        if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CFinaleKirby::Add_AnimEvent()
{

}

void CFinaleKirby::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
    if (eContent == CCollisionCenter::CONTENT_BODY)
    {



    }
}

void CFinaleKirby::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody, _uint iOffSet)
{
    INFO(m_eBodyState) = eBody;

    m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation, INFO(m_eBodyState), iOffSet);
}

_bool CFinaleKirby::isAnimFinish()
{
    if (m_pModelCom[INFO(m_eBodyState)] == nullptr)
        return false;

    return m_pModelCom[INFO(m_eBodyState)]->IsFinished();
}

_float CFinaleKirby::Get_AnimTrackPosition()
{
    return m_pModelCom[m_tKirbyInfo.m_eBodyState]->Get_AnimTrackPosition();
}

void CFinaleKirby::Bone_Rotation(_float fTimeDelta)
{
    // 자동차일때,
    if (INFO(m_eBodyState) == BODY_DUMPDEFAULT && Get_State() != DUMPTSTATE_CUT)
    {
        _float fTurnAngle = -INFO(m_fMoveSpeed) * 100.f;

        CBone* pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("L_WheelAJ");
        _float4x4* BoneMatrix = pBone->Get_EditMatrixPtr();
        CUtils::Turn_OtherMatrix(*BoneMatrix, _float4(1.f, 0.f, 0.f, 0.f), fTimeDelta, fTurnAngle);

        pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("R_WheelAJ");
        BoneMatrix = pBone->Get_EditMatrixPtr();
        CUtils::Turn_OtherMatrix(*BoneMatrix, _float4(1.f, 0.f, 0.f, 0.f), fTimeDelta, fTurnAngle);




        pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("C_FrontBodyJ");
        BoneMatrix = pBone->Get_EditMatrixPtr();
        _float fHandleAngle = ToDegree(acos(_float4(1.f, 0.f, 0.f, 0.f).Dot(INFO(m_vHandleDir))));

        // 좌회전을 한다.
        if (XMVector3Cross(_float4(1.f, 0.f, 0.f, 0.f), INFO(m_vHandleDir)).m128_f32[1] < 0.f)
        {
            fHandleAngle *= -1.f;
        }

        _float4x4 RotationMatrix = _float4x4::Identity;
        CUtils::Turn_OtherMatrix(RotationMatrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, fHandleAngle);
        *BoneMatrix = RotationMatrix;
    }
}

void CFinaleKirby::Update_EffectSocket()
{
    if (INFO(m_eBodyState) == BODY_DUMPCUT || INFO(m_eBodyState) == BODY_DUMPDEFAULT)
    {
        _float4x4 pFrontBoneWorldMatrix = m_pTransformCom->ComputeBoneWorldMatrix(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("C_ContainerBJ"));
        _float4 vFrontBonePos = CUtils::Get_State_Vector_Matrix(pFrontBoneWorldMatrix, CUtils::STATE_POSITION);

        _float4x4 pBackBoneWorldMatrix = m_pTransformCom->ComputeBoneWorldMatrix(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("C_WheelDJ"));
        _float4 vBackBonePos = CUtils::Get_State_Vector_Matrix(pBackBoneWorldMatrix, CUtils::STATE_POSITION);

        _float4 vRootLook = vFrontBonePos - vBackBonePos;
        vRootLook.Normalize();
        _float4 vRootRight = XMVector3Cross(_float4(0.f, 1.f, 0.f, 0.f), vRootLook);
        vRootRight.Normalize();
        _float4 vRootUp = XMVector3Cross(vRootLook, vRootRight);
        vRootUp.Normalize();

        //이펙트 소켓 업데이트
        m_EffectSocket = {
            vRootRight.x, vRootRight.y, vRootRight.z, vRootRight.w,
            vRootUp.x, vRootUp.y, vRootUp.z, vRootUp.w,
            vRootLook.x, vRootLook.y, vRootLook.z, vRootLook.w,
            vFrontBonePos.x, vFrontBonePos.y, vFrontBonePos.z, vFrontBonePos.w
        };

        if (vRootRight != _float4(0.f, 0.f, 0.f, 0.f))
            CUtils::Turn_OtherMatrix(m_EffectSocket, vRootRight, 1.f, 15.f);
    }
}

void CFinaleKirby::Setting_KirbyBalance()
{
    // 커비는 항상 m_vMoveDir)를 바라본다.
    _float4 vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
    m_pTransformCom->Look_At_ForLandObject(vPos + INFO(m_vMoveDir));

    // 보정
    _vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
    _vector vEditRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
    vEditRight = XMVector3Normalize(vEditRight);
    _vector vEditLook = XMVector3Cross(vEditRight, XMVectorSet(0.f, 1.f, 0.f, 0.f));
    vEditLook = XMVector3Normalize(vEditLook);
    m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vEditLook));
    m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vEditRight));
    m_pTransformCom->Set_State(CTransform::STATE_UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));

    // 카메라 기준 실시간 방향 탐색
    CTransform* pCameraTransform = m_pCamera->Get_TransformCom();
    _float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
    _float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
    _float fCX = vCamLook.x;
    _float fCZ = vCamLook.z;
    _float fKX = INFO(m_vMoveDir).x;
    _float fKZ = INFO(m_vMoveDir).z;
    _float fAngle = (atan2f(fCX, fCZ) * 180.0f / XM_PI) - (atan2f(fKX, fKZ) * 180.0f / XM_PI);
    if (fAngle < 0.f) fAngle += 360.0f;
    if (fAngle >= 337.5f || fAngle < 22.5f)		  INFO(m_eKirbyDir) = DIR_FRONT;
    else if (fAngle >= 22.5f && fAngle < 67.5f)   INFO(m_eKirbyDir) = DIR_LF;
    else if (fAngle >= 67.5f && fAngle < 112.5f)  INFO(m_eKirbyDir) = DIR_LEFT;
    else if (fAngle >= 112.5f && fAngle < 157.5f) INFO(m_eKirbyDir) = DIR_LB;
    else if (fAngle >= 157.5f && fAngle < 202.5f) INFO(m_eKirbyDir) = DIR_BACK;
    else if (fAngle >= 202.5f && fAngle < 247.5f) INFO(m_eKirbyDir) = DIR_RB;
    else if (fAngle >= 247.5f && fAngle < 292.5f) INFO(m_eKirbyDir) = DIR_RIGHT;
    else if (fAngle >= 292.5 && fAngle < 337.5f)  INFO(m_eKirbyDir) = DIR_RF;

}

void CFinaleKirby::Kirby_SystemTick(_float fTimeDelta)
{
    // Dof 초점을 커비에게 맞춘다.
    _vector vDOFPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
    vDOFPos.m128_f32[1] += 0.5f;
    m_pGameInstance->Update_DofFocus(vDOFPos);

    // 무적 상태 관리소
    OverPower();

    if (INFO(m_eBodyState) == BODY_DUMPDEFAULT)
    {
        if (Get_State() == DUMPTSTATE_CUT)
        {
            m_bMotionBlur = false;
        }
        else
        {
            m_bMotionBlur = true;
        }
    }


}

HRESULT CFinaleKirby::Kirby_SystemInitialize()
{
    // 타겟 카메라를 만들어준다.
    if (FAILED(Make_TargetToCams()))
        return E_FAIL;

    // 완전히 기본상태로 먼저 세팅한다.
    INFO(m_eBodyState) = BODY_DEFAULT;
    INFO(m_eMouthState) = MOUTH_IDLE;
    INFO(m_eEyeState) = EYE_IDLE;

    // 커비가 레벨별로 시작할 때, 바라보는 방향을 정해준다.
    Kirby_LookInitialize();

    // 파싱으로 레벨전환될때 HP와 COIN개수를 이동시킵니다.
    CLevelChanger::LEVEL_DATA tLevelData = CLevelChanger::Get_Instance()->Load();
    m_fHp = tLevelData.fKirbyHP;
    m_uCoin = (_uint)tLevelData.fKirbyCoin;
    // m_eAbilityType = ;
    m_fAttack = 5.f; // 고정

    // 임시
    m_fHp = 100.f; // 기존 사용하던 HP입니다.
    m_fMaxHp = 100.f;
    m_eAbilityType = ABILITY_DEFAULT;

    // 혹여나, 버그가 발생할까봐 확실하게 블러 true화
    m_bMotionBlur = true;
    m_bRimLight = true;
    m_bStencil = true;

    return S_OK;
}

void CFinaleKirby::Kirby_LookInitialize()
{
    _uint uLevel = *m_pCurrentLevelID;
    _float4 fCameraLook = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_LOOK);
    _float4 fCameraRight = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_RIGHT);

    fCameraLook.y = 0.f;
    fCameraLook = XMVector4Normalize(fCameraLook);
    fCameraRight = XMVector4Normalize(fCameraRight);

    // 카메라를 정면으로 바라봄
    INFO(m_vTargetDir) = INFO(m_vMoveDir) = -1.f * fCameraLook;
}

void CFinaleKirby::Kirby_StateInitialize()
{
    Change_State(STATE_IDLE, 60.f, true, true, BODY_DEFAULT);
    m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, true);
}

void CFinaleKirby::Start_CutScene()
{
    m_bMotionBlur = false;
    m_bSlope = false;


    // 카메라의 타겟을 바꿀까요?
    CBone* pBone = m_pModelCom[BODY_DUMPCUT]->Get_BonePtr("TopL");
    _float4x4 pBoneLocalMatrix = *pBone->Get_CombinedTransformationMatrix();
    _float4x4 pBoneWorldMatrix = pBoneLocalMatrix * m_pTransformCom->Get_WorldFloat4x4();
    m_vBonePos = CUtils::Get_State_Vector_Matrix(pBoneWorldMatrix, CUtils::STATE_POSITION);
}

HRESULT CFinaleKirby::Make_TargetToCams()
{
    // 첫 카메라 기준으로 움직이기에 미리 받아둔다.
    if (m_pCamera == nullptr)
    {
        //인트로, 게임플레이 스테이지라면 카메라로 main camera를 저장한다.
        (LEVEL_INTRO <= *m_pCurrentLevelID && *m_pCurrentLevelID < LEVEL_END) ?
            m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main"))) :
            m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free"))); //나머지 레벨이라면 다른 카메라를 저장한다.

        if (m_pCamera == nullptr)
        {
            ALARM_FAIL(TEXT("망했어 카메라 없다"));
            return E_FAIL;
        }
        Safe_AddRef(m_pCamera);
    }

    m_pCamera->Set_Target(m_pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FINALE, /*{ 0.f, 2.f, 3.f }*/{0.f, 2.f, 0.f}, 5.f);
    static_cast<CCamera_Main*>(m_pCamera)->Make_Sequence(CCamera_Main::SEQ_FINALESTART);

    //게임 레벨에 free camera 있다면 그놈에게도 타겟 등록해 준다.
    if (LEVEL_INTRO <= *m_pCurrentLevelID && *m_pCurrentLevelID < LEVEL_END)
    {
        CCamera* pCameraFree = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));
        if (pCameraFree != nullptr)
            pCameraFree->Set_Target(m_pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FIRST);
    }

    return S_OK;
}

HRESULT CFinaleKirby::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_FireDissolve"),
        TEXT("Com_Texture_Mask"), (CComponent**)&m_pMaskTextureCom);
    CHECK_FAILED(hr);

#pragma region Kirby Model
    // 커비의 기본 상태 모델
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyDefault"),
        TEXT("Com_Model_Default"), (CComponent**)&m_pModelCom[BODY_DEFAULT]);
    CHECK_FAILED(hr);

    // 커비의 빨아들이는 상태 모델
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyVacuum"),
        TEXT("Com_Model_Vacuum"), (CComponent**)&m_pModelCom[BODY_VACUUM]);
    CHECK_FAILED(hr);

    // 커비의 Dump Default 상태 모델
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyDumpDefault"),
        TEXT("Com_Model_DumpDefault"), (CComponent**)&m_pModelCom[BODY_DUMPDEFAULT]);
    CHECK_FAILED(hr);

    // 커비의 Dump Vacuum 상태 모델
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyDumpVacuum"),
        TEXT("Com_Model_DumpVacuum"), (CComponent**)&m_pModelCom[BODY_DUMPVACUUM]);
    CHECK_FAILED(hr);

    // 커비의 Dump Cut 상태 모델
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyDumpCut"),
        TEXT("Com_Model_DumpCut"), (CComponent**)&m_pModelCom[BODY_DUMPCUT]);
    CHECK_FAILED(hr);


#pragma endregion

#pragma region Kirby Eye
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_idle"),
        TEXT("Com_Texture_Eye_Idle"), (CComponent**)&m_pEyeTexture[EYE_IDLE]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_doubt"),
        TEXT("Com_Texture_Eye_Doubt"), (CComponent**)&m_pEyeTexture[EYE_SADNESS]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_close"),
        TEXT("Com_Texture_Eye_Close"), (CComponent**)&m_pEyeTexture[EYE_CLOSE]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_blink"),
        TEXT("Com_Texture_Eye_Blink"), (CComponent**)&m_pEyeTexture[EYE_BLINK]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_anger"),
        TEXT("Com_Texture_Eye_Anger"), (CComponent**)&m_pEyeTexture[EYE_ANGER]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_pupil"),
        TEXT("Com_Texture_Eye_Pupil"), (CComponent**)&m_pEyeTexture[EYE_PUPIL]);
    CHECK_FAILED(hr);
#pragma endregion

#pragma region Kirby Mouth
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_base"),
        TEXT("Com_Texture_Mouth_Idle"), (CComponent**)&m_pMouthTexture[MOUTH_IDLE]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_anger"),
        TEXT("Com_Texture_Mouth_Anger"), (CComponent**)&m_pMouthTexture[MOUTH_ANGER]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_happy"),
        TEXT("Com_Texture_Mouth_Happy"), (CComponent**)&m_pMouthTexture[MOUTH_HAPPY]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_smile"),
        TEXT("Com_Texture_Mouth_Smile"), (CComponent**)&m_pMouthTexture[MOUTH_SMILE]);
    CHECK_FAILED(hr);
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_surprise"),
        TEXT("Com_Texture_Mouth_Surprise"), (CComponent**)&m_pMouthTexture[MOUTH_SURPRISE]);
    CHECK_FAILED(hr);
#pragma endregion

    /* For.Com_CharacterController */
    _float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
    CCharacterController::CONTROLLER_DESC desc{};
    desc.vInitialPos = vPos;
    desc.fOffset = 0.5f;
    desc.tCapsuleShape.fHeight = 0.4f;// 1.f;
    desc.tCapsuleShape.fRadius = 0.4f;// 0.5f;
    hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
        TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
    CHECK_FAILED(hr);

    // FOR ANIMTOOL
    m_ppModelForAnimTool = &m_pModelCom[BODY_DEFAULT];
    m_uModelCnt = BODY_END;

    CHitBox::HITBOX_DESC HitBox{};
    HitBox.pOwner = this;
    HitBox.pDesc = &m_tColliderDesc[BODY];
    HitBox.pCollisionType = FINALE_PLAYER;
    if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
        return E_FAIL;
    Set_BodyCollider(COLLIDER_SPHERE, 1.f, 0.f, 2.f);

    /* FSM */
    SetUp_FSM();

    /* 구독 시스템 */
    SetUp_Event();

    return S_OK;
}

_float4 CFinaleKirby::Compute_RootPos()
{
    CBone* pBone = m_pModelCom[BODY_DUMPCUT]->Get_BonePtr("TopL");
    _float4x4 pBoneLocalMatrix = *pBone->Get_CombinedTransformationMatrix();
    _float4x4 pBoneWorldMatrix = pBoneLocalMatrix * m_pTransformCom->Get_WorldFloat4x4();
    return CUtils::Get_State_Vector_Matrix(pBoneWorldMatrix, CUtils::STATE_POSITION);
}

HRESULT CFinaleKirby::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    return S_OK;
}

_bool CFinaleKirby::Kirby_FaceCustom(BODYSTATE _eBodyState, _uint _iMeshIndex)
{
    // Default 상태의 입 부위 // Balloon 상태의 입 부위
    if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 0))
    {
        m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
        m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
        m_pMouthTexture[INFO(m_eMouthState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyMouthTexture", 0);

        m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
        m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));


        m_pShaderCom->Begin(ANIMMODEL_KIRBYMOUTH);
        m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
        return true;
    }
    // Default 상태의 눈 부위 // Vacuum 상태의 눈 부위 // Balloon 상태의 눈 부위
    else if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 3) ||
        (_eBodyState == BODY_VACUUM && _iMeshIndex == 2 ||
            /*_eBodyState == BODY_DUMPVACUUM && _iMeshIndex == 6 ||*/
            _eBodyState == BODY_DUMPDEFAULT && _iMeshIndex == 7 ||
            _eBodyState == BODY_DUMPCUT && _iMeshIndex == 7)
        )
    {
        m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
        m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
        m_pEyeTexture[INFO(m_eEyeState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", 0);

        m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
        m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

        m_pShaderCom->Begin(ANIMMODEL_KIRBYEYE);
        m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
        return true;
    }
    // Vacuum 상태의 구강 부위
    else if (_eBodyState == BODY_VACUUM && _iMeshIndex == 3)
    {
        m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
        m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);

        _bool bRimLight = false;
        m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
        m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

        m_pShaderCom->Begin(ANIMMODEL_NORMAL_X);
        m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
        return true;
    }

    return false;
}

_float CFinaleKirby::Get_AnimTrackPos()
{
    return m_pModelCom[INFO(m_eBodyState)]->Get_AnimTrackPosition();
}

void CFinaleKirby::SetUp_FSM()
{
    m_pFSM = CFSM::Create();

    m_pFSM->Add_State(STATE_IDLE, CKirbyDump_Cut_State::Create());
    m_pFSM->Add_State(STATE_RUN, CKirbyDump_Cut_State::Create());

    m_pFSM->Add_State(STATE_INHALESTART, CKirbyDump_Cut_State::Create());
    m_pFSM->Add_State(STATE_INHALE, CKirbyDump_Cut_State::Create());
    m_pFSM->Add_State(STATE_SUPERINHALESTART, CKirbyDump_Cut_State::Create());
    m_pFSM->Add_State(STATE_VACUUM, CKirbyDump_Cut_State::Create());
    m_pFSM->Add_State(DUMPTSTATE_CUT, CKirbyDump_Cut_State::Create());
    m_pFSM->Add_State(DUMPSTATE_CUTDEMOKIRBY, CKirbyDump_Cut_State::Create());

    m_pFSM->Add_State(DUMPSTATE_IDLING, CKirbyDump_Run_State::Create());
    m_pFSM->Add_State(DUMPSTATE_JUMP, CKirbyDump_Jump_State::Create());
    m_pFSM->Add_State(DUMPSTATE_LANDING, CKirbyDump_Jump_State::Create());

    m_pFSM->Add_State(DUMPCUTSTATE_CUT1, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT2, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT6, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT7, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT8, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT9, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT10, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT11, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT12, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT13, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT14, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT15, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT16, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT17, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT18, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT19, CKirbyDump_Cut2_State::Create());
    m_pFSM->Add_State(DUMPCUTSTATE_CUT20, CKirbyDump_Cut2_State::Create());

    CFSM::FSM_INFO		FSM_Info_Desc = {};
    FSM_Info_Desc.iState = STATE_IDLE;
    FSM_Info_Desc.uNumModel = BODY_END;
    FSM_Info_Desc.pModel = &m_pModelCom[BODY_DEFAULT];
    m_pFSM->Initialize(&FSM_Info_Desc);
}

void CFinaleKirby::SetUp_Event()
{
}

void CFinaleKirby::Compute_RimLight(_float fTimeDelta)
{
    m_fDeformTime += fTimeDelta * 3.f;
    m_fUVOffsetTime += fTimeDelta * 0.5f;

    m_fDissolveRatio = (sin(m_fUVOffsetTime * 5.f) + 1.f) * 0.25f;

    if (m_iComputeMode == 0)
    {
        if (m_fDeformTime > 1.f)
        {
            m_vDeformRimColor = { 1.f, 1.f, 0.f };
            m_iComputeMode = 1;
            m_fDeformTime = 0.f;
        }
        else
            m_vDeformRimColor = { 1.f, m_fDeformTime, 0.f };
    }
    else if (m_iComputeMode == 1)
    {
        if (m_fDeformTime > 1.f)
        {
            m_vDeformRimColor = { 0.f, 1.f, 0.f };
            m_iComputeMode = 2;
            m_fDeformTime = 0.f;
        }
        else
            m_vDeformRimColor = { 1.f - m_fDeformTime, 1.f, 0.f };
    }
    else if (m_iComputeMode == 2)
    {
        if (m_fDeformTime > 1.f)
        {
            m_vDeformRimColor = { 0.f, 1.f, 1.f };
            m_iComputeMode = 3;
            m_fDeformTime = 0.f;
        }
        else
            m_vDeformRimColor = { 0.f, 1.f, m_fDeformTime };
    }
    else if (m_iComputeMode == 3)
    {
        if (m_fDeformTime > 1.f)
        {
            m_vDeformRimColor = { 0.f, 0.f, 1.f };
            m_iComputeMode = 4;
            m_fDeformTime = 0.f;
        }
        else
            m_vDeformRimColor = { 0.f, 1.f - m_fDeformTime, m_fDeformTime };
    }
    else if (m_iComputeMode == 4)
    {
        if (m_fDeformTime > 1.f)
        {
            m_vDeformRimColor = { 1.f, 0.f, 1.f };
            m_iComputeMode = 5;
            m_fDeformTime = 0.f;
        }
        else
            m_vDeformRimColor = { m_fDeformTime, 0.f, 1.f };
    }
    else if (m_iComputeMode == 5)
    {
        if (m_fDeformTime > 1.f)
        {
            m_vDeformRimColor = { 1.f, 0.f, 0.f };
            m_iComputeMode = 0;
            m_fDeformTime = 0.f;
        }
        else
            m_vDeformRimColor = { 1.f, 0.f, m_fDeformTime };
    }

    
}

_float4 CFinaleKirby::Get_BoneWorldPos(const _char* pBoneName)
{
    //CBone* pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr(pBoneName);
    _float4x4 BoneWorldMatrix = m_pTransformCom->ComputeBoneWorldMatrix(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr(pBoneName));
    _float4 vBoneWorldPos = CUtils::Get_State_Vector_Matrix(BoneWorldMatrix, CUtils::STATE_POSITION);

    return vBoneWorldPos;
}

void CFinaleKirby::OverPower()
{
    if (m_fPreHp > m_fHp)
    {
        m_bOverPower = true;
    }

    if (m_bOverPower == true)
    {
        m_fOverPowerTime += m_fTimeDelta;
        m_fFlashOverPowerTime += m_fTimeDelta;
        _float fFlashTime = 0.02f;

        if (m_fFlashOverPowerTime > fFlashTime)
        {
            m_fOverPowerColor = m_fOverPowerColor == 0.f ? 0.25f : 0.f;
            m_fFlashOverPowerTime -= fFlashTime;
        }

        if (m_fOverPowerTime > 3.f)
        {
            m_bOverPower = false;
            m_fOverPowerTime = 0.f;
            m_fOverPowerColor = 0.f;
            m_fFlashOverPowerTime = 0.f;
        }
    }

    m_fPreHp = m_fHp;
}

void CFinaleKirby::HitStop_System(_float fTimeDelta)
{
    if (m_bHitStop == true)
    {
        m_fTimeDelta = 0.f;
        m_fHitStopTime += fTimeDelta;

        if (m_fHitStopTime > m_fHitStopMaxTime)
        {
            m_fHitStopTime = 0.f;
            m_bHitStop = false;
        }
    }

}

CFinaleKirby* CFinaleKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CFinaleKirby* pInstance = new CFinaleKirby(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CFinaleKirby"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CFinaleKirby::Clone(void* pArg)
{
    CFinaleKirby* pInstance = new CFinaleKirby(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CFinaleKirby"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CFinaleKirby::Free()
{
    __super::Free();

    Delete_AllEffect();

    for (auto& pModelCom : m_pModelCom)
        Safe_Release(pModelCom);
    for (auto& pEyeTexture : m_pEyeTexture)
        Safe_Release(pEyeTexture);
    for (auto& pMouthTexture : m_pMouthTexture)
        Safe_Release(pMouthTexture);

    Safe_Release(m_pCamera);
    Safe_Release(m_pMaskTextureCom);

    if (INFO(m_pLight1) != nullptr)
        Safe_Release(INFO(m_pLight1));
    if (INFO(m_pLight2) != nullptr)
        Safe_Release(INFO(m_pLight2));

}
