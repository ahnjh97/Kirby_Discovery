#include "stdafx.h"
#include "FSM.h"
#include "Kirby.h"
#include "Camera_Free.h"

#include "KirbyDefault_State.h"
#include "KirbyBalloon_State.h"
#include "KirbyVacuum_State.h"
#include "KirbyDamage_State.h"
#include "KirbyContents_State.h"
#include "KirbySword_State.h"
#include "KirbyBoom_State.h"

#include "KirbyWeapons.h"
#include "KirbyArmours.h"
#include "Trigger.h"

#include "Utils.h"
#include "Bone.h"


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
	m_eCollisionGroup = PLAYER;

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

	INFO(m_eBodyState) = BODY_DEFAULT;
	INFO(m_eMouthState) = MOUTH_IDLE;
	INFO(m_eEyeState) = EYE_IDLE;


	// 첫 카메라 기준으로 움직이기에 미리 받아둔다.
	if (m_pCamera == nullptr)
	{
		//인트로, 게임플레이 스테이지라면 카메라로 main camera를 저장한다.
		(*m_pCurrentLevelID == LEVEL_INTRO || *m_pCurrentLevelID == LEVEL_GAMEPLAY) ?
			m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main"))) :

		//나머지 레벨이라면 다른 카메라를 저장한다.
		m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));

		if (m_pCamera == nullptr)
		{
			ALARM_FAIL(TEXT("망했어 카메라 없다"));
			return E_FAIL;
		}
		Safe_AddRef(m_pCamera);
	}

	m_pCamera->Set_Target(m_pTransformCom);

	//게임 레벨에 free camera 있다면 그놈에게도 타겟 등록해 준다.
	if ((*m_pCurrentLevelID == LEVEL_INTRO || *m_pCurrentLevelID == LEVEL_GAMEPLAY))
	{
		CCamera* pCameraFree = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));
		if (pCameraFree != nullptr)
			pCameraFree->Set_Target(m_pTransformCom);
	}


	_float4 m_pCameraLook = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_LOOK);
	m_pCameraLook.y = 0.f;
	m_pCameraLook = XMVector4Normalize(m_pCameraLook);
	INFO(m_vMoveDir) = -1.f * m_pCameraLook;
	INFO(m_vTargetDir) = INFO(m_vMoveDir);

	m_pModelCom[INFO(m_eBodyState)]->Set_Animation(STATE_IDLE, 60.f, true, true);

	m_fMaxHp = 100.f;
	m_fHp = 100.f;
	m_fAttack = 5.f;
	m_eAbilityType = ABILITY_DEFAULT;
	//m_eAbilityType = ABILITY_SWORD;

	m_pControllerCom->RegisterAsPlayer();
	m_pControllerCom->Register_Controller();

	// 폭탄 궤적을 만들어 놓는다.
	Ready_BombOrbit();


	
	Add_AnimEvent();

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

	if(m_pHitBoxTrigger->Is_Alive())
		m_pHitBoxTrigger->Tick(m_fTimeDelta);

	//if (m_pGameInstance->Get_DIKeyState(DIK_5, KEY_DOWN))
	//	m_pHitBoxTrigger->Check_Collision();

	return OBJ_NOEVENT;
}

void CKirby::Late_Tick(_float fTimeDelta)
{
	m_pModelCom[INFO(m_eBodyState)]->Play_Animation(m_fTimeDelta);

	if (INFO(m_eBodyState) != BODY_DEFAULT)
		m_pModelCom[BODY_DEFAULT]->Play_Animation(m_fTimeDelta);

	m_pWeapons->Late_Tick(m_fTimeDelta);
	m_pArmours->Late_Tick(m_fTimeDelta);


	if (m_fOrbitRenderDelay > 0.5f)
	{
		// 조준했당께요!
		INFO(m_bBombAimming) = true;

		for (auto& Glow : m_OrbitGlows)
			Glow->Late_Tick(fTimeDelta);
	}

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND,	 this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW,		 this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);
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


		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_KIRBY)))
			return E_FAIL;

		m_pModelCom[INFO(m_eBodyState)]->Render(i);
	}

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
	ImGui::Text("HP : %d", (_int)m_fHp);
	ImGui::Text("m_iTestAnim : %d", m_iTestAnim);
	ImGui::Text("m_bInitializeTargetPos : %d", m_bInitializeTargetPos);
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
		m_pHitBoxTrigger->Check_Collision();

		});
	m_pModelCom[BODY_SWORDDEFAULT]->Add_Event("StopDamage", [this]() {
		//m_pHitBoxTrigger->Close_Collision();

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
		// 흡수중인 몬스터
		if (pObject->Get_PhyXState() == PO_VACUUMING)
		{
			// 일단 EAT으로 넘기는건 같으나, EAT이 끝날 시점에 내가 삼켰던 것이 무엇이였는지 판단 후 애니메이션이 분기된다.
			INFO(m_isEat) = true;
			INFO(m_eEyeState) = EYE_IDLE;
			INFO(m_eMouthState) = MOUTH_ANGER;
			Change_State(STATE_EAT, 100.f, false, false, BODY_BALLOON);
			// 임시 보관소. 먹은게 끝났을 떄, 비로소 커비의 어빌리티 타입이 바뀐다.
			INFO(m_eTemporaryEatType) = pObject->Get_AbilityType();

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



}

_float3 CKirby::Make_RepulsiveDir(CPhysXObject* pObject)
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vObjectPos = pObject->Get_TransformCom()->Get_State_Vector(CTransform::STATE_POSITION);

	m_vDamegeDir = XMVector3Normalize(vPos - vObjectPos);

	return XMVector3Normalize(vObjectPos - vPos);
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

	//Test
	if (m_pGameInstance->Get_DIKeyState(DIK_P, KEY_DOWN))
	{
		m_iTestAnim++;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, 60.f, true, true);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_O, KEY_DOWN))
	{
		m_iTestAnim--;
		if (m_iTestAnim < 0)
			m_iTestAnim = 0;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, 60.f, true, true);

	}

	if (m_pGameInstance->Get_DIKeyState(DIK_0, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_DEFAULT;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, 60.f, true, true);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_9, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_BALLOON;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, 60.f, true, true);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_8, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_VACUUM;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, 60.f, true, true);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_7, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_BOOMDEFAULT;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, 60.f, true, true);
	}

#pragma endregion
}

HRESULT CKirby::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxAnimModel"),
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
	m_pControllerCom->Set_Object(this);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom[BODY_DEFAULT];
	m_uModelCnt = BODY_END;

	/* FSM */
	SetUp_FSM();

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

	/* 커비의 HITBOX */
	CTrigger::TRIGGER_DESC tTriggerDesc{};
	tTriggerDesc.iTriggerType = CTrigger::TRIGGER_HITBOX;
	tTriggerDesc.iTriggerIndex = 0;
	tTriggerDesc.eCollisionGroup = HITBOX_PLYAER;
	tTriggerDesc.vTriggerSize = _float3(2.5f, 1.f, 2.5f);
	m_pHitBoxTrigger = static_cast<CTrigger*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Trigger"), &tTriggerDesc));
	CHECK_NULLPTR(m_pHitBoxTrigger);
	m_pHitBoxTrigger->Set_Owner(this);

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
	// Default 상태의 입 부위 // Balloon 상태의 입 부위
	if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 0) ||
		(_eBodyState == BODY_BALLOON && _iMeshIndex == 4) ||
		(_eBodyState == BODY_SWORDDEFAULT && _iMeshIndex == 0) ||
		(_eBodyState == BODY_SWORDBALLOON && _iMeshIndex == 4) ||
		(_eBodyState == BODY_BOOMDEFAULT && _iMeshIndex == 0))
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
		(_eBodyState == BODY_BOOMDEFAULT && _iMeshIndex == 3))
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



	CFSM::FSM_INFO		FSM_Info_Desc = {};
	FSM_Info_Desc.iState = STATE_IDLE;
	FSM_Info_Desc.uNumModel = BODY_END;
	FSM_Info_Desc.pModel = &m_pModelCom[BODY_DEFAULT];
	m_pFSM->Initialize(&FSM_Info_Desc);

}

void CKirby::Update_PartObjectMatrix()
{
	m_ArmourMatrix = *(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("HatL")->Get_CombinedTransformationMatrix());
	m_WeaponMatrix = *(m_pModelCom[INFO(m_eBodyState)]->Get_BonePtr("RHaveL")->Get_CombinedTransformationMatrix());
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

		if (m_fHitStopTime > 0.12f)
		{
			m_fHitStopTime = 0.f;
			m_bHitStop = false;
		}
	}
}

void CKirby::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody, _uint iOffSet)
{
	INFO(m_eBodyState) = eBody;
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

void CKirby::DefaultIdle()
{
	if (m_pModelCom[BODY_DEFAULT] == nullptr)
		return;

	m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, false);
}

_float4 CKirby::Compute_TerrainPosition()
{
	if (m_pControllerCom == nullptr)
		return _float4();

	return m_pControllerCom->Compute_TerrainPosition_Vector();
}

void CKirby::Kirby_SystemTick(_float fTimeDelta)
{
	// 그림자는 무조건 커비를 따라간다.
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vLightPos = vPos;
	vLightPos.m128_f32[1] += 60.f;
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


	// 물고 있을 때, 물고있는 객체를 계속 나의 입에 위치시키는 로직이다.
	if (INFO(m_pObject) != nullptr)
	{
		// 커비 입 속에 있다면?
		if (INFO(m_pObject)->Get_PhyXState() == PO_KIRBYMOUTH)
		{
			CCharacterController* pObjectController = static_cast<CCharacterController*>(INFO(m_pObject)->Get_Component(TEXT("Com_Controller")));
			if (pObjectController == nullptr)
			{
				CTransform* pObjectTransform = INFO(m_pObject)->Get_TransformCom();
				_vector vMouthPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				vMouthPos.m128_f32[1] += 1.f;
				pObjectTransform->Set_State(CTransform::STATE_POSITION, vMouthPos);
			}
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

	// 사다리 상태 초기화
	INFO(m_bCanLadder) = false;
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
	__super::Free();

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

	Safe_Release(m_pHitBoxTrigger);

	// Bomb
	Safe_Release(m_pOrbit);
	for (auto& Glow : m_OrbitGlows)
		Safe_Release(Glow);
	m_OrbitGlows.clear();

}

