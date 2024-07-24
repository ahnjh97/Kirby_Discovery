#include "stdafx.h"
#include "Buffahorn.h"
#include "FSM.h"
#include "Buffahorn_State.h"
#include "HitBox.h"

CBuffahorn::CBuffahorn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CBuffahorn::CBuffahorn(const CBuffahorn& rhs)
	: CMonster{ rhs }
{
}

HRESULT CBuffahorn::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CBuffahorn::Initialize(void* pArg)
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

	m_pModelCom->Set_Animation(BUFFAHORN_CHARGEWAIT, 50.f, true, true);


	m_fMaxHp = 10.f;
	m_fHp = 10.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_BIG;
	m_eAbilityType = ABILITY_DEFAULT;
	m_eEyeState = BUFFAHORNEYE_IDLE;

	return S_OK;
}

_int CBuffahorn::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();


	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
	{
		Change_State(BUFFAHORN_DAMAGE, 50.f, false, true);
	}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CBuffahorn::Late_Tick(_float fTimeDelta)
{
	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 12.0f))
	{
		// 날아갈 땐, 애니메이션 재생이 되지 않는다.
		if (m_ePhyXState != PO_FLYAWAY)
		{
			if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
				m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fAccTime * 0.3f) : m_pModelCom->Play_Animation(m_fAccTime);
		}

		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CBuffahorn::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (Custom_Face(i) == true)
			continue;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		//몸통(1)은 normal O, 눈까리(0)는 normal x 패스
		if (i == 0 && m_bRenderBody)
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Render(i)))
				return E_FAIL;
		}
		else if (i == 1 && m_bRenderEye)
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Render(i)))
				return E_FAIL;
		}

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBuffahorn::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBuffahorn::Render_IMGUI()
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


void CBuffahorn::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{


		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			// 커비랑 몸박인데, 몸 끼리 박으면 소는 안 아파하는거 같더라
			// 맞아 어빌리티만 공격판정인데 
			Change_State(BUFFAHORN_DAMAGE, 50.f, false, true);
		}
	}


}

void CBuffahorn::Change_State(BUFFAHORN_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CBuffahorn::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CBuffahorn::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Buffahorn"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Buffahorn_Eye"),
		TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 2.3f;
	desc.tCapsuleShape.fHeight = 1.5f;
	desc.tCapsuleShape.fRadius = 1.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 3.f, 3.f);

	SetUp_FSM();

	return S_OK;
}

HRESULT CBuffahorn::Bind_ShaderResources()
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

void CBuffahorn::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(BUFFAHORN_CHARGEWAIT, CBuffahorn_Idle_State::Create());

	m_pFSM->Add_State(BUFFAHORN_FIND, CBuffahorn_Find_State::Create());

	m_pFSM->Add_State(BUFFAHORN_WAIT, CBuffahorn_Wait_State::Create());

	m_pFSM->Add_State(BUFFAHORN_RUNSTART, CBuffahorn_Run_State::Create());
	m_pFSM->Add_State(BUFFAHORN_RUN, CBuffahorn_Run_State::Create());

	m_pFSM->Add_State(BUFFAHORN_BRAKE, CBuffahorn_Brake_State::Create());
	m_pFSM->Add_State(BUFFAHORN_BRAKEEND, CBuffahorn_Brake_State::Create());

	m_pFSM->Add_State(BUFFAHORN_JUMP, CBuffahorn_Jump_State::Create());
	m_pFSM->Add_State(BUFFAHORN_RETURNJUMPEND, CBuffahorn_Jump_State::Create());

	m_pFSM->Add_State(BUFFAHORN_DAMAGE, CBuffahorn_Damage_State::Create());
	m_pFSM->Add_State(BUFFAHORN_BOUNCETOTURN, CBuffahorn_BounceToTurn_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = BUFFAHORN_CHARGEWAIT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

_bool CBuffahorn::Custom_Face(_uint iMeshIndex)
{
	if (iMeshIndex == 1)
	{
		HRESULT hr;

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIndex);
		CHECK_FAILED(hr);

		hr = m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", (_uint)m_eEyeState);
		CHECK_FAILED(hr);

		_bool bStencil = true;
		_bool bRimLight = true;
		_bool bMotionBlur = true;
		m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool));

		m_pShaderCom->Begin(ANIMMODEL_EYE);
		m_pModelCom->Render(iMeshIndex);

		return true;
	}

	return false;
}

CBuffahorn* CBuffahorn::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBuffahorn* pInstance = new CBuffahorn(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBuffahorn"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBuffahorn::Clone(void* pArg)
{
	CBuffahorn* pInstance = new CBuffahorn(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBuffahorn"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBuffahorn::Free()
{
	__super::Free();

	Safe_Release(m_pEyeTextureCom);
}
