#include "stdafx.h"
#include "Simba.h"
#include "FSM.h"
#include "Simba_State.h"
#include "MultiEffect.h"
#include "HitBox.h"
#include "Bone.h"

CSimba::CSimba(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CSimba::CSimba(const CSimba& rhs)
	: CMonster{ rhs }
{
}

HRESULT CSimba::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CSimba::Initialize(void* pArg)
{
	MONSTER_DESC* pMonDesc = nullptr;

	if (nullptr != pArg)
	{
		pMonDesc = (MONSTER_DESC*)pArg;

		pMonDesc->fSpeedPerSec = 7.f;
		pMonDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_eMonState = pMonDesc->eMonState;
	}

	if (FAILED(__super::Initialize(pMonDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;
	m_pModelCom->Set_Animation(0, 1.f, true, true);
	m_fMaxHp = 500.f;
	m_fHp = 500.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_BIG;
	m_eEyeState = SIMBAEYE_LONG;

	m_iEyeMesh = m_pModelCom->Find_MeshIndex(string("BodyM__EyeC"));
	m_iEyeLidMesh = m_pModelCom->Find_MeshIndex(string("EyelidM__EyelidC"));

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (i == m_iEyeMesh || i == m_iEyeLidMesh)
			continue;

		m_vecMeshes.push_back(i);
	}

	Add_AnimEvent();

	return S_OK;
}

_int CSimba::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CSimba::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CSimba::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (auto& index : m_vecMeshes)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", index, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", index, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", index, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", index)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(index)))
			return E_FAIL;
	}

	// Render Eye Mesh
	if (FAILED(m_pEyeTextureCom[EYETEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_eEyeState)))
		return E_FAIL;
	if (FAILED(m_pEyeTextureCom[EYETEX_NORMAL]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", m_eEyeState)))
		return E_FAIL;
	if (FAILED(m_pEyeTextureCom[EYETEX_MRA]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture")))
		return E_FAIL;
	if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", m_iEyeMesh)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
		return E_FAIL;
	if (FAILED(m_pModelCom->Render(m_iEyeMesh)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSimba::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CSimba::Add_AnimEvent()
{
	__super::Add_AnimEvent();

	// 1. 한 애니메이션에서 같은 이름의 이벤트 가능
	// 2. 재생 기준은 애님툴에서 지정한 애니메이션인지 + 시작 프레임이 애니메이션 프레임안에 들어가는 지
	// 3. 두번째 인자로 넣어준 람다가 시작 프레임 한번만 실행된다.
}

#ifdef _DEBUG
void CSimba::Render_IMGUI()
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

void CSimba::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	
}

void CSimba::Change_State(SIMBA_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState(eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

HRESULT CSimba::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Simba"), TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SimbaEye_Diffuse"), TEXT("Com_Texture_EyeDiffuse"), (CComponent**)&m_pEyeTextureCom[EYETEX_DIFFUSE]);
	CHECK_FAILED(hr);
	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SimbaEye_Normal"), TEXT("Com_Texture_EyeNormal"), (CComponent**)&m_pEyeTextureCom[EYETEX_NORMAL]);
	CHECK_FAILED(hr);
	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SimbaEye_MRA"), TEXT("Com_Texture_EyeMRA"), (CComponent**)&m_pEyeTextureCom[EYETEX_MRA]);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	m_vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_vPos;
	desc.fOffset = 0.8f;
	desc.tCapsuleShape.fHeight = 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"), TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPos);

	/* FSM */
	SetUp_FSM();


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = BOSS_FINALBOSS;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 1.f, 1.5f, 0.85f);

	return S_OK;
}

HRESULT CSimba::Bind_ShaderResources()
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

void CSimba::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();

	//m_pFSM->Add_State(AWOOFY_WALK, CSimba_Walk_State::Create());
	
	//상태 Initialize
	CFSM::FSM_INFO	FSM_Desc = {};
	FSM_Desc.iState = SIMBA_END;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);

}

CSimba* CSimba::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSimba* pInstance = new CSimba(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSimba"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSimba::Clone(void* pArg)
{
	CSimba* pInstance = new CSimba(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSimba"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSimba::Free()
{
	__super::Free();

	for(_uint i = 0; i < EYETEX_END; i++)
		Safe_Release(m_pEyeTextureCom[i]);
}