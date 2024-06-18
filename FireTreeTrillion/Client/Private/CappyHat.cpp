#include "stdafx.h"
#include "CappyHat.h"
#include "CappyBody.h"
#include "FSM.h"
#include "CappyHat_State.h"

CCappyHat::CCappyHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CCappyHat::CCappyHat(const CCappyHat& rhs)
	: CMonster{ rhs }
{
}

HRESULT CCappyHat::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CCappyHat::Initialize(void* pArg)
{
	CAPPYHAT_DESC* pCappyDesc = nullptr;

	if (nullptr != pArg)
	{
		pCappyDesc = (CAPPYHAT_DESC*)pArg;

		pCappyDesc->fSpeedPerSec = 7.f;
		pCappyDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_WorldMatrix = pCappyDesc->WorldMatrix;
		m_pGameObject = pCappyDesc->pGameObject;
		Safe_AddRef(m_pGameObject);
	}

	if (FAILED(__super::Initialize(pCappyDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(CAPPYHAT_WAIT, 50.f, true, true);
	m_pTransformCom->Set_WorldMatrix(m_WorldMatrix);

	m_fMaxHp = 15.f;
	m_fHp = 15.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;
	//m_ePhyXState = PO_NORMAL;

	return S_OK;
}

_int CCappyHat::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	if (static_cast<CCappyBody*>(m_pGameObject)->Get_Dead())
		m_bDead = true;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
	{
		if(false == m_bController)
		{
			static_cast<CCappyBody*>(m_pGameObject)->Change_State(CCappyBody::CAPPYBODY_HATLOSE, 55.f, true, true);
			Change_State(CCappyHat::CAPPYHAT_FLY, 55.f, true, true);

			HRESULT hr;

			/* For.Com_CharacterController */
			m_vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
			CCharacterController::CONTROLLER_DESC desc{};
			desc.vInitialPos = m_vPosition;
			desc.tCapsuleShape.fHeight = 0.5f;
			desc.tCapsuleShape.fRadius = 0.5f;
			hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
				TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
			CHECK_FAILED(hr);
			m_pControllerCom->Set_Object(this);
			m_pControllerCom->Activate(false);
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

			m_bController = true;
		}
	}
	else if(false == m_bController)
	{
		if (CCappyBody::CAPPYBODY_KASAUP3 != static_cast<CCappyBody*>(m_pGameObject)->Get_State())
		{
			m_pModelCom->Set_Animation(CAPPYHAT_FLY, 50.f, false, true);

			// 버섯 머리 뼈에 붙힘
			_float4x4 WorldMatrix = static_cast<CCappyBody*>(m_pGameObject)->Compute_BoneWorldMatrix();
			m_pTransformCom->Set_WorldMatrix(WorldMatrix);
			m_fHeightTime = 0.f;
		}
		else
		{
			m_pModelCom->Set_Animation(CAPPYHAT_FLY, 50.f, false, true);

			_float fAnimRatio = static_cast<CCappyBody*>(m_pGameObject)->Get_AnimRatio();
			_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			_float fnewY = vPos.m128_f32[1];

			// 애니메이션 비율에 따라 위치를 조정합니다.
			if (0.5f > fAnimRatio)
				fnewY += (0.8f * (0.5f - fAnimRatio) + 0.1f) * m_fTimeDelta * 40.f;
			else
				fnewY -= (0.8f * (fAnimRatio - 0.5f) + 0.1f) * m_fTimeDelta * 40.f;

			// 현재 높이를 유지하면서 새로운 y값을 설정합니다.
			vPos.m128_f32[1] = fnewY;

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

			//// 버섯 머리 뼈에 붙힘
			//_float4x4 WorldMatrix = dynamic_cast<CCappyBody*>(m_pGameObject)->Compute_BoneWorldMatrix(true);

			//m_pTransformCom->Set_WorldMatrix(WorldMatrix);
		}
	}

	//__super::Tick(m_fTimeDelta);

	// FSM 제어
	if (m_pFSM != nullptr)
		m_pFSM->Update(this, fTimeDelta);

	return OBJ_NOEVENT;
}

void CCappyHat::Late_Tick(_float fTimeDelta)
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

HRESULT CCappyHat::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		//if (Custom_Face(i) == true)
		//	continue;

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

HRESULT CCappyHat::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CCappyHat::Render_IMGUI()
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

void CCappyHat::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(CAPPYHAT_FLY, 50.f, false, true);
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
}

void CCappyHat::Collision_Overlap(CGameObject* pGameObject)
{
}

void CCappyHat::Change_State(CAPPYHAT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CCappyHat::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CCappyHat::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_CappyHat"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	///* For.Com_Texture */
	//hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Awoofy_Eye"),
	//	TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	//CHECK_FAILED(hr);

	///* For.Com_CharacterController */
	//m_vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	//CCharacterController::CONTROLLER_DESC desc{};
	//desc.vInitialPos = m_vPosition;
	//desc.tCapsuleShape.fHeight = 0.1f;
	//desc.tCapsuleShape.fRadius = 0.3f;
	//hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
	//	TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//CHECK_FAILED(hr);
	//m_pControllerCom->Set_Object(this);

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

	// FOR ANIMTOOL
	//m_ppModelForAnimTool = &m_pModelCom;

	/* FSM */
	SetUp_FSM();

	return S_OK;
}

HRESULT CCappyHat::Bind_ShaderResources()
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

void CCappyHat::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(CAPPYHAT_WAIT, CCappyHat_Idle_State::Create());
	m_pFSM->Add_State(CAPPYHAT_FLY, CCappyHat_Damage_State::Create());

	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = CAPPYHAT_WAIT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CCappyHat* CCappyHat::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCappyHat* pInstance = new CCappyHat(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCappyHat"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCappyHat::Clone(void* pArg)
{
	CCappyHat* pInstance = new CCappyHat(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCappyHat"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCappyHat::Free()
{
	__super::Free();

	Safe_Release(m_pGameObject);
}
