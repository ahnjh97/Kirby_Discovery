#include "stdafx.h"
#include "TestModel.h"
#include "Light.h"
//#include "Utils.h"

#include "RigidBody.h"
#include "FSM.h"
#include "TestModel_State.h"

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
    m_eCollisionGroup = COLLISION_TYPE::INTERACT;

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
    _vector vPos = XMVectorSet(-128.f, 6.f, -80.f, 1.f);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
    m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-16.f));

    if (FAILED(Add_Components()))
        return E_FAIL;

    m_pModelCom->Set_Animation(0, 60.f, true);

    // 예시코드 1 : 태양광
    //LIGHT_DESC			LightDesc{};
    //LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
    //LightDesc.vDirection = _float4(0.f, -1.f, 0.f, 0.f);

    //LightDesc.vDiffuse = _float4(0.8f, 0.8f, 0.8f, 1.f);
    //LightDesc.vAmbient = _float4(0.6f, 0.6f, 0.6f, 1.f);
    //LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 1.f);
    //if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
    //    return E_FAIL;


    // 예시코드 2 : 따라다니게 하기 예시 코드 + 점 광원 예시 코드
   /* LightDesc.eType = LIGHT_DESC::TYPE_POINT;
    LightDesc.vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
    LightDesc.fRange = 5.f;
    LightDesc.vDiffuse = _float4(1.f, 0.8f, 0.1f, 1.f);
    LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
    if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
        return E_FAIL;

    m_pLight = CGameInstance::Get_Instance()->Get_LightLastAddress();
    Safe_AddRef(m_pLight);*/


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

    // 점프용 velocity(속도)
    m_fJumpVelocity -= GRAVITY * fTimeDelta;
    /*if (true == m_isJump)
        m_isJump = m_pControllerCom->Jump(m_pTransformCom, m_fJumpVelocity, fTimeDelta);
    else
        m_pControllerCom->FreeFall(m_pTransformCom, fTimeDelta);*/

    // 예시코드 5 : 계산기 예시 코드 (월드 매트리스로 예시든거임 이건 정신나간 코드이므로 참고해주셈)
    // 예시코드 6 : DInput + KeyPress 예시 코드
    //if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_PRESS))
    //{
    //    _float4x4 Worldmatrix = m_pTransformCom->Get_WorldFloat4x4();
    //    _vector vLook = CUtils::Get_State_Vector_Matrix(Worldmatrix, CUtils::STATE_LOOK);
    //    _vector vPos = CUtils::Get_State_Vector_Matrix(Worldmatrix, CUtils::STATE_POSITION);
    //    _float fSpeed = 3.f;

    //    vPos += vLook * fTimeDelta * fSpeed;

    //    CUtils::Set_State_Matrix(Worldmatrix, CUtils::STATE_POSITION, vPos);

    //    m_pTransformCom->Set_WorldMatrix(Worldmatrix);
    //}
    
    // 예시코드 6 : DInput + KeyPress 예시 코드 
    if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_PRESS))
    {
        _float fSpeed = m_pTransformCom->Get_SpeedPerSec();
        // for test
        //fSpeed = 5.f;
        //m_pControllerCom->Move(m_pTransformCom, fSpeed, fTimeDelta);
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
        m_fJumpVelocity = 5.f;
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
        if (m_iTestAnim > 290)
            m_iTestAnim = 290;

        m_pModelCom->Set_Animation(m_iTestAnim, 60.f, true, true);
    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_O, KEY_DOWN))
    {
        m_iTestAnim--;
        if (m_iTestAnim < 0)
            m_iTestAnim = 0;

        m_pModelCom->Set_Animation(m_iTestAnim, 60.f, true, true);

    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_I, KEY_DOWN))
    {
        //m_pModelCom->Set_Animation(m_iTestAnim, true, true);
    }

    // FSM 제어
    Update_FSMState(fTimeDelta);
    m_pFSM->Update(this, fTimeDelta);

    return OBJ_NOEVENT;
}

void CTestModel::Late_Tick(_float fTimeDelta)
{
    m_pModelCom->Play_Animation(fTimeDelta);

    //SetOn_Slope(fTimeDelta);

    if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
    {
        //m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
    }

    //if (m_pGameInstance->Get_DIKeyState(DIK_L, KEY_DOWN))
    //{
    //    CGameObject* pCamera = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Camera"), 0);
    //    _float4 vForce = static_cast<CTransform*>(pCamera->Get_Component(g_strTransformTag))->Get_State_Float4(CTransform::STATE_LOOK);
    //    //_float4 vForce = CUtils::Make_RandomAngle_Vector(20.f, XMVectorSet(0.f, 1.f, 0.f, 0.f));
    //    _float3 force = _float3{ vForce.x * 10000.f, vForce.y * 10000.f, vForce.z * 10000.f };
    //    m_pRigidBodyCom->Add_Force(force);
    //}

    //m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
#ifdef _DEBUG
    /*m_pGameInstance->RenderGrid();*/
#endif
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
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(1)))
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

#ifdef _DEBUG
void CTestModel::Render_IMGUI()
{
    if (ImGui::TreeNode("Guizmo"))
    {
        _float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
        m_pGameInstance->EditTransform(matWorld);
        m_pTransformCom->Set_WorldMatrix(matWorld);
        ImGui::Separator(); ImGui::NewLine();
        ImGui::TreePop();
    }

    ImGui::Text("FSM : %d", m_eCurrentState);
    ImGui::Separator(); ImGui::NewLine();

    __super::Render_IMGUI();
}
#endif

// ==================================== 커비 전용 ===================================
void CTestModel::SetOn_Slope(_float fTimeDelta)
{
    // 지면의 up벡터
   /* PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
    _vector vTerrainNormal = CUtils::To_Vector(slope);

    Lerp_UpVector(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), vTerrainNormal, 10.f, fTimeDelta);*/
}

/// <summary> 객체와 지면의 up벡터를 비교하여 객체의 각도를 보간한다. </summary>
/// <param name="_vOriginUp"> 객체의 up 벡터 </param>
/// <param name="_vTargetUp"> 지면의 노말 벡터 </param>
/// <param name="_maxAngle"> 해당 각도보다 크면 각도 보간이 된다. </param>
void CTestModel::Lerp_UpVector(_fvector _vOriginUp, _fvector _vTargetUp, _float _maxAngle, _float fTimeDelta)
{
    _float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(_vTargetUp, _vOriginUp));

    if (fAngle >= XMConvertToRadians(_maxAngle))
    {
        _vector vRight = XMVector3Cross(XMVector3Normalize(_vOriginUp), XMVector3Normalize(_vTargetUp));
        m_pTransformCom->Turn(vRight, fTimeDelta * fAngle * m_fOffsetTurn);
    }
}
// ==========================================================================

HRESULT CTestModel::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_BushM"),
        TEXT("Com_Model"), (CComponent**)&m_pModelCom);
    CHECK_FAILED(hr);
    // for animTool
    m_ppModelForAnimTool = &m_pModelCom;

    /* For.Com_RigidBody */
    //CRigidBody::RIGIDBODY_DESC rigidDesc {};
    //rigidDesc.bTrigger = false;
    //rigidDesc.eShapeType = RIGID_BOX;
    //rigidDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
    //hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
    //    TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
    //CHECK_FAILED(hr);
    //m_pRigidBodyCom->Set_Object(this);
    ////m_pRigidBodyCom->Set_PhysXObject(this);
    //m_pRigidBodyCom->Activate(true);

    ///* For.Com_CharacterController */
    //_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
    //CCharacterController::CONTROLLER_DESC desc{};
    //desc.vInitialPos = vPos;
    //hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
    //    TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);

    /* FSM */
    SetUp_FSM();

    return S_OK;
}

// not yet [JYWI]
void CTestModel::Add_RigidBody(const wstring& KeyName, void* pArg)
{
    HRESULT hr;

    CRigidBody* pRigidBody = nullptr;
    hr = Add_Component(TEXT("Prototype_Component_RigidBody"), KeyName,
                       (CComponent**)&pRigidBody, pArg);
    CHECK_FAILED(hr);

    // not yet
    //m_mapRigidBodies.emplace(KeyName, pRigidBody);
}

void CTestModel::SetUp_FSM()
{
    // FSM 상태 초기화
    m_pFSM = CFSM::Create();
    m_pFSM->Add_State(ATTACK,   CTestModel_Attack_State::Create());
    m_pFSM->Add_State(IDLE,     CTestModel_Idle_State::Create());
    m_pFSM->Add_State(RUN,      CTestModel_Run_State::Create());

    // 상태 Initialize
    CFSM::FSM_INFO		FSM_Desc = {};
    FSM_Desc.iState = m_eCurrentState = IDLE;
    FSM_Desc.pModel = &m_pModelCom;
    m_pFSM->Initialize(&FSM_Desc);
}

void CTestModel::Update_FSMState(_float fTimeDelta)
{
	//switch (m_eCurrentState)
	//{
	//case ATTACK:
	//{
	//	/* ATTACK → IDLE */
	//	//if (m_pBody_Player->IsOverTrackPercent(defaultAnimRatio))
 //       //   Change_State(IDLE, 1.f, true, true);

 //       /* ATTACK → RUN */
	//}
	//break;

	//case IDLE:
	//{
	//	/* IDLE → ATTACK */
	//	if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
	//		Change_State(ATTACK, 1.f, false, true);


	//	/* IDLE → RUN */
	//	//if (m_pGameInstance->Get_DIKeyState(DIK_W, KEY_PRESS) ||
	//	//	m_pGameInstance->Get_DIKeyState(DIK_A, KEY_PRESS) ||
	//	//	m_pGameInstance->Get_DIKeyState(DIK_S, KEY_PRESS) ||
	//	//	m_pGameInstance->Get_DIKeyState(DIK_D, KEY_PRESS))
	//	//	Change_State(RUN, 2.f, true, true);
	//}
	//break;

	//case RUN:
	//{
	//	/* RUN → ATTACK */

	//	/* RUN → IDLE */
	//	//if (false == m_pGameInstance->Get_DIKeyState(DIK_W, KEY_PRESS) &&
	//	//	false == m_pGameInstance->Get_DIKeyState(DIK_A, KEY_PRESS) &&
	//	//	false == m_pGameInstance->Get_DIKeyState(DIK_S, KEY_PRESS) &&
	//	//	false == m_pGameInstance->Get_DIKeyState(DIK_D, KEY_PRESS))
	//	//		Change_State(IDLE, 1.f, true, true);
	//}
	//break;
	//}
}

void CTestModel::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
    m_eCurrentState = eState;
    m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
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
    //Safe_Release(m_pRigidBodyCom);
    Safe_Release(m_pControllerCom);
    
    Safe_Release(m_pLight);
    Safe_Release(m_pFSM);
    
    // not yet [240520]
    //for (auto& iter : m_mapRigidBodies)
    //    Safe_Release(iter.second);
}

