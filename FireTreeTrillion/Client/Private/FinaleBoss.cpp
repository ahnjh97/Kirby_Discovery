#include "stdafx.h"
#include "FinaleBoss.h"
#include "FSM.h"
#include "FinaleBoss_State.h"
#include "Bone.h"

CFinaleBoss::CFinaleBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CFinaleBoss::CFinaleBoss(const CFinaleBoss& rhs)
	: CMonster{ rhs }
{
}

HRESULT CFinaleBoss::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleBoss::Initialize(void* pArg)
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

	m_pModelCom->Set_Animation(FINALEBOSS_DEMOWAITAIR, 50.f, true, true);

    return S_OK;
}

_int CFinaleBoss::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (*m_pCurrentLevelID != LEVEL_TOOL_ANIM)
	{
		// FSM 제어
		if (m_pFSM != nullptr)
			m_pFSM->Update(this, m_fTimeDelta);
	}

	return OBJ_NOEVENT;
}

void CFinaleBoss::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CFinaleBoss::Render()
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
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CFinaleBoss::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CFinaleBoss::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	//ImGui::Text("RePress : %d", m_bRePressBlock);
	//ImGui::Text("Land : %d", INFO(m_isLanding));

	//ImGui::Text("JUMP : %d", INFO(m_isJump));
	//ImGui::Text("Velocity : %.2f", INFO(m_fJumpVelocity));
	//ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	//ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	//	ImGui::Text("MoveDir X : %.2f \tMoveDir Y : %.2f \tMoveDir Z : %.2f ", INFO(m_vMoveDir).x, INFO(m_vMoveDir).y, INFO(m_vMoveDir).z); ImGui::NewLine();
	//	ImGui::Text("TargetDir X : %.2f \tTargetDir Y : %.2f \tTargetDir Z : %.2f ", INFO(m_vTargetDir).x, INFO(m_vTargetDir).y, INFO(m_vTargetDir).z);
	__super::Render_IMGUI();
}
#endif

void CFinaleBoss::Add_AnimEvent()
{
}

void CFinaleBoss::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CFinaleBoss::Change_State(FINALEBOSS_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CFinaleBoss::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

_float4 CFinaleBoss::Compute_RootPos()
{
	CBone* pBone = m_pModelCom->Get_BonePtr("TopL");
	_float4x4 pBoneLocalMatrix = *pBone->Get_CombinedTransformationMatrix();
	_float4x4 pBoneWorldMatrix = pBoneLocalMatrix * m_pTransformCom->Get_WorldFloat4x4();
	return CUtils::Get_State_Vector_Matrix(pBoneWorldMatrix, CUtils::STATE_POSITION);
}

HRESULT CFinaleBoss::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FinaleBoss"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	///* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	//CCharacterController::CONTROLLER_DESC desc{};
	//desc.vInitialPos = vPos;
	//desc.fOffset = 1.f;
	//desc.tCapsuleShape.fHeight = 1.f;
	//desc.tCapsuleShape.fRadius = 1.f;
	//hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
	//	TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	SetUp_FSM();

	return S_OK;
}

HRESULT CFinaleBoss::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CFinaleBoss::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(FINALEBOSS_DEMOWAITAIR, CFinaleBoss_Idle_State::Create());

	m_pFSM->Add_State(FINALEBOSS_CUT1, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT2, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT3, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT4, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT5, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT6, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT7, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT8, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT9, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT10, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT11, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT12, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT13, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT14, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT15, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT16, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT17, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT18, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT19, CFinaleBoss_CutScene_State::Create());
	m_pFSM->Add_State(FINALEBOSS_CUT20, CFinaleBoss_CutScene_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = FINALEBOSS_DEMOWAITAIR;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CFinaleBoss* CFinaleBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinaleBoss* pInstance = new CFinaleBoss(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinaleBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinaleBoss::Clone(void* pArg)
{
	CFinaleBoss* pInstance = new CFinaleBoss(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinaleBoss"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinaleBoss::Free()
{
	__super::Free();
}
