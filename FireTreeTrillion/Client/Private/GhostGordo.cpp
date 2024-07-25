#include "stdafx.h"
#include "GhostGordo.h"
#include "FSM.h"
#include "HitBox.h"
#include "GhostGordo_State.h"

CGhostGordo::CGhostGordo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CGhostGordo::CGhostGordo(const CGhostGordo& rhs)
	: CMonster { rhs }
{
}

HRESULT CGhostGordo::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CGhostGordo::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;

		pGameObjectDesc->fSpeedPerSec = 7.f;
		pGameObjectDesc->fRotationPerSec = XMConvertToRadians(90.0f);
	}

	if (FAILED(__super::Initialize(pGameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(GORDO_APPEAR, 50.f, false, true);


	//m_fMaxHp = 5.f;
	//m_fHp = 5.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;

	return S_OK;
}

_int CGhostGordo::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
	{
		m_pGameInstance->PlaySound_Free(L"Gordo_Dead.wav", 0.1f);
		return Ready_Dead();
	}

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (m_pGameInstance->Get_DIKeyState(DIK_M, KEY_PRESS))
		m_bLight = true;
	else
		m_bLight = false;

	__super::Tick(m_fTimeDelta);

	//if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
	//	Change_State(PHANTA_DAMAGE, 50.f, false, true);

	return OBJ_NOEVENT;
}

void CGhostGordo::Late_Tick(_float fTimeDelta)
{
	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		// 날아갈 땐, 애니메이션 재생이 되지 않는다.
		if (m_ePhyXState != PO_FLYAWAY)
		{
			if (Compute_OptimizationAnimation(m_fTimeDelta) == true && m_ePhyXState != PO_PRESSED)
				m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fAccTime * 0.3f) : m_pModelCom->Play_Animation(m_fAccTime);
		}
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CGhostGordo::Render()
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

HRESULT CGhostGordo::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGhostGordo::Render_IMGUI()
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

void CGhostGordo::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	//if (eContent == CCollisionCenter::CONTENT_BODY)
	//{
	//	if (m_ePhyXState == PO_NORMAL)
	//	{
	//		Change_State(PHANTA_DAMAGE, 50.f, false, true);
	//	}
	//}
	//else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	//{

	//}
	//else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	//{
	//	if (m_ePhyXState == PO_NORMAL)
	//	{
	//		Change_State(PHANTA_DAMAGE, 50.f, false, true);
	//	}
	//}
}

void CGhostGordo::Change_State(GORDO_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CGhostGordo::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CGhostGordo::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_GhostGordo"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_CharacterController */
	m_vOriginPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_vOriginPosition;
	desc.fOffset = 0.f;
	desc.tCapsuleShape.fRadius = 0.4f;
	desc.tCapsuleShape.fHeight = 0.4f;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vOriginPosition);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 1.f, 2.f, 0.85f);

	m_vOriginLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	m_vOriginLook.m128_f32[1] = 0.f;

	SetUp_FSM();

	return S_OK;
}

HRESULT CGhostGordo::Bind_ShaderResources()
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

	_float fWhite = { 0.f };
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhite, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}
 
void CGhostGordo::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	m_pFSM->Add_State(GORDO_APPEAR, CGhostGordo_Idle_State::Create());
	m_pFSM->Add_State(GORDO_EYECLOSEWAIT, CGhostGordo_Idle_State::Create());
	m_pFSM->Add_State(GORDO_EYEOPENSTART, CGhostGordo_Idle_State::Create());

	m_pFSM->Add_State(GORDO_EYEOPENWAIT, CGhostGordo_Move_State::Create());

	m_pFSM->Add_State(GORDO_LOOK, CGhostGordo_Look_State::Create());

	FSM_Desc.iState = GORDO_APPEAR;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CGhostGordo* CGhostGordo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGhostGordo* pInstance = new CGhostGordo(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGhostGordo"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGhostGordo::Clone(void* pArg)
{
	CGhostGordo* pInstance = new CGhostGordo(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGhostGordo"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGhostGordo::Free()
{
	__super::Free();
}
