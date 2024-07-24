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
#include "Fire.h"

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

	m_fMaxHp = 320.f;
	m_fHp = 320.f;
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

#pragma region 애니메이션 관련
	m_pModelCom->Set_Animation(Simba_DemoAppear1Cut2, 66.66f, false, false);

	m_pModelCom->EmplaceBackPartialAnim(Simba_DamageFaceSub);
	m_pModelCom->EmplaceBackPartialAnim(Simba_LipSyncSub);
	m_pModelCom->EmplaceBackPartialAnim(Simba_LipSyncSubA);

	m_setAppear1Anims = { Simba_DemoAppear1Cut2, Simba_DemoAppear1Cut2Wait, Simba_DemoAppear1Cut3, Simba_DemoAppear1Cut3Wait,
		Simba_DemoAppear1Cut4, Simba_DemoAppear1Cut4Wait };

	m_setUndamagableAnims = { Simba_Death, Simba_DemoDeadCut1, Simba_DemoDeadCut2, Simba_Roar2, Simba_Damage };

	m_setResetRequiredAnims = { Simba_AttackJumpHit, Simba_BiteRush, Simba_DimensionClaw, Simba_DimensionClawContinue,
		Simba_DimensionLaser, Simba_DoubleClaw, Simba_FinalCrusher, Simba_QuickClawL, Simba_QuickClawR, Simba_QuickClaw2L, Simba_QuickClaw2R };

#pragma endregion

#pragma region 이벤트 함수포인터 바인딩
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
#pragma endregion

	SetUpHitBoxTimings();

	Set_Slope(false);

	m_matDefault = m_pTransformCom->Get_WorldFloat4x4();

#pragma region 뼈 포인터
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

	m_pLeftFootBone = m_pModelCom->Get_BonePtr("L_ToeJ");
	Safe_AddRef(m_pLeftFootBone);
	m_pRightFootBone = m_pModelCom->Get_BonePtr("R_ToeJ");
	Safe_AddRef(m_pRightFootBone);
	m_pMouthBone = m_pModelCom->Get_BonePtr("T_MouthJ");
	Safe_AddRef(m_pMouthBone);
#pragma endregion

	SetCamSequence(CCamera_Main::SEQ_SIMBA_START);

	CreateDimensionClawActor();

#pragma region 돌, 파티클 풀링
	vector<_uint> vecTunnelRocks = { 2, 4, 5, 7, 8, 9, 10, 12, 13, 16 };
	GAMEOBJECT_DESC tDesc{};

	for (_uint i = 0; i < 40; i++) {
		for (auto& rockIdx : vecTunnelRocks) {

			tDesc.wstrModelName = TEXT("TunnelRock") + to_wstring(rockIdx);
			m_vecSimbaRocks.emplace_back(dynamic_cast<CSimbaRock*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_SimbaRock"), &tDesc)));
			m_vecDebris.emplace_back(dynamic_cast<CDebris*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Debris"), &tDesc)));
		}
	}
#pragma endregion

	return S_OK;
}

_int CSimba::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_matLeftHand = m_pTransformCom->ComputeBoneWorldMatrix(m_pLeftHandBone); // 소켓용
	m_matRightHand = m_pTransformCom->ComputeBoneWorldMatrix(m_pRightHandBone); // 소켓용
	m_matLip = m_pTransformCom->ComputeBoneWorldMatrix(m_pLipBone); // Socket Dragon
	_float4 vMouthPos = m_pTransformCom->ComputeBoneWorldPos(m_pMouthBone);
	memcpy(&(m_matMouth.m[3]), &vMouthPos, sizeof(vMouthPos));

	m_fHpRatio = m_fHp / m_fMaxHp;

	ResetRotation();
	m_bRenderDimensionClaw = false;
	m_bLaserActivated = false;
	m_bStateChanged = false;
	m_bRenderRing = false;

	if (true == m_pModelCom->IsFinished() || m_pModelCom->Get_Trackposition() == 0.f) // IsAnimFinished
		Reset_HitBoxTimingMap(SIMBA_ANIM(m_pModelCom->Get_CurAnimIndex()));

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CheckSpawning();

	if (m_pGameInstance->Get_KeyState(DIK_CAPSLOCK, KEY_PRESS))
	{
		if (m_pGameInstance->Get_KeyState(DIK_1, KEY_DOWN)) {
			//m_bEyeBloom = true;
			//m_eEyeState = SIMBAEYE_NONE; // 디버깅용
			if (0 == CUtils::Make_RandomInt(0, 1))
				Change_State(Simba_QuickClawStartL, 50.f, false, true);
			else
				Change_State(Simba_QuickClawStartR, 50.f, false, true);
		}

		else if (m_pGameInstance->Get_KeyState(DIK_2, KEY_DOWN))
			Change_State(Simba_FinalCrusherStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_3, KEY_DOWN))
			Change_State(Simba_DoubleClawChargeStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_4, KEY_DOWN))
			Change_State(Simba_AttackJumpPre, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_5, KEY_DOWN))
			Change_State(Simba_DimensionClawStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_6, KEY_DOWN))
			Change_State(Simba_BiteRushStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_7, KEY_DOWN))
			Change_State(Simba_DimensionLaserStart, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_8, KEY_DOWN))
			Change_State(Simba_Wait2, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_9, KEY_DOWN))
			Change_State(Simba_BiteRushJumpStartL, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_0, KEY_DOWN))
			Change_State(Simba_BiteRushJumpStartR, 50.f, false, true);
		else if (m_pGameInstance->Get_KeyState(DIK_GRAVE, KEY_DOWN))
			Change_State(Simba_Death, 50.f, false, true);

		if (true == m_bStateChanged) {
			SetUpSecondTarget();
			SIMBA_ANIM eState = SIMBA_ANIM(m_pModelCom->Get_CurAnimIndex());
			Reset_HitBoxTimingMap(eState);
			if (m_setDimensionClawAnims.end() == m_setDimensionClawAnims.find(eState))
				HideDimensionClawActor();
			if (m_setDimensionLaserAnims.end() == m_setDimensionLaserAnims.find(eState))
				HideDimensionLaserActor();
		}
	}

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

	Check_HitBoxActivation();

	PlayLipSinc();

	if (m_fHp <= 0.f && false == m_bDeathAnimPlayed)
	{
		m_bDeathAnimPlayed = true;
		TransformToDefault(0.f);
		Change_State(Simba_DemoDeadCut1, 50.f, false, true);
		Set_SimbaEye(CSimba::SIMBAEYE_NONE);
		CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_THRONEBREAK);
	}

	DetermineSimbaRotation();

	if (false == m_bDimensionClawActivated)
		m_fDeactiveTime += m_fTimeDelta;

	if (true == m_bDimensionClawActivated || (0.f < m_fDeactiveTime && 3.f > m_fDeactiveTime))
		MoveDimensionClaw(m_fTimeDelta);

	for (auto& index : m_listUsedRocks)
		m_vecSimbaRocks[index]->Tick(m_fTimeDelta);

	for (auto& index : m_listUsedDebris)
		m_vecDebris[index]->Tick(m_fTimeDelta);

	RemoveDeadRocksFromList();
	RemoveDeadDebrisFromList();

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
	m_iEyeRenderCount = 0;
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	if (true == m_bEyeBloom)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CSimba::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (0 == m_iEyeRenderCount) {
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

		// Render Eye Mesh
		if (FAILED(m_pEyeTextureCom[EYETEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_eEyeState)))
			return E_FAIL;
		if (FAILED(m_pEyeTextureCom[EYETEX_NORMAL]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", m_eEyeState)))
			return E_FAIL;
		if (FAILED(m_pEyeTextureCom[EYETEX_MRA]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture")))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", m_iEyeMesh)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_SIMBAEYE_DEFAULT)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(m_iEyeMesh)))
			return E_FAIL;

		m_iEyeRenderCount--;
	}

	else if (true == m_bEyeBloom)
	{
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
	m_pModelCom->Add_Event("FinalCrusherCharge", [this]() {
		m_pGameInstance->PlaySound_Free(L"TakeItem01.wav", 0.5f);

		});

	m_pModelCom->Add_Event("FinalCrusherCharge", [this]() {
		m_pGameInstance->PlaySound_Free(L"TakeItem01.wav", 0.5f);
		});

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
	m_iFireCount = 0;
	m_iSmokeCount = 0;

	if (true == m_bPhaseTwo)
	{
		if (m_setDimensionClawAnims.end() == m_setDimensionClawAnims.find(eState))
			HideDimensionClawActor();
		if (m_setDimensionLaserAnims.end() == m_setDimensionLaserAnims.find(eState))
			HideDimensionLaserActor();
	}

	m_bStateChanged = m_pFSM->ChangeState(eState, _fAnimSpeed, _bLoop, _bInterpolation);
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

	_float fY = m_pTransformCom->Get_State(CTransform::STATE_POSITION).y + 0.3f;
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

		if (0 == CUtils::Make_RandomInt(0, 1))
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

_bool CSimba::IsDamagable()
{
	if (m_setUndamagableAnims.end() != m_setUndamagableAnims.find(SIMBA_ANIM(Get_State())))
		return false;

	return true;
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

	//이펙트
	DimensionClaw();

	m_pDimensionClawActor->setKinematicTarget(CUtils::ToPxTransform(matWorld));
}

void CSimba::MoveDimensionClaw(_float fTimeDelta)
{
	if (nullptr == m_pDimensionClawActor)
		return;

	PxTransform pxTransform = m_pDimensionClawActor->getGlobalPose();

	_float4x4 matWorld = CUtils::To_Float4x4(pxTransform);
	_float4 vLook{}, vPos{};
	memcpy(&vLook, &(matWorld.m[2]), sizeof(_float4));
	memcpy(&vPos, &(matWorld.m[3]), sizeof(_float4));
	vLook.Normalize();

	vPos += vLook * 26.f * fTimeDelta;

	memcpy(&(matWorld.m[3]), &vPos, sizeof(_float4));

	m_pDimensionClawActor->setGlobalPose(CUtils::ToPxTransform(matWorld));

	//이펙트 다는 매트릭스 동기화
	m_DimensionClawMat = matWorld;

	//공격 밑 데칼 이펙트 출력
	//_float3 vCollidingPoint =
	//	CUtils::Compute_CollidingPoint(static_cast<_float3>(vPos), _float3::Down,
	//		{ 0.f, 1.f, -66.f }, { 26.f, 1.f, 26.f });

}

void CSimba::HideDimensionClawActor()
{
	m_bDimensionClawActivated = false;
	m_fDeactiveTime = 0.f;
	if (nullptr != m_pDimensionClawActor)
		m_pDimensionClawActor->setGlobalPose(PxTransform(0, 0, 0));
}

void CSimba::HideDimensionLaserActor()
{
	if (nullptr != m_pSimbaLaser)
		static_cast<CSimbaLaser*>(m_pSimbaLaser)->HideLaser();
}

void CSimba::LaserAttack(_float fTimeDelta)
{
	_float4x4 matWorld = m_pTransformCom->ComputeBoneWorldMatrix(m_pLaserBone);
	_float fScale = 24.f;
	CUtils::Set_Scaled_Matrix(matWorld, fScale, fScale, fScale);
	_float4 vLook{};
	memcpy(&vLook, &(matWorld.m[2]), sizeof(_float4));
	vLook = _float4(-vLook.x, -vLook.y, -vLook.z, 0);
	memcpy(&(matWorld.m[2]), &vLook, sizeof(_float4));
	m_pSimbaLaserTransform->Set_WorldMatrix(matWorld);
	m_pSimbaLaser->Tick(fTimeDelta);
	m_bLaserActivated = true;
}

void CSimba::SpawnRocks(_uint iAnimIdx)
{
	_vector vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vFloatLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vFloatRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);

	if (Simba_QuickClawL == iAnimIdx || Simba_QuickClaw2L == iAnimIdx || Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
	{
		_float4 vPos{};
		if (Simba_QuickClawL == iAnimIdx || Simba_QuickClaw2L == iAnimIdx)
			vPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		else if (Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
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
	_float4 vFloatLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vFloatRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
	_uint iLastIndex{};

	if (Simba_QuickClawL == iAnimIdx || Simba_QuickClaw2L == iAnimIdx || Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
	{
		if (Simba_QuickClawR == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
			vFloatRight = -vFloatRight;

		_uint iNumDebris = 50;
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		vPos.y = 0;
		_float fOffsetDis = 3.f;
		if (Simba_QuickClaw2L == iAnimIdx || Simba_QuickClaw2R == iAnimIdx)
			fOffsetDis = 1.f;

		for (_uint i = m_iNextDebrisIndex; i < m_iNextDebrisIndex + iNumDebris; i++)
		{
			_uint iCurIdx = i;
			_uint iVecSize = m_vecDebris.size();
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;

			_float fScale = CUtils::Make_RandomFloat(0.08f, 0.2f);
			_float fY = CUtils::Make_RandomFloat(18.f, 32.f);

			_float3 vDir = CUtils::TurnDirectionVector(vFloatRight, _float3(0, 1, 0), CUtils::Make_RandomFloat(-80, 80));
			_float fDis = CUtils::Make_RandomFloat(12.f, 16.f);
			_float fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);

			m_vecDebris[iCurIdx]->Set_DebrisInfo(vPos + vFloatRight * fOffsetDis, fScale, fY, vDir * fDis, 2.5f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iLastIndex = iCurIdx + 1;
		}
	}
	else if (Simba_FinalCrusher == iAnimIdx)
	{
		_uint iNumDebris = 100;

		_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;
		vPos.y = 0;

		for (_uint i = m_iNextDebrisIndex; i < m_iNextDebrisIndex + iNumDebris; i++)
		{
			_uint iCurIdx = i;
			_uint iVecSize = m_vecDebris.size();
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;

			_float fScale = CUtils::Make_RandomFloat(0.08f, 0.2f);
			_float fY = CUtils::Make_RandomFloat(16.f, 28.f);

			_float3 vDir = CUtils::TurnDirectionVector(vFloatLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(0, 360));
			_float fDis = CUtils::Make_RandomFloat(7.f, 11.f);
			_float fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);

			m_vecDebris[iCurIdx]->Set_DebrisInfo(vPos + vDir * 2.f, fScale, fY, vDir * fDis, 2.5f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iLastIndex = iCurIdx + 1;
		}
	}
	else if (Simba_DoubleClawDash == iAnimIdx || (Simba_DoubleClaw == iAnimIdx && 0.2f > Get_AnimRatio()))
	{
		_uint iNumDebris = 1;
		_float4 vLeftHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		_float4 vRightHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);
		vLeftHandPos.y = 0;
		vRightHandPos.y = 0;

		for (_uint i = m_iNextDebrisIndex; i < m_iNextDebrisIndex + iNumDebris; i++)
		{
			_uint iCurIdx = i;
			_uint iVecSize = m_vecDebris.size();
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;

			_float fScale = CUtils::Make_RandomFloat(0.08f, 0.2f);
			_float fY = CUtils::Make_RandomFloat(12.f, 28.f);

			_float3 vDir = CUtils::TurnDirectionVector(vFloatLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(-80, 80));
			_float fDis = CUtils::Make_RandomFloat(12.f, 16.f);
			_float fRightOffset = CUtils::Make_RandomFloat(-2.5f, 2.5f);
			_float fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);
			m_vecDebris[iCurIdx]->Set_DebrisInfo(vLeftHandPos + (vFloatRight * fRightOffset) - vFloatLook * 3.3f, fScale, fY, vDir * fDis, 2.5f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iCurIdx++;
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;
			fScale = CUtils::Make_RandomFloat(0.08f, 0.2f);
			fY = CUtils::Make_RandomFloat(18.f, 32.f);
			vDir = CUtils::TurnDirectionVector(vFloatLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(-80, 80));
			fDis = CUtils::Make_RandomFloat(12.f, 16.f);
			fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);

			m_vecDebris[iCurIdx]->Set_DebrisInfo(vRightHandPos + (vFloatRight * fRightOffset) - vFloatLook * 3.3f, fScale, fY, vDir * fDis, 2.5f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iLastIndex = iCurIdx + 1;
		}
	}
	else if (Simba_DoubleClaw == iAnimIdx)
	{
		_uint iNumDebris = 100;

		_float4 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;
		vPos.y = 0;

		for (_uint i = m_iNextDebrisIndex; i < m_iNextDebrisIndex + iNumDebris; i++)
		{
			_uint iCurIdx = i;
			_uint iVecSize = m_vecDebris.size();
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;

			_float fScale = CUtils::Make_RandomFloat(0.12f, 0.24f);
			_float fY = CUtils::Make_RandomFloat(26.f, 40.f);

			_float3 vDir = CUtils::TurnDirectionVector(vFloatLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(-80, 80));
			_float fDis = CUtils::Make_RandomFloat(14.f, 19.f);
			_float fRightOffset = CUtils::Make_RandomFloat(-4.8f, 4.8f);
			_float fLookOffset = CUtils::Make_RandomFloat(-2.5f, 4.5f);
			_float fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);
			m_vecDebris[iCurIdx]->Set_DebrisInfo(vPos + (vFloatRight * fRightOffset) + vFloatLook * fLookOffset, fScale, fY, vDir * fDis, 2.5f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iLastIndex = iCurIdx + 1;
		}
	}
	else if (Simba_AttackJumpHit == iAnimIdx)
	{
		_uint iNumDebris = 40;
		_float4 vPos{};
		if (0 == m_iDebrisCount)
			vPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		else
			vPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);
		vPos.y = 0.f;

		for (_uint i = m_iNextDebrisIndex; i < m_iNextDebrisIndex + iNumDebris; i++)
		{
			_uint iCurIdx = i;
			_uint iVecSize = m_vecDebris.size();
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;

			_float fScale = CUtils::Make_RandomFloat(0.08f, 0.2f);
			_float fY = CUtils::Make_RandomFloat(12.f, 24.f);

			_float3 vDir = CUtils::TurnDirectionVector(vFloatLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(-80, 80));
			_float fDis = CUtils::Make_RandomFloat(6.f, 10.f);
			_float fRightOffset = CUtils::Make_RandomFloat(-2.f, 2.f);

			_float fLookOffset = CUtils::Make_RandomFloat(0.5f, 3.5f);
			_float fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);

			m_vecDebris[iCurIdx]->Set_DebrisInfo(vPos + (vFloatRight * fRightOffset) + vFloatLook * fLookOffset, fScale, fY, vDir * fDis, 2.5f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iLastIndex = iCurIdx + 1;
		}
	}
	else if (Simba_DimensionLaser == iAnimIdx)
	{
		_uint iNumDebris = 4;
		_float4 vLaserPos = m_pSimbaLaserTransform->Get_State(CTransform::STATE_POSITION);
		_float3 vLaserLook = m_pSimbaLaserTransform->Get_State(CTransform::STATE_LOOK);
		vLaserLook.Normalize();

		_float fLaserDis = m_pSimbaLaserTransform->RayCast(CTransform::STATIC, vLaserLook, 40.f);
		_float4 fResultPos = vLaserPos + (vLaserLook * fLaserDis);
		fResultPos.y = 0.f;


		//효선아 여기야 레이저 파티클
		//레이저와의 충돌 자국
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = static_cast<_float3>(fResultPos);
		FXDesc.vInitScale = { 3.f, 3.f, 3.f };
		FXDesc.vInitPos = { 0.f, 1.f, 0.f };
		Add_Effect("HS_lion cross decal", FXDesc, false);


		//충돌 시 튀는 파티클
		CParticle::PARTICLE_DESC ParticleDesc{};
		ParticleDesc.vInitPos = static_cast<_float3>(fResultPos);
		ParticleDesc.vInitScale = { 2.f, 2.f, 2.f };

		Add_Effect("HS_perfect laser collide particle", ParticleDesc);

		for (_uint i = m_iNextDebrisIndex; i < m_iNextDebrisIndex + iNumDebris; i++)
		{
			_uint iCurIdx = i;
			_uint iVecSize = m_vecDebris.size();
			if (iCurIdx >= iVecSize)
				iCurIdx -= iVecSize;

			_float fScale = CUtils::Make_RandomFloat(0.15f, 0.26f);
			_float fY = CUtils::Make_RandomFloat(20.f, 34.f);

			_float3 vDir = CUtils::TurnDirectionVector(vFloatLook, _float3(0, 1, 0), CUtils::Make_RandomFloat(0, 360));
			_float fDis = CUtils::Make_RandomFloat(8.f, 13.f);
			_float fRightOffset = CUtils::Make_RandomFloat(-4.f, 4.f);
			_float fFallSpeed = CUtils::Make_RandomFloat(9.f, 16.f);
			_float fLookOffset = CUtils::Make_RandomFloat(-0.3f, 0.7f);
			m_vecDebris[iCurIdx]->Set_DebrisInfo(fResultPos + (vFloatLook * fLookOffset) + (vFloatRight * fRightOffset), fScale, fY, vDir * fDis, 3.f, fFallSpeed);
			m_listUsedDebris.push_back(iCurIdx);

			iLastIndex = iCurIdx + 1;
		}
	}

	m_iNextDebrisIndex = iLastIndex;
	m_iDebrisCount++;
}

void CSimba::SetUpSecondTarget()
{
	CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
	if (pCamera != nullptr)
		pCamera->Set_Target(m_pTransformCom, CCamera::TARGET_SECOND, CCamera::FOCUS_FINALBOSS, { 0.f, 2.f, 0.f }, 10.f);
}

void CSimba::CheckFinalCrusherRingCollision(_float fTimeDelta)
{
	m_fRingOuterRadius += fTimeDelta * 18.7f;
	m_fRingInnerRadius = m_fRingOuterRadius * 0.92f;
	if (m_fRingInnerRadius < 1.f)
		m_fRingInnerRadius = 1.f;

	m_bRenderRing = true;
	CTransform* pKirbyTransform = m_pKirby->Get_TransformCom();
	_float3 vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);
	_float fDis = (vKirbyPos - m_vRingPos).Length();

	// 바깥원과 안쪽 원 사이에 있고, 높이 차이가 1 미만일때 충돌
	if (fDis > m_fRingOuterRadius || fDis < m_fRingInnerRadius || vKirbyPos.y > m_vRingPos.y + 1)
		return;

#pragma region 충돌했을때 커비 넉백
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

		pKirby->Set_DamageMoving(vKnockbackDir * 1.8f, 8.f); // 심바 전용 넉백

		_float fMonsterAttack = Get_Attack();
		pKirby->Minus_Hp(fMonsterAttack);
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		pCamera->Make_Shake(1.2f, 0.5f, _float2(0.f, -1.f));

		pKirby->Collision(CCollisionCenter::CONTENT_ATTACK, this);
	}
#pragma endregion
}

void CSimba::SpawnFire(_uint iAnimIdx)
{
	CFire::FIREDESC Firedesc = {};
	_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_float4 vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);

	for (auto& leftBones : m_vecLeftNailBones)
	{
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(leftBones);
		for (_uint i = 0; i < 1; i++)
		{
			Firedesc.fUpRange = CUtils::Make_RandomFloat(7.f, 10.f);
			_float fRandColor1 = CUtils::Make_RandomFloat(0.1f, 0.17f);
			Firedesc.vFirstColor = _float4(1.0f, fRandColor1, 0.0f, 1.f);
			_float fRandColor2 = CUtils::Make_RandomFloat(0.8f, 0.9f);
			Firedesc.vTargetColor = _float4(1.0f, fRandColor2, 0.0f, 1.f);
			Firedesc.fScale = CUtils::Make_RandomFloat(5.f, 9.f);
			Firedesc.fTimeRatio = CUtils::Make_RandomFloat(1.2f, 1.6f);
			_float fLook = CUtils::Make_RandomFloat(-1.5f, 1.5f);
			_float fRight = CUtils::Make_RandomFloat(-1.f, 1.f);

			Firedesc.vFirePos = vPos + (vLook * fLook) + (vRight * fRight) + vLook * 2.f;
			Firedesc.vFirePos.y -= CUtils::Make_RandomFloat(0.45f, 0.85f);

			if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Fire"), TEXT("Prototype_GameObject_Fire"), &Firedesc)))
				return;
			Firedesc.vFirstColor = _float4(1.0f, 0.26f, 0.0f, 1.f);
			Firedesc.fScale *= 0.75f;
			if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Fire"), TEXT("Prototype_GameObject_Fire"), &Firedesc)))
				return;
		}
	}

	for (auto& rightBones : m_vecRightNailBones)
	{
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(rightBones);
		for (_uint i = 0; i < 1; i++)
		{
			Firedesc.fUpRange = CUtils::Make_RandomFloat(7.f, 10.f);
			_float fRandColor1 = CUtils::Make_RandomFloat(0.1f, 0.17f);
			Firedesc.vFirstColor = _float4(1.0f, fRandColor1, 0.0f, 1.f);
			_float fRandColor2 = CUtils::Make_RandomFloat(0.8f, 0.9f);
			Firedesc.vTargetColor = _float4(1.0f, 0.87f, 0.0f, 1.f);
			Firedesc.fScale = CUtils::Make_RandomFloat(5.f, 9.f);
			Firedesc.fTimeRatio = CUtils::Make_RandomFloat(1.2f, 1.6f);
			_float fLook = CUtils::Make_RandomFloat(-1.5f, 1.5f);
			_float fRight = CUtils::Make_RandomFloat(-1.f, 1.f);

			Firedesc.vFirePos = vPos + (vLook * fLook) + (vRight * fRight) + vLook * 2.f;
			Firedesc.vFirePos.y -= CUtils::Make_RandomFloat(0.45f, 0.85f);

			if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Fire"), TEXT("Prototype_GameObject_Fire"), &Firedesc)))
				return;
			Firedesc.vFirstColor = _float4(1.0f, 0.26f, 0.0f, 1.f);
			Firedesc.fScale *= 0.75f;
			if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Fire"), TEXT("Prototype_GameObject_Fire"), &Firedesc)))
				return;
		}
	}

	m_iFireCount++;
}

void CSimba::QuickClawNailFlash(_uint eSimbaAnim) // YW : Effect 영우형 여기임 검지손톱 번쩍
{
	if (Simba_QuickClawStartL == eSimbaAnim) // 왼손
	{
		_float3 vOffset = _float3(); // Right Up Look 오프셋 계수
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(m_vecLeftNailBones[INDEX], vOffset);

	}
	else if (Simba_QuickClawStartR == eSimbaAnim) // 오른손
	{
		_float3 vOffset = _float3(); // Right Up Look 오프셋 계수
		_float4 vPos = m_pTransformCom->ComputeBoneWorldPos(m_vecRightNailBones[INDEX], vOffset);

	}
}
// 완료
void CSimba::QuickClawSlash(_uint eSimbaAnim)
{
	CEffect::FX_DESC effectDesc{};
	_float3 vDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vDir.Normalize();

	//effectDesc.vInitRot = ComputeAngleForEffect();
	effectDesc.vInitScale = _float3(2.5f, 2.5f, 2.5f);
	effectDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

	if (Simba_QuickClawL == eSimbaAnim) // 왼손 1타
	{
		effectDesc.vInitRot = _float3(0, -48, -32);
		effectDesc.vInitPos = _float3(0, 5.9f, 3.3f);
		Add_Effect("HS_lion claw L", effectDesc);
	}
	else if (Simba_QuickClawR == eSimbaAnim) // 오른손 1타
	{
		effectDesc.vInitRot = _float3(0, 48, 32);
		effectDesc.vInitPos = _float3(0, 5.9f, 3.3f);
		Add_Effect("HS_lion claw R", effectDesc);

	}
	else if (Simba_QuickClaw2L == eSimbaAnim) // 왼손 2타
	{
		effectDesc.vInitRot = _float3(0, -47.f, -3.f);
		effectDesc.vInitPos = _float3(0, 2.5f, 3.3f);
		Add_Effect("HS_lion claw L", effectDesc);
	}
	else if (Simba_QuickClaw2R == eSimbaAnim) // 오른손 2타
	{
		effectDesc.vInitRot = _float3(0, 47.f, 3.f);
		effectDesc.vInitPos = _float3(0, 2.5f, 3.3f);
		
		Add_Effect("HS_lion claw R", effectDesc);
	}
}
// 완료
void CSimba::FinalCrusherCharge()
{
	CEffect::FX_DESC effectDesc{};
	_float3 vDir = -m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vDir.Normalize();

	effectDesc.vInitRot = ComputeAngleForEffect(-1);
	effectDesc.vInitScale = _float3(1.03f, 1.03f, 0.94f);
	_float3 vPos = GET_POS;
	vPos -= vDir * 0.45f;
	effectDesc.vInitPos = vPos;
	Add_Effect("HS_lion hit charge", effectDesc);
}
// 완료 
void CSimba::FinalCrusherSwing() // YW : Effect 영우형 여기임 양주먹 내려치기시작
{
	//_float3 vPos = GET_POS;
	//_float3 vLookDegree = CUtils::Make_Degree_FromDir(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	_float3 vScale = { 2.18f, 2.18f, 2.18f };

	//팔 궤적
	CEffect::FX_DESC SingleFXDesc{};
	//SingleFXDesc.vInitPos = _float3(0, 0, 0.f);
	//SingleFXDesc.vInitRot = ComputeAngleForEffect();
	SingleFXDesc.vInitScale = vScale;
	SingleFXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	Add_Effect("HS_lion hammer stomp trail", SingleFXDesc);
}
// 완료 
void CSimba::FinalCrusherSmash() // YW : Effect 영우형 여기임 양주먹 바닥에 찍는 타이밍
{
	_float3 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;
	vPos.y = 2.3f;
	//_float3 vLookDegree = CUtils::Make_Degree_FromDir(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	_float3 vScale = { 2.8f, 2.8f, 2.8f };
	_float3 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	//찍기 효과
	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.vInitPos = vPos + vLook * -16.f;
	MultiFXDesc.vInitRot = ComputeAngleForEffect();
	//MultiFXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	MultiFXDesc.vInitScale = vScale;
	Add_Effect("HS_lion stomp floor", MultiFXDesc);
}

// 완료
void CSimba::FinalCrusherRing() // YW : Effect 영우형 여기임 퍼지는 원 이펙트
{
	CEffect::FX_DESC effectDesc{};
	_float3 vPos = (m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone) + m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone)) * 0.5f;
	vPos.y = 2.3f;
	effectDesc.vInitPos = vPos;
	Add_Effect("HS_lion floor atk circle", effectDesc);
	m_vRingPos = vPos;
	m_fRingInnerRadius = 0;
	m_fRingOuterRadius = 0;
}

void CSimba::JumpStartSmoke() // YW : Effect 영우형 여기임 점프 시작할때 회색방구
{
	_float3 vPos = GET_POS;
	_float3 vScale = { 3.f, 3.f, 3.f };

	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.vInitPos = vPos + _float3(0.f, 2.f, 0.f);
	MultiFXDesc.vInitScale = vScale;

	Add_Effect("HS_FB fly smoke", MultiFXDesc);
}

void CSimba::LandingSmoke() // YW : Effect 영우형 여기임 점프 후 착지 회색방구
{
	_float3 vPos = GET_POS;
	_float3 vScale = { 2.f, 2.f, 2.f };

	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.vInitPos = vPos + _float3(0.f, .3f, 0.f);

	Add_Effect("DDD land smoke", MultiFXDesc);
}

//대쉬
void CSimba::AttackJumpWind() // YW : Effect 영우형 여기임 점프 공격할때 주위 바람 
{
	//_float3 vPos = GET_POS;
	_float3 vScale = { 2.f, 2.f, 2.f };
	//_float3 vLookDegree = CUtils::Make_Degree_FromDir(-m_pTransformCom->Get_State(CTransform::STATE_LOOK));

	CEffect::FX_DESC SingleFXDesc{};

	SingleFXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SingleFXDesc.vInitScale = vScale;
	SingleFXDesc.vInitPos = _float3(0, 5.4f, 3.7f);
	/*SingleFXDesc.vInitRot = vLookDegree;*/
	Add_Effect("HS_lion dash", SingleFXDesc);
}

void CSimba::AttackJumpHit() // 도약 공격 끝나고 착지타이밍에 나오는 빛가닥들 
{
	if (0 == m_iRockCount) // 왼손 바닥에 닿을때
	{

	}
	else if (1 == m_iRockCount) // 오른손 바닥에 닿을때
	{

	}
}

void CSimba::DoubleClawDashGround() // YW : Effect 영우형 여기임 양손으로 바닥 계속 긁을때 튀기는 작은 불씨들
{

}

//파티클
void CSimba::DoubleClawGround() // YW : Effect 영우형 여기임 양슨으로 바닥 긁다가 공격이펙트 직전 튀기는 큰 불씨들
{

}

//양손 발톱 + 불꽃  //완료
void CSimba::DoubleClawSweep()// YW : Effect 영우형 여기임 바닥 긁다가 순간적으로 공격 이펙트 (트레일, 불꽃)
{
	//_float3 vPos = GET_POS;
	_float3 vScale = { 2.8f, 2.8f, 2.8f };
	//_float3 vLookDegree = CUtils::Make_Degree_FromDir(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

	CEffect::FX_DESC SingleFXDesc{};

	SingleFXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SingleFXDesc.vInitPos = _float3(-3.f, 5, 9.f);
	SingleFXDesc.vInitRot = _float3(17.f, 0, 90);
	SingleFXDesc.vInitScale = vScale;
	Add_Effect("HS_lion claw L", SingleFXDesc);

	SingleFXDesc.vInitPos = _float3(3.f, 5, 9.f);
	SingleFXDesc.vInitRot = _float3(-17.f, 0, -90);
	Add_Effect("HS_lion claw R", SingleFXDesc);
}

//크로스 발톱 공격
void CSimba::DimensionClaw()
{
	Delete_Effect("HS_lion L cross");
	Delete_Effect("HS_lion R cross");
	//Delete_AllEffect();
	_float3 vPos = GET_POS;
	_float3 vScale = { 3.2f, 3.2f, 3.2f };

	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.pSocketMatrix = &m_DimensionClawMat;
	MultiFXDesc.vInitScale = vScale;
	MultiFXDesc.fStartDelay = .3f;

	Add_Effect("HS_lion L cross", MultiFXDesc, true);
	Add_Effect("HS_lion R cross", MultiFXDesc, true);
}

void CSimba::TeethBite(_bool bRight)
{
	//
	_float3 vScale = { 3.f, 3.f, 3.f };

	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.pSocketMatrix = &m_matMouth;
	MultiFXDesc.vInitScale = vScale;

	MultiFXDesc.vInitRot = { -90.f, bRight ? -45.f : 45.f, 0.f };

	Add_Effect("HS_lion tooth", MultiFXDesc);
}

void CSimba::DimensionLaserVomit() // DimensionLaser 주위에 나오는 액체부스거리들
{
	// Fire클래스
}

void CSimba::DimensionLaser() // 진짜 DimensionLaser
{
}

void CSimba::DimensionLaserParticles()
{
}

void CSimba::WalkSmoke() // 걸을때 발 땅에 닿을때 나오는 회색방구
{
}

void CSimba::RoarElecParts() // 아직 호출안됨 // 아마 한번 호출 된 이후 전기 계속 생성해야할듯
{
}

void CSimba::BiteRushJumpSmoke(_uint iAnimIndex) // 손 발에서 여러방향으로 회색방구 나오게 해주쎄요
{
	if (Simba_BiteRushJumpStartL == iAnimIndex || Simba_BiteRushJumpStartR == iAnimIndex)
	{
		_float3 vLeftHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
		vLeftHandPos.y = 2.3f;
		_float3 vRightHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);
		vRightHandPos.y = 2.3f;
		_float3 vLeftFootPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftFootBone);
		vLeftFootPos.y = 2.3f;
		_float3 vRightFootPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightFootBone);
		vRightFootPos.y = 2.3f;
	}
	else if (Simba_BiteRushLandingL == iAnimIndex)
	{
		if (0 == m_iSmokeCount) // 왼손, 오른발
		{
			_float3 vLeftHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
			vLeftHandPos.y = 2.3f;
			_float3 vRightFootPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightFootBone);
			vRightFootPos.y = 2.3f;
		}
		else if (1 == m_iSmokeCount) // 왼발
		{
			_float3 vLeftFootPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftFootBone);
			vLeftFootPos.y = 2.3f;
		}
		else if (2 == m_iSmokeCount) // 오른손
		{
			_float3 vRightHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);
			vRightHandPos.y = 2.3f;
		}
	}
	else if (Simba_BiteRushLandingR == iAnimIndex)
	{
		if (0 == m_iSmokeCount) // 오른손, 왼발
		{
			_float3 vRightHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightHandBone);
			vRightHandPos.y = 2.3f;
			_float3 vLeftFootPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftFootBone);
			vLeftFootPos.y = 2.3f;
		}
		else if (1 == m_iSmokeCount) // 오른발
		{
			_float3 vRightFootPos = m_pTransformCom->ComputeBoneWorldPos(m_pRightFootBone);
			vRightFootPos.y = 2.3f;
		}
		else if (2 == m_iSmokeCount) // 왼손
		{
			_float3 vLeftHandPos = m_pTransformCom->ComputeBoneWorldPos(m_pLeftHandBone);
			vLeftHandPos.y = 2.3f;
		}
	}

	m_iSmokeCount++;
}

void CSimba::DimensionClawFire() // 왼손 오른손 하나씩
{
	// Fire클래스

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
	if (LEVEL_SIMBA == *m_pGameInstance->Get_CurrentLevelID())
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
	for (_uint i = Simba_FinalCrusher; i <= Simba_FinalCrusherStartRepeatEnd; i++)
		m_pFSM->Add_State(i, CSimba_FinalCrusher::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	for (_uint i = Simba_DoubleClaw; i <= Simba_DoubleClawEnd; i++)
		m_pFSM->Add_State(i, CSimba_DoubleClaw::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	m_pFSM->Add_State(Simba_Wait2, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Jump, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_JumpStart, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Fall, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
	m_pFSM->Add_State(Simba_Landing, CSimba_Jump::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));

	for (_uint i = Simba_BackStep; i <= Simba_BackStepStart; i++)
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

	for (_uint i = Simba_DimensionClaw; i <= Simba_DimensionClawWait; i++) {
		m_pFSM->Add_State(i, CSimba_DimensionClaw::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_setDimensionClawAnims.insert(SIMBA_ANIM(i));
	}

	for (_uint i = Simba_BiteRush; i <= Simba_BiteRushEnd; i++) {
		m_pFSM->Add_State(i, CSimba_BiteRush::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_mapRotation[BITERUSH].insert(SIMBA_ANIM(i));
	}
	for (_uint i = Simba_BiteRushStart; i <= Simba_BiteRushTiredWait; i++) {
		m_pFSM->Add_State(i, CSimba_BiteRush::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_mapRotation[BITERUSH].insert(SIMBA_ANIM(i));
	}

	for (_uint i = Simba_DimensionLaser; i <= Simba_DimensionLaserWait; i++) {
		m_pFSM->Add_State(i, CSimba_DimensionLaser::Create(m_pControllerCom, m_pTransformCom, m_pKirby, pKirbyTransform));
		m_setDimensionLaserAnims.insert(SIMBA_ANIM(i));
	}

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

	for (_uint i = 1; i < 9; i++)
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

			TCHAR* tcharBuffer = new TCHAR[20];
			wstring wstrSound = L"lion" + to_wstring(CUtils::Make_RandomInt(0, 16)) + L".wav";
			wcscpy_s(tcharBuffer, wstrSound.size() + 1, wstrSound.c_str());
			m_pGameInstance->StopSound(CHANNEL_BOSSVOICE);
			m_pGameInstance->PlayMySound(tcharBuffer, CHANNEL_BOSSVOICE, 0.2f);
			Safe_Delete_Array(tcharBuffer);
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
	m_pGameInstance->StopSound(CHANNEL_BGM);
	m_pGameInstance->PlayMySound(L"SimbaAfterDialog.wav", CHANNEL_BGM, 0.37f);
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
			wstrMonsterName = TEXT("AwoofyWild"); // AwoofyWild로 바꿔야할수도 흠
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
		m_bWave2Summoned = true;
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

			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SIMBA, g_strLayerMonster, wstrTag, &monsterDesc)))
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
		else if (1 == m_iEffectCount)
			wstrMonsterName = TEXT("AwoofyWild");
	}
	else if (12 == iTriggerIndex)
	{
		if (0 == m_iEffectCount)
			wstrMonsterName = TEXT("Awoofy"); // AwoofyWild로 바꿔야할수도 흠
		else if (1 == m_iEffectCount)
			wstrMonsterName = TEXT("Rabbit");
		else if (2 == m_iEffectCount)
			wstrMonsterName = TEXT("RabbitBig");
	}

	_float fY{};
	if (TEXT("Awoofy") == wstrMonsterName || TEXT("Rabbit") == wstrMonsterName || TEXT("RabbitBig") == wstrMonsterName)
		fY = 2.4f;
	else if (TEXT("AwoofyWild") == wstrMonsterName)
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
		else if (0.7f < m_fSpawnTime && 1 == m_iEffectCount)
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
		else if (0.3f < m_fSpawnTime && 1 == m_iEffectCount)
			SpawnEffects(12);
		else if (0.7f < m_fSpawnTime && 2 == m_iEffectCount)
			SpawnEffects(12);

		if (1.2f < m_fSpawnTime && 0 == m_iMonsterCount)
			SpawnMonsters(12);
		else if (1.5f < m_fSpawnTime && 1 == m_iMonsterCount)
			SpawnMonsters(12);
		else if (1.9f < m_fSpawnTime && 2 == m_iMonsterCount)
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

void CSimba::CreateDimensionClawActor()
{
	auto pPhysics = m_pGameInstance->Get_Physics();
	PxMaterial* pMtrl = m_pGameInstance->Get_Material();

	PxTransform transform(PxVec3(0, 0, 0));
	PxRigidDynamic* pRigidDynamic = pPhysics->createRigidDynamic(transform);
	PxBoxGeometry boxGeometry(10.f, 1.f, 5.f);

	PxQuat rotation1(XMConvertToRadians(40), PxVec3(0, 0, 1)); // z축기준 35도 회전
	PxQuat rotation2(-XMConvertToRadians(40), PxVec3(0, 0, 1)); // z축기준 35도 회전
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

void CSimba::RemoveDeadDebrisFromList()
{
	for (auto& iter = m_listUsedDebris.begin(); iter != m_listUsedDebris.end();)
	{
		if (true == m_vecDebris[*iter]->Get_Dead())
			iter = m_listUsedDebris.erase(iter);
		else
			iter++;
	}
}

_float3 CSimba::ComputeAngleForEffect(_float fReverseLook)
{
	_float3 vDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK) * fReverseLook;
	vDir.Normalize();
	_float3 vLook = { 0.f, 0.f, 1.f };

	_float fAngleLook = atan2f(vLook.z, vLook.x);
	_float fAngleDiff = fAngleLook - atan2f(vDir.z, vDir.x);
	fAngleDiff = ToDegree(fAngleDiff);

	return _float3(0.f, fAngleDiff, 0.f);
}

#ifdef _DEBUG

void CSimba::RenderRing()
{
	vector<_vector> vecOuterRingPoints;
	vector<_vector> vecInnerRingPoints;
	for (_uint i = 0; i < 36; i++)
	{
		_float4 vDir = CUtils::TurnDirectionVector(XMVectorSet(1, 0, 0, 0), _float3(0, 1, 0), i * 10.f);
		_float4 vOuterPos = m_vRingPos + vDir * m_fRingOuterRadius;
		vOuterPos.w = 1.f;
		vecOuterRingPoints.push_back(vOuterPos);

		_float4 vInnerPos = m_vRingPos + vDir * m_fRingInnerRadius;
		vInnerPos.w = 1.f;
		vecInnerRingPoints.push_back(vInnerPos);
	}

	RenderPolygon(vecOuterRingPoints);
	RenderPolygon(vecInnerRingPoints);
}

void CSimba::RenderPolygon(vector<_vector>& worldPoints)
{
	if (m_pGameInstance->Get_HitBoxRender() == false)
		return;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	_matrix ViewMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW); // CPipeLine::D3DTS_VIEW
	_matrix ProjMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ); // CPipeLine::D3DTS_PROJ
	_matrix VPMatrix = XMMatrixMultiply(ViewMatrix, ProjMatrix);

	auto TransformToScreen = [&](_vector worldPos)
		{
			_vector screenPos = XMVector3TransformCoord(worldPos, VPMatrix);
			screenPos = XMVectorMultiplyAdd(screenPos, XMVectorSet(0.5f, -0.5f, 1.0f, 0.0f), XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f));
			screenPos = XMVectorMultiply(screenPos, XMVectorSet(g_iWinSizeX, g_iWinSizeY, 1.f, 0.f));
			return ImVec2(XMVectorGetX(screenPos), XMVectorGetY(screenPos));
		};

	// 월드 좌표를 화면 좌표로 변환
	vector<ImVec2> screenPoints;
	for (const auto& point : worldPoints)
		screenPoints.push_back(TransformToScreen(point));

	// 점들을 이어서 다각형 그리기
	for (size_t i = 0; i < screenPoints.size(); ++i)
	{
		const ImVec2& p1 = screenPoints[i];
		const ImVec2& p2 = screenPoints[(i + 1) % screenPoints.size()]; // 마지막 점은 첫 번째 점과 연결
		drawList->AddLine(p1, p2, IM_COL32(255, 255, 0, 255), 2.0f);
	}
}
#endif // DEBUG

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

	Safe_Release(m_pMouthBone);
	Safe_Release(m_pLeftFootBone);
	Safe_Release(m_pRightFootBone);
	Safe_Release(m_pLipBone);
	Safe_Release(m_pLaserBone);
	Safe_Release(m_pLeftHandBone);
	Safe_Release(m_pRightHandBone);

	Safe_Release(m_pKirby);
	Safe_Release(m_pSimbaLaserTransform);
	Safe_Release(m_pSimbaLaser);

	for (_uint i = 0; i < EYETEX_END; i++)
		Safe_Release(m_pEyeTextureCom[i]);
}