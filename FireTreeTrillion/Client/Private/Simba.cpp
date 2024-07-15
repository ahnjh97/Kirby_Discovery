#include "stdafx.h"
#include "Simba.h"
#include "FSM.h"
#include "Simba_State.h"
#include "MultiEffect.h"
#include "HitBox.h"
#include "Bone.h"
#include "EventCenter.h"
#include "Bone.h"

CSimba::CSimba(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CSimba::CSimba(const CSimba& rhs)
	: CMonster{ rhs }
{
}

void CSimba::InsertHitboxActivationTiming(SIMBA_ANIM eAnimIdx, vector<tuple<_float, _bool, COLLISION_VALUE>>& _vecTimings)
{
	sort(_vecTimings.begin(), _vecTimings.end(), [](const tuple<float, bool, COLLISION_VALUE>& a, const tuple<float, bool, COLLISION_VALUE>& b) {
		return get<0>(a) < get<0>(b);
		});

	m_mapHitBoxTiming.insert_or_assign(eAnimIdx, _vecTimings);
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

	m_pKirby = m_pGameInstance->Get_GameObject(LEVEL_SIMBA, TEXT("Layer_Player"));
	Safe_AddRef(m_pKirby);

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fMaxHp = 500.f;
	m_fHp = 500.f;
	m_fMaxHp = 50.f;
	m_fHp = 50.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_BIG;
	m_eEyeState = SIMBAEYE_LONG;

	m_iEyeMesh = m_pModelCom->Find_MeshIndex(string("BodyM__EyeC"));
	m_iEyeLidMesh = m_pModelCom->Find_MeshIndex(string("EyelidM__EyelidC"));
	_uint iMantIndex = m_pModelCom->Find_MeshIndex(string("MantM__MantC"));
	_uint iFurIndex = m_pModelCom->Find_MeshIndex(string("FurL__MantC"));
	m_vecMantMeshes = { iMantIndex, iFurIndex };

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		if (i == m_iEyeMesh || i == m_iEyeLidMesh || i == iMantIndex || i == iFurIndex)
			continue;
		m_vecMeshes.push_back(i);
	}

	m_pModelCom->Set_Animation(Simba_DemoAppear1Cut2, 66.66f, false, false);

	m_pModelCom->EmplaceBackPartialAnim(Simba_DamageFaceSub);
	m_pModelCom->EmplaceBackPartialAnim(Simba_LipSyncSub);
	m_pModelCom->EmplaceBackPartialAnim(Simba_LipSyncSubA);

	CEventCenter* pEventCenter = CEventCenter::Get_Instance();

	function<void(CGameObject*)> func{};
	func = bind(&CSimba::OnAppearStart, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_APPEAR_START, this, func);

	func = bind(&CSimba::OnNextDialog1, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_NEXT_DIALOG1, this, func);

	func = bind(&CSimba::OnNextDialog2, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_NEXT_DIALOG2, this, func);

	func = bind(&CSimba::OnLastDialog, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_LAST_DIALOG, this, func);

	func = bind(&CSimba::OnAppearEnd, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_APPEAR_END, this, func);

	func = bind(&CSimba::OnWave1Dead, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_WAVE1DEAD, this, func);

	func = bind(&CSimba::OnWave2Dead, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_WAVE2DEAD, this, func);

	Add_AnimEvent();

	SetUpHitBoxTimings();

	Set_Slope(false);

	m_matDefault = m_pTransformCom->Get_WorldFloat4x4();

	m_pLipBone = m_pModelCom->Get_BonePtr("T_LLip0J");
	Safe_AddRef(m_pLipBone);

	m_pRotationBone = m_pModelCom->Get_BonePtr("RotL");
	Safe_AddRef(m_pRotationBone);
	m_pRotationBoneMatrix = m_pRotationBone->Get_EditMatrixPtr();

	m_setAppear1Anims = { Simba_DemoAppear1Cut2, Simba_DemoAppear1Cut2Wait, Simba_DemoAppear1Cut3, Simba_DemoAppear1Cut3Wait, 
		Simba_DemoAppear1Cut4, Simba_DemoAppear1Cut4Wait };

	m_setUndamagableAnims = { Simba_Death, Simba_DemoDeadCut1, Simba_DemoDeadCut2 };

	return S_OK;
}

_int CSimba::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fHpRatio =  m_fHp / m_fMaxHp;

	if (true == m_pModelCom->IsFinished() || m_pModelCom->Get_Trackposition() == 0.f) // IsAnimFinished
		Reset_HitBoxTimingMap(SIMBA_ANIM(m_pModelCom->Get_CurAnimIndex()));

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	__super::Tick(m_fTimeDelta);

	if (LEVEL_TOOL_ANIM == *m_pCurrentLevelID)
		return OBJ_NOEVENT;

	if (true == m_pModelCom->IsPartialAnimFinished())
		m_bPlayPartialAnim = false;

	if (m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{
		if (m_pGameInstance->Get_KeyState(DIK_SPACE, KEY_DOWN))
		{
			_uint iCurAnimIndex = m_pModelCom->Get_CurAnimIndex();
			_uint iNewAnimIndex = iCurAnimIndex + 1;
			_uint iNumAnimations = m_pModelCom->Get_AnimCnt();
			if (iNumAnimations == iNewAnimIndex)
				iNewAnimIndex = 0;
			m_pModelCom->Set_Animation(iNewAnimIndex, 40.f, true, false);
		}
		if (m_pGameInstance->Get_KeyState(DIK_NUMPAD1, KEY_DOWN)) {
			m_bPhaseTwo = true;
			Change_State(Simba_Damage, 50.f, false, true);
		}
	}

	Check_HitBoxActivation();

	if (m_setAppear1Anims.end() != m_setAppear1Anims.find(SIMBA_ANIM(Get_State())) && m_pGameInstance->Get_KeyState(DIK_A, KEY_DOWN)) {
		if (false == m_bPlayPartialAnim) {
			m_pModelCom->Reset_PartialAnimation(Simba_LipSyncSub, 50.f, false, false);
			m_bPlayPartialAnim = true;
		}
	}

	if (m_fHp <= 0.f && false == m_bDeathAnimPlayed)
	{
		m_bDeathAnimPlayed = true;
		Turn_RotationBoneMatrix(0.f);
		TransformToDefault(0.f);
		Change_State(Simba_Death, 2.f, false, true);
	}

	if (0.45f > m_fHpRatio && 0.f < m_fHpRatio && m_bPhaseTwo == false) {
		m_bPhaseTwo = true;
		Turn_RotationBoneMatrix(-2.5f);
		Change_State(Simba_Damage, 50.f, false, true);
	}

	return OBJ_NOEVENT;
}

void CSimba::Late_Tick(_float fTimeDelta)
{
	_bool bIsFinished = m_pModelCom->IsFinished();
	m_pModelCom->Play_Animation(m_fTimeDelta);

	if (false == bIsFinished)
	{
		_uint iAnimIdx = m_pModelCom->Get_CurAnimIndex();
		if (true == m_bPlayPartialAnim && m_setUndamagableAnims.end() == m_setUndamagableAnims.find(SIMBA_ANIM(iAnimIdx))) {

			_float fPartialAnimRatio = m_pModelCom->Get_PartialAnimRatio();
			_float4x4 matLipTransformMatrix = m_pLipBone->Get_TransformationMatrix();
			if (0.6f < fPartialAnimRatio && (-0.05f > matLipTransformMatrix._42 && -0.08f < matLipTransformMatrix._42))
				m_bPlayPartialAnim = false;
			else
				m_pModelCom->Play_PartialAnimation(m_fTimeDelta);
		}
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
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

	if (FAILED(m_pShaderCom->Begin(ANIMMODEL_SIMBAEYE)))
		return E_FAIL;
	if (FAILED(m_pModelCom->Render(m_iEyeMesh)))
		return E_FAIL;

	if (true == m_bRenderMant) // Render Mant Mesh 
	{
		_bool bFalse = { false };
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bFalse, sizeof(_bool))))
			return E_FAIL;

		for (auto& idx : m_vecMantMeshes)
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", idx, TextureType_DIFFUSE)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", idx, TextureType_NORMALS)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", idx, TextureType_METALNESS)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", idx)))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Render(idx)))
				return E_FAIL;
		}
	}

	if (true == m_bRenderEyeLid)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iEyeLidMesh, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", m_iEyeLidMesh, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", m_iEyeLidMesh, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", m_iEyeLidMesh)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(m_iEyeLidMesh)))
			return E_FAIL;
	}
	
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
	if (true == m_bPlayPartialAnim)
		return;

	m_pModelCom->Reset_PartialAnimation(Simba_DamageFaceSub, 40.f, false, false);
	m_bPlayPartialAnim = true;
}

void CSimba::Change_State(SIMBA_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState(eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CSimba::CreateHpBar()
{
	if (false == m_bHpBarCreated)
	{
		m_bHpBarCreated = true;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_BossUI"), TEXT("Prototype_GameObject_HUD_BossHpBar"), this)))
			return;
	}
}

void CSimba::Turn_RotationBoneMatrix(_float fAngle)
{
	_float4x4 RotationMatrix = _float4x4::Identity;
	if (0 == fAngle) 
	{
		*m_pRotationBoneMatrix = RotationMatrix;
		return;
	}

	CUtils::Turn_OtherMatrix(RotationMatrix, _float4(1, 0, 0, 0), 1.f, fAngle);
	*m_pRotationBoneMatrix = RotationMatrix;
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
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.255f;
	desc.tCapsuleShape.fHeight = 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"), TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	/* FSM */
	if(LEVEL_SIMBA == *m_pGameInstance->Get_CurrentLevelID())
		SetUp_FSM();

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = BOSS_SIMBA;
	HitBox.pSocket = m_pModelCom->Get_BonePtr("C_BodyJ");

	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_SPHERE, 2.f, 7.f, 5.5f);

	CHitBox::HITBOX_DESC tAttack{};
	tAttack.pOwner = this;
	tAttack.pDesc = &m_tColliderDesc[ATTACK]; // Left Hand
	tAttack.pCollisionType = HITBOX_MONSTER;
	tAttack.pSocket = m_pModelCom->Get_BonePtr("L_HaveL");
	//tAttack.vBoneOffset = _float3(0, 0, -3);
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
		return E_FAIL;
	
	tAttack.pDesc = &m_tColliderDesc[ATTACK2]; // Right Hand
	tAttack.pSocket = m_pModelCom->Get_BonePtr("R_HaveL");
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
		return E_FAIL;

	tAttack.pDesc = &m_tColliderDesc[ATTACK3]; // Body
	tAttack.pSocket = m_pModelCom->Get_BonePtr("C_BodyJ");
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
		return E_FAIL;

	//Activate_SphereCollider(0.f, 4.5f);
	//Activate_SphereCollider(0.f, 4.5f, ATTACK2);
	//Activate_SphereCollider(2.f, 8.f, ATTACK3);
	Activate_FrustumCollider(0.f, 8.f, 150.f, ATTACK);
	Activate_FrustumCollider(0.f, 8.f, 150.f, ATTACK2);
	Activate_FrustumCollider(0.f, 11.5f, 150.f, ATTACK3);
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
	CTransform* pKirbyTransform = m_pKirby->Get_TransformCom();

	m_pFSM = CFSM::Create();

	for (_uint i = Simba_DemoAppear1Cut10; i <= Simba_DemoAppear1Cut9Wait; i++)
		m_pFSM->Add_State(i, CSimba_Appear1::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_DemoAppear2Cut1, CSimba_Appear2::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_DemoAppear2Cut2, CSimba_Appear2::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Walk, CSimba_Walk::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for (_uint i = Simba_QuickClaw2L; i <= Simba_QuickClawStartR; i++)
		m_pFSM->Add_State(i, CSimba_QuickClaw::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	for(_uint i = Simba_FinalCrusher; i <= Simba_FinalCrusherStartRepeatEnd; i++)
		m_pFSM->Add_State(i, CSimba_FinalCrusher::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	for(_uint i = Simba_DoubleClaw; i <= Simba_DoubleClawEnd; i++)
		m_pFSM->Add_State(i, CSimba_DoubleClaw::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Jump, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_JumpStart, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Fall, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Landing, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for(_uint i = Simba_BackStep; i <= Simba_BackStepStart; i++)
		m_pFSM->Add_State(i, CSimba_BackStep::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for(_uint i = Simba_AttackJump; i <= Simba_AttackJumpWait; i++)
		m_pFSM->Add_State(i, CSimba_AttackJump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Damage, CSimba_Damage::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Roar2, CSimba_Roar::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for(_uint i = Simba_BiteRushJumpL; i<= Simba_BiteRushLandingR; i++)
		m_pFSM->Add_State(i, CSimba_BiteRushJump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for(_uint i = Simba_DimensionClaw; i<= Simba_DimensionClawWait; i++)
		m_pFSM->Add_State(i, CSimba_DimensionClaw::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for(_uint i = Simba_BiteRush; i<= Simba_BiteRushEnd; i++)
		m_pFSM->Add_State(i, CSimba_BiteRush::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	for (_uint i = Simba_BiteRushStart; i <= Simba_BiteRushStartStraight; i++)
		m_pFSM->Add_State(i, CSimba_BiteRush::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for (_uint i = Simba_DimensionLaser; i <= Simba_DimensionLaserWait; i++)
		m_pFSM->Add_State(i, CSimba_DimensionLaser::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Death, CSimba_Death::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_DemoDeadCut1, CSimba_Death::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_DemoDeadCut2, CSimba_Death::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	//상태 Initialize
	CFSM::FSM_INFO	FSM_Desc = {};
	FSM_Desc.iState = Simba_DemoAppear1Cut2;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

void CSimba::SetUpHitBoxTimings()
{
	vector<tuple<_float, _bool, COLLISION_VALUE>> vecTiming;

	for(_uint i = 1; i < 9; i++)
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK));
	InsertHitboxActivationTiming(Simba_QuickClawL, vecTiming);
	InsertHitboxActivationTiming(Simba_QuickClaw2L, vecTiming);
	vecTiming.clear();

	for (_uint i = 1; i < 9; i++)
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK2));
	InsertHitboxActivationTiming(Simba_QuickClawR, vecTiming);
	InsertHitboxActivationTiming(Simba_QuickClaw2R, vecTiming);
	vecTiming.clear();

	for (_uint i = 1; i < 9; i++) {
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK));
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK2));
	}
	InsertHitboxActivationTiming(Simba_DoubleClaw, vecTiming);
	vecTiming.clear();

	for (_uint i = 1; i < 5; i++) {
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK));
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK2));
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.1f, false, ATTACK3));
	}
	InsertHitboxActivationTiming(Simba_FinalCrusher, vecTiming);
	vecTiming.clear();
}

void CSimba::Check_HitBoxActivation()
{
	SIMBA_ANIM eAnimIdx = SIMBA_ANIM(m_pModelCom->Get_CurAnimIndex());
	_float fAnimRatio = m_pModelCom->Get_AnimRatio();

	auto pair = m_mapHitBoxTiming.find(eAnimIdx);
	if (pair != m_mapHitBoxTiming.end())
	{
		auto& vecHitBoxTiming = pair->second;

		for (auto& tuple : vecHitBoxTiming)
		{
			if (get<0>(tuple) <= fAnimRatio)
			{
				if (false == get<1>(tuple))
				{
					get<1>(tuple) = true;
					Activate_Attack(get<2>(tuple));
					return;
				}
			}
		}
	}
}

void CSimba::Reset_HitBoxTimingMap(SIMBA_ANIM eAnimIdx)
{
	auto pair = m_mapHitBoxTiming.find(eAnimIdx);
	if (pair != m_mapHitBoxTiming.end())
	{
		auto& vecHitBoxTiming = pair->second;
		for (auto& tuple : vecHitBoxTiming)
			get<1>(tuple) = false;
	}
}

void CSimba::TransformToDefault(_float fOffsetY)
{
	_float4x4 matWorld = m_matDefault;
	//matWorld._42 = m_matDefault._42 - 2.005f;
	matWorld._42 = m_matDefault._42 - 1.97f;
	m_pTransformCom->Set_WorldMatrix(matWorld);
	m_pControllerCom->Set_Position(m_pTransformCom, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pControllerCom->FreeFall(m_pTransformCom, m_pGameInstance->Get_SecondTimer(), 6.f, fOffsetY);
	m_bPhaseTwo = false;
}

void CSimba::OnAppearStart(CGameObject* pObj)
{
	// 대사 시작
	m_bRenderMant = true;
	Change_State(Simba_DemoAppear1Cut2, 66.66f, false, false);
	TransformToDefault(0);
}

void CSimba::OnNextDialog1(CGameObject* pObj)
{
	Change_State(Simba_DemoAppear1Cut4, 66.66f, false, true);
	TransformToDefault(0);
}

void CSimba::OnNextDialog2(CGameObject* pObj)
{
	Change_State(Simba_DemoAppear1Cut3, 66.66f, false, true);
	TransformToDefault(0);
}

void CSimba::OnLastDialog(CGameObject* pObj)
{
	Change_State(Simba_DemoAppear1Cut10, 50.f, false, true);
	TransformToDefault(0);
}

void CSimba::OnAppearEnd(CGameObject* pObj)
{
	Change_State(Simba_DemoAppear1Cut9, 50.f, false, true);
	TransformToDefault(0);
	SpawnMonsters(11);
}

void CSimba::OnWave1Dead(CGameObject* pObj)
{
	SpawnMonsters(12);
}

void CSimba::OnWave2Dead(CGameObject* pObj)
{
	m_bRenderMant = true;
	m_bRenderEyeLid = false;
	Change_State(Simba_DemoAppear2Cut1, 66.66f, false, false);
	TransformToDefault(-0.3f);
}

void CSimba::SpawnMonsters(_uint iTriggerIndex)
{
	wstring wstrLayerTag = TEXT("Layer_Wave");
	if (11 == iTriggerIndex)
		wstrLayerTag += TEXT("1");
	else if(12 == iTriggerIndex)
		wstrLayerTag += TEXT("2");

	list<CGameObject*>* pObjList = m_pGameInstance->Get_List(LEVEL_SIMBA, wstrLayerTag);
	if (nullptr != pObjList && false == pObjList->empty())
		return;
		
	wstring wstrPrototypeTag = TEXT("Prototype_GameObject_");

	for (auto& monsterDesc : m_vecMonsterDescs)
	{
		if (iTriggerIndex == monsterDesc.eMonState)
		{
			wstring wstrTag;
			if (TEXT("Awoofy") == monsterDesc.wstrModelName || TEXT("AwoofyWild") == monsterDesc.wstrModelName)
				wstrTag = wstrPrototypeTag + TEXT("Awoofy");
			else if(TEXT("Rabbit") == monsterDesc.wstrModelName || TEXT("RabbitBig") == monsterDesc.wstrModelName)
				wstrTag = wstrPrototypeTag + TEXT("Rabbit");

			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SIMBA, wstrLayerTag, wstrTag, &monsterDesc)))
				return;
		}
	}
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
	CEventCenter::Get_Instance()->Unsubscribe(this);
	Safe_Release(m_pLipBone);
	Safe_Release(m_pRotationBone);
	Safe_Release(m_pKirby);

	__super::Free();

	for(_uint i = 0; i < EYETEX_END; i++)
		Safe_Release(m_pEyeTextureCom[i]);
}