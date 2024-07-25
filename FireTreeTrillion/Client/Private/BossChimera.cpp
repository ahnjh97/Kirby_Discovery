#include "stdafx.h"
#include "BossChimera.h"
#include "FSM.h"
#include "Awoofy_State.h"
#include "MultiEffect.h"

#include "HitBox.h"

CBossChimera::CBossChimera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CBossChimera::CBossChimera(const CBossChimera& rhs)
	: CMonster{ rhs }
{
}

HRESULT CBossChimera::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CBossChimera::Initialize(void* pArg)
{
	MONSTER_DESC* pMonDesc = nullptr;

	if (nullptr != pArg)
	{
		pMonDesc = (MONSTER_DESC*)pArg;

		pMonDesc->fSpeedPerSec = 7.f;
		pMonDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_eMonState = (MONSTER_STATE)pMonDesc->eMonState;
	}

	if (FAILED(__super::Initialize(pMonDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	//if(MON_WAIT == m_eMonState)
	//	m_pModelCom->Set_Animation(AWOOFY_GROOMING, 45.f, false, true);
	//else if(MON_CIRCLE == m_eMonState)
	//	m_pModelCom->Set_Animation(AWOOFY_WALK, 45.f, true, true);
	//else if(MON_SLEEP == m_eMonState)
	//	m_pModelCom->Set_Animation(AWOOFY_SLEEP, 45.f, true, true);

	m_fMaxHp = 10.f;
	m_fHp = 10.f;
	m_fAttack = 8.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;
	//m_eEyeState = AWOOFYEYE_IDLE;

	Add_AnimEvent();

	return S_OK;
}

_int CBossChimera::Tick(_float fTimeDelta)
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

	//if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
	//	Change_State(CBossChimera::AWOOFY_DAMAGE, 120.f, true, false);


	__super::Tick(m_fTimeDelta);


	return OBJ_NOEVENT;
}

void CBossChimera::Late_Tick(_float fTimeDelta)
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

HRESULT CBossChimera::Render()
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

HRESULT CBossChimera::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CBossChimera::Add_AnimEvent()
{
	__super::Add_AnimEvent();
	
	// 1. 한 애니메이션에서 같은 이름의 이벤트 가능
	// 2. 재생 기준은 애님툴에서 지정한 애니메이션인지 + 시작 프레임이 애니메이션 프레임안에 들어가는 지
	// 3. 두번째 인자로 넣어준 람다가 시작 프레임 한번만 실행된다.
	m_pModelCom->Add_Event("Bboong", [this]() {
		//파티클 생성
		static _float fBbongTime{ 0.f };
		fBbongTime += GetTickCount64();
		if (.2f < fBbongTime)
		{
			CMultiEffect::MULTI_FX_DESC FXDesc{};
			_float4 vMyPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			FXDesc.vInitPos = { vMyPos.x, vMyPos.y + .3f, vMyPos.z };
			FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };

			_float3 vDir = -m_pTransformCom->Get_State(CTransform::STATE_LOOK);
			vDir.Normalize();
			_float3 vLook = { 0.f, 0.f, 1.f };

			_float fAngleLook = atan2f(vLook.z, vLook.x);
			_float fAngleDiff = fAngleLook - atan2f(vDir.z, vDir.x);
			fAngleDiff = ToDegree(fAngleDiff);

			_float3 vAngle = { 0.f, fAngleDiff, 0.f };
			FXDesc.vInitRot = vAngle;

			if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
				return;

			fBbongTime = 0.f;
		}
		});

	m_pModelCom->Add_Event("PlaySound", [this]() {
		// 사운드 처리
		//m_pGameInstance->PlaySound_Free(L"TakeItem01.wav", 0.5f);
		});

	m_pModelCom->Add_Event("ApplyDamage", [this]() {
		//데미지 처리
		});
}

#ifdef _DEBUG
void CBossChimera::Render_IMGUI()
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

void CBossChimera::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			//Change_State(CBossChimera::AWOOFY_DAMAGE, 50.f, false, true);
			//m_eEyeState = AWOOFYEYE_HAPPY;
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			//Change_State(CBossChimera::AWOOFY_DAMAGE, 50.f, false, true);
			//m_eEyeState = AWOOFYEYE_HAPPY;
		}
	}
}

/*
void CBossChimera::Change_State(AWOOFY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}
*/

_bool CBossChimera::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

_bool CBossChimera::IsAnimFinished(_uint iCurrentAnimIndex)
{
	return m_pModelCom->IsFinished(iCurrentAnimIndex);
}

void CBossChimera::Compute_Angle(_vector vOrginLook, _vector vTargetLook)
{
	//// 정규화 및 회전 축 계산
	//vOrginLook.m128_f32[1] = 0.f;
	//vTargetLook.m128_f32[1] = 0.f;
	XMVECTOR vOriginLookNormalized = XMVector3Normalize(vOrginLook);
	XMVECTOR vTargetLookNormalized = XMVector3Normalize(vTargetLook);

	//// 회전 각도 계산
	m_fAngle = acos(XMVectorGetX(XMVector3Dot(vOriginLookNormalized, vTargetLookNormalized)));
	_float fY = ::XMVectorGetY(::XMVector3Cross(vOriginLookNormalized, vTargetLookNormalized));
	if (fY < 0)
		m_fAngle = -m_fAngle;
}

_bool CBossChimera::Custom_Face(_uint iMeshIndex)
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

HRESULT CBossChimera::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Awoofy"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);
	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Awoofy_Eye"),
		TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	m_vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_vPos;
	desc.fOffset = 1.f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);
	//m_pControllerCom->Set_Object(this);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPos);

	/* FSM */
	SetUp_FSM();


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.f, 0.85f);

	return S_OK;
}

HRESULT CBossChimera::Bind_ShaderResources()
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

void CBossChimera::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();

	//m_pFSM->Add_State(AWOOFY_WALK, CBossChimera_Walk_State::Create());

	//m_pFSM->Add_State(AWOOFY_WAIT, CBossChimera_Idle_State::Create());
	//m_pFSM->Add_State(AWOOFY_GROOMING, CBossChimera_Idle_State::Create());
	//m_pFSM->Add_State(AWOOFY_LOOKAROUND, CBossChimera_Idle_State::Create());
	//m_pFSM->Add_State(AWOOFY_SLEEP, CBossChimera_Idle_State::Create());

	//m_pFSM->Add_State(AWOOFY_RUN, CBossChimera_Run_State::Create());
	//m_pFSM->Add_State(AWOOFY_FIND, CBossChimera_Find_State::Create());
	//m_pFSM->Add_State(AWOOFY_BRAKE, CBossChimera_Brake_State::Create());
	//m_pFSM->Add_State(AWOOFY_LOOKAROUNDAFTERBRAKE, CBossChimera_LookAroundAfterBrake_State::Create());

	//m_pFSM->Add_State(AWOOFY_DAMAGE, CBossChimera_Damage_State::Create());

	// 상태 Initialize
	//CFSM::FSM_INFO		FSM_Desc = {};
	//if (MON_WAIT == m_eMonState)
	//	FSM_Desc.iState = AWOOFY_GROOMING;
	//else if (MON_CIRCLE == m_eMonState)
	//	FSM_Desc.iState = AWOOFY_WALK;
	//else if (MON_SLEEP == m_eMonState)
	//	FSM_Desc.iState = AWOOFY_SLEEP;
	//FSM_Desc.pModel = &m_pModelCom;
	//m_pFSM->Initialize(&FSM_Desc);
}

CBossChimera* CBossChimera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossChimera* pInstance = new CBossChimera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBossChimera"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBossChimera::Clone(void* pArg)
{
	CBossChimera* pInstance = new CBossChimera(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBossChimera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossChimera::Free()
{
	__super::Free();

	Safe_Release(m_pEyeTextureCom);
}

