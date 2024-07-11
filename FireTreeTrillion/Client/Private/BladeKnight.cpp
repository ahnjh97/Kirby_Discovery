#include "stdafx.h"
#include "BladeKnight.h"
#include "FSM.h"
#include "BladeKnight_State.h"
#include "BladeKnightSword.h"
#include "HitBox.h"

CBladeKnight::CBladeKnight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CBladeKnight::CBladeKnight(const CBladeKnight& rhs)
	: CMonster{ rhs }
{
}

HRESULT CBladeKnight::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CBladeKnight::Initialize(void* pArg)
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

	if (FAILED(Add_PartObjects()))
		return E_FAIL;

	m_pModelCom->Set_Animation(BLADEKNIGHT_WAIT, 50.f, true, true);

	m_fMaxHp = 15.f;
	m_fHp = 15.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_SWORD;

	Add_AnimEvent();

	return S_OK;
}

_int CBladeKnight::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	// 만약, 밟히면 그 순간 그냥 찐빵되고 죽는다.
	if (m_ePhyXState == PO_PRESSED)
	{
		m_pTransformCom->Set_Scaled(1.f, 0.1f, 1.f);
		m_fPressedTime += m_fTimeDelta;

		if (m_fPressedTime > 1.5f)
			m_bDead = true;
		return OBJ_NOEVENT;
	}

	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
		Change_State(BLADEKNIGHT_DAMAGE, 50.f, false, true);


	__super::Tick(m_fTimeDelta);

	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CBladeKnight::Late_Tick(_float fTimeDelta)
{
	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(m_fTimeDelta);

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

HRESULT CBladeKnight::Render()
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

HRESULT CBladeKnight::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBladeKnight::Render_IMGUI()
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

void CBladeKnight::Add_AnimEvent()
{
	__super::Add_AnimEvent();

	m_pModelCom->Add_Event("Attack", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		HitBoxChanger(m_pFSM->Get_State());
		});
}

void CBladeKnight::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(BLADEKNIGHT_DAMAGE, 50.f, false, true);
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(BLADEKNIGHT_DAMAGE, 50.f, false, true);
		}
	}
}

void CBladeKnight::Change_State(BLADEKNIGHT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CBladeKnight::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CBladeKnight::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_BladeKnight"),
								TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);
	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 0.8f;
	desc.tCapsuleShape.fRadius = 0.4f;
	desc.tCapsuleShape.fHeight = 0.4f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);
	//m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	SetUp_FSM();



	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 1.f, 1.5f, 0.85f);

	HitBox.pDesc = &m_tColliderDesc[ATTACK];
	HitBox.pCollisionType = HITBOX_MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBladeKnight::Add_PartObjects()
{
	/* For.Part_Weapon */
	CPartObject* pWeaponObject = { nullptr };
	CBladeKnightSword::BLADEKNIGHTSWORD_DESC	BladeKnightSwordDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));

	BladeKnightSwordDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	BladeKnightSwordDesc.pSocket = pModel->Get_BonePtr("RHaveL");

	pWeaponObject = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BladeKnightSword"), &BladeKnightSwordDesc));
	if (nullptr == pWeaponObject)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pWeaponObject);

	return S_OK;
}

HRESULT CBladeKnight::Bind_ShaderResources()
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

void CBladeKnight::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(BLADEKNIGHT_WAIT, CBladeKnight_Idle_State::Create());

	m_pFSM->Add_State(BLADEKNIGHT_FIND, CBladeKnight_Find_State::Create());
	m_pFSM->Add_State(BLADEKNIGHT_FINDWAIT, CBladeKnight_Find_State::Create());

	m_pFSM->Add_State(BLADEKNIGHT_MOVE, CBladeKnight_Move_State::Create());

	m_pFSM->Add_State(BLADEKNIGHT_ATTACKSTART, CBladeKnight_Attack_State::Create());
	m_pFSM->Add_State(BLADEKNIGHT_ATTACK, CBladeKnight_Attack_State::Create());
	m_pFSM->Add_State(BLADEKNIGHT_DOUBLEATTACK, CBladeKnight_Attack_State::Create());

	m_pFSM->Add_State(BLADEKNIGHT_RETREAT, CBladeKnight_Retreat_State::Create());

	m_pFSM->Add_State(BLADEKNIGHT_TORNADOATTACKCHARGE, CBladeKnight_TornadoAttack_State::Create());
	m_pFSM->Add_State(BLADEKNIGHT_TORNADOATTACK, CBladeKnight_TornadoAttack_State::Create());

	m_pFSM->Add_State(BLADEKNIGHT_DAMAGE, CBladeKnight_Damage_State::Create());

	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = BLADEKNIGHT_WAIT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

void CBladeKnight::HitBoxChanger(_uint eState)
{
	switch (eState)
	{
	case BLADEKNIGHT_ATTACK:
		Activate_FrustumCollider(0.5f, 4.f, 180.f);
		break;
	case BLADEKNIGHT_DOUBLEATTACK:
		Activate_FrustumCollider(0.5f, 4.f, 180.f);
		break;
	case BLADEKNIGHT_TORNADOATTACK:
		Activate_SphereCollider(0.5f, 3.5f);
		break;
	default:
		break;
	}
}

CBladeKnight* CBladeKnight::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBladeKnight* pInstance = new CBladeKnight(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBladeKnight"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBladeKnight::Clone(void* pArg)
{
	CBladeKnight* pInstance = new CBladeKnight(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBladeKnight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBladeKnight::Free()
{
	__super::Free();

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);

	m_PartObjects.clear();
}
