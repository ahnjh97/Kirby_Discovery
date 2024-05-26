#include "stdafx.h"
#include "FSM.h"
#include "Kirby.h"
#include "Camera_Free.h"

#include "KirbyDefault_State.h"

CKirby::CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CKirby::CKirby(const CKirby& rhs)
	: CGameObject{ rhs }
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

	INFO(m_eBodyState) = BODY_DEFAULT;
	INFO(m_eMouthState) = MONTH_IDLE;
	INFO(m_eEyeState) = EYE_IDLE;
	m_eJumpState = STATE_JUMPL;

	// 카메라 기준으로 움직이기에 미리 받아둔다.
	if (m_pCamera == nullptr)
	{
		m_pCamera = static_cast<CCamera_Free*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Camera"), 0));
		Safe_AddRef(m_pCamera);
	}

	m_pCamera->Set_Target(m_pTransformCom);


	_float4 m_pCameraLook = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_LOOK);
	m_pCameraLook.y = 0.f;
	m_pCameraLook = XMVector4Normalize(m_pCameraLook);
	INFO(m_vMoveDir) = -1.f * m_pCameraLook;
	INFO(m_vTargetDir) = INFO(m_vMoveDir);

	//m_pTransformCom->Look_At_ForLandObject(vPos + INFO(m_vMoveDir));

	m_pModelCom[INFO(m_eBodyState)]->Set_Animation(STATE_IDLE, 60.f, true, true);

	m_eCollisionGroup = PLAYER;
	
	return S_OK;
}

_int CKirby::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	// 커비의 기본적인 축 보정, 밸런스 보정을 담당한다.
	Setting_KirbyBalance();
	// 지면충돌과 경사 보정
	SetOn_Slope(fTimeDelta);
	// 키 입력에 대한 상태처리
	Key_Input(fTimeDelta);

	// 점프
	Kirby_Jump(fTimeDelta);

	// 유틸업데이트가 들어가있다.
	//****** FSM Update, Shadow ChaseUpdate, IdleResetUpdate ******//
	Kirby_SystemTick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CKirby::Late_Tick(_float fTimeDelta)
{
	m_pModelCom[INFO(m_eBodyState)]->Play_Animation(fTimeDelta);

	_vector vPos = XMVectorSetW(CUtils::To_Vector(m_pControllerCom->Compute_TerrainPosition()), 1.f);
	if (true == m_pControllerCom->Is_Terrain())
	{
		int a = 10;
	}

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
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

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(1)))
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

	ImGui::Text("RePress : %d", m_bRePressBlock);
	ImGui::Text("Land : %d", INFO(m_isLanding));

	ImGui::Text("JUMP : %d", INFO(m_isJump));
	ImGui::Text("Velocity : %.2f", INFO(m_fJumpVelocity));
	ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

//	ImGui::Text("MoveDir X : %.2f \tMoveDir Y : %.2f \tMoveDir Z : %.2f ", INFO(m_vMoveDir).x, INFO(m_vMoveDir).y, INFO(m_vMoveDir).z); ImGui::NewLine();
//	ImGui::Text("TargetDir X : %.2f \tTargetDir Y : %.2f \tTargetDir Z : %.2f ", INFO(m_vTargetDir).x, INFO(m_vTargetDir).y, INFO(m_vTargetDir).z);
	__super::Render_IMGUI();
}

_uint CKirby::Get_State()
{
	return m_pFSM->Get_State();
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

	// 카메라 기준 실시간 방향 제어
	CTransform* pCameraTransform = m_pCamera->Get_TransformCom();
	_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));
	_float fCX = vCamLook.x;
	_float fCZ = vCamLook.z;
	_float fKX = INFO(m_vMoveDir).x;
	_float fKZ = INFO(m_vMoveDir).z;
	_float fAngle = (atan2f(fCX, fCZ) * 180.0f / XM_PI) - (atan2f(fKX, fKZ) * 180.0f / XM_PI);
	if (fAngle < 0.f) fAngle += 360.0f;
	if (fAngle >= 337.5f || fAngle < 22.5f) m_eKirbyDir = DIR_FRONT;
	else if (fAngle >= 22.5f && fAngle < 67.5f) m_eKirbyDir = DIR_LF;
	else if (fAngle >= 67.5f && fAngle < 112.5f) m_eKirbyDir = DIR_LEFT;
	else if (fAngle >= 112.5f && fAngle < 157.5f) m_eKirbyDir = DIR_LB;
	else if (fAngle >= 157.5f && fAngle < 202.5f) m_eKirbyDir = DIR_BACK;
	else if (fAngle >= 202.5f && fAngle < 247.5f) m_eKirbyDir = DIR_RB;
	else if (fAngle >= 247.5f && fAngle < 292.5f) m_eKirbyDir = DIR_RIGHT;
	else if (fAngle >= 292.5 && fAngle < 337.5f) m_eKirbyDir = DIR_RF;
}

void CKirby::Key_Input(_float fTimeDelta)
{

	JoyStick_Input(fTimeDelta);
	ZXCV_Input(fTimeDelta);


#pragma region 커비 연구소 (애니메이션 제어)
	//Test
	if (m_pGameInstance->Get_DIKeyState(DIK_P, KEY_DOWN))
	{
		m_iTestAnim++;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[INFO(m_eBodyState)]->Set_TickPerSecond(60.f);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_O, KEY_DOWN))
	{
		m_iTestAnim--;
		if (m_iTestAnim < 0)
			m_iTestAnim = 0;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[INFO(m_eBodyState)]->Set_TickPerSecond(60.f);

	}

	if (m_pGameInstance->Get_DIKeyState(DIK_0, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_DEFAULT;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[INFO(m_eBodyState)]->Set_TickPerSecond(60.f);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_9, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_BALLOON;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[INFO(m_eBodyState)]->Set_TickPerSecond(60.f);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_8, KEY_DOWN))
	{
		INFO(m_eBodyState) = BODY_VACUUM;
		m_pModelCom[INFO(m_eBodyState)]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[INFO(m_eBodyState)]->Set_TickPerSecond(60.f);
	}
#pragma endregion

}

void CKirby::JoyStick_Input(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_PRESS))
	{
		INFO(m_isController) = true;

		if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			INFO(m_vTargetDir) = Make_TargetDir(DIR_LF);
		else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			INFO(m_vTargetDir) = Make_TargetDir(DIR_RF);
		else
			INFO(m_vTargetDir) = Make_TargetDir(DIR_FRONT);

		if (Can_JoyStickUsing())
			m_pFSM->ChangeState(STATE_RUN, 100.f, true, true);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_DOWN, KEY_PRESS))
	{
		INFO(m_isController) = true;

		if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			INFO(m_vTargetDir) = Make_TargetDir(DIR_LB);
		else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			INFO(m_vTargetDir) = Make_TargetDir(DIR_RB);
		else
			INFO(m_vTargetDir) = Make_TargetDir(DIR_BACK);

		if (Can_JoyStickUsing())
			m_pFSM->ChangeState(STATE_RUN, 100.f, true, true);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		INFO(m_isController) = true;

		INFO(m_vTargetDir) = Make_TargetDir(DIR_LEFT);
		if (Can_JoyStickUsing())
			m_pFSM->ChangeState(STATE_RUN, 100.f, true, true);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		INFO(m_isController) = true;

		INFO(m_vTargetDir) = Make_TargetDir(DIR_RIGHT);
		if (Can_JoyStickUsing())
			m_pFSM->ChangeState(STATE_RUN, 100.f, true, true);

	}
	else
	{
		INFO(m_isController) = false;

		if (Can_JoyStickUsing())
		{
			// Idle상태에서 대기하는 귀여운 상황들을 연출한다.
			Idle_Animation(fTimeDelta);
		}

	}
}

void CKirby::ZXCV_Input(_float fTimeDelta)
{

	// 점프 중이 아닐 때
	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) && INFO(m_isJump) != true)
	{
		// 점프의 랜덤 애니메이션 재생
		m_eJumpState == STATE_JUMPL ? m_eJumpState = STATE_JUMPR : m_eJumpState = STATE_JUMPL;

		// 점프의 초기 파워
		INFO(m_fJumpVelocity) = 22.f;
		// 점프의 부울 값
		INFO(m_isJump) = true;
		// 최소 단위로 점프를 눌렀을 때, 공중으로 뜨는 최소 시간 0.15초
		m_fChangeVelocityZeroTime = 0.f;
		// 공중에서 체공하는 시간 0.15초
		m_fHoldAirTime = 0.f;
		// 점프키를 누르는 시간
		m_fJumpHoldTime = 0.f;

		// 초기화
		m_bRePressBlock = false;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_UP))
	{
		// 때는 순간 트루가 되고, 점프 가능 시점에 다시 누를 때 까지 C에대한 누적 등 반응하지 않는다.
		m_bRePressBlock = true;
	}

	if (m_bRePressBlock == false && m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS) && m_fJumpHoldTime < 0.3f)
	{
		m_fJumpHoldTime += fTimeDelta;
	}
	else
	{
		// 최소 점프 제한 키 유지 시간
		_float fJumpHoldTime = 0.1f;
		// 공중 체공 시간
		_float fHoldAirTime = 0.22f;
		// 최소단위 공중 올라가는 시간
		_float fChangeVelocityZeroTime = 0.09f;

		// 0.1초 이하로 눌렀을 때
		if (m_fJumpHoldTime < fJumpHoldTime)
		{
			// 최소 공중 체공시간 0.15초
			m_fChangeVelocityZeroTime += fTimeDelta;

			// 공중 체공시간으로 하자. 최소 시간이 지났고, 공중 체공 사이 시간동안!
			if (m_fChangeVelocityZeroTime > fChangeVelocityZeroTime)
			{
				m_fHoldAirTime += fTimeDelta;
				if (m_fHoldAirTime < fHoldAirTime)
				{
					_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
					INFO(m_fJumpVelocity) = fStopVelocityPower + (fabs(m_fHoldAirTime - fHoldAirTime * 0.5f) * (-1.f / (fHoldAirTime * 0.5f)));
				}
			}
		}
		// 0.1초 이상 눌렀을 때.
		else
		{
			// 공중 체공 시간으로 하자. 공중 체공 사이 시간동안!
			m_fHoldAirTime += fTimeDelta;
			if (m_fHoldAirTime < fHoldAirTime)
			{
				_float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
				INFO(m_fJumpVelocity) = fStopVelocityPower + (fabs(m_fHoldAirTime - fHoldAirTime * 0.5f) * (-1.f / (fHoldAirTime * 0.5f)));
			}
		}
	}
}

void CKirby::SetOn_Slope(_float fTimeDelta)
{
	// 지면의 up벡터
	PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
	_vector vTerrainNormal = CUtils::To_Vector(slope);
	Lerp_UpVector(vTerrainNormal, 10.f, fTimeDelta);
}

void CKirby::Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta)
{
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(_vTargetUp, m_vOriginUp));

	// 구면 선형 보간 : m_vOriginUp을 vTargetUp 방향으로 보간
	_float fInterpolateAngle = fTimeDelta * XMConvertToRadians(_maxAngle) * m_fOffsetTurn;

	if (fAngle > fInterpolateAngle) { fAngle = fInterpolateAngle / fAngle; }
	else fAngle = 1.0f;

	m_vOriginUp = XMQuaternionSlerp(m_vOriginUp, _vTargetUp, fAngle);
	m_vOriginUp = XMVector3Normalize(m_vOriginUp);

	_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vNewRight = XMVector3Cross(m_vOriginUp, vLook);
	vNewRight = XMVector3Normalize(vNewRight);
	_vector vNewLook = XMVector3Cross(vNewRight, m_vOriginUp);
	vNewLook = XMVector3Normalize(vNewLook);

	// OriginUp을 기준으로 다시 재 설정
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, vNewRight);
	m_pTransformCom->Set_State(CTransform::STATE_UP, m_vOriginUp);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, vNewLook);
}

void CKirby::Kirby_Jump(_float fTimeDelta)
{
	// 점프 중일 때
	if (true == INFO(m_isJump))
	{
		if (m_fJumpHoldTime > 0.3f)
		{
			m_pFSM->ChangeState(STATE_JUMPEND, 60.f, false, true);
		}
		else
			m_pFSM->ChangeState(m_eJumpState, 50.f, false, true);
	}
	// 착지했다면,
	else if (true == INFO(m_isLanding))
	{
		// 최소 애니메이션이 재생되는 시간이다. ( 방향키를 누르면 0.2초 후 바로 Run 상태가 됨 )
		_float fChangeRunTime = 0.08f;
		m_fChangeRunTime += fTimeDelta;

		if (m_fJumpHoldTime > 0.2f)
			m_pFSM->ChangeState(STATE_LANDINGEND, 30.f, false, false);
		else
			m_pFSM->ChangeState(STATE_LANDINGSMALL, 50.f, false, false);

		// 바로 방향키를 갈겼다면
		if (m_fChangeRunTime > fChangeRunTime && INFO(m_isController))
		{
			INFO(m_isLanding) = false;
			INFO(m_eEyeState) = CKirby::EYE_IDLE;
			m_fChangeRunTime = 0.f;
		}
		// 랜딩 애니메이션이 끝났다면,
		else if (m_pModelCom[INFO(m_eBodyState)]->IsFinished())
		{
			INFO(m_isLanding) = false;
			INFO(m_eEyeState) = CKirby::EYE_IDLE;
			m_fChangeRunTime = 0.f;
		}
	}
	// 자유 낙하 ( 점프 중도 아니고, 착지 중도 아니다. ) 
	// 이럴때 떨어지는 애니메이션이 재생되어야 할 것이다.
	else
	{

		m_pControllerCom->FreeFall(m_pTransformCom, fTimeDelta, INFO(m_fGravityOffset));
	}

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
			TEXT("Com_Texture_Mouth_Idle"), (CComponent**)&m_pMouthTexture[MONTH_IDLE]);
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
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	m_pControllerCom->Set_PhysXObject(this);

	/* FSM */
	SetUp_FSM();

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
		(_eBodyState == BODY_BALLOON && _iMeshIndex == 4))
	{
		m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
		m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
		m_pMouthTexture[INFO(m_eMouthState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyMouthTexture", 0);
		m_pShaderCom->Begin(3);
		m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
		return true;
	}
	// Default 상태의 눈 부위 // Vacuum 상태의 눈 부위 // Balloon 상태의 눈 부위
	else if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 3) ||
		(_eBodyState == BODY_VACUUM && _iMeshIndex == 0) ||
		(_eBodyState == BODY_BALLOON && _iMeshIndex == 3))
	{
		m_pModelCom[INFO(m_eBodyState)]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
		m_pModelCom[INFO(m_eBodyState)]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
		m_pEyeTexture[INFO(m_eEyeState)]->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", 0);
		m_pShaderCom->Begin(4);
		m_pModelCom[INFO(m_eBodyState)]->Render(_iMeshIndex);
		return true;
	}

	return false;
}

void CKirby::Idle_Animation(_float fTimeDelta)
{
	m_fIdleStreachTime += fTimeDelta;
	m_bKirbyIdleChangeTrigger = true;

	// 5초 마다, 아이들 상태에서 어떤 행동을 한다.
	if (m_fIdleStreachTime > 5.f)
	{
		m_iIdleChoose == 0 ?
			m_pFSM->ChangeState(STATE_IDLESTREACH, 60.f, false, true) :
			m_pFSM->ChangeState(STATE_IDLELOOKAROUND, 60.f, false, true);

		if (m_pModelCom[INFO(m_eBodyState)]->IsFinished())
		{
			m_iIdleChoose == 0 ? m_iIdleChoose = 1 : m_iIdleChoose = 0;
			m_fIdleStreachTime = 0.f;
		}
	}
	else
	{
		m_pFSM->ChangeState(STATE_IDLE, 60.f, true, true);
		INFO(m_eEyeState) = EYE_IDLE;
	}

}

_float4 CKirby::Make_TargetDir(DIR _eDir)
{
	CTransform* pCameraTransform = m_pCamera->Get_TransformCom();
	_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));

	if (_eDir == DIR_RIGHT)
		return vCamRight;
	else if (_eDir == DIR_LEFT)
		return -1.f * vCamRight;
	else if (_eDir == DIR_FRONT)
		return vCamLook;
	else if (_eDir == DIR_BACK)
		return -1.f * vCamLook;

	else if (_eDir == DIR_LB)
		return XMVector4Normalize((-1.f * vCamRight) + (-1.f * vCamLook));
	else if (_eDir == DIR_RB)
		return XMVector4Normalize(vCamRight + (-1.f * vCamLook));
	else if (_eDir == DIR_LF)
		return XMVector4Normalize((-1.f * vCamRight) + (vCamLook));
	else if (_eDir == DIR_RF)
		return XMVector4Normalize(vCamRight + vCamLook);

	return _float4(0.f, 0.f, 0.f, 0.f);
}

void CKirby::SetUp_FSM()
{

	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(STATE_IDLE, CKirbyDefault_Idle_State::Create());
	m_pFSM->Add_State(STATE_IDLESTREACH, CKirbyDefault_Idle_State::Create());
	m_pFSM->Add_State(STATE_IDLELOOKAROUND, CKirbyDefault_Idle_State::Create());

	m_pFSM->Add_State(STATE_RUN, CKirbyDefault_Run_State::Create());

	m_pFSM->Add_State(STATE_JUMPL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPR, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPEND, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPFALL, CKirbyDefault_Jump_State::Create());//
	m_pFSM->Add_State(STATE_LANDINGEND, CKirbyDefault_Jump_State::Create());//
	m_pFSM->Add_State(STATE_LANDINGSMALL, CKirbyDefault_Jump_State::Create());//

	m_pFSM->Add_State(STATE_FALL, CKirbyDefault_Jump_State::Create());//

	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = STATE_IDLE;
	FSM_Desc.pModel = m_pModelCom[BODY_DEFAULT];
	m_pFSM->Initialize(&FSM_Desc);

	// 셋업

}

void CKirby::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirby::Kirby_SystemTick(_float fTimeDelta)
{
	// Idle상태 제어
	// Idle류 상태가 아닐경우 딴짓하는 Time이 리셋된다.
	if (false == (m_pFSM->Get_State() == STATE_IDLE ||
		m_pFSM->Get_State() == STATE_IDLESTREACH ||
		m_pFSM->Get_State() == STATE_IDLELOOKAROUND))
	{
		if (m_bKirbyIdleChangeTrigger)
		{
			m_iIdleChoose == 0 ? m_iIdleChoose = 1 : m_iIdleChoose = 0;
			m_bKirbyIdleChangeTrigger = false;
		}
		m_fIdleStreachTime = 0.f;
	}


	// 그림자는 무조건 커비를 따라간다.
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vLightPos = vPos;
	vLightPos.m128_f32[1] += 30.f;
	vLightPos.m128_f32[2] -= 1.f;
	m_pGameInstance->Update_LightShadow(vLightPos, vPos);


	// FSM 제어
	if (m_pFSM != nullptr)
		m_pFSM->Update(this, fTimeDelta);
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

	Safe_Release(m_pShaderCom);
	for (auto& pModelCom : m_pModelCom)
		Safe_Release(pModelCom);
	for (auto& pEyeTexture : m_pEyeTexture)
		Safe_Release(pEyeTexture);
	for (auto& pMouthTexture : m_pMouthTexture)
		Safe_Release(pMouthTexture);

	Safe_Release(m_pControllerCom);
	Safe_Release(m_pCamera);
	Safe_Release(m_pFSM);

}
