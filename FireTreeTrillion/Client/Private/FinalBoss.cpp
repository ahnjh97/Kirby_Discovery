#include "stdafx.h"
#include "FinalBoss.h"
#include "FSM.h"
#include "FinalBoss_State.h"
#include "FinalBossSpear.h"
#include "RayArrow.h"
#include "Camera_Main.h"
#include "Gully.h"
#include "HitBox.h"
#include "Debris.h"

CFinalBoss::CFinalBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CFinalBoss::CFinalBoss(const CFinalBoss& rhs)
	: CMonster{ rhs }
{
}

HRESULT CFinalBoss::Initialize_Prototype()
{
	m_eCollisionGroup = MONSTER;

	return S_OK;
}

HRESULT CFinalBoss::Initialize(void* pArg)
{
	FINALBOSS_DESC* pBossDesc = nullptr;

	if (nullptr != pArg)
	{
		pBossDesc = (FINALBOSS_DESC*)pArg;

		pBossDesc->fSpeedPerSec = 7.f;
		pBossDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vecRallyPoint = pBossDesc->vecRallyPoints;
	}

	if (FAILED(__super::Initialize(pBossDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_PartObjects()))
		return E_FAIL;

	m_fMaxHp = 500.f;
	m_fHp = 500.f;
	m_fAttack = 10.f;
	m_eVacuumSize = SIZE_BIG;
	m_eBossState = STATE_FLYING;
	m_fTimeDelay = 1.f;
	m_iDebrsiMaxCnt = 0;

	m_pModelCom->Set_Animation(FINALBOSS_DEMOAPPEARCUT5, 70.f, false, true);

	Make_TargetToCams();
	Add_AnimEvent();

	if (*m_pCurrentLevelID != LEVEL_TOOL_ANIM)
	{
		// 도랑 풀링
		for (size_t i = 0; i < 120; i++)
		{
			HRESULT hr;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Gully"), TEXT("Prototype_GameObject_Gully"));
			CHECK_FAILED(hr);

			CGully* pGully = dynamic_cast<CGully*>(m_pGameInstance->Get_LastGameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Gully")));
			m_vecGully.push_back(pGully);
			Safe_AddRef(pGully);
		}

		// 파티클 풀링
		for (_uint j = 0; j < 25; j++)
		{
			for (_uint i = 0; i < DEBRISCNT; i++)
			{
				HRESULT hr;
				GAMEOBJECT_DESC tDesc{};
				tDesc.wstrModelName = TEXT("TunnelRock") + to_wstring(i);
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_ParticleDebris"), TEXT("Prototype_GameObject_Debris"), &tDesc);
				CHECK_FAILED(hr);


				CDebris* pDebris = dynamic_cast<CDebris*>(m_pGameInstance->Get_LastGameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_ParticleDebris")));
				m_vecDebris.push_back(pDebris);
				Safe_AddRef(pDebris);
			}
		}

		HRESULT hr;
		GAMEOBJECT_DESC tDesc{};
		tDesc.wstrModelName = TEXT("TunnelRock") + to_wstring(17);
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_ParticleDebris"), TEXT("Prototype_GameObject_Debris"), &tDesc);
		CHECK_FAILED(hr);

		CDebris* pMagneticDebris = dynamic_cast<CDebris*>(m_pGameInstance->Get_LastGameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_ParticleDebris")));
		m_vecMagneticDebris.push_back(pMagneticDebris);
		Safe_AddRef(pMagneticDebris);
	}

	return S_OK;
}

_int CFinalBoss::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer() * m_fTimeDelay;

	if (m_pGameInstance->Get_KeyState(DIK_K, KEY_DOWN) || m_fHp < m_fMaxHp * 0.45f)
	{
		Set_BossState(STATE_2PAZE);
		m_pControllerCom->Set_Position(m_pTransformCom, m_vecRallyPoint[1]);
		m_pTransformCom->Look_At(m_vecRallyPoint[0]);
		Change_State(FINALBOSS_DAMAGE, 50.f, false, true);
	}
	else if (m_pGameInstance->Get_KeyState(DIK_L, KEY_DOWN))
	{
		Change_State(FINALBOSS_DAMAGE, 50.f, false, true);
	}

	if (true == m_bGlide)
	{
		_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_float fAmplitude = 1.f;	// 진폭
		_float fFrequency = 1.5f;   // 주기

		_vector vGlidePos = {};
		// 애니메이션에 따라 속도 가중치 ?!
		m_fGlideTime += m_fTimeDelta * 0.095f;
		vGlidePos = vPos + m_fGlideTime * (m_vDir - vPos);

		// 사인 곡선을 따라 y 조정
		vGlidePos.m128_f32[1] += fAmplitude * sin(fFrequency * m_fGlideTime * 2.f * 3.14f);

		m_pControllerCom->Move(m_pTransformCom, vGlidePos, m_fTimeDelta);
	}
	else
		m_fGlideTime = 0.f;

	//풀링임
	if (true == m_bGully)
	{
		//m_fGullyTime += m_fTimeDelta;

		if (false == m_bShake)
		{
			m_fTimeDelay = 0.8f;
			m_bShake = true;
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			if (pCamera != nullptr)
				pCamera->Make_Shake(1.f, 1.f);
		}

		//if(0.1f < m_fGullyTime)
		//{
			//m_fGullyTime = 0.f;

		// 쟁기질
		_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) * 0.8f;
		vPos.m128_f32[1] = 0.f;
		m_vecGully[m_iGullyCnt]->Set_Gully(vPos, 6.f);
		++m_iGullyCnt;
		vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT) * 0.8f;

		vPos.m128_f32[1] = 0.f;
		m_vecGully[m_iGullyCnt]->Set_Gully(vPos, 6.f);
		++m_iGullyCnt;
		if (m_vecGully.size() <= m_iGullyCnt)
			m_iGullyCnt = 0;

		vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		vPos.m128_f32[1] -= 6.f;

		// 파편 파티클 튀는거 
		if (m_vecDebris.size() > m_iDebrsiMaxCnt)
			m_iDebrsiMaxCnt += DEBRISCNT;
		else
		{
			m_iDebrsiMaxCnt = DEBRISCNT;
			m_iDebrisCnt = 0;
		}

		// 파편 파티클 살리기
		for (m_iDebrisCnt; m_iDebrisCnt < m_iDebrsiMaxCnt; ++m_iDebrisCnt)
			m_vecDebris[m_iDebrisCnt]->Set_ParticleDebris(vPos);

		//}
	}
	else
	{
		m_fTimeDelay = 1.f;
		m_bShake = false;
	}

	if (true == m_bEffect)
	{
		m_bEffect = false;
		_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 5.f;
		vPos.m128_f32[1] -= 0.5f;
		m_vecMagneticDebris[0]->Set_ParticleEffect(XMVectorSetW(vPos, 1.f), 2.5f);

		for (_uint i = 0; i < 3; ++i)
		{
			// 파편 파티클 튀는거 
			if (m_vecDebris.size() > m_iDebrsiMaxCnt)
				m_iDebrsiMaxCnt += DEBRISCNT;
			else
			{
				m_iDebrsiMaxCnt = DEBRISCNT;
				m_iDebrisCnt = 0;
			}

			// 파편 파티클 살리기
			for (m_iDebrisCnt; m_iDebrisCnt < m_iDebrsiMaxCnt; ++m_iDebrisCnt)
				m_vecDebris[m_iDebrisCnt]->Set_ParticleDebris(vPos, 1.f, _float2(5.f, 15.f), _float2(3.f, 5.f));
		}
	}

	__super::Tick(m_fTimeDelta);

	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CFinalBoss::Late_Tick(_float fTimeDelta)
{
	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(m_fTimeDelta);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 100.0f))
	{
		m_pModelCom->Play_Animation(m_fTimeDelta);

		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CFinalBoss::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
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

HRESULT CFinalBoss::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CFinalBoss::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}


	ImGui::Text("Air Pattern");
	if (ImGui::Button("Stab"))
	{
		m_bStab = true;
	}
	if (ImGui::Button("Gully"))
	{
		m_bSlash = true;
	}
	if (ImGui::Button("Meteor"))
	{
		m_bMeteor = true;
	}
	if (ImGui::Button("Laser"))
	{
		m_bLaser = true;
	}
	if (ImGui::Button("Air Arrow"))
	{
		m_bArrowAir = true;
	}
	if (ImGui::Button("SideStep(L, R Random)"))
	{
		m_bSide = true;
	}

	ImGui::Text("Ground Pattern");
	if (ImGui::Button("BackStep"))
	{
		m_bBackStep = true;
	}
	if (ImGui::Button("Ground Arrow"))
	{
		m_bArrowGround = true;
	}
	if (ImGui::Button("Swing(Variation 3 Random)"))
	{
		m_bSwing = true;
	}
	if (ImGui::Button("Thrust"))
	{
		m_bThrust = true;
	}
	if (ImGui::Button("Spike"))
	{
		m_bSpike = true;
	}

	ImGui::Text("Special Pattern");
	ImGui::Text("Key K : Spawn Meteor After CutScene");
	ImGui::Text("Key L : Gimmick Clone");
	//ImGui::Text("RePress : %d", m_bRePressBlock);
	//ImGui::Text("Land : %d", INFO(m_isLanding));

	//ImGui::Text("JUMP : %d", INFO(m_isJump));
	//ImGui::Text("Velocity : %.2f", INFO(m_fJumpVelocity));
	//ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	//ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}

#endif

void CFinalBoss::Add_AnimEvent()
{
	__super::Add_AnimEvent();

	m_pModelCom->Add_Event("Attack", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		HitBoxChanger(m_pFSM->Get_State());
		});
}

void CFinalBoss::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{

}

void CFinalBoss::Change_State(FINALBOSS_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CFinalBoss::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

HRESULT CFinalBoss::Make_TargetToCams()
{
	if (*m_pCurrentLevelID == LEVEL_TOOL_ANIM)
		return S_OK;

	CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	CHECK_NULLPTR(pCameraMain);

	pCameraMain->Set_Target(m_pTransformCom, CCamera::TARGET_SECOND, CCamera::FOCUS_BOTH);

	return S_OK;
}

HRESULT CFinalBoss::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FinalBoss"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.f;
	desc.tCapsuleShape.fHeight = 1.f;
	desc.tCapsuleShape.fRadius = 1.f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	//m_pControllerCom->Set_Object(this);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = BOSS_FINALBOSS;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 1.5f, 3.f, 3.f);

	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[ATTACK];
	HitBox.pCollisionType = HITBOX_MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	SetUp_FSM();

	return S_OK;
}

HRESULT CFinalBoss::Add_PartObjects()
{
	/* For.Part_Weapon */
	CPartObject* pWeaponObject = { nullptr };
	CFinalBossSpear::FINALBOSSSPEAR_DESC	FinalBossSpearDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));

	FinalBossSpearDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	FinalBossSpearDesc.pSocket = pModel->Get_BonePtr("R_HaveL");

	pWeaponObject = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_FinalBossSpear"), &FinalBossSpearDesc));
	if (nullptr == pWeaponObject)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pWeaponObject);

	return S_OK;
}

HRESULT CFinalBoss::Bind_ShaderResources()
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

void CFinalBoss::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(FINALBOSS_DEMOAPPEARCUT5, CFinalBoss_Appear_State::Create());

	m_pFSM->Add_State(FINALBOSS_WAITAIR, CFinalBoss_Idle_State::Create());
	m_pFSM->Add_State(FINALBOSS_WAIT, CFinalBoss_Idle_State::Create());

	// 찌르기 패턴
	m_pFSM->Add_State(FINALBOSS_STABREADY, CFinalBoss_Stab_State::Create());
	m_pFSM->Add_State(FINALBOSS_STABSTART, CFinalBoss_Stab_State::Create());
	m_pFSM->Add_State(FINALBOSS_STABWAIT, CFinalBoss_Stab_State::Create());
	m_pFSM->Add_State(FINALBOSS_STAB, CFinalBoss_Stab_State::Create());
	m_pFSM->Add_State(FINALBOSS_STABEND, CFinalBoss_Stab_State::Create());

	// 백스텝 활공
	m_pFSM->Add_State(FINALBOSS_AWAYFASTREADY, CFinalBoss_GlideBack_State::Create());
	m_pFSM->Add_State(FINALBOSS_AWAYFASTSTART, CFinalBoss_GlideBack_State::Create());
	m_pFSM->Add_State(FINALBOSS_AWAYFAST, CFinalBoss_GlideBack_State::Create());
	m_pFSM->Add_State(FINALBOSS_AWAYFASTENDAIR, CFinalBoss_GlideBack_State::Create());

	// 왼쪽 횡 활공
	m_pFSM->Add_State(FINALBOSS_TURNLEFTAIRSTART, CFinalBoss_Glide_State::Create());
	m_pFSM->Add_State(FINALBOSS_TURNLEFTAIR, CFinalBoss_Glide_State::Create());
	m_pFSM->Add_State(FINALBOSS_TURNLEFTAIREND, CFinalBoss_Glide_State::Create());
	// 오른쪽 횡 활공
	m_pFSM->Add_State(FINALBOSS_TURNRIGHTAIRSTART, CFinalBoss_Glide_State::Create());
	m_pFSM->Add_State(FINALBOSS_TURNRIGHTAIR, CFinalBoss_Glide_State::Create());
	m_pFSM->Add_State(FINALBOSS_TURNRIGHTAIREND, CFinalBoss_Glide_State::Create());

	// 슬래시 패턴
	m_pFSM->Add_State(FINALBOSS_SLASHREADY, CFinalBoss_Slash_State::Create());
	m_pFSM->Add_State(FINALBOSS_SLASHSTART, CFinalBoss_Slash_State::Create());
	m_pFSM->Add_State(FINALBOSS_SLASH, CFinalBoss_Slash_State::Create());
	// 2차 슬래시 패턴
	m_pFSM->Add_State(FINALBOSS_SLASHCHAINREADY, CFinalBoss_Chain_State::Create());
	m_pFSM->Add_State(FINALBOSS_SLASHCHAINSTABREADY, CFinalBoss_Chain_State::Create());

	// 스윙 패턴
	m_pFSM->Add_State(FINALBOSS_SWINGRIGHTSTART, CFinalBoss_Swing_State::Create());
	m_pFSM->Add_State(FINALBOSS_SWINGRIGHT, CFinalBoss_Swing_State::Create());
	m_pFSM->Add_State(FINALBOSS_SWINGRIGHTEND, CFinalBoss_Swing_State::Create());
	// 강스윙
	m_pFSM->Add_State(FINALBOSS_SWINGFINISHLEFT, CFinalBoss_Swing_State::Create());
	m_pFSM->Add_State(FINALBOSS_SWINGFINISHLEFTEND, CFinalBoss_Swing_State::Create());
	// 일반 스윙
	m_pFSM->Add_State(FINALBOSS_SWINGLEFTSTART, CFinalBoss_Swing_State::Create());
	m_pFSM->Add_State(FINALBOSS_SWINGLEFT, CFinalBoss_Swing_State::Create());
	m_pFSM->Add_State(FINALBOSS_SWINGLEFTEND, CFinalBoss_Swing_State::Create());

	// 화살 쏘는 패턴
	// 지상
	m_pFSM->Add_State(FINALBOSS_RAYARROWREADY, CFinalBoss_Arrow_State::Create());
	m_pFSM->Add_State(FINALBOSS_RAYARROWSTART, CFinalBoss_Arrow_State::Create());
	m_pFSM->Add_State(FINALBOSS_RAYARROWEND, CFinalBoss_Arrow_State::Create());
	// 공중
	m_pFSM->Add_State(FINALBOSS_RAYARROWREADYAIR, CFinalBoss_Arrow_State::Create());
	m_pFSM->Add_State(FINALBOSS_RAYARROWSTARTAIR, CFinalBoss_Arrow_State::Create());
	m_pFSM->Add_State(FINALBOSS_RAYARROWENDAIR, CFinalBoss_Arrow_State::Create());

	// 땅에서 찌르기 패턴
	m_pFSM->Add_State(FINALBOSS_FLASHTHRUSTREADY, CFinalBoss_Thrust_State::Create());
	m_pFSM->Add_State(FINALBOSS_FLASHTHRUSTSTART, CFinalBoss_Thrust_State::Create());
	m_pFSM->Add_State(FINALBOSS_FLASHTHRUST, CFinalBoss_Thrust_State::Create());
	m_pFSM->Add_State(FINALBOSS_FLASHTHRUSTEND, CFinalBoss_Thrust_State::Create());
	m_pFSM->Add_State(FINALBOSS_FLASHTHRUSTSWINGFINISHLEFT, CFinalBoss_Thrust_State::Create());

	// 레이져 패턴
	m_pFSM->Add_State(FINALBOSS_DIMENSIONLASEREADY, CFinalBoss_Laser_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONLASERCHARGE, CFinalBoss_Laser_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONLASERSTART, CFinalBoss_Laser_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONLASER, CFinalBoss_Laser_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONLASEREND, CFinalBoss_Laser_State::Create());

	// 대못박기 패턴
	m_pFSM->Add_State(FINALBOSS_DIMENSIONSPIKEREADY, CFinalBoss_Spike_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONSPIKEREADYWAIT, CFinalBoss_Spike_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONSPIKESTART, CFinalBoss_Spike_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONSPIKE, CFinalBoss_Spike_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONSPIKEWAIT, CFinalBoss_Spike_State::Create());
	m_pFSM->Add_State(FINALBOSS_DIMENSIONSPIKEEND, CFinalBoss_Spike_State::Create());

	// 점프
	m_pFSM->Add_State(FINALBOSS_JUMPREADY, CFinalBoss_Jump_State::Create());
	m_pFSM->Add_State(FINALBOSS_JUMPSTART, CFinalBoss_Jump_State::Create());
	m_pFSM->Add_State(FINALBOSS_JUMPEND, CFinalBoss_Jump_State::Create());

	// 데미지 
	m_pFSM->Add_State(FINALBOSS_DAMAGE, CFinalBoss_Damage_State::Create());

	// 2페이즈 시작
	m_pFSM->Add_State(FINALBOSS_ROAR, CFinalBoss_Roar_State::Create());

	// 메테오 패턴
	m_pFSM->Add_State(FINALBOSS_SUMMONSTART, CFinalBoss_Meteor_State::Create());
	m_pFSM->Add_State(FINALBOSS_SUMMONWAIT, CFinalBoss_Meteor_State::Create());
	m_pFSM->Add_State(FINALBOSS_SUMMON, CFinalBoss_Meteor_State::Create());
	m_pFSM->Add_State(FINALBOSS_SUMMONEND, CFinalBoss_Meteor_State::Create());

	// 진짜를 찾아라 회복 패턴
	m_pFSM->Add_State(FINALBOSS_RECOVERYSTART, CFinalBoss_Recovery_State::Create());
	m_pFSM->Add_State(FINALBOSS_RECOVERYWAIT, CFinalBoss_Recovery_State::Create());
	m_pFSM->Add_State(FINALBOSS_RECOVERYEND, CFinalBoss_Recovery_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = FINALBOSS_DEMOAPPEARCUT5;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

void CFinalBoss::HitBoxChanger(_uint eState)
{
	CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());

	switch (eState)
	{
	case FINALBOSS_SWINGRIGHT:
	{
		if (pCamera != nullptr)
			pCamera->Make_Shake(0.3f, 0.5f);

		Activate_FrustumCollider(0.5f, 10.f, 180.f);
	}
		break;
	case FINALBOSS_SWINGLEFT:
		if (pCamera != nullptr)
			pCamera->Make_Shake(0.3f, 0.5f);

		Activate_FrustumCollider(0.5f, 10.f, 180.f);
		break;
	case FINALBOSS_SWINGFINISHLEFT:
		if (pCamera != nullptr)
			pCamera->Make_Shake(0.3f, 0.5f);

		Activate_FrustumCollider(0.5f, 10.f, 180.f);
		break;
	default:
		break;
	}
}

CFinalBoss* CFinalBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinalBoss* pInstance = new CFinalBoss(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinalBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinalBoss::Clone(void* pArg)
{
	CFinalBoss* pInstance = new CFinalBoss(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinalBoss"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinalBoss::Free()
{
	__super::Free();

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);
	m_PartObjects.clear();

	for (auto iter : m_vecGully)
		Safe_Release(iter);
	m_vecGully.clear();

	for (auto iter : m_vecDebris)
		Safe_Release(iter);
	m_vecDebris.clear();

	for (auto iter : m_vecMagneticDebris)
		Safe_Release(iter);
	m_vecMagneticDebris.clear();
}
