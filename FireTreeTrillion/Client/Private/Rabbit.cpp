#include "stdafx.h"
#include "Rabbit.h"
#include "FSM.h"
#include "Rabbit_State.h"

#include "HitBox.h"

CRabbit::CRabbit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CRabbit::CRabbit(const CRabbit& rhs)
	: CMonster{ rhs }
{
}

HRESULT CRabbit::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRabbit::Initialize(void* pArg)
{
	wstring wstrModelName;
	if (LEVEL_SIMBA == *m_pGameInstance->Get_CurrentLevelID() || LEVEL_FINALBOSS == *m_pGameInstance->Get_CurrentLevelID())
	{
		MONSTER_DESC tDesc{};
		if(nullptr != pArg)
			tDesc = *(MONSTER_DESC*)pArg;
		tDesc.fSpeedPerSec = 7.f;
		tDesc.fRotationPerSec = XMConvertToRadians(90.0f);
		m_eRabbitState = RS_TARGET;
		wstrModelName = tDesc.wstrModelName;
		if (FAILED(__super::Initialize(&tDesc)))
			return E_FAIL;
	}
	else if (LEVEL_TOOL_ANIM == *m_pGameInstance->Get_CurrentLevelID())
		wstrModelName = TEXT("Rabbit");
	else
	{
		MONSTER_DESC* pRabbitDesc = nullptr;

		if (nullptr != pArg)
		{
			pRabbitDesc = (MONSTER_DESC*)pArg;

			pRabbitDesc->fSpeedPerSec = 7.f;
			pRabbitDesc->fRotationPerSec = XMConvertToRadians(90.0f);
			m_eRabbitState = (RABBIT_STATE)pRabbitDesc->eMonState;
			wstrModelName = pRabbitDesc->wstrModelName;
		}

		if (FAILED(__super::Initialize(pRabbitDesc)))
			return E_FAIL;
	}

	if (LEVEL_TOOL_ANIM != *m_pCurrentLevelID)
	{
		size_t underscorePos = wstrModelName.find(L'_');
		if (underscorePos != wstring::npos)
			wstrModelName = wstrModelName.substr(underscorePos + 1);
	}

	m_eCollisionGroup = MONSTER;
	if (FAILED(Add_Components(wstrModelName)))
		return E_FAIL;

	//Change_State(RABBIT_WAIT, 45.f, false, true);
	m_pModelCom->Set_Animation(RABBIT_WAIT, 45.f, false, true);

	//CTransform* pKirby = dynamic_cast<CTransform*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Player"), 0));
	//Compute_Parabola(pKirby);

	m_fMaxHp = 10.f;
	m_fHp = 10.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_SMALL;
	m_eAbilityType = ABILITY_DEFAULT;
	m_eEyeState = RABBITEYE_IDLE;

	m_iEyeMeshIdx = m_pModelCom->Find_MeshIndex(string("Eye"));

	if (LEVEL_SIMBA == *m_pCurrentLevelID || LEVEL_FINALBOSS == *m_pGameInstance->Get_CurrentLevelID()) {
		m_bFind = true;
		Set_Slope(false);
	}
		
	return S_OK;
}

_int CRabbit::Tick(_float fTimeDelta)
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

	// 빨릴 때
	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
		Change_State(CRabbit::RABBIT_DAMAGE, 120.f, true, false);

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CRabbit::Late_Tick(_float fTimeDelta)
{
	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		// 날아갈 땐, 애니메이션 재생이 되지 않는다.
		if (m_ePhyXState != PO_FLYAWAY)
		{
			//if (Compute_OptimizationAnimation(m_fTimeDelta) == true && m_ePhyXState != PO_PRESSED)
			/*m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fAccTime * 0.3f) : */m_pModelCom->Play_Animation(m_fTimeDelta);
		}
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
		m_bIsCulling = false;
	}
	else
		m_bIsCulling = true;
}

HRESULT CRabbit::Render()
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

		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CRabbit::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CRabbit::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}


	ImGui::Checkbox(u8"눈까리", &bRenderEye);
	ImGui::Checkbox(u8"몸통", &bRenderBody);


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

void CRabbit::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(CRabbit::RABBIT_DAMAGE, 50.f, false, true);
			m_eEyeState = RABBITEYE_HAPPY;
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			Change_State(CRabbit::RABBIT_DAMAGE, 50.f, false, true);
			m_eEyeState = RABBITEYE_HAPPY;
		}
	}
}

void CRabbit::Change_State(RABBIT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CRabbit::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

void CRabbit::Compute_Parabola(_vector vEndPos)
{
	// 포물선 세팅을 위한 변수
	m_vStartPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	m_vEndPos = vEndPos;
	m_vGoPos = m_vStartPos;
	m_fMaxHeight = m_vStartPos.y + 4.f;

	m_fEndHight = m_vEndPos.y - m_vStartPos.y; // 도착지점의 높이와 시작지점 높이의 차를 구해줌
	m_fHeight = m_fMaxHeight - m_vStartPos.y;

	m_fGravity = GRAVITY * 3.f /*2.f * m_fHeight / (m_fMaxTime * m_fMaxTime)*/;

	m_fAxisY = sqrtf(2.f * m_fGravity * m_fHeight);

	_float b = -2.f * m_fAxisY;
	_float c = 2.f * m_fEndHight;

	_float fResult = b * b - 4.f * m_fGravity * c;

	if (0.f > fResult)
		m_fEndTime = -b;
	else
		m_fEndTime = (-b + sqrtf(fResult)) / (2.f * m_fGravity);

	m_fAxisX = -(m_vStartPos.x - m_vEndPos.x) / m_fEndTime;
	m_fAxisZ = -(m_vStartPos.z - m_vEndPos.z) / m_fEndTime;
}

_vector CRabbit::JumpAttak(_float fTimeDelta)
{
	m_vGoPos.x = m_vStartPos.x + m_fAxisX * fTimeDelta;
	m_vGoPos.y = m_vStartPos.y + (m_fAxisY * fTimeDelta) - (0.5f * m_fGravity * fTimeDelta * fTimeDelta);
	m_vGoPos.z = m_vStartPos.z + m_fAxisZ * fTimeDelta;

	return m_vGoPos;
}

HRESULT CRabbit::Add_Components(const wstring& _wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelName;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_Texture */
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Rabbit_Eye"),
		TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	CHECK_FAILED(hr);

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.f;
		
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
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.f, 0.85f);

	SetUp_FSM();

	return S_OK;
}

HRESULT CRabbit::Bind_ShaderResources()
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

void CRabbit::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(RABBIT_DAMAGE, CRabbit_Damage_State::Create());

	m_pFSM->Add_State(RABBIT_WAIT, CRabbit_Idle_State::Create());
	m_pFSM->Add_State(RABBIT_LOOKAROUND, CRabbit_Idle_State::Create());

	m_pFSM->Add_State(RABBIT_FIND, CRabbit_Find_State::Create());

	m_pFSM->Add_State(RABBIT_JUMPSTART, CRabbit_Jump_State::Create());
	m_pFSM->Add_State(RABBIT_JUMP, CRabbit_Jump_State::Create());
	m_pFSM->Add_State(RABBIT_JUMPFALL, CRabbit_Jump_State::Create());
	m_pFSM->Add_State(RABBIT_JUMPEND, CRabbit_Jump_State::Create());

	m_pFSM->Add_State(RABBIT_JUMPLANDING, CRabbit_JumpLanding_State::Create());

	//m_pFSM->Add_State(RABBIT_BRAKE, CRabbit_Brake_State::Create());
	//m_pFSM->Add_State(RABBIT_LOOKAROUNDAFTERBRAKE, CRabbit_LookAroundAfterBrake_State::Create());

	//m_pFSM->Add_State(RABBIT_DAMAGE, CRabbit_Damage_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = RABBIT_WAIT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

_bool CRabbit::Custom_Face(_uint iMeshIndex)
{
	if (iMeshIndex == m_iEyeMeshIdx)
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

CRabbit* CRabbit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRabbit* pInstance = new CRabbit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CRabbit"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRabbit::Clone(void* pArg)
{
	CRabbit* pInstance = new CRabbit(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CRabbit"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRabbit::Free()
{
	__super::Free();

	Safe_Release(m_pEyeTextureCom);
}
