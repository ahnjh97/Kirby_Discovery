#include "stdafx.h"
#include "Simba.h"
#include "FSM.h"
#include "Simba_State.h"
#include "MultiEffect.h"
#include "HitBox.h"
#include "Bone.h"
#include "EventCenter.h"
#include "Bone.h"
#include "Camera_Main.h"
#include "Ability.h"
#include "SummonEffect.h"
#include "Effect.h"
#include "Kirby.h"
#include "DimensionClaw.h"
#include "CollisionCenter.h"
#include "SimbaLaser.h"
#include "SimbaRock.h"
#include "Debris.h"

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

void CSimba::SetCamSequence(_uint iCamSeq)
{
	CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
	if (pCamera != nullptr)
		pCamera->Make_Sequence(CCamera_Main::CAMSEQ(iCamSeq));
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
		m_eMonState = (MONSTER_STATE)pMonDesc->eMonState;
	}

	if (FAILED(__super::Initialize(pMonDesc)))
		return E_FAIL;

	m_pKirby = m_pGameInstance->Get_GameObject(LEVEL_SIMBA, TEXT("Layer_Player"));
	Safe_AddRef(m_pKirby);

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fMaxHp = 250.f;
	m_fHp = 250.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_BIG;
	m_eEyeState = SIMBAEYE_BIG;

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

	m_pLaserBone = m_pModelCom->Get_BonePtr("LaserL");
	Safe_AddRef(m_pLaserBone);
	m_pLaserBoneMatrix = m_pLaserBone->Get_CombinedTransformationMatrix();

	m_pLeftHandBone = m_pModelCom->Get_BonePtr("L_HaveL");
	Safe_AddRef(m_pLeftHandBone);
	m_pRightHandBone = m_pModelCom->Get_BonePtr("R_HaveL");
	Safe_AddRef(m_pRightHandBone);

	m_vecLeftNailBones.emplace_back(m_pModelCom->Get_BonePtr("L_indexNailJ"));
	m_vecLeftNailBones.emplace_back(m_pModelCom->Get_BonePtr("L_middleNailJ"));
	m_vecLeftNailBones.emplace_back(m_pModelCom->Get_BonePtr("L_pinkyNailJ"));
	m_vecLeftNailBones.emplace_back(m_pModelCom->Get_BonePtr("L_ringNailJ"));
	m_vecLeftNailBones.emplace_back(m_pModelCom->Get_BonePtr("L_thumbNailJ"));
	for (auto& bone : m_vecLeftNailBones)
		Safe_AddRef(bone);

	m_vecRightNailBones.emplace_back(m_pModelCom->Get_BonePtr("R_indexNailJ"));
	m_vecRightNailBones.emplace_back(m_pModelCom->Get_BonePtr("R_middleNailJ"));
	m_vecRightNailBones.emplace_back(m_pModelCom->Get_BonePtr("R_pinkyNailJ"));
	m_vecRightNailBones.emplace_back(m_pModelCom->Get_BonePtr("R_ringNailJ"));
	m_vecRightNailBones.emplace_back(m_pModelCom->Get_BonePtr("R_thumbNailJ"));
	for (auto& bone : m_vecRightNailBones)
		Safe_AddRef(bone);

	m_setAppear1Anims = { Simba_DemoAppear1Cut2, Simba_DemoAppear1Cut2Wait, Simba_DemoAppear1Cut3, Simba_DemoAppear1Cut3Wait,
		Simba_DemoAppear1Cut4, Simba_DemoAppear1Cut4Wait };

	m_setUndamagableAnims = { Simba_Death, Simba_DemoDeadCut1, Simba_DemoDeadCut2 };

	m_setResetRequiredAnims = { Simba_AttackJumpHit, Simba_BiteRush, Simba_DimensionClaw, Simba_DimensionClawContinue,
		Simba_DimensionLaser, Simba_DoubleClaw, Simba_FinalCrusher, Simba_QuickClawL, Simba_QuickClawR, Simba_QuickClaw2L, Simba_QuickClaw2R };

	SetCamSequence(CCamera_Main::SEQ_SIMBA_START);

	CreateDimensionClawActor();

	vector<_uint> vecTunnelRocks = { 2, 4, 5, 7, 8, 9, 10, 12, 13, 16 };
	GAMEOBJECT_DESC tDesc{};

	for (_uint i = 0; i < 40; i++) {
		for (auto& rockIdx : vecTunnelRocks) {

			tDesc.wstrModelName = TEXT("TunnelRock") + to_wstring(rockIdx);
			m_vecSimbaRocks.emplace_back(dynamic_cast<CSimbaRock*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_SimbaRock"), &tDesc)));
			m_vecDebris.emplace_back(dynamic_cast<CDebris*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Debris"), &tDesc)));
		}
	}

	return S_OK;
}

_int CSimba::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fHpRatio = m_fHp / m_fMaxHp;

	ResetRotation();
	m_bRenderDimensionClaw = false;

	if (true == m_bLaserActivated)
		LaserAttack();

	if (true == m_pModelCom->IsFinished() || m_pModelCom->Get_Trackposition() == 0.f) // IsAnimFinished
		Reset_HitBoxTimingMap(SIMBA_ANIM(m_pModelCom->Get_CurAnimIndex()));

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CheckSpawning();

	__super::Tick(m_fTimeDelta);

	if (LEVEL_TOOL_ANIM == *m_pCurrentLevelID)
		return OBJ_NOEVENT;

	if (true == m_pModelCom->IsPartialAnimFinished())
		m_bPlayPartialAnim = false;

	if ((m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS) && m_pGameInstance->Get_KeyState(DIK_D, KEY_PRESS) && m_pGameInstance->Get_KeyState(DIK_F, KEY_DOWN))
		|| (0.6f > m_fHpRatio && 0.f < m_fHpRatio && m_bPhaseTwo == false))
	{
		m_bPhaseTwo = true;
		Change_State(Simba_Damage, 50.f, false, true);
	}

	if (m_pGameInstance->Get_KeyState(DIK_CAPSLOCK, KEY_PRESS))
	{
		if (m_pGameInstance->Get_KeyState(DIK_1, KEY_DOWN))
			Change_State(Simba_QuickClawStartL, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_2, KEY_DOWN))
			Change_State(Simba_FinalCrusherStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_3, KEY_DOWN))
			Change_State(Simba_DoubleClawChargeStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_4, KEY_DOWN))
			Change_State(Simba_AttackJumpPre, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_5, KEY_DOWN))
			Change_State(Simba_DimensionClawStart, 50.f, false, true);
		else if(m_pGameInstance->Get_KeyState(DIK_6, KEY_DOWN))
			Change_State(Simba_BiteRushStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_7, KEY_DOWN))
			Change_State(Simba_DimensionLaserStart, 50.f, false, true);
		else if(m_pGameInstance->Get_KeyState(DIK_8, KEY_DOWN))
			Change_State(Simba_Wait2, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_9, KEY_DOWN))
			Change_State(Simba_BiteRushJumpStartL, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_0, KEY_DOWN))
			Change_State(Simba_BiteRushJumpStartR, 50.f, false, true);

		SetUpSecondTarget();
		HideDimensionClawActor();
		HideDimensionLaserActor();
	}

	Check_HitBoxActivation();

	PlayLipSinc();

	if (m_fHp <= 0.f && false == m_bDeathAnimPlayed)
	{
		m_bDeathAnimPlayed = true;
		TransformToDefault(0.f);
		Change_State(Simba_Death, 2.f, false, true);
	}

	DetermineSimbaRotation();

	if (m_pSimbaLaser != nullptr && true == m_bLaserActivated)
		m_pSimbaLaser->Tick(m_fTimeDelta);

	for (auto& index : m_listUsedRocks)
		m_vecSimbaRocks[index]->Tick(m_fTimeDelta);

	for (auto& index : m_listUsedDebris)
		m_vecDebris[index]->Tick(m_fTimeDelta);

	RemoveDeadRocksFromList();

	return OBJ_NOEVENT;
}

void CSimba::Late_Tick(_float fTimeDelta)
{
	if (m_pSimbaLaser != nullptr && true == m_bLaserActivated)
		m_pSimbaLaser->Late_Tick(m_fTimeDelta);

	for (auto& index : m_listUsedRocks)
		m_vecSimbaRocks[index]->Late_Tick(m_fTimeDelta);

	for (auto& index : m_listUsedDebris)
		m_vecDebris[index]->Late_Tick(m_fTimeDelta);

	_bool bIsFinished = m_pModelCom->IsFinished();
	m_pModelCom->Play_Animation(m_fTimeDelta);

	if (false == bIsFinished)
		PlayPartialAnimation();

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

void CSimba::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (true == m_bPlayPartialAnim)
		return;

	m_pModelCom->Reset_PartialAnimation(Simba_DamageFaceSub, 40.f, false, false);
	m_bPlayPartialAnim = true;
}

void CSimba::Change_State(SIMBA_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_iStarCount = 0;
	m_iRockCount = 0;
	m_iDebrisCount = 0;

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

void CSimba::SpawnStar(_uint iAnimIdx) // 준수형 별 여기임
{
	HRESULT hr{};
	CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
	AbilityItemDesc.fAngle = 0.f;
	AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;

	_float fY = m_pTransformCom->Get_State(CTransform::STATE_POSITION).y;
	_vector vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_vector vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
	_float4 vFloatLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vFloatRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);

	if (Simba_QuickClawL == iAnimIdx || Simba_QuickClaw2L == iAnimIdx)
	{
		AbilityItemDesc.fRotateDir = 1.f;
		AbilityItemDesc.vDir = vRight * CUtils::Make_RandomFloat(0.7f, 1.4f);
		AbilityItemDesc.vPosition = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + vFloatLook * 2.5f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}
	else if (Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx) {
		AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = -vRight * CUtils::Make_RandomFloat(0.7f, 1.4f);
		AbilityItemDesc.vPosition = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone) + vFloatLook * 2.5f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}
	else if (Simba_FinalCrusher == iAnimIdx)
	{
		_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;

		AbilityItemDesc.fRotateDir = 1.f;
		AbilityItemDesc.vDir = vRight * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos + vFloatRight * 3.3f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);

		AbilityItemDesc.fRotateDir = 1.f;
		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos + vFloatLook * 3.5f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);

		AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = -vRight * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos - vFloatRight * 3.3f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}
	else if (Simba_AttackJumpHit == iAnimIdx)
	{
		_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;

		AbilityItemDesc.fRotateDir = 1.f;
		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos + vFloatRight * 4.5f + vFloatLook * 2.2f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);

		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos + vFloatLook * 2.7f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);

		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos - vFloatRight * 4.5f + vFloatLook * 2.2f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}
	else if (Simba_DoubleClaw == iAnimIdx)
	{
		_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;

		if(0 == CUtils::Make_RandomInt(0, 1))
			AbilityItemDesc.fRotateDir = 1.f;
		else
			AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos + vFloatRight * 2.2f + vFloatLook * 3.5f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);

		if (0 == CUtils::Make_RandomInt(0, 1))
			AbilityItemDesc.fRotateDir = 1.f;
		else
			AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos + vFloatLook * 4.2f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);

		if (0 == CUtils::Make_RandomInt(0, 1))
			AbilityItemDesc.fRotateDir = 1.f;
		else
			AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);
		AbilityItemDesc.vPosition = vPos - vFloatRight * 2.2f + vFloatLook * 3.5f;
		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}
	else if (Simba_DimensionClaw == iAnimIdx || Simba_DimensionClawContinue == iAnimIdx)
	{
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		if (0 == CUtils::Make_RandomInt(0, 1))
			AbilityItemDesc.fRotateDir = 1.f;
		else
			AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = XMVectorZero();
		
		if (true == m_bDimensionClawUpAttack)
		{
			if (0 == m_iStarCount) {
				AbilityItemDesc.vPosition = vPos + vFloatLook * 8.f + vFloatRight * 8.f;
				AbilityItemDesc.vPosition.y = fY;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);

				if (0 == CUtils::Make_RandomInt(0, 1))
					AbilityItemDesc.fRotateDir = 1.f;
				else
					AbilityItemDesc.fRotateDir = -1.f;
				AbilityItemDesc.vPosition = vPos + vFloatLook * 8.f - vFloatRight * 8.f;
				AbilityItemDesc.vPosition.y = fY;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);
			}
				
			else if (1 == m_iStarCount) {
				AbilityItemDesc.vPosition = vPos + vFloatLook * 18.f + vFloatRight * 8.f;
				AbilityItemDesc.vPosition.y = fY;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);

				if (0 == CUtils::Make_RandomInt(0, 1))
					AbilityItemDesc.fRotateDir = 1.f;
				else
					AbilityItemDesc.fRotateDir = -1.f;
				AbilityItemDesc.vPosition = vPos + vFloatLook * 18.f - vFloatRight * 8.f;
				AbilityItemDesc.vPosition.y = fY;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);
			}
		}
		else
		{
			AbilityItemDesc.vDir = vLook * CUtils::Make_RandomFloat(0.2f, 0.7f);

			if (0 == m_iStarCount)
				AbilityItemDesc.vPosition = vPos + vFloatLook * 8.f;
			else if (1 == m_iStarCount)
				AbilityItemDesc.vPosition = vPos + vFloatLook * 18.f;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
			CHECK_FAILED(hr);
		}
	}
	else if (Simba_BiteRush == iAnimIdx)
	{
		if (true == m_bBiteRushSpawnStarAtLeft)
		{
			AbilityItemDesc.fRotateDir = -1.f;
			AbilityItemDesc.vDir = -vFloatRight * CUtils::Make_RandomFloat(0.05f, 0.2f);
			AbilityItemDesc.vPosition = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) - vFloatRight * 1.5f - vFloatLook * 2.2f;
		}
		else
		{
			AbilityItemDesc.fRotateDir = 1.f;
			AbilityItemDesc.vDir = vFloatRight * CUtils::Make_RandomFloat(0.05f, 0.2f);
			AbilityItemDesc.vPosition = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone) + vFloatRight * 1.5f - vFloatLook * 2.f;
		}

		AbilityItemDesc.vPosition.y = fY - 0.2f;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}
	else if (Simba_DimensionLaser)
	{
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		if (0 == CUtils::Make_RandomInt(0, 1))
			AbilityItemDesc.fRotateDir = 1.f;
		else
			AbilityItemDesc.fRotateDir = -1.f;
		AbilityItemDesc.vDir = XMVectorZero();

		if (0 == m_iStarCount)
			AbilityItemDesc.vPosition = vPos + vFloatLook * 4.f;
		else if (1 == m_iStarCount)
			AbilityItemDesc.vPosition = vPos + vFloatLook * 9.f;
		else if (2 == m_iStarCount)
			AbilityItemDesc.vPosition = vPos + vFloatLook * 15.f;
		else if (3 == m_iStarCount)
			AbilityItemDesc.vPosition = vPos + vFloatLook * 22.f;

		AbilityItemDesc.vPosition.y = fY;
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
		CHECK_FAILED(hr);
	}

	m_iStarCount++;
}

_bool CSimba::IsKirbyOnMyLeft()
{
	if (nullptr == m_pKirby)
		return false;

	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	CTransform* pKirbyTransform = m_pKirby->Get_TransformCom();
	if (nullptr == pKirbyTransform)
		return false;
	_vector vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);

	_vector vDir = vKirbyPos - vPos;
	vDir = XMVector3Normalize(XMVectorSetY(vDir, 0));

	_vector crossProduct = XMVector3Cross(vLook, vDir);
	_float fCrossResultY = XMVectorGetY(crossProduct);

	if (fCrossResultY > 0.f)
		return true;
	else
		return false;
}

void CSimba::SetUpDimensionClawWorldMatrix()
{
	if (nullptr == m_pDimensionClawActor)
		return;

	_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;
	if (true == m_bDimensionClawUpAttack)
		vPos.y = 6.5f;
	else
		vPos.y = 0.8f;
	_float4x4 matWorld = m_pTransformCom->Get_WorldMatrix();
	memcpy(&matWorld.m[3], &vPos, sizeof(_float4));

	m_pDimensionClawActor->setKinematicTarget(CUtils::ToPxTransform(matWorld));
}

void CSimba::MoveDimensionClaw(_float fTimeDelta)
{
	CUtils::MoveActor(m_pDimensionClawActor, _float3(0, 0, 20), m_fTimeDelta);
}

void CSimba::HideDimensionClawActor()
{
	if(nullptr != m_pDimensionClawActor)
		m_pDimensionClawActor->setGlobalPose(PxTransform(0, 0, 0));
}

void CSimba::HideDimensionLaserActor()
{
	if (nullptr != m_pSimbaLaser)
		static_cast<CSimbaLaser*>(m_pSimbaLaser)->HideLaser();
}

void CSimba::SpawnRocks(_uint iAnimIdx)
{
	_vector vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vFloatLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vFloatRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);

	if (Simba_QuickClawL == iAnimIdx || Simba_QuickClaw2L == iAnimIdx || Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
	{
		_float4 vPos{};
		if(Simba_QuickClawL == iAnimIdx || Simba_QuickClaw2L == iAnimIdx)
			vPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		else if(Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
			vPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);

		_uint iNumRocks = 25;
		for (_uint i = 0; i < iNumRocks; i++)
		{
			_uint index = i + m_iRockCount * iNumRocks;
			if (Simba_QuickClaw2L == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
				index += 100;
			_float4 vOffset = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(0, 360)) * CUtils::Make_RandomFloat(0.f, 3.6f);
			_float4 vResultPos = vPos + vOffset + vFloatLook;
			vResultPos.y = CUtils::Make_RandomFloat(1.9f, 2.4f);

			m_vecSimbaRocks[index]->SetUpSimbaRock(vResultPos);
			m_listUsedRocks.push_back(index);
		}
	}
	else if (Simba_FinalCrusher == iAnimIdx)
	{
		_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;
		vPos.y = CUtils::Make_RandomFloat(1.9f, 2.4f);

		_uint iNumRocks = 40;
		for (_uint i = 0; i < iNumRocks; i++)
		{
			_uint index = i + m_iRockCount * iNumRocks;
			_float fDis{};
			if (0 == m_iRockCount)
				fDis = CUtils::Make_RandomFloat(1.5f, 2.2f);
			else if (1 == m_iRockCount)
				fDis = CUtils::Make_RandomFloat(2.2f, 3.f);
			else if (2 == m_iRockCount)
				fDis = CUtils::Make_RandomFloat(3.f, 3.8f);
			_float4 vOffset = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(0, 360)) * fDis;
			m_vecSimbaRocks[index]->SetUpSimbaRock(vPos + vOffset + vFloatLook);
			m_listUsedRocks.push_back(index);
		}
	}
	else if (Simba_DoubleClaw == iAnimIdx)
	{
		_float4 vLeftHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		_float4 vRightHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);

		_uint iNumRocks = 3;
		for (_uint i = 0; i < iNumRocks; i++)
		{
			_uint index = i + m_iRockCount * iNumRocks;
			_float4 vOffset = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(0, 360)) * CUtils::Make_RandomFloat(0.f, 1.2f);
			_float4 vResultPos = vLeftHandPos + vOffset + vFloatLook;
			vResultPos.y = CUtils::Make_RandomFloat(1.9f, 2.4f);

			m_vecSimbaRocks[index]->SetUpSimbaRock(vResultPos);
			m_listUsedRocks.push_back(index);

			_float4 vOffset2 = CUtils::TurnDirectionVector(vLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(0, 360)) * CUtils::Make_RandomFloat(0.f, 1.2f);
			_float4 vResultPos2 = vRightHandPos + vOffset2 + vFloatLook;
			vResultPos2.y = CUtils::Make_RandomFloat(1.9f, 2.4f);
			index += 200;
			m_vecSimbaRocks[index]->SetUpSimbaRock(vResultPos2);
			m_listUsedRocks.push_back(index);
		}
	}
	else if (Simba_AttackJumpHit == iAnimIdx)
	{
		_float4 vLeftHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		_float4 vRightHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);

		_uint iNumRocks = 50;
		for (_uint i = 0; i < iNumRocks; i++)
		{
			_uint index = i + m_iRockCount * iNumRocks;
			_float fRightFactor = CUtils::Make_RandomFloat(-3.f, 3.f);
			_float4 fRightOffset = vFloatRight * fRightFactor;
			
			_float4 vLookOffset = vFloatLook * CUtils::Make_RandomFloat(1.5f, 2.6f) * sqrt(3.f - abs(fRightFactor)) * 0.8f;
			_float4 vOffset = vLookOffset + fRightOffset + vFloatLook * 1.2f;

			if (0 == m_iRockCount) {
				
				_float4 vResultPos = vLeftHandPos + vOffset;
				vResultPos.y = CUtils::Make_RandomFloat(1.9f, 2.4f);
				m_vecSimbaRocks[index]->SetUpSimbaRock(vResultPos);
				m_listUsedRocks.push_back(index);
			}
				
			else if (1 == m_iRockCount) {
				_float4 vResultPos = vRightHandPos + vOffset;
				vResultPos.y = CUtils::Make_RandomFloat(1.9f, 2.4f);

				_uint iNewIndex = index + 100;
				m_vecSimbaRocks[iNewIndex]->SetUpSimbaRock(vResultPos);
				m_listUsedRocks.push_back(iNewIndex);
			}
		}
	}

	m_iRockCount++;
}

void CSimba::SpawnDebris(_uint iAnimIdx)
{
	if (Simba_QuickClawL == iAnimIdx)
	{

	}
	else if (Simba_QuickClawR == iAnimIdx)
	{

	}
	else if (Simba_QuickClaw2L == iAnimIdx)
	{

	}
	else if (Simba_QuickClaw2R == iAnimIdx)
	{

	}
	else if (Simba_FinalCrusher == iAnimIdx)
	{

	}
	else if (Simba_DoubleClaw == iAnimIdx)
	{

	}
	else if (Simba_AttackJumpHit == iAnimIdx)
	{

	}
	else if (Simba_DimensionLaser == iAnimIdx)
	{

	}

	m_iDebrisCount++;
}

void CSimba::SetUpSecondTarget()
{
	CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
	if (pCamera != nullptr)
		pCamera->Set_Target(m_pTransformCom, CCamera::TARGET_SECOND, CCamera::FOCUS_BOTH);
}

void CSimba::QuickClawNailFlash(_uint eSimbaAnim)
{
	if (Simba_QuickClawStartL == eSimbaAnim) // YW : Effect 영우형 여기임 왼쪽 검지손톱 번쩍
	{
		_float3 vOffset = _float3(); // Right Up Look 오프셋 계수
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(m_vecLeftNailBones[INDEX], vOffset);

	}
	else if (Simba_QuickClawStartR == eSimbaAnim) // YW : Effect 영우형 여기임 오른쪽 검지손톱 번쩍
	{
		_float3 vOffset = _float3(); // Right Up Look 오프셋 계수
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(m_vecRightNailBones[INDEX], vOffset);

	}
}

void CSimba::QuickClawNailTrail() // YW : Effect 영우형 여기임 왼쪽 검지손톱 번쩍
{
}

void CSimba::FinalCrusherSwing() // YW : Effect 영우형 여기임 양주먹 내려치기시작
{
}

void CSimba::FinalCrusherSmash() // YW : Effect 영우형 여기임 양주먹 바닥에 찍는 타이밍
{
	
}

void CSimba::JumpStartSmoke() // YW : Effect 영우형 여기임 점프 시작할때 회색방구
{
}

void CSimba::LandingSmoke() // YW : Effect 영우형 여기임 점프 후 착지 회색방구
{
}

void CSimba::AttackJumpWind() // YW : Effect 영우형 여기임 점프 공격할때 주위 바람 
{
}

void CSimba::DoubleClawDashGround() // YW : Effect 영우형 여기임 양손으로 바닥 계속 긁을때 튀기는 작은 불씨들 (아직은 이 함수 호출 안함)
{
}

void CSimba::DoubleClawGround() // YW : Effect 영우형 여기임 양슨으로 바닥 긁다가 공격이펙트 직전 튀기는 큰 불씨들 (아직은 이 함수 호출 안함)
{
}

void CSimba::DoubleClawSweep()// YW : Effect 영우형 여기임 바닥 긁다가 순간적으로 공격 이펙트 (트레일, 불꽃)
{
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
	tAttack.pCollisionType = HITBOX_SIMBA;
	tAttack.pSocket = m_pModelCom->Get_BonePtr("L_HaveL");
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

	Activate_FrustumCollider(0.f, 8.f, 150.f, ATTACK);
	Activate_FrustumCollider(0.f, 8.f, 150.f, ATTACK2);
	Activate_FrustumCollider(0.f, 11.5f, 150.f, ATTACK3);

	m_pSimbaLaser = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_SimbaLaser"));
	m_pSimbaLaserTransform = m_pSimbaLaser->Get_TransformCom();
	Safe_AddRef(m_pSimbaLaserTransform);

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

void CSimba::PlayPartialAnimation()
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

	m_pFSM->Add_State(Simba_Wait2, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Jump, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_JumpStart, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Fall, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Landing, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for(_uint i = Simba_BackStep; i <= Simba_BackStepStart; i++)
		m_pFSM->Add_State(i, CSimba_BackStep::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for (_uint i = Simba_AttackJump; i <= Simba_AttackJumpWait; i++) {
		m_pFSM->Add_State(i, CSimba_AttackJump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_mapRotation[ATTACKJUMP].insert(SIMBA_ANIM(i));
	}
		
	m_pFSM->Add_State(Simba_Damage, CSimba_Damage::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Roar2, CSimba_Roar::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for (_uint i = Simba_BiteRushFallL; i <= Simba_BiteRushLandingR; i++) {
		m_pFSM->Add_State(i, CSimba_BiteRushJump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_mapRotation[BITERUSHJUMP].insert(SIMBA_ANIM(i));
	}
	
	for(_uint i = Simba_DimensionClaw; i<= Simba_DimensionClawWait; i++)
		m_pFSM->Add_State(i, CSimba_DimensionClaw::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for (_uint i = Simba_BiteRush; i <= Simba_BiteRushEnd; i++) {
		m_pFSM->Add_State(i, CSimba_BiteRush::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_mapRotation[BITERUSH].insert(SIMBA_ANIM(i));
	}	
	for (_uint i = Simba_BiteRushStart; i <= Simba_BiteRushTiredWait; i++){
		m_pFSM->Add_State(i, CSimba_BiteRush::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_mapRotation[BITERUSH].insert(SIMBA_ANIM(i));
	}

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

	for (_uint i = 0; i < 10; i++)
	{
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.007f, false, ATTACK));
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.006f, false, ATTACK2));
		vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(i * 0.007f, false, ATTACK3));
	}
	InsertHitboxActivationTiming(Simba_AttackJumpHit, vecTiming);
	vecTiming.clear();

	vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(0.f, false, ATTACK3));
	vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(0.475f, false, ATTACK3));
	vecTiming.emplace_back(tuple<_float, _bool, COLLISION_VALUE>(0.95f, false, ATTACK3));
	InsertHitboxActivationTiming(Simba_BiteRush, vecTiming);
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

void CSimba::PlayLipSinc()
{
	if (m_setAppear1Anims.end() != m_setAppear1Anims.find(SIMBA_ANIM(Get_State())) && m_pGameInstance->Get_KeyState(DIK_A, KEY_DOWN)) {
		if (false == m_bPlayPartialAnim) {
			m_pModelCom->Reset_PartialAnimation(Simba_LipSyncSub, 50.f, false, false);
			m_bPlayPartialAnim = true;
		}
	}
}

void CSimba::TransformToDefault(_float fOffsetY)
{
	_float4x4 matWorld = m_matDefault;
	//matWorld._42 = m_matDefault._42 - 2.005f;
	matWorld._42 = m_matDefault._42 - 1.97f;
	m_pTransformCom->Set_WorldMatrix(matWorld);
	m_pControllerCom->Set_Position(m_pTransformCom, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 6.f, fOffsetY);
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
	SetCamSequence(CCamera_Main::SEQ_SIMBA_SHOULDER);
}

void CSimba::OnNextDialog2(CGameObject* pObj)
{
	Change_State(Simba_DemoAppear1Cut3, 66.66f, false, true);
	TransformToDefault(0);
	SetCamSequence(CCamera_Main::SEQ_SIMBA_FRONTVIEW);
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
	TriggerMonsterSpawning(11);
}

void CSimba::OnWave1Dead(CGameObject* pObj)
{
	TriggerMonsterSpawning(12);
}

void CSimba::OnWave2Dead(CGameObject* pObj)
{
	m_bRenderMant = true;
	m_bRenderEyeLid = false;
	Change_State(Simba_DemoAppear2Cut1, 66.66f, false, false);

	CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
	if (pCamera != nullptr)
		pCamera->Make_Sequence(CCamera_Main::SEQ_SIMBA_BATTLESTART);

	TransformToDefault(-0.3f);
}

void CSimba::TriggerMonsterSpawning(_uint iTriggerIndex)
{
	if (11 == iTriggerIndex)
		m_bSummon1 = true;
	else if (12 == iTriggerIndex)
		m_bSummon2 = true;
}

void CSimba::SpawnMonsters(_uint iTriggerIndex)
{
	wstring wstrLayerTag = TEXT("Layer_Wave");
	if (11 == iTriggerIndex)
		wstrLayerTag += TEXT("1");
	else if (12 == iTriggerIndex)
		wstrLayerTag += TEXT("2");

	wstring wstrPrototypeTag = TEXT("Prototype_GameObject_");
	wstring wstrMonsterName;

	if (11 == iTriggerIndex) {
		if (0 == m_iMonsterCount)
			wstrMonsterName = TEXT("Awoofy");
		else if (1 == m_iMonsterCount)
			wstrMonsterName = TEXT("AwoofyWild");
	}
	else if (12 == iTriggerIndex)
	{
		if (0 == m_iMonsterCount)
			wstrMonsterName = TEXT("Awoofy"); // AwoofyWild로 바꿔야할수도 흠
		else if (1 == m_iMonsterCount)
			wstrMonsterName = TEXT("Rabbit");
		else if (2 == m_iMonsterCount)
			wstrMonsterName = TEXT("RabbitBig");
	}

	_float fY{}, fScaleOffset{};
	if (TEXT("Awoofy") == wstrMonsterName || TEXT("Rabbit") == wstrMonsterName) {
		fY = 2.4f;
		fScaleOffset = 1.f;
	}
	else if (TEXT("AwoofyWild") == wstrMonsterName) {
		fY = 2.7f;
		fScaleOffset = 1.1f;
	}
	else if (TEXT("RabbitBig") == wstrMonsterName) {
		fY = 2.4f;
		fScaleOffset = 1.2f;
	}
		
	for (auto& monsterDesc : m_vecMonsterDescs)
	{
		if (iTriggerIndex == monsterDesc.eMonState)
		{
			if (monsterDesc.wstrModelName != wstrMonsterName)
				continue;

			monsterDesc.matWorld._42 = fY;

			wstring wstrTag;
			if (TEXT("Awoofy") == monsterDesc.wstrModelName || TEXT("AwoofyWild") == monsterDesc.wstrModelName)
				wstrTag = wstrPrototypeTag + TEXT("Awoofy");
			else if (TEXT("Rabbit") == monsterDesc.wstrModelName || TEXT("RabbitBig") == monsterDesc.wstrModelName) {
				wstrTag = wstrPrototypeTag + TEXT("Rabbit");
				monsterDesc.matWorld._42 = 2.f;
			}

			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SIMBA, wstrLayerTag, wstrTag, &monsterDesc)))
				return;

			CEffect::FX_DESC FXDesc{};
			_float3 vMonPos = _float3(monsterDesc.matWorld._41, fY + 1.f, monsterDesc.matWorld._43);
			_float3 vRight = _float3(monsterDesc.matWorld._11, monsterDesc.matWorld._12, monsterDesc.matWorld._13);
			_float3 vUp = _float3(monsterDesc.matWorld._21, monsterDesc.matWorld._22, monsterDesc.matWorld._23);
			_float3 vLook = _float3(monsterDesc.matWorld._31, monsterDesc.matWorld._32, monsterDesc.matWorld._33);
			_float fAngle = { 0.f };
			for (_uint i = 0; i < 3; ++i)
			{
				_float fDistance = CUtils::Make_RandomFloat(0.3f, 0.8f);
				_float fRandAngle = CUtils::Make_RandomFloat(0.f, 90.f);
				vRight.Normalize();
				_float3 vRotateRight = CUtils::TurnDirectionVector(vRight, vLook, ((_float)i * 120.f) + fRandAngle);
				FXDesc.vInitPos = vMonPos + fDistance * vRotateRight + vLook;
				FXDesc.vInitRot = { fRandAngle, 0.f, fRandAngle };
				FXDesc.vInitScale = { fDistance + fScaleOffset, fDistance + fScaleOffset, fDistance + fScaleOffset };
				//FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

				Add_Effect("BbongJS", FXDesc, false);
			}
		}
	}

	m_iMonsterCount++;

	if (11 == iTriggerIndex && 1 < m_iMonsterCount) {
		m_bSummon1 = false;
		m_fSpawnTime = 0.f;
		m_iEffectCount = 0;
		m_iMonsterCount = 0;
	}
	else if (12 == iTriggerIndex && 2 < m_iMonsterCount) {
		m_bSummon2 = false;
		m_fSpawnTime = 0.f;
		m_iEffectCount = 0;
		m_iMonsterCount = 0;
	}
}

void CSimba::SpawnEffects(_uint iTriggerIndex)
{
	wstring wstrLayerTag = TEXT("Layer_Effect");
	if (11 == iTriggerIndex)
		wstrLayerTag += TEXT("1");
	else if (12 == iTriggerIndex)
		wstrLayerTag += TEXT("2");

	wstring wstrPrototypeTag = TEXT("Prototype_GameObject_");
	HRESULT hr{};
	CSummonEffect::SUMMONEFFECT_DESC SummonEffectDesc = {};

	wstring wstrMonsterName;

	if (11 == iTriggerIndex) {
		if (0 == m_iEffectCount)
			wstrMonsterName = TEXT("Awoofy");
		else if(1 == m_iEffectCount)
			wstrMonsterName = TEXT("AwoofyWild");
	}
	else if (12 == iTriggerIndex)
	{
		if (0 == m_iEffectCount)
			wstrMonsterName = TEXT("Awoofy"); // AwoofyWild로 바꿔야할수도 흠
		else if (1 == m_iEffectCount)
			wstrMonsterName = TEXT("Rabbit");
		else if(2 == m_iEffectCount)
			wstrMonsterName = TEXT("RabbitBig");
	}

	_float fY{};
	if (TEXT("Awoofy") == wstrMonsterName || TEXT("Rabbit") == wstrMonsterName || TEXT("RabbitBig") == wstrMonsterName)
		fY = 2.4f;
	else if (TEXT("AwoofyWild") == wstrMonsterName )
		fY = 2.7f;

	for (auto& monsterDesc : m_vecMonsterDescs)
	{
		if (iTriggerIndex == monsterDesc.eMonState)
		{
			if (monsterDesc.wstrModelName != wstrMonsterName)
				continue;

			_float fScale = { 0.f };
			wstring wstrTag;
			if (TEXT("Awoofy") == monsterDesc.wstrModelName || TEXT("Rabbit") == monsterDesc.wstrModelName)
				fScale = 3.6f;
			else if (TEXT("AwoofyWild") == monsterDesc.wstrModelName || TEXT("RabbitBig") == monsterDesc.wstrModelName)
				fScale = 5.8f;

			SummonEffectDesc.vPosition = _float4(monsterDesc.matWorld._41, fY + 1.f, monsterDesc.matWorld._43, monsterDesc.matWorld._44);
			SummonEffectDesc.fScale = fScale;
			SummonEffectDesc.fAlpha = 0.9f;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), wstrLayerTag, TEXT("Prototype_GameObject_SummonEffect"), &SummonEffectDesc);
			CHECK_FAILED(hr);
		}
	}

	m_iEffectCount++;
}

void CSimba::CheckSpawning()
{
	if (true == m_bSummon1)
	{
		m_fSpawnTime += m_fTimeDelta;

		if (0.f < m_fSpawnTime && 0 == m_iEffectCount)
			SpawnEffects(11);
		else if(0.7f < m_fSpawnTime && 1 == m_iEffectCount)
			SpawnEffects(11);

		if (1.2f < m_fSpawnTime && 0 == m_iMonsterCount)
			SpawnMonsters(11);
		else if (1.9f < m_fSpawnTime && 1 == m_iMonsterCount)
			SpawnMonsters(11);
	}
	else if (true == m_bSummon2)
	{
		m_fSpawnTime += m_fTimeDelta;

		if (0.f < m_fSpawnTime && 0 == m_iEffectCount)
			SpawnEffects(12);
		else if (0.5f < m_fSpawnTime && 1 == m_iEffectCount)
			SpawnEffects(12);
		else if (1.1f < m_fSpawnTime && 2 == m_iEffectCount)
			SpawnEffects(12);

		if (1.2f < m_fSpawnTime && 0 == m_iMonsterCount)
			SpawnMonsters(12);
		else if (1.7f < m_fSpawnTime && 1 == m_iMonsterCount)
			SpawnMonsters(12);
		else if (2.3f < m_fSpawnTime && 2 == m_iMonsterCount)
			SpawnMonsters(12);
	}
}

void CSimba::DetermineSimbaRotation()
{
	m_fAngle = 0.f;
	_uint iState = Get_State();
	_float fAnimRatio = m_pModelCom->Get_AnimRatio();

	for (_uint i = 0; i < ROTATION_END; i++)
	{
		if (m_mapRotation[i].end() != m_mapRotation[i].find(SIMBA_ANIM(iState)))
		{
			if (ATTACKJUMP == i) {
				TurnSimba(AttackJump);
				break;
			}
			else if (BITERUSH == i)
			{
				TurnSimba(BiteRush);
				break;
			}
			else if (BITERUSHJUMP == i)
			{
				TurnSimba(BiteRushJump);
				break;
			}
		}
	}

	if (Simba_Damage == iState)
		TurnSimba(-2.5f);

	if (Simba_DimensionLaserStart == iState)
	{
		if (0.2f > fAnimRatio) {
			_float fRatio = RATIO(fAnimRatio, 0.f, 0.2f);
			_float fStart = BiteRush;
			fRatio = EASE_OUT(fRatio);
			_float fAngle = LERP(fStart, 0, fRatio);
			TurnSimba(fAngle);
		}
	}
}

void CSimba::TurnSimba(_float fAngle)
{
	fAngle *= 0.95f;
	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), -1, fAngle);
	m_fAngle = fAngle;
}

void CSimba::ResetRotation()
{
	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), -1, -m_fAngle);
}

void CSimba::LaserAttack()
{
	_float4x4 matWorld = m_pTransformCom->ComputeBoneWorldMatrix(m_pLaserBone);
	_float fScale = 24.f;
	CUtils::Set_Scaled_Matrix(matWorld, fScale, fScale, fScale);
	_float4 vLook{};
	memcpy(&vLook, &(matWorld.m[2]), sizeof(_float4));
	vLook = _float4(-vLook.x, -vLook.y, -vLook.z, 0);
	memcpy(&(matWorld.m[2]), &vLook, sizeof(_float4));
	m_pSimbaLaserTransform->Set_WorldMatrix(matWorld);
	m_pSimbaLaser->Activate_Attack();
}

void CSimba::CreateDimensionClawActor()
{
	auto pPhysics = m_pGameInstance->Get_Physics();
	PxMaterial* pMtrl = m_pGameInstance->Get_Material();
	
	PxTransform transform(PxVec3(0, 0, 0));
	PxRigidDynamic* pRigidDynamic = pPhysics->createRigidDynamic(transform);
	PxBoxGeometry boxGeometry(10.f, 1.f, 5.f);
	
	PxQuat rotation1(XMConvertToRadians(35), PxVec3(0, 0, 1)); // z축기준 35도 회전
	PxQuat rotation2(-XMConvertToRadians(35), PxVec3(0, 0, 1)); // z축기준 35도 회전
	PxTransform transform1(PxVec3(0.f, 0.f, 0.f), rotation1);
	PxTransform transform2(PxVec3(0.f, 0.f, 0.f), rotation2);

	PxShape* pShape1 = pPhysics->createShape(boxGeometry, *pMtrl);
	pShape1->setLocalPose(transform1);
	pShape1->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	pShape1->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	pShape1->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	pRigidDynamic->attachShape(*pShape1);

	PxShape* pShape2 = pPhysics->createShape(boxGeometry, *pMtrl);
	pShape2->setLocalPose(transform2);
	pShape2->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	pShape2->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
	pShape2->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
	pRigidDynamic->attachShape(*pShape2);
	
	m_pGameInstance->AddActor(*pRigidDynamic);
	m_pDimensionClawActor = pRigidDynamic;
	m_pDimensionClawActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	m_pGameInstance->Register_Trigger(m_pDimensionClawActor, TRIGGER_SIMBA_ATTACK, 0);

	function<void(_int)> func = bind(&CSimba::OnSimbaAttackTrigger, this);
	m_pGameInstance->Emplace_TriggerFunc(TRIGGER_SIMBA_ATTACK, func);

	pShape1->release();
	pShape2->release();
}

void CSimba::OnSimbaAttackTrigger()
{
	CKirby* pKirby = static_cast<CKirby*>(m_pKirby);

	if (true == CCollisionCenter::Get_Instance()->Kirby_Dodge_SlowMotionSystem(pKirby))
		return;
	
	if (pKirby->isOverPower() == false) // 무적이 아닐 경우
	{
		CTransform* pKirbyTransform = m_pKirby->Get_TransformCom();
		_vector vKirbyPos = pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION);
		_float4 vDistance = vKirbyPos - GET_POS;
		vDistance.y = 0.f;
		vDistance.Normalize();

		_float4 vNewDir{};
		_float4 vSimbaRight = m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
		vSimbaRight.Normalize();

		if (true == IsKirbyOnMyLeft())
			vNewDir = vDistance + vSimbaRight * 2.5f;
		else
			vNewDir = vDistance - vSimbaRight * 2.5f;
		vNewDir.Normalize();
		_vector vKnockbackDir = vNewDir;

		pKirby->Set_DamageMoving(vKnockbackDir * 2.3f, 8.2f); // 심바 전용 넉백

		_float fMonsterAttack = Get_Attack();
		pKirby->Minus_Hp(fMonsterAttack);
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		pCamera->Make_Shake(1.2f, 0.5f, _float2(0.f, -1.f));

		pKirby->Collision(CCollisionCenter::CONTENT_ATTACK, this);
	}
}

void CSimba::RemoveDeadRocksFromList()
{
	for (auto& iter = m_listUsedRocks.begin(); iter != m_listUsedRocks.end();)
	{
		if (true == m_vecSimbaRocks[*iter]->Get_Hide())
			iter = m_listUsedRocks.erase(iter);
		else
			iter++;
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
	m_pGameInstance->ReleaseActor(m_pDimensionClawActor);
	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();

	for (auto& simbaRock : m_vecSimbaRocks)
		Safe_Release(simbaRock);
	for (auto& debris : m_vecDebris)
		Safe_Release(debris);

	for (auto& bone : m_vecLeftNailBones)
		Safe_Release(bone);
	for (auto& bone : m_vecRightNailBones)
		Safe_Release(bone);

	Safe_Release(m_pLipBone);
	Safe_Release(m_pLaserBone);
	Safe_Release(m_pLeftHandBone);
	Safe_Release(m_pRightHandBone);
	Safe_Release(m_pKirby);
	Safe_Release(m_pSimbaLaserTransform);
	Safe_Release(m_pSimbaLaser);

	for(_uint i = 0; i < EYETEX_END; i++)
		Safe_Release(m_pEyeTextureCom[i]);
}