#include "stdafx.h"
#include "CappyBody.h"
#include "FSM.h"
#include "CappyBody_State.h"
#include "Bone.h"
#include "CappyHat.h"

#include "HitBox.h"

CCappyBody::CCappyBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CCappyBody::CCappyBody(const CCappyBody& rhs)
	: CMonster{ rhs }
{
}

HRESULT CCappyBody::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CCappyBody::Initialize(void* pArg)
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

	m_pModelCom->Set_Animation(CAPPYBODY_HIDINGWAITA, 60.f, false, true);


	m_fMaxHp = 15.f;
	m_fHp = 15.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;

	CCappyHat::CAPPYHAT_DESC CappyHatDesc = {};
	CappyHatDesc.WorldMatrix = Compute_BoneWorldMatrix();
	CappyHatDesc.pGameObject = this;
	 
	if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_CappyHat"), TEXT("Prototype_GameObject_CappyHat"), &CappyHatDesc)))
		return E_FAIL;


	return S_OK;
}

_int CCappyBody::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();


	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
	{
		Change_State(CAPPYBODY_DAMAGE, 120.f, true, false);
	}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CCappyBody::Late_Tick(_float fTimeDelta)
{
	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	// 날아갈 땐, 애니메이션 재생이 되지 않는다.
	if (m_ePhyXState != PO_FLYAWAY)
	{
		m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fTimeDelta * 0.3f) : m_pModelCom->Play_Animation(m_fTimeDelta);
	}

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 12.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CCappyBody::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		//if (Custom_Face(i) == true)
		//	continue;
		if(m_bRender == true)
		{
			if (0 == i)
				continue;
		}

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

HRESULT CCappyBody::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CCappyBody::Render_IMGUI()
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

void CCappyBody::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(CAPPYBODY_DAMAGE, 50.f, false, true);
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(CAPPYBODY_DAMAGE, 50.f, false, true);
		}
	}

}

void CCappyBody::Change_State(CAPPYBODY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CCappyBody::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

_float4x4 CCappyBody::Compute_BoneWorldMatrix(_bool bDecompose)
{
	CBone* pBone = m_pModelCom->Get_BonePtr("CappyHatL");

	_float4x4 WorldMatrix = m_pTransformCom->Get_WorldFloat4x4();

	if (false == bDecompose)
	{
		XMStoreFloat4x4(&m_WorldMatrix, XMLoadFloat4x4(pBone->Get_CombinedTransformationMatrix()) * XMLoadFloat4x4(&WorldMatrix));
		m_WorldMatrix._42 += 0.15f;
	}
	else
	{
		_vector vScale, vRotate, vTrans;
		XMMatrixDecompose(&vScale, &vRotate, &vTrans, WorldMatrix);
		WorldMatrix = XMMatrixAffineTransformation(vScale, XMVectorZero(), XMVectorZero(), vTrans);
		XMStoreFloat4x4(&m_WorldMatrix, XMLoadFloat4x4(pBone->Get_CombinedTransformationMatrix()) * XMLoadFloat4x4(&WorldMatrix));
	}

	return m_WorldMatrix;
}

HRESULT CCappyBody::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_CappyBody"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	///* For.Com_Texture */
	//hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Awoofy_Eye"),
	//	TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	//CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.f;
	//desc.tCapsuleShape.fHeight = 0.5f;
	//desc.tCapsuleShape.fRadius = 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);
	m_pControllerCom->Set_Object(this);


	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.f, 0.85f);


	/* FSM */
	SetUp_FSM();

	return S_OK;
}

HRESULT CCappyBody::Bind_ShaderResources()
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

void CCappyBody::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(CAPPYBODY_HIDINGWAITA, CCappyBody_Idle_State::Create());
	m_pFSM->Add_State(CAPPYBODY_HIDINGWAITB, CCappyBody_Idle_State::Create());
	m_pFSM->Add_State(CAPPYBODY_HIDINGJUMPA, CCappyBody_Idle_State::Create());
	m_pFSM->Add_State(CAPPYBODY_HIDINGJUMPB, CCappyBody_Idle_State::Create());

	m_pFSM->Add_State(CAPPYBODY_KASAUP1, CCappyBody_Find_State::Create());
	m_pFSM->Add_State(CAPPYBODY_KASAUP2, CCappyBody_Find_State::Create());
	m_pFSM->Add_State(CAPPYBODY_KASAUP3, CCappyBody_Find_State::Create());

	m_pFSM->Add_State(CAPPYBODY_WAIT, CCappyBody_Run_State::Create());

	m_pFSM->Add_State(CAPPYBODY_HATLOSE, CCappyBody_HatLose_State::Create());

	m_pFSM->Add_State(CAPPYBODY_DAMAGE, CCappyBody_Damage_State::Create());

	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = CAPPYBODY_HIDINGWAITA;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CCappyBody* CCappyBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappyBody* pInstance = new CCappyBody(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCappyBody"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCappyBody::Clone(void* pArg)
{
	CCappyBody* pInstance = new CCappyBody(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCappyBody"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappyBody::Free()
{
	__super::Free();
}
