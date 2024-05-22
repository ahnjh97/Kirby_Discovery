#include "stdafx.h"
#include "TestModel.h"
#include "Light.h"
//#include "Utils.h"

CTestModel::CTestModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CTestModel::CTestModel(const CTestModel& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CTestModel::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTestModel::Initialize(void* pArg)
{
    GAMEOBJECT_DESC		GameObjectDesc{};
    GameObjectDesc.fSpeedPerSec = 7.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    //CGameInstance::Get_Instance()->Test();
    // position 세팅은 항상 Add_Components() 앞에 둘것
    _vector vPos = XMVectorSet(0.f, 500.f, 0.f, 1.f);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
    if (FAILED(Add_Components()))
        return E_FAIL;

    m_fSpeed = 5.f;

    m_iTestAnim = 0;
    m_pModelCom->Set_Animation(m_iTestAnim, true);


    // 예시코드 2 : 따라다니게 하기 예시 코드 + 점 광원 예시 코드
    LIGHT_DESC			LightDesc{};
    LightDesc.eType = LIGHT_DESC::TYPE_POINT;
    LightDesc.vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
    LightDesc.fRange = 5.f;
    LightDesc.vDiffuse = _float4(1.f, 0.8f, 0.1f, 1.f);
    LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
    if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
        return E_FAIL;

    m_pLight = CGameInstance::Get_Instance()->Get_LightLastAddress();
    Safe_AddRef(m_pLight);

    return S_OK;
}

_int CTestModel::Tick(_float fTimeDelta)
{
    // 예시코드 3 : 사망 시, 죽이는 코드
    if (m_bDead == true)
        return OBJ_DEAD;


    // 예시코드 4 : 광원 따라다니게 하는 코드
    if (m_pLight != nullptr)
        m_pLight->Update_LightPos(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));

    // [JS] : CharacterController로 넘기고 Character에서 정리하기
    m_fFallVelocity -= 9.81f * fTimeDelta;
    if (true == m_isJump)
    {
        // [JS] : 위 준수, 아래 지영
        //m_isJump = m_pRigidBodyCom->Jump(m_pTransformCom, m_fFallVelocity, fTimeDelta);
        m_isJump = m_pControllerCom->Jump(m_pTransformCom, m_fFallVelocity, fTimeDelta);
    }
    else
    {
        // [JS] : 위 준수, 아래 지영
        //m_pRigidBodyCom->FreeFall(m_pTransformCom, fTimeDelta);
        m_pControllerCom->FreeFall(m_pTransformCom, fTimeDelta);
    }

    // 예시코드 5 : 계산기 예시 코드 (월드 매트리스로 예시든거임 이건 정신나간 코드이므로 참고해주셈)
    // 예시코드 6 : DInput + KeyPress 예시 코드 
    if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_PRESS))
    {
        _float fSpeed = m_pTransformCom->Get_SpeedPerSec();
        // for test
        fSpeed = 5.f;
        // [JS] : 위 준수, 아래 지영
        //m_pRigidBodyCom->Go_Straight(m_pTransformCom, fSpeed, fTimeDelta);
        m_pControllerCom->Move(m_pTransformCom, fSpeed, fTimeDelta);
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
    {
        m_pTransformCom->Turn(XMVectorSet(0.f, -1.f, 0.f, 0.f), fTimeDelta);
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
    {
        m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_SPACE, KEY_DOWN))
    {
        m_isJump = true;
        m_fFallVelocity = 5.f;
    }

    //if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_PRESS))
    //{
    //    _float4x4 Worldmatrix = m_pTransformCom->Get_WorldFloat4x4();
    //    _vector vLook = CUtils::Get_State_Vector_Matrix(Worldmatrix, CUtils::STATE_LOOK);
    //    _vector vPos  = CUtils::Get_State_Vector_Matrix(Worldmatrix, CUtils::STATE_POSITION);
    //    _float fSpeed = 3.f;

    //    vPos += vLook * fTimeDelta * fSpeed;

    //    CUtils::Set_State_Matrix(Worldmatrix, CUtils::STATE_POSITION, vPos);

    //    m_pTransformCom->Set_WorldMatrix(Worldmatrix);
    //}

    // 예시코드 7 : 랜덤 벡터 뽑기
    // 예시코드 8 : DInput + KeyDown 예시코드
    if (m_pGameInstance->Get_DIKeyState(DIK_E, KEY_DOWN))
    {
        // 0.5f 길이의 랜덤한 방향의 벡터 생성
        _vector vRandomVector = CUtils::Make_Random_Vector(0.5f);
        _vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

        m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vRandomVector);
    }


    // 예시코드 9 : Radial Blur Center
    if (m_pGameInstance->Get_DIKeyState(DIK_1, KEY_DOWN))
    {
        m_pGameInstance->Setting_RadialBlur(10.f, 10.f);
    }

    // 예시코드 10 : Radial Blur Player
    if (m_pGameInstance->Get_DIKeyState(DIK_2, KEY_DOWN))
    {
        _vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
        m_pGameInstance->Setting_RadialBlur(vPos, 5.f, 10.f);
    }



    if (m_pGameInstance->Get_DIKeyState(DIK_P, KEY_DOWN))
    {
        m_iTestAnim++;
        if (m_iTestAnim > 2)
            m_iTestAnim = 2;

        m_pModelCom->Set_Animation(m_iTestAnim, true);
    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_O, KEY_DOWN))
    {
        m_iTestAnim--;
        if (m_iTestAnim < 0)
            m_iTestAnim = 0;

        m_pModelCom->Set_Animation(m_iTestAnim, true);
    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_I, KEY_DOWN))
    {
        m_pModelCom->Set_Animation(m_iTestAnim, true, true);
    }

    return OBJ_NOEVENT;
}

void CTestModel::Late_Tick(_float fTimeDelta)
{
    m_pModelCom->Play_Animation(fTimeDelta);

    SetOn_Slope(fTimeDelta);

    if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
    {
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_L, KEY_DOWN))
    {
        _float4 vForce = m_pTransformCom->Get_State_Float4(CTransform::STATE_UP);
        _float3 force = _float3{ vForce.x * 10000.f, vForce.y * 10000.f, vForce.z * 10000.f };
        m_pRigidBodyCom->Add_Force(force);
    }

    //// =============== RigidBody를 사용한 예시 소스 ===============
    //// physX에 내 Transform 던지기 
    //m_pRigidBodyCom->Update(m_pTransformCom);
    //// physX에서 변경된 Transform 가져오기
    //m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
}

HRESULT CTestModel::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(0)))
            return E_FAIL;

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CTestModel::Render_LightDepth()
{
    if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
        return E_FAIL;

    return S_OK;
}

void CTestModel::Render_IMGUI()
{
    __super::Render_IMGUI();

    if (ImGui::TreeNode("Guizmo"))
    {
        _float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
        m_pGameInstance->EditTransform(matWorld);
        m_pTransformCom->Set_WorldMatrix(matWorld);
        ImGui::Separator(); ImGui::NewLine();
        ImGui::TreePop();
    }
}

//[JS] : 이걸 CharacterContoller에 만들어두기
//void CTestModel::SetOn_Slope()
void CTestModel::SetOn_Slope(_float fTimeDelta)
{
    PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
    _vector vUp = CUtils::To_Vector(slope);

    LerpUpVector(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), vUp, 10.f, fTimeDelta);
}

void CTestModel::LerpUpVector(_fvector _vOriginUp, _fvector _vTargetUp, _float _maxAngle, _float fTimeDelta)
{
    _float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(_vTargetUp, _vOriginUp));

    if (fAngle >= XMConvertToRadians(_maxAngle))
    {
        _vector vRight = XMVector3Cross(XMVector3Normalize(_vOriginUp), XMVector3Normalize(_vTargetUp));
        m_pTransformCom->Turn(vRight, fTimeDelta * fAngle * 7.f);
    }
}

HRESULT CTestModel::Add_Components()
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxAnimModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Dee"),
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    /* For.Com_RigidBody */
    if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_RigidBody"),
        TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom)))
        return E_FAIL;

    _float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
    CCharacterController::CONTROLLER_DESC desc{};
    desc.vInitialPos = vPos;
    /* For.Com_RigidBody */
    if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_CharacterController"),
        TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc)))
        return E_FAIL;
    m_pControllerCom->Set_PhysXObject(this);

    //m_pRigidBodyCom->Set_PhysXObject(this);
    //m_pRigidBodyCom->Activate(true);

    return S_OK;
}

// not yet [JYWI]
void CTestModel::Add_RigidBody(const wstring& KeyName, void* pArg)
{
    HRESULT hr;

    CRigidBody* pRigidBody = nullptr;
    hr = Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_RigidBody"), KeyName,
                       (CComponent**)&pRigidBody, pArg);
    CHECK_FAILED(hr);

    // not yet
    //m_mapRigidBodies.emplace(KeyName, pRigidBody);
}

HRESULT CTestModel::Bind_ShaderResources()
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

CTestModel* CTestModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestModel* pInstance = new CTestModel(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CTestModel"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestModel::Clone(void* pArg)
{
    CTestModel* pInstance = new CTestModel(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Created : CTestModel"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestModel::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pRigidBodyCom);
    Safe_Release(m_pControllerCom);
    
    Safe_Release(m_pLight);

    // not yet [240520]
    //for (auto& iter : m_mapRigidBodies)
    //    Safe_Release(iter.second);
}
