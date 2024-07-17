#include "stdafx.h"
#include "Kirby.h"
#include "LevelChanger.h"
#include "FSM.h"
#include "Camera_Free.h"

#include "KirbyDefault_State.h"
#include "KirbyBalloon_State.h"
#include "KirbyVacuum_State.h"
#include "KirbyDamage_State.h"
#include "KirbyContents_State.h"
#include "KirbySword_State.h"
#include "KirbyBoom_State.h"
#include "KirbyCar_State.h"
#include "KirbyHammer_State.h"
#include "KirbyBulb_State.h"
#include "KirbyCrash_State.h"

#include "KirbyWeapons.h"
#include "KirbyArmours.h"
#include "MultiEffect.h"

#include "Utils.h"
#include "Bone.h"
#include "HitBox.h"
#include "Camera_Main.h"
#include "EventCenter.h"

#include "Ability.h"
#include "Deform.h"

#include "Light.h"
#include "Crumble.h"
#include "BulbFlare.h"
#include "Gm_DynamicField.h"


CKirby::CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
{
}

CKirby::CKirby(const CKirby& rhs)
	: CCharacter{ rhs }
{
}

HRESULT CKirby::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKirby::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (nullptr != pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_PartObjects()))
		return E_FAIL;

	if (FAILED(Kirby_SystemInitialize()))
		return E_FAIL;

	// 디버깅 용
	m_eAbilityType = ABILITY_HAMMER;
	if (LEVEL_SIMBA == *m_pCurrentLevelID)
		m_eAbilityType = ABILITY_HAMMER;

	// 커비의 상태에 따라, 애니메이션이 시작된다.
	Kirby_StateInitialize();

	m_pControllerCom->RegisterAsPlayer();
	Set_WeaponAnim(3);

	return S_OK;
}

_int CKirby::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_FirstTimer();
	HitStop_System(fTimeDelta);

	// 파트 오브젝트의 뼈 행렬을 업데이트한다.
	Update_PartObjectMatrix();

	// 커비의 기본적인 축 보정, 밸런스 보정을 담당한다.
	Setting_KirbyBalance();

	// 테스트 전용
	Key_Input(m_fTimeDelta);

	// 유틸업데이트가 들어가있다.
	__super::Tick(m_fTimeDelta);
	Kirby_SystemTick(m_fTimeDelta);

	m_pWeapons->Tick(m_fTimeDelta);
	m_pArmours->Tick(m_fTimeDelta);

	if (*m_pCurrentLevelID == LEVEL_PARK)
	{
		RayCast_Crumbles();
		RayCast_DynamicFields();
	}
		
	return OBJ_NOEVENT;
}

void CKirby::Late_Tick(_float fTimeDelta)
{
	m_pModelCom[INFO(m_eBodyState)]->Play_Animation(m_fTimeDelta);

	if (INFO(m_eBodyState) != BODY_DEFAULT)
		m_pModelCom[BODY_DEFAULT]->Play_Animation(m_fTimeDelta);

	if ((INFO(m_eBodyState) == BODY_CARDEFAULT || INFO(m_eBodyState) == BODY_CARVACUUM ||
		INFO(m_eBodyState) == BODY_BULBDEFAULT || INFO(m_eBodyState) == BODY_BULBVACUUM) == false)
		m_pWeapons->Late_Tick(m_fTimeDelta);

	m_pArmours->Late_Tick(m_fTimeDelta);


	if (m_fOrbitRenderDelay > 0.5f)
	{
		// 조준했당께요!
		INFO(m_bBombAimming) = true;

		for (auto& Glow : m_OrbitGlows)
			Glow->Late_Tick(fTimeDelta);
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND,	 this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW,		 this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);

	if (INFO(m_eBodyState) == BODY_BULBDEFAULT || INFO(m_eBodyState) == BODY_BULBVACUUM)
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
		m_iRenderCount = 1;
	}
}

HRESULT CKirby::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom[INFO(m_eBodyState)]->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (Kirby_FaceCustom(INFO(m_eBodyState), i) == true)
			continue;

		if (m_iRenderCount == 0)
			continue;

		if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
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
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fOverPowerColor", &m_fOverPowerColor, sizeof(_float))))
			return E_FAIL;

		if (INFO(m_eBodyState) == BODY_BULBDEFAULT)
		{
			_bool bOn = true;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_isBulb", &bOn, sizeof(_bool))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_bBulbOn", &INFO(m_bLightOn), sizeof(_bool))))
				return E_FAIL;
			_float4 vBulbPos = Get_BulbLightPos();
			if (FAILED(m_pShaderCom->Bind_RawValue("g_BulbPosition", &vBulbPos, sizeof(_float4))))
				return E_FAIL;
		}
		else
		{
			_bool bOff = false;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_isBulb", &bOff, sizeof(_bool))))
				return E_FAIL;
		}


		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_KIRBY)))
			return E_FAIL;

		m_pModelCom[INFO(m_eBodyState)]->Render(i);
	}


	m_iRenderCount--;
	return S_OK;
}

HRESULT CKirby::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom[INFO(m_eBodyState)])))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CKirby::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	ImGui::Text("m_fHp : %.2f", m_fHp);
	ImGui::Text("m_fCrashChargeTime : %.2f", INFO(m_fCrashChargeTime));
	ImGui::Text("m_bBlockOtherVacuum : %d", INFO(m_bBlockOtherVacuum));
	ImGui::Text("m_vLadderPoint.x : %.2f, m_vLadderPoint.y : %.2f m_vLadderPoint.z : %.2f", INFO(m_vLadderPoint).x, INFO(m_vLadderPoint).y, INFO(m_vLadderPoint).z);
	ImGui::Text("m_vLadderLook.x : %.2f, m_vLadderLook.y : %.2f m_vLadderLook.z : %.2f", INFO(m_vLadderLook).x, INFO(m_vLadderLook).y, INFO(m_vLadderLook).z);
	ImGui::Text("m_vPos.x : %.2f, m_vPos.y : %.2f m_vPos.z : %.2f", vPos.x, vPos.y, vPos.z);

	ImGui::Text("PREATTACKSTATE : %d", INFO(m_ePreAttackState));
	ImGui::Text("TemporaryEatType : %d", INFO(m_eTemporaryEatType));

	ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	//m_pHitBox->Render_IMGUI();

	__super::Render_IMGUI();
}
#endif

HRESULT CKirby::Render_DeferredInfo()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom[INFO(m_eBodyState)]->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{

		if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_DEFERREDINFO)))
			return E_FAIL;

		if (FAILED(m_pModelCom[INFO(m_eBodyState)]->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CKirby::Add_AnimEvent()
{
	__super::Add_AnimEvent();

	// 1. 한 애니메이션에서 같은 이름의 이벤트 가능
	// 2. 재생 기준은 애님툴에서 지정한 애니메이션인지 + 시작 프레임이 애니메이션 프레임안에 들어가는 지
	// 3. 두번째 인자로 넣어준 람다가 시작 프레임 한번만 실행된다.
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("ApplyDamage", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		HitBoxChanger(m_pFSM->Get_State());
		});

	m_pModelCom[BODY_HAMMER]->Add_Event("ApplyDamage", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		HitBoxChanger(m_pFSM->Get_State());
		});

	m_pModelCom[BODY_CRASHDEFAULT]->Add_Event("ApplyDamage", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		Activate_SphereCollider(1.f, 15.f);
		});

	m_pModelCom[BODY_CRASHDEFAULT]->Add_Event("ApplyDamage1", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		Activate_SphereCollider(1.f, 22.f);
		});

	m_pModelCom[BODY_CRASHDEFAULT]->Add_Event("ApplyDamage2", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		Activate_SphereCollider(1.f, 29.f);
		});

	m_pModelCom[BODY_CRASHDEFAULT]->Add_Event("ApplyDamage3", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		Activate_SphereCollider(1.f, 36.f);
		});

	m_pModelCom[BODY_CRASHDEFAULT]->Add_Event("ApplyDamage4", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		Activate_SphereCollider(1.f, 42.f);
		});


	// 사운드 처리

#pragma region SLASH

	//SideSlash
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("Sound_Slash", [this]() {
		m_pGameInstance->PlaySound_Free(L"Slash.wav", 0.5f);
		});

	//SuperSpinSlashEnd, GigantSpinSlash
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("Sound_SuperSlash", [this]() {
		m_pGameInstance->PlaySound_Free(L"SuperSlash.wav", 0.5f);
		});


#pragma endregion

#pragma region SPINSLASH

	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("Sound_SpinSlash", [this]() {
		m_pGameInstance->PlaySound_Free(L"Spin.wav", 0.5f);
		});

	//SuperSpinSlashLoop
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("Sound_SuperSpinSlash", [this]() {
		m_pGameInstance->PlaySound_Free(L"SuperSpin.wav", 0.5f);
		});

#pragma endregion

#pragma region CHARGE

	//SpinSlashCharge :: SWORDSTATE_SPINSLASHCHARGE
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("Sound_Charge", [this]() {
		m_pGameInstance->PlaySound_Free(L"Charge.wav", 0.5f);
		});
	
	//완료) SuperSpinSlashChargeStart :: SWORDSTATE_SUPERSPINSLASHCHARGE
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("Sound_SuperCharge", [this]() {
		m_pGameInstance->PlaySound_Free(L"SuperCharge.wav", 0.5f);
		});

#pragma endregion



}

void CKirby::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	// 몸끼리 부딪혔을때
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		// 흡수중인 몬스터일 경우 충돌은 되지 않고, 내 입속으로 들어간다.
		if (pObject->Get_PhyXState() == PO_VACUUMING)
		{
			// 일단 EAT으로 넘기는건 같으나, EAT이 끝날 시점에 내가 삼켰던 것이 무엇이였는지 판단 후 애니메이션이 분기된다.
			INFO(m_fVacuumTime) = 0.f;
			INFO(m_isEat) = true;
			INFO(m_eEyeState) = EYE_IDLE;
			INFO(m_eMouthState) = MOUTH_ANGER;
			Change_State(STATE_EAT, 100.f, false, false, BODY_BALLOON);
			// 임시 보관소. 먹은게 끝났을 떄, 비로소 커비의 어빌리티 타입이 바뀐다.
			INFO(m_eTemporaryEatType) = pObject->Get_AbilityType();

			// 입 속에 있는 걸로 바꿔준다.
			if (pObject != nullptr)
				pObject->Set_PhyXState(PO_KIRBYMOUTH);

			Delete_AllEffect();
		}
		// 입에 머금은 상태의 몬스터
		else if (pObject->Get_PhyXState() == PO_KIRBYMOUTH)
		{

		}
		// 발사중인 몬스터
		else if (pObject->Get_PhyXState() == PO_FLYAWAY)
		{

		}
		else if (pObject->Get_PhyXState() == PO_FLYDEADAWAY)
		{

		}
		else
		{
			if (m_bOverPower == true)
				return;

			// 초기화해줄놈들
			INFO(m_bFirstChargeEffectTrigger) = true;
			INFO(m_bSecondChargeEffectTrigger) = true;


			if (pObject->Get_Attack() > 10.f && m_eAbilityType != ABILITY_DEFAULT)
			{
				HRESULT hr = S_OK;
				CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
				AbilityItemDesc.vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				AbilityItemDesc.eAbilityType = m_eAbilityType;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);
				m_eAbilityType = ABILITY_DEFAULT;
			}

			// 먹은 상태인 경우
			if (INFO(m_isEat) == true)
			{
				Change_State(STATE_EATDAMAGE, 60.f, false, false, BODY_BALLOON);
			}
			// 나는 상태일 경우 . . .
			else if (true == (Get_State() == STATE_FLIGHTSTART || Get_State() == STATE_FLIGHTFALL || Get_State() == STATE_FLIGHT ||
				Get_State() == STATE_FLIGHTLANDING || Get_State() == STATE_FLIGHTLIMIT || Get_State() == STATE_FLIGHTLIMITFALL))
			{
				Change_State(STATE_FILGHTDAMAGE, 60.f, false, false, BODY_BALLOON);
			}
			// 평범한 상태에서...
			else if (INFO(m_eBodyState) == BODY_CARDEFAULT)
			{
				if (INFO(m_bBooster) == false)
				{
					INFO(m_bCarJump) = true;
					Change_State(CARSTATE_DAMAGE, 60.f, false, false, BODY_CARDEFAULT, OFFSET_CAR);
				}
			}
			else if (INFO(m_eBodyState) == BODY_BULBDEFAULT)
			{
				Change_State(BULBSTATE_DAMAGE, 60.f, false, false, BODY_BULBDEFAULT, OFFSET_BULB);
				INFO(m_pLight)->Interpolate_Light(_float4(0.7f, 0.2f, 0.2f, 0.f), 3.f, 1.f);
				INFO(m_bLightOn) = false;
			}
			else
			{
				Change_State(STATE_DAMAGE, 60.f, false, false, BODY_DEFAULT);
			}

			Delete_AllEffect();
		}
	}
	// 맵 오브젝트들과의 충돌.
	else if (eContent == CCollisionCenter::CONTENT_KICK)
	{
		// 내가 빨아들일때만 충돌반응함.
		if (pObject->Get_PhyXState() == PO_VACUUMING)
		{
			// 일단 EAT으로 넘기는건 같으나, EAT이 끝날 시점에 내가 삼켰던 것이 무엇이였는지 판단 후 애니메이션이 분기된다.
			INFO(m_fVacuumTime) = 0.f;
			INFO(m_isEat) = true;
			INFO(m_eEyeState) = EYE_IDLE;
			INFO(m_eMouthState) = MOUTH_ANGER;
			Change_State(STATE_EAT, 100.f, false, false, BODY_BALLOON);
			// 임시 보관소. 먹은게 끝났을 떄, 비로소 커비의 어빌리티 타입이 바뀐다.
			INFO(m_eTemporaryEatType) = pObject->Get_AbilityType();

			// 입 속에 있는 걸로 바꿔준다.
			if (pObject != nullptr)
				pObject->Set_PhyXState(PO_KIRBYMOUTH);

			Delete_AllEffect();
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		// 흡수중인 몬스터일 경우 충돌은 되지 않고, 내 입속으로 들어간다.
		if (pObject->Get_PhyXState() == PO_VACUUMING)
		{
			// 일단 EAT으로 넘기는건 같으나, EAT이 끝날 시점에 내가 삼켰던 것이 무엇이였는지 판단 후 애니메이션이 분기된다.
			INFO(m_fVacuumTime) = 0.f;
			INFO(m_isEat) = true;
			INFO(m_eEyeState) = EYE_IDLE;
			INFO(m_eMouthState) = MOUTH_ANGER;
			Change_State(STATE_EAT, 100.f, false, false, BODY_BALLOON);
			// 임시 보관소. 먹은게 끝났을 떄, 비로소 커비의 어빌리티 타입이 바뀐다.
			INFO(m_eTemporaryEatType) = pObject->Get_AbilityType();

			// 입 속에 있는 걸로 바꿔준다.
			if (pObject != nullptr)
				pObject->Set_PhyXState(PO_KIRBYMOUTH);

			Delete_AllEffect();
		}
		// 입에 머금은 상태의 몬스터
		else if (pObject->Get_PhyXState() == PO_KIRBYMOUTH)
		{

		}
		// 발사중인 몬스터
		else if (pObject->Get_PhyXState() == PO_FLYAWAY)
		{

		}
		else if (pObject->Get_PhyXState() == PO_FLYDEADAWAY)
		{

		}
		else
		{
			if (m_bOverPower == true)
				return;

			// 초기화해줄놈들
			INFO(m_bFirstChargeEffectTrigger) = true;
			INFO(m_bSecondChargeEffectTrigger) = true;


			if (pObject->Get_Attack() > 10.f && m_eAbilityType != ABILITY_DEFAULT)
			{
				HRESULT hr = S_OK;
				CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
				AbilityItemDesc.vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				AbilityItemDesc.eAbilityType = m_eAbilityType;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);

				m_eAbilityType = ABILITY_DEFAULT;
			}


			// 먹은 상태인 경우
			if (INFO(m_isEat) == true)
			{
				Change_State(STATE_EATDAMAGE, 60.f, false, false, BODY_BALLOON);
			}
			// 나는 상태일 경우 . . .
			else if (true == (Get_State() == STATE_FLIGHTSTART || Get_State() == STATE_FLIGHTFALL || Get_State() == STATE_FLIGHT ||
				Get_State() == STATE_FLIGHTLANDING || Get_State() == STATE_FLIGHTLIMIT || Get_State() == STATE_FLIGHTLIMITFALL))
			{
				Change_State(STATE_FILGHTDAMAGE, 60.f, false, false, BODY_BALLOON);
			}
			// 평범한 상태에서...
			else
			{
				Change_State(STATE_DAMAGE, 60.f, false, false, BODY_DEFAULT);
			}

			Delete_AllEffect();
		}
	}
	// 일단, 넓은 범위일땐 무조건 충돌 할 것이다.
	else if (eContent == CCollisionCenter::CONTENT_DEFORM)
	{

		if (pObject->Get_PhyXState() == PO_VACUUMING)
		{
			if (static_cast<CDeform*>(pObject)->Get_DeformType() == CDeform::DEFORM_CAR)
			{
				Change_State(CARVACUUMSTATE_DEFORM, 60.f, false, false, BODY_CARVACUUM, OFFSET_CARVACUUM);
			}
			else if (static_cast<CDeform*>(pObject)->Get_DeformType() == CDeform::DEFORM_BULB)
			{
				Change_State(BULBVACUUMSTATE_DEFORM, 60.f, false, false, BODY_BULBVACUUM, OFFSET_BULBVACUUM);
			}

			INFO(m_pObject)->Set_Dead();
			Safe_Release(INFO(m_pObject));
			INFO(m_pObject) = nullptr;
			INFO(m_bisDeforming) = false;
			INFO(m_eEyeState) = EYE_IDLE;
			CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
			CTransform* pCameraTransform = pCamera->Get_TransformCom();
			_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
			_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
			INFO(m_vTargetDir) = XMVector3Normalize(vCamLook + vCamRight) * -1.f;
			Delete_AllEffect();
		}
		else if (pObject->Get_PhyXState() == PO_NORMAL)
		{
			// CollisionCenter 에서 흡수 가능을 판정을 내렸었다면, X키를 누르면 어떤 상태든 상관없이 흡수 할 수 있다.
			if ((Get_State() == STATE_IDLE || 
				Get_State() == STATE_RUN || 
				Get_State() == SWORDSTATE_RUN || 
				Get_State() == SWORDSTATE_WAIT ||
				Get_State() == HAMMERSTATE_IDLE ||
				Get_State() == HAMMERSTATE_RUN)
				== false
				)
				return;

			if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && INFO(m_bisDeforming) == false)
			{
				if (INFO(m_pObject) != nullptr)
					return;

				CMultiEffect::MULTI_FX_DESC FXDesc{};
				FXDesc.vInitPos = { 0.f, .6f, .4f };
				FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
				if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Vacuum_v3"), &FXDesc)))
					return;
				Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));


				_float4 vDeformPos = pObject->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
				_float4 vMyPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				_float4 vMyLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
				_float4 vDistance = vDeformPos - vMyPos;
				vDistance.y = 0.f;
				vDistance.Normalize();

				_float fDegree = ToDegree(acos(vDistance.Dot(vMyLook)));

				// 전방 90도가 아닐 경우
				if (fDegree > 45.f)
					return;

				// Deforming을 트루로 만든다. 길게 애니메이션이 재생될 준비를 한다. 이건 밖에서 예외처리 될 것이다.
				INFO(m_bisDeforming) = true;
				INFO(m_vObjectScale) = pObject->Get_TransformCom()->Get_Scaled();
				INFO(m_fObjectDistance) = (vDeformPos - vMyPos).Length();
				INFO(m_pObject) = pObject;
				Safe_AddRef(INFO(m_pObject));
				// 커비가 동일한 애니메이션으로 몬스터를 포착해서 꽤 긴 시간동안 서로 짝짝꿍하겠다는 것이다.
				INFO(m_pObject)->Set_PhyXState(PO_VACUUMING);
				Change_State(CKirby::STATE_VACUUMHUSTLELV2, 50.f, true, true, CKirby::BODY_VACUUM);
			}
		}


	}
}

void CKirby::Ready_BombOrbit()
{
	m_OrbitGlows.reserve(15);

	CBombOrbitGlow* pBombOrbitGlow = { nullptr };

	for (_int i = 0; i < 15; ++i)
	{
		pBombOrbitGlow = static_cast<CBombOrbitGlow*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BombOrbitGlow")));
		if (pBombOrbitGlow == nullptr)
		{
			ALARM_FAIL(TEXT("OrbitGlow Create Failed"));
			return;
		}
		m_OrbitGlows.emplace_back(pBombOrbitGlow);
	}

	m_pOrbit = static_cast<CBombOrbit*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BombOrbit")));
	if (m_pOrbit == nullptr)
	{
		ALARM_FAIL(TEXT("Orbit Create Failed"));
		return;
	}
}

void CKirby::Update_BombOrbit(_float fTimeDelta)
{

	if (INFO(m_bBombOrbit) == true)
	{
		m_fOrbitRenderDelay += fTimeDelta;
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos.y += 0.7f;
		if (m_bInitializeTargetPos == true)
		{
			_float4 vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
			_float fLookOffset = 6.f;
			_float4 FinalTargetDir = (vLook * fLookOffset);
			INFO(m_vBombTargetDir) = FinalTargetDir;
			m_bInitializeTargetPos = false;
		}

		// 실시간 Pos
		INFO(m_vBombTargetPos) = vPos + INFO(m_vBombTargetDir);
		
		// 포물선 생성 0 ~ 15의 거리.
		m_fOrbitTime += fTimeDelta * 0.6f;
		if (m_fOrbitTime > 0.1f)
			m_fOrbitTime -= 0.1f;

		_bool bFind = { false };

		for (_int i = 0; i < 15; i++)
		{
			_float fOrbitTime = m_fOrbitTime + (0.1f * (_float)i);

			_float4 vOriginPos = Compute_Parabola((0.1f * (_float)i), vPos, INFO(m_vBombTargetPos));
			m_OrbitGlows[i]->Update_GlowPosition(
				// 현재 위치를 던진다.
				Compute_Parabola(fOrbitTime, vPos, INFO(m_vBombTargetPos)), 
				// 오리지날 위치를 던진다.
				vOriginPos);

			if (bFind == true)
				continue;

			_float4 vOrbitPos = { 0.f, 0.f, 0.f, 0.f };
			_float4 vOrbitLook = { 0.f, 0.f, 0.f, 0.f };
			_float3 vDir = XMVector3Normalize(Compute_Parabola(0.05f + (0.1f * (_float)i), vPos, INFO(m_vBombTargetPos)) - vOriginPos);
			if (i == 0 && m_fOrbitRenderDelay > 0.5f)
				INFO(m_vBombThrowDir) = XMVectorSetW(vDir, 0.f);

			if (m_OrbitGlows[i]->RayCast_Terrain(vDir, vOrbitPos, vOrbitLook) == true && bFind == false)
			{
				bFind = true;
				m_pOrbit->Update_OrbitPosition(vOrbitPos, vOrbitLook);

				if (m_fOrbitRenderDelay > 0.5f)
					m_pOrbit->Late_Tick(fTimeDelta);
			}
		}
	}
	else
	{

		if (m_bInitializeTargetPos == false)
		{
			m_bInitializeTargetPos = true;
			m_fOrbitRenderDelay = 0.f;
		}

	}
}

_float4 CKirby::Compute_Parabola(_float fOrbitTime, _float4 vStartPos, _float4 vEndPos)
{
	// 중력 가속도 상수 (임의의 값, 필요에 따라 조정 가능)
	const _float fGravity = 9.8f;

	// 포물선 최고점 계산 (시작 y 좌표보다 5만큼 높음)
	const _float fPeakHeight =/* vStartPos.y + */5.0f;

	// 전체 시간 정의 ( 궤적 왕복시간 1.초 )
	const _float fTotalTime = 1.0f;

	// 초기 속도 계산 (y축)
  // 최고점에서의 속도는 0이므로 v0 = sqrt(2 * g * h)에서 유도됩니다.
	_float fInitialVelocityY = -4 * fPeakHeight * (fOrbitTime - 0.5f) * (fOrbitTime - 0.5f) + fPeakHeight;
	// x, z 축 속도
	_float fVelocityX = (vEndPos.x - vStartPos.x) / fTotalTime;
	_float fVelocityZ = (vEndPos.z - vStartPos.z) / fTotalTime;

	//// 현재 시간에서의 위치 계산
	_float fCurrentX = vStartPos.x + fVelocityX * fOrbitTime;
	_float fCurrentY = vStartPos.y + fInitialVelocityY;
	_float fCurrentZ = vStartPos.z + fVelocityZ * fOrbitTime;

	return _float4(fCurrentX, fCurrentY, fCurrentZ, 1.0f);
}

void CKirby::Setting_KirbyBalance()
{
	// 커비는 항상 m_vMoveDir)를 바라본다.
	_float4 vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	m_pTransformCom->Look_At_ForLandObject(vPos + INFO(m_vMoveDir));

	// 보정
	_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vEditRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	vEditRight = XMVector3Normalize(vEditRight);
	_vector vEditLook = XMVector3Cross(vEditRight, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	vEditLook = XMVector3Normalize(vEditLook);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vEditLook));
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vEditRight));
	m_pTransformCom->Set_State(CTransform::STATE_UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));

	// 카메라 기준 실시간 방향 탐색
	CTransform* pCameraTransform = m_pCamera->Get_TransformCom();
	_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	_float4 vCamLook  = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
	_float fCX = vCamLook.x;
	_float fCZ = vCamLook.z;
	_float fKX = INFO(m_vMoveDir).x;
	_float fKZ = INFO(m_vMoveDir).z;
	_float fAngle = (atan2f(fCX, fCZ) * 180.0f / XM_PI) - (atan2f(fKX, fKZ) * 180.0f / XM_PI);
	if (fAngle < 0.f) fAngle += 360.0f;
	if (fAngle >= 337.5f || fAngle < 22.5f)		  INFO(m_eKirbyDir) = DIR_FRONT;
	else if (fAngle >= 22.5f && fAngle < 67.5f)   INFO(m_eKirbyDir) = DIR_LF;
	else if (fAngle >= 67.5f && fAngle < 112.5f)  INFO(m_eKirbyDir) = DIR_LEFT;
	else if (fAngle >= 112.5f && fAngle < 157.5f) INFO(m_eKirbyDir) = DIR_LB;
	else if (fAngle >= 157.5f && fAngle < 202.5f) INFO(m_eKirbyDir) = DIR_BACK;
	else if (fAngle >= 202.5f && fAngle < 247.5f) INFO(m_eKirbyDir) = DIR_RB;
	else if (fAngle >= 247.5f && fAngle < 292.5f) INFO(m_eKirbyDir) = DIR_RIGHT;
	else if (fAngle >= 292.5 && fAngle < 337.5f)  INFO(m_eKirbyDir) = DIR_RF;
}

void CKirby::Key_Input(_float fTimeDelta)
{
#pragma region 커비 연구소 (애니메이션 제어)
	if (m_pGameInstance->Get_DIKeyState(DIK_B, KEY_DOWN))
	{
		Change_State(BULBVACUUMSTATE_DEFORM, 60.f, false, false, BODY_BULBVACUUM, OFFSET_BULBVACUUM);
	}	
#pragma endregion
}

HRESULT CKirby::Make_TargetToCams()
{
	// 첫 카메라 기준으로 움직이기에 미리 받아둔다.
	if (m_pCamera == nullptr)
	{
		//인트로, 게임플레이 스테이지라면 카메라로 main camera를 저장한다.
		(LEVEL_INTRO <= *m_pCurrentLevelID && *m_pCurrentLevelID < LEVEL_END) ?
			m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main"))) :
			m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free"))); //나머지 레벨이라면 다른 카메라를 저장한다.

		if (m_pCamera == nullptr)
		{
			ALARM_FAIL(TEXT("망했어 카메라 없다"));
			return E_FAIL;
		}
		Safe_AddRef(m_pCamera);
	}

	_float3 vAnchor = _float3();
	if (*m_pCurrentLevelID == LEVEL_SIMBA)
		vAnchor = { 0.f, 3.f, 0.f };

	m_pCamera->Set_Target(m_pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FIRST, vAnchor);

	//게임 레벨에 free camera 있다면 그놈에게도 타겟 등록해 준다.
	if (LEVEL_INTRO <= *m_pCurrentLevelID && *m_pCurrentLevelID < LEVEL_END)
	{
		CCamera* pCameraFree = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));
		if (pCameraFree != nullptr)
			pCameraFree->Set_Target(m_pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FIRST);
	}

	return S_OK;
}

HRESULT CKirby::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

#pragma region Kirby Model
	// 커비의 기본 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyDefault"),
		TEXT("Com_Model_Default"), (CComponent**)&m_pModelCom[BODY_DEFAULT]);
	CHECK_FAILED(hr);

	// 커비의 빨아들이는 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyVacuum"),
		TEXT("Com_Model_Vacuum"), (CComponent**)&m_pModelCom[BODY_VACUUM]);
	CHECK_FAILED(hr);

	// 커비의 풍선 모드 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyBalloon"),
		TEXT("Com_Model_Balloon"), (CComponent**)&m_pModelCom[BODY_BALLOON]);
	CHECK_FAILED(hr);

	// 커비의 Sword Body 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbySwordDefault"),
		TEXT("Com_Model_SwordDefault"), (CComponent**)&m_pModelCom[BODY_SWORDDEFAULT]);
	CHECK_FAILED(hr);

	// 커비의 Sword Balloon 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbySwordBalloon"),
		TEXT("Com_Model_SwordBalloon"), (CComponent**)&m_pModelCom[BODY_SWORDBALLOON]);
	CHECK_FAILED(hr);

	// 커비의 Boom Body 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyBoomDefault"),
		TEXT("Com_Model_BoomDefault"), (CComponent**)&m_pModelCom[BODY_BOOMDEFAULT]);
	CHECK_FAILED(hr);

	// 커비의 Car Body Default 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyCarDefault"),
		TEXT("Com_Model_CarDefault"), (CComponent**)&m_pModelCom[BODY_CARDEFAULT]);
	CHECK_FAILED(hr);

	// 커비의 Car Body Vacuum 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyCarVacuum"),
		TEXT("Com_Model_CarVacuum"), (CComponent**)&m_pModelCom[BODY_CARVACUUM]);
	CHECK_FAILED(hr);

	// 커비의 Hammer Default 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyHammerDefault"),
		TEXT("Com_Model_HammerDefault"), (CComponent**)&m_pModelCom[BODY_HAMMER]);
	CHECK_FAILED(hr);

	// 커비의 Bulb Default 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyBulbDefault"),
		TEXT("Com_Model_BulbDefault"), (CComponent**)&m_pModelCom[BODY_BULBDEFAULT]);
	CHECK_FAILED(hr);

	// 커비의 Bulb Vacuum 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyBulbVacuum"),
		TEXT("Com_Model_BulbVacuum"), (CComponent**)&m_pModelCom[BODY_BULBVACUUM]);
	CHECK_FAILED(hr);

	// 커비의 Crash Default 상태 모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyCrashDefault"),
		TEXT("Com_Model_CrashDefault"), (CComponent**)&m_pModelCom[BODY_CRASHDEFAULT]);
	CHECK_FAILED(hr);

#pragma endregion

#pragma region Kirby Eye
 	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_idle"),
		TEXT("Com_Texture_Eye_Idle"), (CComponent**)&m_pEyeTexture[EYE_IDLE]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_doubt"),
		TEXT("Com_Texture_Eye_Doubt"), (CComponent**)&m_pEyeTexture[EYE_SADNESS]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_close"),
		TEXT("Com_Texture_Eye_Close"), (CComponent**)&m_pEyeTexture[EYE_CLOSE]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_blink"),
		TEXT("Com_Texture_Eye_Blink"), (CComponent**)&m_pEyeTexture[EYE_BLINK]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_anger"),
		TEXT("Com_Texture_Eye_Anger"), (CComponent**)&m_pEyeTexture[EYE_ANGER]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_pupil"),
		TEXT("Com_Texture_Eye_Pupil"), (CComponent**)&m_pEyeTexture[EYE_PUPIL]);
	CHECK_FAILED(hr);
#pragma endregion

#pragma region Kirby Mouth
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_base"),
		TEXT("Com_Texture_Mouth_Idle"), (CComponent**)&m_pMouthTexture[MOUTH_IDLE]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_anger"),
		TEXT("Com_Texture_Mouth_Anger"), (CComponent**)&m_pMouthTexture[MOUTH_ANGER]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_happy"),
		TEXT("Com_Texture_Mouth_Happy"), (CComponent**)&m_pMouthTexture[MOUTH_HAPPY]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_smile"),
		TEXT("Com_Texture_Mouth_Smile"), (CComponent**)&m_pMouthTexture[MOUTH_SMILE]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_mouth_surprise"),
		TEXT("Com_Texture_Mouth_Surprise"), (CComponent**)&m_pMouthTexture[MOUTH_SURPRISE]);
	CHECK_FAILED(hr);
#pragma endregion

	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 0.5f;
	desc.tCapsuleShape.fHeight = 0.4f;// 1.f;
	desc.tCapsuleShape.fRadius = 0.4f;// 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom[BODY_DEFAULT];
	m_uModelCnt = BODY_END;

	/* FSM */
	SetUp_FSM();

	/* 구독 시스템 */
	SetUp_Event();

	return S_OK;
}

HRESULT CKirby::Add_PartObjects()
{
	CKirbyWeapons::KIRBYWEAPON_DESC	WeaponDesc{};

	WeaponDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	WeaponDesc.pBoneMatrix = &m_WeaponMatrix;
	WeaponDesc.pAbilityType = &m_eAbilityType;
	WeaponDesc.pWhite = &m_fWhiteColorDiffuse;
	WeaponDesc.pOverPower = &m_fOverPowerColor;
	m_pWeapons = static_cast<CKirbyWeapons*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_KirbyWeapons"), &WeaponDesc));
	CHECK_NULLPTR(m_pWeapons);

	CKirbyArmours::KIRBYARMOURS_DESC ArmourDesc{};
	ArmourDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	ArmourDesc.pBoneMatrix = &m_ArmourMatrix;
	ArmourDesc.pAbilityType = &m_eAbilityType;
	ArmourDesc.pWhite = &m_fWhiteColorDiffuse;
	ArmourDesc.pOverPower = &m_fOverPowerColor;
	m_pArmours = static_cast<CKirbyArmours*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_KirbyArmours"), &ArmourDesc));
	CHECK_NULLPTR(m_pArmours);

	
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = PLAYER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.7f, 0.f, 0.6f);


	HitBox.pDesc = &m_tColliderDesc[ATTACK];
	HitBox.pCollisionType = HITBOX_PLYAER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	//Activate_FrustumCollider(0.5f, 4.f, 90.f);


	return S_OK;
}

HRESULT CKirby::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

_bool CKirby::Kirby_FaceCustom(BODYSTATE _eBodyState, _uint _iMeshIndex)
{
	if (m_iRenderCount == 0)
	{
		if (
			(_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 2) ||
			(_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 3) ||
			(_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 1)
			)
		{
			//m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
			m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
			_bool bRimLight = false;
			m_pShaderCom->Bind_RawValue("g_vBulbColor", &m_vBulbColor, sizeof(_float4));
			m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

			m_pShaderCom->Begin(ANIMMODEL_BULBLIGHT);
			m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
			return true;
		}
		else if ((_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 4))
		{
			m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
			m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
			m_pEyeTexture[INFO(m_eEyeState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", 0);

			m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
			m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

			m_pShaderCom->Begin(ANIMMODEL_KIRBYEYE);
			m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
		}
	}
	else
	{
		// Default 상태의 입 부위 // Balloon 상태의 입 부위
		if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 0) ||
			(_eBodyState == BODY_BALLOON && _iMeshIndex == 4) ||
			(_eBodyState == BODY_SWORDDEFAULT && _iMeshIndex == 0) ||
			(_eBodyState == BODY_SWORDBALLOON && _iMeshIndex == 4) ||
			(_eBodyState == BODY_BOOMDEFAULT && _iMeshIndex == 0) ||
			(_eBodyState == BODY_HAMMER && _iMeshIndex == 0))
		{
			m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
			m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
			m_pMouthTexture[INFO(m_eMouthState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyMouthTexture", 0);

			m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
			m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));


			m_pShaderCom->Begin(ANIMMODEL_KIRBYMOUTH);
			m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
			return true;
		}
		// Default 상태의 눈 부위 // Vacuum 상태의 눈 부위 // Balloon 상태의 눈 부위
		else if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 3) ||
			(_eBodyState == BODY_VACUUM && _iMeshIndex == 2) ||
			(_eBodyState == BODY_BALLOON && _iMeshIndex == 3) ||
			(_eBodyState == BODY_SWORDDEFAULT && _iMeshIndex == 3) ||
			(_eBodyState == BODY_SWORDBALLOON && _iMeshIndex == 3) ||
			(_eBodyState == BODY_BOOMDEFAULT && _iMeshIndex == 3) ||
			(_eBodyState == BODY_CARDEFAULT && _iMeshIndex == 3) ||
			(_eBodyState == BODY_HAMMER && _iMeshIndex == 3) ||
			(_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 4))
		{
			m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
			m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
			m_pEyeTexture[INFO(m_eEyeState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", 0);

			m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
			m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

			m_pShaderCom->Begin(ANIMMODEL_KIRBYEYE);
			m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
			return true;
		}
		// Vacuum 상태의 구강 부위
		else if (_eBodyState == BODY_VACUUM && _iMeshIndex == 3)
		{
			m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
			m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);

			_bool bRimLight = false;
			m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

			m_pShaderCom->Begin(ANIMMODEL_NORMAL_X);
			m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
			return true;
		}
		else if (
			(_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 2) ||
			(_eBodyState == BODY_BULBDEFAULT && _iMeshIndex == 3)
			)

		{
			m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
			m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);

			_bool bRimLight = false;
			m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
			m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));

			m_pShaderCom->Begin(ANIMMODEL_ALPHABLEND);
			m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
			return true;
		}

	}

	return false;
}

void CKirby::SetUp_FSM()
{
	m_pFSM = CFSM::Create();

	// Damege or Death
	m_pFSM->Add_State(STATE_DAMAGE, CKirbyDamage_State::Create());
	m_pFSM->Add_State(STATE_EATDAMAGE, CKirbyDamage_State::Create());
	m_pFSM->Add_State(STATE_FILGHTDAMAGE, CKirbyDamage_State::Create());

	// Default
	m_pFSM->Add_State(STATE_IDLE, CKirbyDefault_Idle_State::Create());
	m_pFSM->Add_State(STATE_IDLESTREACH, CKirbyDefault_Idle_State::Create());
	m_pFSM->Add_State(STATE_IDLELOOKAROUND, CKirbyDefault_Idle_State::Create());

	m_pFSM->Add_State(STATE_RUN, CKirbyDefault_Run_State::Create());
	m_pFSM->Add_State(STATE_RUNSTART, CKirbyDefault_Run_State::Create());

	m_pFSM->Add_State(STATE_JUMPL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPR, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPEND, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPFALL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_LANDINGEND, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_LANDINGSMALL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_FALL, CKirbyDefault_Jump_State::Create());

	// Slide
	m_pFSM->Add_State(STATE_SLIDESTART, CKirbyDefault_Slide_State::Create());
	m_pFSM->Add_State(STATE_SLIDE, CKirbyDefault_Slide_State::Create());
	m_pFSM->Add_State(STATE_SLIDEEND, CKirbyDefault_Slide_State::Create());

	// Happy
	m_pFSM->Add_State(STATE_WAITYAY, CKirbyDefault_Happy_State::Create());
	m_pFSM->Add_State(STATE_WAITSIT, CKirbyDefault_Happy_State::Create());
	m_pFSM->Add_State(STATE_EMOTEWAVEHAND, CKirbyDefault_Happy_State::Create());


	// 가드 및 덤블링
	m_pFSM->Add_State(STATE_DODGEBACK1, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGEBACK2, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGEFRONT1, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGEFRONT2, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGELEFT1, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGELEFT2, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGERIGHT1, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGERIGHT2, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_DODGESTART, CKirbyDefault_Guard_State::Create());
	m_pFSM->Add_State(STATE_GUARD, CKirbyDefault_Guard_State::Create());

	// Balloon
	m_pFSM->Add_State(STATE_EAT, CKirbyBalloon_Idle_State::Create());
	m_pFSM->Add_State(STATE_EATWAIT, CKirbyBalloon_Idle_State::Create());
	m_pFSM->Add_State(STATE_EATRUN, CKirbyBalloon_Run_State::Create());
	m_pFSM->Add_State(STATE_EATJUMP, CKirbyBalloon_Jump_State::Create());
	m_pFSM->Add_State(STATE_EATLANDING, CKirbyBalloon_Jump_State::Create());

	m_pFSM->Add_State(STATE_FLIGHTSTART, CKirbyBalloon_Fly_State::Create());
	m_pFSM->Add_State(STATE_FLIGHTFALL, CKirbyBalloon_Fly_State::Create());
	m_pFSM->Add_State(STATE_FLIGHT, CKirbyBalloon_Fly_State::Create());
	m_pFSM->Add_State(STATE_FLIGHTLANDING, CKirbyBalloon_Fly_State::Create());
	m_pFSM->Add_State(STATE_FLIGHTLIMIT, CKirbyBalloon_Fly_State::Create());
	m_pFSM->Add_State(STATE_FLIGHTLIMITFALL, CKirbyBalloon_Fly_State::Create());

	m_pFSM->Add_State(STATE_SWALLOWSTART, CKirbyBalloon_Swallow_State::Create());
	m_pFSM->Add_State(STATE_SWALLOWEND, CKirbyBalloon_Swallow_State::Create());

	// Vacuum
	m_pFSM->Add_State(STATE_SPIT, CKirbyVacuum_Spit_State::Create());//

	m_pFSM->Add_State(STATE_INHALESTART, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALEEND, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALEFALL, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALE, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALELANDING, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_SUPERINHALE, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_SUPERINHALESTART, CKirbyVacuum_Vacuum_State::Create());

	m_pFSM->Add_State(STATE_INHALEWALK, CKirbyVacuum_VacuumWalk_State::Create());
	m_pFSM->Add_State(STATE_SUPERINHALEWALK, CKirbyVacuum_VacuumWalk_State::Create());

	// Vacuuming
	m_pFSM->Add_State(STATE_VACUUM, CKirbyVacuum_Vacuuming_State::Create());
	m_pFSM->Add_State(STATE_VACUUMHUSTLELV2, CKirbyVacuum_Vacuuming_State::Create());

	// Get
	m_pFSM->Add_State(STATE_GETABILITY, CKirbyGet_State::Create());
	m_pFSM->Add_State(STATE_ITEMGET, CKirbyGet_State::Create());
	m_pFSM->Add_State(STATE_ITENGETWAIT, CKirbyGet_State::Create());
	m_pFSM->Add_State(STATE_ABILITYDUMP, CKirbyGet_State::Create());

#pragma region SWORD
	// For Sword
	m_pFSM->Add_State(SWORDSTATE_WAIT, CKirbySword_Idle_State::Create());
	m_pFSM->Add_State(SWORDSTATE_RUN, CKirbySword_Run_State::Create());
	m_pFSM->Add_State(SWORDSTATE_GUARD, CKirbySword_Guard_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SWORDSLIDESTART, CKirbySword_Guard_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SWORDSLIDE, CKirbySword_Guard_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SWORDSLIDEEND, CKirbySword_Guard_State::Create());
	m_pFSM->Add_State(SWORDSTATE_HAVESWORDWAITFLIGHT, CKirbySword_Fly_State::Create());
	// 1타
	m_pFSM->Add_State(SWORDSTATE_SIDESLASH, CKirbySword_Attack_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SIDESLASHEND, CKirbySword_Attack_State::Create());
	// 2타
	m_pFSM->Add_State(SWORDSTATE_MULITSWORDATTACK, CKirbySword_Attack_State::Create());
	// 3타
	m_pFSM->Add_State(SWORDSTATE_DECISIVESLASH, CKirbySword_Attack_State::Create());
	// 충전 모션 및 회전베기
	m_pFSM->Add_State(SWORDSTATE_SPINSLASHCHARGE, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SUPERSPINSLASHCHARGESTART, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SUPERSPINSLASHCHARGE, CKirbySword_ChargeSpin_State::Create());

	m_pFSM->Add_State(SWORDSTATE_SHUFFIEFRONT, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SHUFFIERIGHT, CKirbySword_ChargeSpin_State::Create());

	m_pFSM->Add_State(SWORDSTATE_GIGANTSPINSLASH, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SUPERSPINSLASHSTART, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SUPERSPINSLASHLOOP, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SPINSLASHEND, CKirbySword_ChargeSpin_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SUPERSPINSLASHEND, CKirbySword_ChargeSpin_State::Create());

	m_pFSM->Add_State(SWORDSTATE_UPWARDSLASH, CKirbySword_JumpAttack_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SWORDDIVE, CKirbySword_JumpAttack_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SWORDSPIN, CKirbySword_JumpAttack_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SWORDSPINSTART, CKirbySword_JumpAttack_State::Create());
	m_pFSM->Add_State(SWORDSTATE_SPINAFTER, CKirbySword_JumpAttack_State::Create());
#pragma endregion

#pragma region BOOM
	// For BOMB
	m_pFSM->Add_State(BOOMSTATE_BOOMFALL, CKirbyBoom_Fall_State::Create());
	m_pFSM->Add_State(BOOMSTATE_BOOMSHOOT, CKirbyBoom_Attack_State::Create());
	m_pFSM->Add_State(BOOMSTATE_THROW, CKirbyBoom_Attack_State::Create());
	m_pFSM->Add_State(BOOMSTATE_THROWAIR, CKirbyBoom_Fall_State::Create());
	m_pFSM->Add_State(BOOMSTATE_THROWCHARGE, CKirbyBoom_ChargeAttack_State::Create());
	m_pFSM->Add_State(BOOMSTATE_THROWROTATE, CKirbyBoom_ChargeAttack_State::Create());
#pragma endregion

#pragma region 사다리 타기
	m_pFSM->Add_State(STATE_LADDERDOWN, CKirbyDefault_Ladder_State::Create());
	m_pFSM->Add_State(STATE_LADDERUP, CKirbyDefault_Ladder_State::Create());
	m_pFSM->Add_State(STATE_LADDERWAIT, CKirbyDefault_Ladder_State::Create());
	m_pFSM->Add_State(STATE_LADDERWAITSTART, CKirbyDefault_Ladder_State::Create());
#pragma endregion

	m_pFSM->Add_State(STATE_SPITDEFORM, CKirbyVacuum_Spit_State::Create());

#pragma region 자동차 애니메이션
	m_pFSM->Add_State(CARSTATE_IDLING, CKirbyCar_Idle_State::Create()); //

	m_pFSM->Add_State(CARSTATE_MOVING, CKirbyCar_Run_State::Create()); //

	m_pFSM->Add_State(CARSTATE_JUMPSTART, CKirbyCar_Jump_State::Create()); //
	m_pFSM->Add_State(CARSTATE_JUMP, CKirbyCar_Jump_State::Create()); //
	m_pFSM->Add_State(CARSTATE_FALL, CKirbyCar_Jump_State::Create()); //
	m_pFSM->Add_State(CARSTATE_LANDING, CKirbyCar_Jump_State::Create()); //

	m_pFSM->Add_State(CARVACUUMSTATE_DEFORM, CKirbyCar_Vacuum_State::Create()); //
	m_pFSM->Add_State(CARSTATE_DEMOEND, CKirbyCar_Vacuum_State::Create()); //

	m_pFSM->Add_State(CARSTATE_BOOST, CKirbyCar_Boost_State::Create());
	m_pFSM->Add_State(CARSTATE_BOOSTEND, CKirbyCar_Boost_State::Create());
	m_pFSM->Add_State(CARSTATE_CRASH, CKirbyCar_Boost_State::Create());

	m_pFSM->Add_State(CARSTATE_DAMAGE, CKirbyCar_Damage_State::Create()); //

	m_pFSM->Add_State(CARSTATE_CUT1, CKirbyCar_Cut_State::Create()); //
	m_pFSM->Add_State(CARSTATE_CUT2, CKirbyCar_Cut_State::Create()); //
#pragma endregion

#pragma region 해머 애니메이션
	m_pFSM->Add_State(HAMMERSTATE_IDLE, CKirbyHammer_Idle_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_RUN, CKirbyHammer_Idle_State::Create()); //

	m_pFSM->Add_State(HAMMERSTATE_JUMPL, CKirbyHammer_Jump_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_JUMPR, CKirbyHammer_Jump_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_JUMPEND, CKirbyHammer_Jump_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_LANDINGEND, CKirbyHammer_Jump_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_LANDINGSMALL, CKirbyHammer_Jump_State::Create()); //


	m_pFSM->Add_State(HAMMERSTATE_HAMMERATTACKSTARTTOY, CKirbyHammer_Attack_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_HAMMERATTACKTOY, CKirbyHammer_Attack_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_HAMMERATTACKHITTOY, CKirbyHammer_Attack_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_HAMMERATTACKFINALTOY, CKirbyHammer_Attack_State::Create()); //

	m_pFSM->Add_State(HAMMERSTATE_ONIGOROSIHAMMERSTART, CKirbyHammer_Onigorosi_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_ONIGOROSIHAMMERCHARGE, CKirbyHammer_Onigorosi_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_ONIGOROSIHAMMERMOVE, CKirbyHammer_Onigorosi_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_ONIGOROSIHAMMERFIRST, CKirbyHammer_Onigorosi_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_ONIGOROSIHAMMEREND, CKirbyHammer_Onigorosi_State::Create()); //

	m_pFSM->Add_State(HAMMERSTATE_WHEELHAMMER, CKirbyHammer_JumpAttack_State::Create()); //
	m_pFSM->Add_State(HAMMERSTATE_WHEELHAMMEREND, CKirbyHammer_JumpAttack_State::Create()); //
#pragma endregion

#pragma region 전구 애니메이션
	m_pFSM->Add_State(BULBSTATE_DAMAGE, CKirbyBulb_Damage_State::Create());

	m_pFSM->Add_State(BULBSTATE_DEMOENDFIRST, CKirbyBulb_Vacuum_State::Create());
	m_pFSM->Add_State(BULBVACUUMSTATE_DEFORM, CKirbyBulb_Vacuum_State::Create());

	m_pFSM->Add_State(BULBSTATE_WAIT, CKirbyBulb_Idle_State::Create());
	m_pFSM->Add_State(BULBSTATE_WAITBRIGHT, CKirbyBulb_Idle_State::Create());

	m_pFSM->Add_State(BULBSTATE_MOVE, CKirbyBulb_Run_State::Create());
	m_pFSM->Add_State(BULBSTATE_MOVEBRIGHT, CKirbyBulb_Run_State::Create());
	m_pFSM->Add_State(BULBSTATE_STOP, CKirbyBulb_Run_State::Create());
	m_pFSM->Add_State(BULBSTATE_STOPBRIGHT, CKirbyBulb_Run_State::Create());

	m_pFSM->Add_State(BULBSTATE_LIGHTON, CKirbyBulb_Light_State::Create());
	m_pFSM->Add_State(BULBSTATE_LIGHTOFF, CKirbyBulb_Light_State::Create());

	m_pFSM->Add_State(BULBSTATE_LIGHTONAIR, CKirbyBulb_Jump_State::Create());
	m_pFSM->Add_State(BULBSTATE_JUMP, CKirbyBulb_Jump_State::Create());
	m_pFSM->Add_State(BULBSTATE_LANDING, CKirbyBulb_Jump_State::Create());
	m_pFSM->Add_State(BULBSTATE_LANDINGBRIGHT, CKirbyBulb_Jump_State::Create());
	m_pFSM->Add_State(BULBSTATE_LANDINGEND, CKirbyBulb_Jump_State::Create());
	m_pFSM->Add_State(BULBSTATE_LANDINGENDBRIGHT, CKirbyBulb_Jump_State::Create());
	m_pFSM->Add_State(BULBSTATE_FALL, CKirbyBulb_Jump_State::Create());
#pragma endregion

#pragma region 크래쉬 애니메이션
	m_pFSM->Add_State(CRASHSTATE_ATTACKCHARGE, CKirbyCrash_Charge_State::Create());
	m_pFSM->Add_State(CRASHSTATE_ATTACKCHARGEMOVE, CKirbyCrash_Charge_State::Create());
	m_pFSM->Add_State(CRASHSTATE_ATTACKCHARGESTART, CKirbyCrash_Charge_State::Create());
	m_pFSM->Add_State(CRASHSTATE_ATTACK, CKirbyCrash_Attack_State::Create());
	m_pFSM->Add_State(CRASHSTATE_ATTACKEND, CKirbyCrash_Attack_State::Create());
	m_pFSM->Add_State(CRASHSTATE_ATTACKSTART, CKirbyCrash_Attack_State::Create());

	m_pFSM->Add_State(CRASHSTATE_BIGATTACKCHARGE, CKirbyCrash_BigCharge_State::Create());
	m_pFSM->Add_State(CRASHSTATE_BIGATTACKCHARGEMOVE, CKirbyCrash_BigCharge_State::Create());
	m_pFSM->Add_State(CRASHSTATE_BIGATTACKCHARGESTART, CKirbyCrash_BigCharge_State::Create());
	m_pFSM->Add_State(CRASHSTATE_BIGATTACK, CKirbyCrash_BigAttack_State::Create());
	m_pFSM->Add_State(CRASHSTATE_BIGATTACKEND, CKirbyCrash_BigAttack_State::Create());
	m_pFSM->Add_State(CRASHSTATE_BIGATTACKSTART, CKirbyCrash_BigAttack_State::Create());

	// 이게 왜있어??
	m_pFSM->Add_State(CRASHSTATE_BIGATTACKFIRE, CKirbyCrash_BigAttack_State::Create());
#pragma endregion


	CFSM::FSM_INFO		FSM_Info_Desc = {};
	FSM_Info_Desc.iState = STATE_IDLE;
	FSM_Info_Desc.uNumModel = BODY_END;
	FSM_Info_Desc.pModel = &m_pModelCom[BODY_DEFAULT];
	m_pFSM->Initialize(&FSM_Info_Desc);

}

void CKirby::SetUp_Event()
{
	//셔터 뿌수기
	function<void(CGameObject*)> func = bind(&CKirby::Event_Racing_Cut1, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_BREAK_CARSHOP, this, func, 1);

	//다리 뿌수기
	func = bind(&CKirby::Event_Racing_Cut2, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_BREAK_RACINGMAP, this, func, 1);


}
// 레이싱맵 컷씬 1.
void CKirby::Event_Racing_Cut1(CGameObject* pObj)
{
	Delete_Effect("Come On Dash");

	INFO(m_bBooster) = false;
	INFO(m_bCarJump) = false;
	CKirby::Change_State(CKirby::CARSTATE_CUT1, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
	m_pGameInstance->Set_FirstTimerRatio(0.2f);
	m_pGameInstance->Set_SecondTimerRatio(0.2f);
	m_pGameInstance->Setting_RadialBlur(20.f, 10.f);

	CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
	pCamera->Make_Shake(2.f);

	m_pControllerCom->Set_Position(m_pTransformCom, { -179.f, 41.f, -117.f, 1.f });
	//_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK) * 6.f;
	//m_pControllerCom->Move_Dir(m_pTransformCom, vLook + _float4(0.f, 2.f, 0.f, 0.f), m_fTimeDelta);
}

void CKirby::Event_Racing_Cut2(CGameObject* pObj)
{
	Delete_Effect("Come On Dash");

	INFO(m_bBooster) = false;
	INFO(m_bCarJump) = false;
	INFO(m_fMoveSpeed) = 0.f;
	m_pControllerCom->Set_Position(m_pTransformCom, { 57.82f, 23.11f, 80.33f, 1.f });

	CKirby::Change_State(CKirby::CARSTATE_CUT2, 60.f, false, false, CKirby::BODY_CARDEFAULT, CKirby::OFFSET_CAR);
}

void CKirby::HitBoxChanger(_uint eState)
{
	switch (eState)
	{
	// 덜 차징 스핀
	case SWORDSTATE_GIGANTSPINSLASH:
		Activate_SphereCollider(0.5f, 4.f);
		break;
	// 슈퍼 차징 스핀
	case SWORDSTATE_SUPERSPINSLASHLOOP:
		Activate_SphereCollider(0.5f, 4.f);
		break;
	// 칼 1타
	case SWORDSTATE_SIDESLASH:
		Activate_FrustumCollider(0.5f, 4.f, 180.f);
		break;
	// 칼 2타
	case SWORDSTATE_MULITSWORDATTACK:
		Activate_FrustumCollider(0.5f, 4.f, 180.f);
		break;
	// 칼 3타
	case SWORDSTATE_DECISIVESLASH:
		Activate_FrustumCollider(0.5f, 5.f, 180.f);
		break;
	// 공중제비 도는 공격
	case SWORDSTATE_SWORDSPIN:
		Activate_SphereCollider(0.5f, 5.f);
		break;
	// 공중어퍼컷 형식의 공격
	case SWORDSTATE_UPWARDSLASH:
		Activate_FrustumCollider(0.5f, 5.f, 90.f);
		break;
	// 해머 5타 통 애님 (막타)
	case HAMMERSTATE_HAMMERATTACKFINALTOY:
		Activate_FrustumCollider(0.5f, 5.f, 180.f);
		break;
	// 평타
	case HAMMERSTATE_HAMMERATTACKHITTOY:
		Activate_FrustumCollider(0.5f, 5.f, 180.f);
		break;
	// 해머 풀 차징 공격
	case HAMMERSTATE_ONIGOROSIHAMMEREND:
		Activate_FrustumCollider(0.5f, 6.f, 180.f);
		break;
	// 해머 덜 차징 공격
	case HAMMERSTATE_ONIGOROSIHAMMERFIRST:
		Activate_FrustumCollider(0.5f, 6.f, 180.f);
		break;
	// 해머 공중 회전 공격
	case HAMMERSTATE_WHEELHAMMER:
		Activate_SphereCollider(0.5f, 5.f);
		break;
		// 해머 공중 회전 공격
	case CRASHSTATE_ATTACK:
		Activate_SphereCollider(1.f, 15.f);
		break;
		// 해머 공중 회전 공격
	case CRASHSTATE_BIGATTACK:
		Activate_SphereCollider(1.f, 15.f);
		break;

	default:
		break;
	}
	m_isKirbyAttacking = true;
}

void CKirby::RayCast_Crumbles()
{
	if (m_pControllerCom == nullptr) return;

	_float4 vDown = _float4(0.f, 1.f, 0.f, 0.f);
	if (m_pControllerCom->RayCastToDynamicActor(vDown, _float3(0, -1, 0)) < 0.5f)
	{
		CGameObject* pObj = FindBox(m_pControllerCom->Get_MostRecentActor());
		if (nullptr != pObj)
		{
			CCrumble* pCrumble = static_cast<CCrumble*>(pObj);
			pCrumble->Break_Crumble();
			return;
		}
	}
}

//LEVEL_PARK의 BlubZone 다이나믹필드와의 체크
void CKirby::RayCast_DynamicFields()
{
	if (m_pControllerCom == nullptr) 
		return;

	/*_vector vLook = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 0));
	_vector vRight = XMVector3Normalize(XMVectorSetY(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), 0));*/

	_vector vLook = XMVectorSet(0, 0, 1, 0);
	_vector vRight = XMVectorSet(1, 0, 0, 0);

	_float fLookOffset = 5.f;
	_float fActivationDistance = 6.f;

	if (20.f <= m_pControllerCom->RayCastToStaticActor(-vRight, 20.f, vLook))
	{
		_float fLeftDis = m_pControllerCom->RayCastToDynamicActor(-vRight, vLook * fLookOffset);
		if (fActivationDistance > fLeftDis)
		{
			CGameObject* pObj = FindDynamicField(m_pControllerCom->Get_MostRecentActor());
			if (nullptr != pObj)
			{
				CGm_DynamicField* pDynamicField = dynamic_cast<CGm_DynamicField*>(pObj);
				if (false == pDynamicField->IsActivated())
					pDynamicField->Set_Interaction(true);
				return;
			}

		}
	}

	if (20.f <= m_pControllerCom->RayCastToStaticActor(vRight, 20.f, vLook))
	{
		_float fRightDis = m_pControllerCom->RayCastToDynamicActor(vRight, vLook * fLookOffset);
		if (fActivationDistance > fRightDis)
		{
			CGameObject* pObj = FindDynamicField(m_pControllerCom->Get_MostRecentActor());
			if (nullptr != pObj)
			{
				CGm_DynamicField* pDynamicField = dynamic_cast<CGm_DynamicField*>(pObj);
				if (false == pDynamicField->IsActivated())
					pDynamicField->Set_Interaction(true);
				return;
			}

		}
	}
}

void CKirby::Update_PartObjectMatrix()
{
	if ((INFO(m_eBodyState) == BODY_CARDEFAULT || INFO(m_eBodyState) == BODY_CARVACUUM || 
		INFO(m_eBodyState) == BODY_BULBDEFAULT || INFO(m_eBodyState) == BODY_BULBVACUUM) == false)
		m_WeaponMatrix = *(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("RHaveL")->Get_CombinedTransformationMatrix());

	m_ArmourMatrix = *(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("HatL")->Get_CombinedTransformationMatrix());
}

void CKirby::Bone_Rotation(_float fTimeDelta)
{
	// 자동차일때,
	if (INFO(m_eBodyState) == BODY_CARDEFAULT)
	{
		_float fTurnAngle = -INFO(m_fMoveSpeed) * 100.f;

		CBone* pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("L_BTireJ");
		_float4x4* BoneMatrix = pBone->Get_EditMatrixPtr();
		CUtils::Turn_OtherMatrix(*BoneMatrix, _float4(1.f, 0.f, 0.f, 0.f), fTimeDelta, fTurnAngle);

		pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("L_FTireJ");
		BoneMatrix = pBone->Get_EditMatrixPtr();
		CUtils::Turn_OtherMatrix(*BoneMatrix, _float4(1.f, 0.f, 0.f, 0.f), fTimeDelta, fTurnAngle);

		pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("R_BTireJ");
		BoneMatrix = pBone->Get_EditMatrixPtr();
		CUtils::Turn_OtherMatrix(*BoneMatrix, _float4(1.f, 0.f, 0.f, 0.f), fTimeDelta, fTurnAngle);

		pBone = m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("R_FTireJ");
		BoneMatrix = pBone->Get_EditMatrixPtr();
		CUtils::Turn_OtherMatrix(*BoneMatrix, _float4(1.f, 0.f, 0.f, 0.f), fTimeDelta, fTurnAngle);
	}


}

void CKirby::Set_WeaponAnim(_uint index)
{
	m_pWeapons->Change_My_WeaponAnim((CKirbyWeapons::ANIM_TYPE)index);
}

_float4 CKirby::Get_BulbLightPos()
{
	CBone* pBone = m_pModelCom[BODY_BULBDEFAULT]->Get_BonePtr("LightL");
	_float4x4 CombineMatrix = *pBone->Get_CombinedTransformationMatrix();
	_float4 vPos = CUtils::Get_State_Vector_Matrix(CombineMatrix, CUtils::STATE_POSITION);
	vPos = _float4::Transform(vPos, m_pTransformCom->Get_WorldFloat4x4());

	return vPos;
}

void CKirby::Set_ControllerPos(_float4 _vPosition)
{
	m_pControllerCom->Set_Position(m_pTransformCom, _vPosition);
}

void CKirby::Large_Light(_float4 vDiffuse, _float fRange, _float fTime)
{
	if (m_pArmours == nullptr)
		return;

	m_pArmours->Large_Light(vDiffuse, fRange, fTime);
}

void CKirby::OverPower()
{
	if (m_fPreHp > m_fHp)
	{
		m_bOverPower = true;
	}

	if (m_bOverPower == true)
	{
		m_fOverPowerTime += m_fTimeDelta;
		m_fFlashOverPowerTime += m_fTimeDelta;
		_float fFlashTime = 0.02f;

		if (m_fFlashOverPowerTime > fFlashTime)
		{
			m_fOverPowerColor = m_fOverPowerColor == 0.f ? 0.25f : 0.f;
			m_fFlashOverPowerTime -= fFlashTime;
		}

		if (m_fOverPowerTime > 3.f)
		{
			m_bOverPower = false;
			m_fOverPowerTime = 0.f;
			m_fOverPowerColor = 0.f;
			m_fFlashOverPowerTime = 0.f;
		}
	}


	m_fPreHp = m_fHp;
}

void CKirby::HitStop_System(_float fTimeDelta)
{
	if (m_bHitStop == true)
	{
		m_fTimeDelta = 0.f;
		m_fHitStopTime += fTimeDelta;

		if (m_fHitStopTime > m_fHitStopMaxTime)
		{
			m_fHitStopTime = 0.f;
			m_bHitStop = false;
		}
	}
}

void CKirby::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody, _uint iOffSet)
{
	INFO(m_eBodyState) = eBody;

	if ( INFO(m_eBodyState) == BODY_CARDEFAULT )
		Set_BodyCollider(COLLIDER_SPHERE, 1.f, 0.f, 2.f);
	else if (INFO(m_eBodyState) == BODY_BULBDEFAULT )
		Set_BodyCollider(COLLIDER_CYLINDER, 1.5f, 3.f, 1.f);
	else
		Set_BodyCollider(COLLIDER_SPHERE, 0.7f, 0.f, 0.6f);


	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation, INFO(m_eBodyState), iOffSet);
}

void CKirby::Set_Animation(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	if (m_pModelCom[INFO(m_eBodyState)] == nullptr)
		return;

	m_pModelCom[INFO(m_eBodyState)]->Set_Animation(eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirby::Set_Animation(_int _iAnimIndex)
{
	m_pModelCom[INFO(m_eBodyState)]->Set_Animation(_iAnimIndex);
}

_bool CKirby::isAnimFinish()
{
	if (m_pModelCom[INFO(m_eBodyState)] == nullptr)
		return false;

	return m_pModelCom[INFO(m_eBodyState)]->IsFinished();
}

_float CKirby::Get_AnimTrackPosition()
{
	return m_pModelCom[m_tKirbyInfo.m_eBodyState]->Get_AnimTrackPosition();
}

void CKirby::DefaultIdle()
{
	if (m_pModelCom[BODY_DEFAULT] == nullptr)
		return;

	m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, false);
}

void CKirby::Kirby_SystemTick(_float fTimeDelta)
{
	// 그림자는 무조건 커비를 따라간다.
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vLightPos = vPos;
	vLightPos.m128_f32[1] += 100.f;
	vLightPos.m128_f32[2] -= 1.f;
	m_pGameInstance->Update_LightShadow(vLightPos, vPos);

	// Dof 초점을 커비에게 맞춘다.
	_vector vDOFPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vDOFPos.m128_f32[1] += 0.5f;
	m_pGameInstance->Update_DofFocus(vDOFPos);

	// 능력이 SWORD 일때, 평타 모션의 순서를 리셋시키는 로직이다.
	if (m_eAbilityType == ABILITY_SWORD)
	{
		// 이 모션이 아닐때만 감소한다.
		if ((Get_State() == SWORDSTATE_SIDESLASH ||
			Get_State() == SWORDSTATE_SIDESLASHEND ||
			Get_State() == SWORDSTATE_MULITSWORDATTACK ||
			Get_State() == SWORDSTATE_DECISIVESLASH) == false)
		{
			if (INFO(m_fAttackTime) > 0.f)
				INFO(m_fAttackTime) -= fTimeDelta;

			if (INFO(m_fAttackTime) < 0.f)
			{
				INFO(m_fAttackTime) = 0.f;
				INFO(m_ePreAttackState) = SWORDSTATE_DECISIVESLASH;
			}
		}
	}


	// 물고 있을 때, 물고있는 객체를 계속 나의 입에 위치시키는 로직이다. Vacuum State 에서 강제로 흡수시키고 충돌처리가 되었을 때 PO_KIRBYMOUTH 상태로 변경될 것이다.
	if (INFO(m_pObject) != nullptr)
	{
		// 커비 입 속에 있다면?
		if (INFO(m_pObject)->Get_PhyXState() == PO_KIRBYMOUTH)
		{
			// 이곳에서도 마찬가지. 컨트롤러를 쓰고 있는지 쓰고 있지 않은지 탐색을 한다.
			CCharacterController* pObjectController = static_cast<CCharacterController*>(INFO(m_pObject)->Get_Component(TEXT("Com_Controller")));

			// 만약, 컨트롤러를 쓰지 않고 있다고 판단되었다면?
			if (pObjectController == nullptr)
			{
				// 직접 트랜스폼을 움직여서 내 위치에서 위쪽으로 맞춰준다.
				CTransform* pObjectTransform = INFO(m_pObject)->Get_TransformCom();
				_vector vMouthPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				vMouthPos.m128_f32[1] += 1.f;
				pObjectTransform->Set_State(CTransform::STATE_POSITION, vMouthPos);
			}
			// 만약, 컨트롤러를 쓰고 있다고 판단될경우? 내 위치에서 위쪽으로 강제로 맞춰준다.
			else
			{
				CTransform* pObjectTransform = INFO(m_pObject)->Get_TransformCom();
				_vector vMouthPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				vMouthPos.m128_f32[1] += 1.f;
				pObjectController->Set_Position(pObjectTransform, vMouthPos);
			}
		}
	}

	// 무적 상태 관리소
	OverPower();

	// 커비의 폭탄 궤적을 생성한다.
	Update_BombOrbit(fTimeDelta);

	// 특정 상황에서 뼈를 돌려준다.
	Bone_Rotation(fTimeDelta);

	// 사다리 상태 초기화
	INFO(m_bCanLadder) = false;

	// 블락 상태 초기화
	INFO(m_bBlockOtherVacuum) = false;

	// 커비가 공격중일땐, 꽤나 오랜 시간동안 무적을 부여받는다.
	if (m_isKirbyAttacking == true)
	{
		// 타임델타를 누적받고
		m_fIsAttackTime += fTimeDelta;
		if (m_fIsAttackTime > 0.5f)
		{
			m_fIsAttackTime = 0.f;
			m_isKirbyAttacking = false;
		}
	}


	// 모션블러가 들어가면 어색한 곳을 해소한다.
	if (m_pFSM->Get_State() == CARSTATE_CUT2)
		m_bMotionBlur = false;
	else
		m_bMotionBlur = true;


	if (INFO(m_eBodyState) == BODY_BULBDEFAULT)
	{
		if (INFO(m_bLightOn) == true)
		{
			_float4 vTargetColor = { 1.f, 1.f, 1.f, 1.f };
			m_vBulbColor += (vTargetColor - m_vBulbColor) / (fTimeDelta * 300.f);

			m_pBulbFlare->Set_Position(m_pTransformCom->Get_State(CTransform::STATE_POSITION), true);

		}
		else if (INFO(m_bLightOn) == false)
		{
			_float4 vTargetColor = { 0.3f, 0.1f, 0.1f, 0.25f };
			m_vBulbColor += (vTargetColor - m_vBulbColor) / (fTimeDelta * 300.f);

			m_pBulbFlare->Set_Position(m_pTransformCom->Get_State(CTransform::STATE_POSITION), false);
		}

		m_pBulbFlare->Tick(fTimeDelta);
		m_pBulbFlare->Late_Tick(fTimeDelta);

	}


	if (INFO(m_bDumpAbilityPress) == true &&
		(m_pFSM->Get_State() == CKirby::STATE_IDLE || m_pFSM->Get_State() == CKirby::STATE_RUN ||
			m_pFSM->Get_State() == CKirby::STATE_RUNSTART || m_pFSM->Get_State() == CKirby::SWORDSTATE_RUN ||
			m_pFSM->Get_State() == CKirby::SWORDSTATE_WAIT || m_pFSM->Get_State() == CKirby::CARSTATE_IDLING ||
			m_pFSM->Get_State() == CKirby::HAMMERSTATE_IDLE || m_pFSM->Get_State() == CKirby::HAMMERSTATE_RUN ||
			m_pFSM->Get_State() == CKirby::BULBSTATE_WAIT || m_pFSM->Get_State() == CKirby::BULBSTATE_MOVE) == false)
		INFO(m_bDumpAbilityPress) = false;


	if (INFO(m_bDumpAbilityPress) == false)
	{
		if (INFO(m_fDumpAbilityTime) > 0.f)
			INFO(m_fDumpAbilityTime) -= fTimeDelta * 2.f;

		if (INFO(m_fDumpAbilityTime) < 0.f)
			INFO(m_fDumpAbilityTime) = 0.f;
	}

	if (INFO(m_iCrashTimeSlow) == 1)
	{
		m_fCrashRestoreTime += fTimeDelta;


		if (m_fCrashRestoreTime > 4.f)
		{
			INFO(m_iCrashTimeSlow) = 0;
			m_pGameInstance->Restore_FirstTimer();
			m_pGameInstance->Restore_SecondTimer();
			m_fCrashRestoreTime = 0.f;
		}
	}
	else if (INFO(m_iCrashTimeSlow) == 2)
	{
		m_fCrashRestoreTime += fTimeDelta;

		if (m_fCrashRestoreTime > 7.f)
		{
			INFO(m_iCrashTimeSlow) = 0;
			m_pGameInstance->Restore_FirstTimer();
			m_pGameInstance->Restore_SecondTimer();
			m_fCrashRestoreTime = 0.f;
		}
	}


	if (INFO(m_bFinalBossDead) == true)
	{
		if (m_bFinalCutTrigger == true)
		{
			m_pControllerCom->Set_Position(m_pTransformCom, _float4(0.f, 0.f, 0.f, 1.f));
			m_bFinalCutTrigger = false;
			Change_State(FINALCUTSTATE_CUT1, 60.f, false, false, BODY_FINALCUT, OFFSET_FINALCUT);
		}
	}

	// 빛 컨트롤
	AssistLight_Control();
}

HRESULT CKirby::Kirby_SystemInitialize()
{
	// 타겟 카메라를 만들어준다.
	if (FAILED(Make_TargetToCams()))
		return E_FAIL;

	// 완전히 기본상태로 먼저 세팅한다.
	INFO(m_eBodyState) = BODY_DEFAULT;
	INFO(m_eMouthState) = MOUTH_IDLE;
	INFO(m_eEyeState) = EYE_IDLE;

	// 커비가 레벨별로 시작할 때, 바라보는 방향을 정해준다.
	Kirby_LookInitialize();

	m_fMaxHp = 100.f;

	// 임시로 능력 디폴트 화
	if (*m_pCurrentLevelID == LEVEL_INTRO)
	{
		m_fHp = 100.f; // 기존 사용하던 HP입니다.
		m_eAbilityType = ABILITY_DEFAULT;
	}
	else
	{
		CLevelChanger::LEVEL_DATA tLevelData = CLevelChanger::Get_Instance()->Load();
		m_fHp	  = tLevelData.fKirbyHP;
		m_uCoin	  = static_cast<_uint>(tLevelData.fKirbyCoin);
		m_fAttack = 5.f; // 고정

		//m_eAbilityType = static_cast<ABILITYTYPE>(tLevelData.iKirbyState);
		LEVEL eLEVEL = static_cast<LEVEL>(tLevelData.iLatestLevel);

		if (*m_pCurrentLevelID == LEVEL_RACING)
		{
			m_eAbilityType = ABILITY_DEFAULT;
			m_pCamera->Set_Target(m_pTransformCom, CCamera::TARGET_FIRST, CCamera::FOCUS_FIRST, _float3{0.f, 0.f, 1.f}, 5.f);
		}
		else if (LEVEL_TOWN == eLEVEL && LEVEL_TOWN == *m_pCurrentLevelID)
		{
			_float3 vNewPos = tLevelData.vLastPos;
			//m_pControllerCom->Set_Position(m_pTransformCom, _float4{ vNewPos.x, vNewPos.y, vNewPos.z, 1.f });
		}
		else
		{
			//m_fHp = 100.f; // 기존 사용하던 HP입니다.
			//m_fMaxHp = 100.f;
			m_eAbilityType = ABILITY_DEFAULT;
		}
	}

	// 폭탄 궤적을 만들어 놓는다.
	Ready_BombOrbit();
	// 애니메이션 이벤트를 삽입한다.
	Add_AnimEvent();
	// 혹여나, 버그가 발생할까봐 확실하게 블러 true화
	m_bMotionBlur = true;

	m_pBulbFlare = static_cast<CBulbFlare*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BulbFlare")));


	return S_OK;
}

void CKirby::Kirby_LookInitialize()
{
	_uint uLevel = *m_pCurrentLevelID;
	_float4 fCameraLook = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_LOOK);
	_float4 fCameraRight = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_RIGHT);

	fCameraLook.y = 0.f;
	fCameraLook = XMVector4Normalize(fCameraLook);
	fCameraRight = XMVector4Normalize(fCameraRight);

	// 카메라 기준 바라보는 방향을 설정한다.
	if (uLevel == LEVEL_RACING)
	{
		// 오른쪽을 보고 시작함.
		INFO(m_vTargetDir) = INFO(m_vMoveDir) = fCameraRight;
		INFO(m_eBodyState) = BODY_CARDEFAULT;
	}
	else
	{
		// 카메라를 정면으로 바라봄
		INFO(m_vTargetDir) = INFO(m_vMoveDir) = -1.f * fCameraLook;
	}
}

void CKirby::Kirby_StateInitialize()
{
	// 차 폼일 경우
	if (INFO(m_eBodyState) == BODY_CARDEFAULT)
	{
		Change_State(CARSTATE_IDLING, 60.f, true, true, BODY_CARDEFAULT, OFFSET_CAR);
		m_pModelCom[BODY_CARDEFAULT]->Set_Animation(14, 60.f, true, true);
	}
	else if (m_eAbilityType == ABILITY_SWORD)
	{
		Change_State(SWORDSTATE_WAIT, 60.f, true, true, BODY_SWORDDEFAULT, OFFSET_SWORD);
		m_pModelCom[BODY_SWORDDEFAULT]->Set_Animation(29, 60.f, true, true);
	}
	else if (m_eAbilityType == ABILITY_BOMB)
	{
		Change_State(STATE_IDLE, 60.f, true, true, BODY_DEFAULT);
		m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, true);
	}
	else if (m_eAbilityType == ABILITY_HAMMER)
	{
		Change_State(HAMMERSTATE_IDLE, 60.f, true, true, BODY_HAMMER, OFFSET_HAMMER);
		m_pModelCom[BODY_HAMMER]->Set_Animation(42, 60.f, true, true);
	}
	else if (m_eAbilityType == ABILITY_DEFAULT)
	{
		Change_State(STATE_IDLE, 60.f, true, true, BODY_DEFAULT);
		m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, true);
	}

}

CGameObject* CKirby::FindToppleableBridge(PxRigidActor* pActor)
{
	auto mapIter = m_mapToppleableBridges.find(pActor);
	if (mapIter != m_mapToppleableBridges.end())
		return mapIter->second;

	return nullptr;
}

CGameObject* CKirby::FindStarBox(PxRigidActor* pActor)
{
	auto mapIter = m_mapStarBoxes.find(pActor);
	if (mapIter != m_mapStarBoxes.end())
		return mapIter->second;

	return nullptr;
}

CGameObject* CKirby::FindBox(PxRigidActor* pActor)
{
	auto mapIter = m_mapBoxes.find(pActor);
	if (mapIter != m_mapBoxes.end())
		return mapIter->second;

	return nullptr;
}

CGameObject* CKirby::FindDynamicField(PxRigidActor* pActor)
{
	auto mapIter = m_mapDynamicFields.find(pActor);
	if (mapIter != m_mapDynamicFields.end())
		return mapIter->second;

	return nullptr;
}

void CKirby::ReleaseAndClearMap(unordered_map<PxRigidActor*, CGameObject*> _map)
{
	for (auto& pair : _map)
		Safe_Release(pair.second);
	_map.clear();
}

void CKirby::AssistLight_Control()
{
	if (INFO(m_pKirbyAssistLight1) == nullptr && INFO(m_eBodyState) == BODY_CARDEFAULT)
	{
		_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		_float4 vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float4 vUp = m_pTransformCom->Get_State(CTransform::STATE_UP);

		LIGHT_DESC			LightDesc{};
		LightDesc.eType = LIGHT_DESC::TYPE_HORONG;
		LightDesc.vPosition = vPos + (vUp * 0.2f) - (vLook * 3.5f) + (vRight * 0.7f);
		LightDesc.fRange = 0.1f;
		LightDesc.vDiffuse = _float4(1.f, 0.6f, 0.2f, 1.f);
		LightDesc.vAmbient = _float4(.5f, .5f, .5f, 1.f);
		LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
		if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
			return;
		INFO(m_pKirbyAssistLight1) = CGameInstance::Get_Instance()->Get_LightLastAddress();
		Safe_AddRef(INFO(m_pKirbyAssistLight1));

		LightDesc.vPosition = vPos + (vUp * 0.2f) - (vLook * 3.5f) - (vRight * 0.7f);
		if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
			return;
		INFO(m_pKirbyAssistLight2) = CGameInstance::Get_Instance()->Get_LightLastAddress();
		Safe_AddRef(INFO(m_pKirbyAssistLight2));

	}


	if (INFO(m_pKirbyAssistLight1) != nullptr)
	{
		if (INFO(m_eBodyState) == BODY_CARDEFAULT)
		{
			if (INFO(m_bBooster) == true)
			{
				INFO(m_pKirbyAssistLight1)->Interpolate_Light(_float4(1.f, 0.6f, 0.2f, 1.f), 10.f, 0.3f);
				INFO(m_pKirbyAssistLight2)->Interpolate_Light(_float4(1.f, 0.6f, 0.2f, 1.f), 10.f, 0.3f);
			}
			else
			{
				INFO(m_pKirbyAssistLight1)->Interpolate_Light(_float4(1.f, 0.6f, 0.1f, 1.f), 0.1f, 0.f);
				INFO(m_pKirbyAssistLight2)->Interpolate_Light(_float4(1.f, 0.6f, 0.1f, 1.f), 0.1f, 0.f);
			}

			_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
			_float4 vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);
			_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vUp = m_pTransformCom->Get_State(CTransform::STATE_UP);

			INFO(m_pKirbyAssistLight1)->Update_LightPos(vPos + (vUp * 0.2f) - (vLook * 3.5f) + (vRight * 0.7f));
			INFO(m_pKirbyAssistLight2)->Update_LightPos(vPos + (vUp * 0.2f) - (vLook * 3.5f) - (vRight * 0.7f));
		}



	}
}

CKirby* CKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKirby* pInstance = new CKirby(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CKirby"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKirby::Clone(void* pArg)
{
	CKirby* pInstance = new CKirby(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CKirby"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKirby::Free()
{
#pragma region 레벨 전환용 데이터 파싱
	if (m_bCloned == true)
	{
		CLevelChanger::LEVEL_DATA tLevelData = {};
		tLevelData.fKirbyCoin = (_float)m_uCoin;
		tLevelData.fKirbyHP = m_fHp;

		// 커비 능력
		tLevelData.iKirbyState  = m_eAbilityType; // QZR
		tLevelData.iLatestLevel = *CGameInstance::Get_Instance()->Get_CurrentLevelID();
		_float4 vPos = GET_POS;
		tLevelData.vLastPos = _float3{ vPos.x, vPos.y, vPos.z };
		CLevelChanger::Get_Instance()->Save(tLevelData);
	}
#pragma endregion

	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();

	ReleaseAndClearMap(m_mapToppleableBridges);
	ReleaseAndClearMap(m_mapStarBoxes);
	ReleaseAndClearMap(m_mapBoxes);
	ReleaseAndClearMap(m_mapDynamicFields);

	for (auto& pModelCom : m_pModelCom)
		Safe_Release(pModelCom);
	for (auto& pEyeTexture : m_pEyeTexture)
		Safe_Release(pEyeTexture);
	for (auto& pMouthTexture : m_pMouthTexture)
		Safe_Release(pMouthTexture);
	Safe_Release(m_pCamera);

	Safe_Release(m_pWeapons);
	Safe_Release(m_pArmours);

	if (INFO(m_pObject) != nullptr)
		Safe_Release(INFO(m_pObject));

	// Bomb
	Safe_Release(m_pOrbit);

	for (auto& Glow : m_OrbitGlows)
		Safe_Release(Glow);
	m_OrbitGlows.clear();

	if (INFO(m_pLight) != nullptr)
		Safe_Release(INFO(m_pLight));

	if (INFO(m_pKirbyAssistLight1) != nullptr)
		Safe_Release(INFO(m_pKirbyAssistLight1));
	if (INFO(m_pKirbyAssistLight2) != nullptr)
		Safe_Release(INFO(m_pKirbyAssistLight2));


	Safe_Release(m_pBulbFlare);
}

