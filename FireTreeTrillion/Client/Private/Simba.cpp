#include "stdafx.h"
#include "Simba.h"
#include "FSM.h"
#include "Simba_State.h"
#include "MultiEffect.h"
#include "HitBox.h"
#include "Bone.h"
#include "EventCenter.h"

CSimba::CSimba(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CSimba::CSimba(const CSimba& rhs)
	: CMonster{ rhs }
{
}

void CSimba::InsertHitboxActivationTiming(_uint iAnimIdx, vector<tuple<_float, _bool, COLLISION_VALUE>>& _vecTimings)
{
	sort(_vecTimings.begin(), _vecTimings.end(), [](const tuple<float, bool, COLLISION_VALUE>& a, const tuple<float, bool, COLLISION_VALUE>& b) {
		return get<0>(a) < get<0>(b);
		});

	m_mapHitBoxTiming.insert_or_assign(iAnimIdx, _vecTimings);
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

	m_pModelCom->Set_Animation(SIMBA_DEMOAPPEAR1CUT2, 50.f, false, true);

	m_vecDamageFaceSubBones = m_pModelCom->Get_ValidBoneIndices(SIMBA_DAMAGEFACESUB);
	m_vecLipSyncSubBones = m_pModelCom->Get_ValidBoneIndices(SIMBA_LIPSYNCSUB);
	m_vecLipSyncSubABones = m_pModelCom->Get_ValidBoneIndices(SIMBA_LIPSYNCSUBA);

	CEventCenter* pEventCenter = CEventCenter::Get_Instance();

	function<void(CGameObject*)> func{};
	func = bind(&CSimba::OnAppearStart, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_APPEAR_START, this, func);

	func = bind(&CSimba::OnAppearEnd, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_APPEAR_END, this, func);

	func = bind(&CSimba::OnWave1Dead, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_WAVE1DEAD, this, func);

	func = bind(&CSimba::OnWave2Dead, this, placeholders::_1);
	pEventCenter->Subscribe(KEVENT_SIMBA_WAVE2DEAD, this, func);

	Add_AnimEvent();

	vector<tuple<_float, _bool, COLLISION_VALUE>> vecTiming = { tuple<_float, _bool, COLLISION_VALUE>(0, false, ATTACK) };
	InsertHitboxActivationTiming(SIMBA_DEMOAPPEAR1CUT2, vecTiming);

	return S_OK;
}

_int CSimba::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	if (true == m_pModelCom->IsFinished() || m_pModelCom->Get_Trackposition() == 0.f) // IsAnimFinished
		Reset_HitBoxTimingMap(m_pModelCom->Get_CurAnimIndex());

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	__super::Tick(m_fTimeDelta);

	if (true == m_pModelCom->IsFinished(SIMBA_DAMAGEFACESUB))
		m_bPlayDamageFaceSub = false;
	if (true == m_pModelCom->IsFinished(SIMBA_LIPSYNCSUB))
		m_bPlayLipSyncSub = false;
	if (true == m_pModelCom->IsFinished(SIMBA_LIPSYNCSUBA))
		m_bPlayLipSyncSubA = false;

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
		if (m_pGameInstance->Get_KeyState(DIK_NUMPAD1, KEY_DOWN))
			Activate_Attack();
		if (m_pGameInstance->Get_KeyState(DIK_NUMPAD2, KEY_DOWN))
			Activate_Attack(ATTACK2);
		if (m_pGameInstance->Get_KeyState(DIK_NUMPAD3, KEY_DOWN))
			Activate_Attack(ATTACK3);
		if (m_pGameInstance->Get_KeyState(DIK_NUMPAD7, KEY_DOWN)) {
			m_pModelCom->Reset_PartialAnimation(SIMBA_LIPSYNCSUB, 50.f, false, true);
			m_bPlayLipSyncSub = true;
		}
		if (m_pGameInstance->Get_KeyState(DIK_NUMPAD8, KEY_DOWN)) {
			m_pModelCom->Reset_PartialAnimation(SIMBA_LIPSYNCSUBA, 50.f, false, true);
			m_bPlayLipSyncSubA = true;
		}
	}

	Check_HitBoxActivation();

	return OBJ_NOEVENT;
}

void CSimba::Late_Tick(_float fTimeDelta)
{
	_bool bIsFinished = m_pModelCom->IsFinished();

	m_pModelCom->Play_Animation(m_fTimeDelta);

	if (false == bIsFinished)
	{
		if (true == m_bPlayDamageFaceSub)
			m_pModelCom->Play_PartialAnimation(SIMBA_DAMAGEFACESUB, m_vecDamageFaceSubBones, m_fTimeDelta, false);
		else if (true == m_bPlayLipSyncSub)
			m_pModelCom->Play_PartialAnimation(SIMBA_LIPSYNCSUB, m_vecLipSyncSubBones, m_fTimeDelta, false);
		else if (true == m_bPlayLipSyncSubA)
			m_pModelCom->Play_PartialAnimation(SIMBA_LIPSYNCSUBA, m_vecLipSyncSubABones, m_fTimeDelta, false);
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
	if (true == m_bPlayDamageFaceSub)
		return;

	m_pModelCom->Reset_PartialAnimation(SIMBA_DAMAGEFACESUB, 50.f, false, true);
	m_bPlayDamageFaceSub = true;
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
	desc.fOffset = 1.255f;
	desc.tCapsuleShape.fHeight = 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"), TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPos);

	/* FSM */
	SetUp_FSM();


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = BOSS_SIMBA;
	HitBox.pSocket = m_pModelCom->Get_BonePtr("ControlL");

	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_CYLINDER, -3.f, 7.f, 3.f);

	CHitBox::HITBOX_DESC tAttack{};
	tAttack.pOwner = this;
	tAttack.pDesc = &m_tColliderDesc[ATTACK]; // Left Hand
	tAttack.pCollisionType = HITBOX_MONSTER;
	tAttack.pSocket = m_pModelCom->Get_BonePtr("L_HaveL");
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
		return E_FAIL;
	
	tAttack.pDesc = &m_tColliderDesc[ATTACK2]; // Right Hand
	tAttack.pSocket = m_pModelCom->Get_BonePtr("R_HaveL");
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
		return E_FAIL;

	tAttack.pDesc = &m_tColliderDesc[ATTACK3]; // Feet
	tAttack.pSocket = m_pModelCom->Get_BonePtr("C_PelvisJ");
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &tAttack)))
		return E_FAIL;

	Activate_SphereCollider(0.6f, 2.5f);
	Activate_SphereCollider(0.6f, 2.5f, ATTACK2);
	Activate_CylinderCollider(-2.f, 2.2f, 3.2f, ATTACK3);

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

	m_pFSM->Add_State(SIMBA_DEMOAPPEAR1CUT2, CSimba_Appear1::Create(m_pControllerCom));
	m_pFSM->Add_State(SIMBA_DEMOAPPEAR2CUT1, CSimba_Appear2::Create(m_pControllerCom));
	m_pFSM->Add_State(SIMBA_DEMOAPPEAR2CUT2, CSimba_Appear2::Create(m_pControllerCom));
	m_pFSM->Add_State(SIMBA_WALK, CSimba_Walk::Create(m_pControllerCom));

	//상태 Initialize
	CFSM::FSM_INFO	FSM_Desc = {};
	FSM_Desc.iState = SIMBA_DEMOAPPEAR1CUT2;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

void CSimba::Check_HitBoxActivation()
{
	_uint iAnimIdx = m_pModelCom->Get_CurAnimIndex();
	_float fTrackPosition = m_pModelCom->Get_AnimTrackPosition();

	auto pair = m_mapHitBoxTiming.find(iAnimIdx);
	if (pair != m_mapHitBoxTiming.end())
	{
		auto& vecHitBoxTiming = pair->second;

		for (auto& tuple : vecHitBoxTiming)
		{
			if (get<0>(tuple) >= fTrackPosition)
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

void CSimba::Reset_HitBoxTimingMap(_uint iAnimIdx)
{
	auto pair = m_mapHitBoxTiming.find(iAnimIdx);
	if (pair != m_mapHitBoxTiming.end())
	{
		auto& vecHitBoxTiming = pair->second;
		for (auto& tuple : vecHitBoxTiming)
			get<1>(tuple) = false;
	}
}

void CSimba::OnAppearStart(CGameObject* pObj)
{
	// 대사 웅얼웅얼 
}

void CSimba::OnAppearEnd(CGameObject* pObj)
{
	// 몬스터 소환로직
}

void CSimba::OnWave1Dead(CGameObject* pObj)
{
	// 몬스터 소환로직
}

void CSimba::OnWave2Dead(CGameObject* pObj)
{
	Change_State(SIMBA_DEMOAPPEAR2CUT1, 50.f, false, true);
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

	__super::Free();

	for(_uint i = 0; i < EYETEX_END; i++)
		Safe_Release(m_pEyeTextureCom[i]);
}