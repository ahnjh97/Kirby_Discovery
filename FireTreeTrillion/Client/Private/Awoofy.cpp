#include "stdafx.h"
#include "Awoofy.h"
#include "FSM.h"
#include "Awoofy_State.h"

CAwoofy::CAwoofy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CAwoofy::CAwoofy(const CAwoofy& rhs)
	: CMonster{ rhs }
{
}

HRESULT CAwoofy::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CAwoofy::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(AWOOFY_GROOMING, 45.f, false, true);

	return S_OK;
}

_int CAwoofy::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	__super::Tick(fTimeDelta);

	Compute_ViewZ();

	__super::SetOn_Slope(fTimeDelta);

	Compute_MotionBlur();

   // FSM 제어
	if (m_pFSM != nullptr)
		m_pFSM->Update(this, fTimeDelta);

	return OBJ_NOEVENT;
}

void CAwoofy::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CAwoofy::Render()
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
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_X)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CAwoofy::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CAwoofy::Render_IMGUI()
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

void CAwoofy::Collision_Attack(CGameObject* pOtherObj)
{
	Change_State(CAwoofy::AWOOFY_DAMAGE, 50.f, false, true);
}

void CAwoofy::Change_State(AWOOFY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CAwoofy::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

_bool CAwoofy::IsAnimFinished(_uint iCurrentAnimIndex)
{
	return m_pModelCom->IsFinished(iCurrentAnimIndex);
}

void CAwoofy::Compute_Angle(_vector vOrginLook, _vector vTargetLook)
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

HRESULT CAwoofy::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Awoofy"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = XMVectorSet(10.f, 20.f, -170.f, 1.f);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);
	m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	SetUp_FSM();

	return S_OK;
}

HRESULT CAwoofy::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_bool bStencil = true;
	_bool bRimLight = true;
	_bool bMotionBlur = true;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CAwoofy::Compute_MotionBlur()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vPreScreenPos = vCurScreenPos;
}

void CAwoofy::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(AWOOFY_WAIT, CAwoofy_Idle_State::Create());
	m_pFSM->Add_State(AWOOFY_GROOMING, CAwoofy_Idle_State::Create());
	m_pFSM->Add_State(AWOOFY_LOOKAROUND, CAwoofy_Idle_State::Create());

	m_pFSM->Add_State(AWOOFY_RUN, CAwoofy_Run_State::Create());
	m_pFSM->Add_State(AWOOFY_FIND, CAwoofy_Find_State::Create());
	m_pFSM->Add_State(AWOOFY_BRAKE, CAwoofy_Brake_State::Create());
	m_pFSM->Add_State(AWOOFY_LOOKAROUNDAFTERBRAKE, CAwoofy_LookAroundAfterBrake_State::Create());

	m_pFSM->Add_State(AWOOFY_DAMAGE, CAwoofy_Damage_State::Create());

	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = AWOOFY_WAIT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CAwoofy* CAwoofy::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAwoofy* pInstance = new CAwoofy(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CAwoofy"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAwoofy::Clone(void* pArg)
{
	CAwoofy* pInstance = new CAwoofy(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CAwoofy"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAwoofy::Free()
{
	__super::Free();

	//Safe_Release(m_pFSM);
}

