#include "stdafx.h"
#include "FSM.h"
#include "Kirby.h"
#include "Camera_Free.h"

#include "KirbyDefault_State.h"
#include "KirbyBalloon_State.h"
#include "KirbyVacuum_State.h"


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

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;


	_float4 vPos = XMVectorSet(0.f, 100.f, 0.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	

	INFO(m_eBodyState) = BODY_DEFAULT;
	INFO(m_eMouthState) = MOUTH_IDLE;
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

	m_pTransformCom->Look_At_ForLandObject(vPos + INFO(m_vMoveDir));

	m_pModelCom[INFO(m_eBodyState)]->Set_Animation(STATE_IDLE, 60.f, true, true);

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
	// 커비의 통통한 로직들
	Kirby_Eat(fTimeDelta);


	// 유틸업데이트가 들어가있다.
	//****** FSM Update, Shadow ChaseUpdate, IdleResetUpdate ******//
	Kirby_SystemTick(fTimeDelta);


	return OBJ_NOEVENT;
}

void CKirby::Late_Tick(_float fTimeDelta)
{
	//m_pModelCom[INFO(m_eBodyState)]->Play_Animation(fTimeDelta);
	m_pModelCom[BODY_DEFAULT]->Play_Animation(fTimeDelta);
	m_pModelCom[BODY_BALLOON]->Play_Animation(fTimeDelta);
	m_pModelCom[BODY_VACUUM]->Play_Animation(fTimeDelta);

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

	ImGui::Text("JumpSpiting : %d", m_bJumpSpiting);
	ImGui::Text("m_IsSpit : %d", m_IsSpit);
	ImGui::Text("m_IsEat : %d", m_IsEat);
	ImGui::Text("m_Vacuum : %d", INFO(m_isVacuum));
	ImGui::Text("FLY : %d", INFO(m_isFly));
	ImGui::Text("Land : %d", INFO(m_isLanding));
	ImGui::Text("JUMP : %d", INFO(m_isJump));
	ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

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
		{
			if (m_IsEat)
				Change_State(STATE_EATRUN, 100.f, true, true, BODY_BALLOON);
			else
				Change_State(STATE_RUN, 100.f, true, true, BODY_DEFAULT);
		}

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
		{
			if (m_IsEat)
				Change_State(STATE_EATRUN, 100.f, true, true, BODY_BALLOON);
			else
				Change_State(STATE_RUN, 100.f, true, true, BODY_DEFAULT);
		}
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		INFO(m_isController) = true;

		INFO(m_vTargetDir) = Make_TargetDir(DIR_LEFT);

		if (Can_JoyStickUsing())
		{
			if (m_IsEat)
				Change_State(STATE_EATRUN, 100.f, true, true, BODY_BALLOON);
			else
				Change_State(STATE_RUN, 100.f, true, true, BODY_DEFAULT);
		}

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		INFO(m_isController) = true;

		INFO(m_vTargetDir) = Make_TargetDir(DIR_RIGHT);

		if (Can_JoyStickUsing())
		{
			if (m_IsEat)
				Change_State(STATE_EATRUN, 100.f, true, true, BODY_BALLOON);
			else
				Change_State(STATE_RUN, 100.f, true, true, BODY_DEFAULT);
		}
	}
	else
	{
		INFO(m_isController) = false;

		// 평범한 상태라면
		if (Can_JoyStickUsing())
		{
			// 먹고 있는 상태의 아이들
			if (m_IsEat)
				Change_State(STATE_EATWAIT, 100.f, true, true, BODY_BALLOON);
			// Idle상태에서 대기하는 귀여운 상황들을 연출한다.
			else
				Idle_Animation(fTimeDelta);
		}
	}
}

void CKirby::ZXCV_Input(_float fTimeDelta)
{
	// ZXCV 를 누를 자격이 없다면 리턴한다.
	if (Can_ZXCVUsing() == false)
		return;

	Z_Input(fTimeDelta);
	X_Input(fTimeDelta);

	if (!INFO(m_isVacuum))
		C_Input(fTimeDelta);

	V_Input(fTimeDelta);

}

void CKirby::Z_Input(_float fTimeDelta)
{
}

void CKirby::X_Input(_float fTimeDelta)
{
	// 공격 이후에 바로 빨아들이면 안 되니까, 딜레이를 준다.
	m_fVacuumDelay += fTimeDelta;


	// 먹은 상태에서 X 키를 눌러 발사하는 로직이다. 발싸!!!
	if (m_IsEat == true && m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN))
	{
		Change_State(STATE_SPIT, 100.f, false, false, BODY_VACUUM);
		m_IsSpit = true;

		m_fVacuumDelay = 0.f;

		// 랜딩모션일 경우, 바로 캔슬되고 뱉는다.
		m_tKirbyInfo.m_isLanding = false;
		m_IsEat = false;

		// 만약, 점프 중에 발사를 했다면, 점프 애니메이션을 통제하여야 한다.
		if (m_tKirbyInfo.m_isJump == true)
			m_bJumpSpiting = true;
	}


	// 커비의 흡수 로직들
	Kirby_Vacuum(fTimeDelta);


}

void CKirby::C_Input(_float fTimeDelta)
{
	// 점프 중일 때, C를 눌렀을 경우 나는 상태가 되어버린다.
	// Fly 상태는 Jump 중이면서 Fly 상태일 것이다.
	if (INFO(m_isJump) == true && INFO(m_isFly) == false)
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
		{
			// 날기의 초기 파워
			INFO(m_fJumpVelocity) = 4.f;
			INFO(m_isFly) = true;
			INFO(m_isJump) = false;
			Change_State(STATE_FLIGHTSTART, 60.f, false, false, BODY_BALLOON);

			// 날기 시작하고 다른건 필요없기에. 바로 리턴한다. 추후 문제있으면 고칠 것
			return;
		}
	}

	// 나는 중이 아닐때 발동하는 C 제어
	if (INFO(m_isFly) != true)
	{
		// 점프 중이 아닐 때
		if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN)
			&& INFO(m_isJump) != true)
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
	// 나는 중일 때 발동하는 C !!!!!!!!
	else if (INFO(m_isFly) == true)
	{
		// 꾹 눌렀을 때 0.3초마다 Flight한다
		m_fFlyKeyPressDelay += fTimeDelta;
		// 총 비행 량
		m_fFlyTime += fTimeDelta;
		_float fFlyTime = 5.5f;

		if (m_pFSM->Get_State() == STATE_EAT && m_pModelCom[INFO(m_eBodyState)]->IsFinished())
		{
			// IDLE로 바꿔놔야 어색하지 않다.
			m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, false);

			Change_State(STATE_FLIGHTLANDING, 40.f, false, false, BODY_VACUUM);
		}
		else if (m_pFSM->Get_State() == STATE_FLIGHTLANDING && m_pModelCom[INFO(m_eBodyState)]->IsFinished())
		{
			// 비로소, 날기가 종료된다.
			INFO(m_isFly) = false;
			m_fFlyTime = 0.f;
		}

		// 바닥에 닿으면 EAT -> FLIGHTLANDING 애니메이션 순으로 진행되도록 한다.
		if (m_pControllerCom->Is_Terrain())
			Change_State(STATE_EAT, 150.f, false, true, BODY_BALLOON);


		// 바닥에 안착했을 땐 다시 올라가면 절대 안된다.
		if (m_pFSM->Get_State() != STATE_FLIGHTLANDING)
		{

			if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
			{
				// 날면서 뽀잉
				if (m_fFlyTime < fFlyTime)
				{
					INFO(m_fJumpVelocity) = 4.f;
					Change_State(STATE_FLIGHT, 60.f, false, false, BODY_BALLOON);
				}
				else
					Change_State(STATE_FLIGHTLIMIT, 60.f, false, false, BODY_BALLOON);
			}
			else if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS) && m_fFlyKeyPressDelay > 0.3f)
			{
				// 날면서 뽀잉
				if (m_fFlyTime < fFlyTime)
				{
					INFO(m_fJumpVelocity) = 4.f;
					Change_State(STATE_FLIGHT, 60.f, false, false, BODY_BALLOON);
				}
				else
					Change_State(STATE_FLIGHTLIMIT, 60.f, false, false, BODY_BALLOON);
			}


			// FLIGHT, FLIGHTSTART 일 경우 모델의 ANIM 이 끝나면 다시 FLIGHTFALL 상태로 돌아간다.
			if ((m_pFSM->Get_State() == STATE_FLIGHT ||
				m_pFSM->Get_State() == STATE_FLIGHTSTART || 
				m_pFSM->Get_State() == STATE_FLIGHTLIMIT
				) == true
				&& m_pModelCom[INFO(m_eBodyState)]->IsFinished())
			{
				// 나는 도중엔 항상 이 친구로 돌아가도록 한다.
				if (m_fFlyTime < fFlyTime)
					Change_State(STATE_FLIGHTFALL, 60.f, true, true, BODY_BALLOON);
				else
					Change_State(STATE_FLIGHTLIMITFALL, 60.f, true, true, BODY_BALLOON);
			}
		}
	}

}

void CKirby::V_Input(_float fTimeDelta)
{

}

void CKirby::SetOn_Slope(_float fTimeDelta)
{
	// 지면의 up벡터
	PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
	_vector vTerrainNormal = CUtils::To_Vector(slope);
	Lerp_UpVector(vTerrainNormal, 20.f, fTimeDelta);
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
	// 점프 중일 때를 담당한다.
	if (true == INFO(m_isJump) && INFO(m_isFly) == false)
	{
		if (m_bJumpSpiting || m_bJumpVacuum)
			return;

		if (m_fJumpHoldTime > 0.3f)
		{
			// 만약 먹은 상태였다면
			if(m_IsEat == true)
				Change_State(STATE_EATJUMP, 50.f, false, true, BODY_BALLOON);
			else
				Change_State(STATE_JUMPEND, 60.f, false, true, BODY_DEFAULT);
		}
		else
		{
			// 만약 먹은 상태였다면
			if (m_IsEat == true)
				Change_State(STATE_EATJUMP, 50.f, false, true, BODY_BALLOON);
			else
				Change_State(m_eJumpState, 50.f, false, true, BODY_DEFAULT);
		}
	}
	// 착지했을 때를 담당한다.
	else if (true == INFO(m_isLanding))
	{
		// 최소 애니메이션이 재생되는 시간이다. ( 방향키를 누르면 0.2초 후 바로 Run 상태가 됨 )
		_float fChangeRunTime = 0.08f;
		switch (INFO(m_eBodyState))
		{
		case BODY_DEFAULT:
			fChangeRunTime = 0.08f;
			break;
		case BODY_BALLOON:
			fChangeRunTime = 0.15f;
			break;
		}

		m_fChangeRunTime += fTimeDelta;

		// 만약 먹은 상태였다면
		if (m_IsEat == true)
			Change_State(STATE_EATLANDING, 50.f, false, false, BODY_BALLOON);
		// Default 상태였다면
		else
		{
			if (m_fJumpHoldTime > 0.2f)
				Change_State(STATE_LANDINGEND, 30.f, false, false, BODY_DEFAULT);
			else
				Change_State(STATE_LANDINGSMALL, 50.f, false, false, BODY_DEFAULT);
		}

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
	// 날고 있을 때
	else if (INFO(m_isFly) == true)
	{


	}
	// 자유 낙하 ( 점프 중도 아니고, 착지 중도 아니다. ) 
	// 이럴때 떨어지는 애니메이션이 재생되어야 할 것이다.
	else
	{
		// 자유낙하가 아니면서, 1보다 높은 지형에 올라갔을 때 발동된다. Fall 발동
		if (m_pControllerCom->Compute_Height() > 2.f && m_bFreeFall == false)
			m_bFreeFall = true;
		
		// 자유낙하일 경우 FALL 애니메이션이 발동되고, 터레인 지형에 닿았을 때 비로소 탈출한다.
		if (m_bFreeFall == true)
		{
			// 먹은 상태였다면, FALL 애니메이션은 없다.
			if (m_IsEat == true)
				Change_State(STATE_EATJUMP, 50.f, false, true, BODY_BALLOON);
			else
				Change_State(STATE_FALL, 50.f, false, true, BODY_DEFAULT);

			if (true == m_pControllerCom->Is_Terrain())
			{
				m_bFreeFall = false;
				INFO(m_isLanding) = true;
			}
		}
		// 자유낙하 이후 바닥에 닿았으면 Anim Idle로 자유낙하 (지형을 타기) 한다.
		else if (m_bFreeFall == false)
			m_pControllerCom->FreeFall(m_pTransformCom, fTimeDelta, INFO(m_fGravityOffset));
	}

}

void CKirby::Kirby_Eat(_float fTimeDelta)
{
	// 테스트 (먹었을 때 0.1초정도간 발생하는 애니메이션 로직, 추후 수정)
	if (m_pGameInstance->Get_DIKeyState(DIK_I, KEY_DOWN))
	{
		//INFO(m_eBodyState) == BODY_DEFAULT ? INFO(m_eBodyState) = BODY_BALLOON : INFO(m_eBodyState) = BODY_DEFAULT;
		Change_State(STATE_EAT, 100.f, false, false, BODY_BALLOON);
		m_bIsEatingAnim = true;
	}

	// 먹고 나서 BALLOON 상태로 유지되게 하는 로직이다. 
	if (m_bIsEatingAnim == true && m_pModelCom[INFO(m_eBodyState)]->IsFinished())
	{
		m_bIsEatingAnim = false;
		// 이제부터 먹은 상태이다.
		m_IsEat = true;
	}

	// 뱉고 나서 다시 날씬해진다. Default로 돌아가야 해.
	if (m_IsSpit == true && m_pModelCom[INFO(m_eBodyState)]->IsFinished())
	{
		m_IsSpit = false;

		// 공중에서 만약, 뱉었다면..
		if (m_bJumpSpiting == true)
		{
			Change_State(STATE_FALL, 100.f, false, false, BODY_DEFAULT);
			m_bJumpSpiting = false;
		}
	}

}

void CKirby::Kirby_Vacuum(_float fTimeDelta)
{
	// 0.2초 보다 작게 눌렀을 경우 0.3초간은 무조건 흡수한다.
	const _float fminVacuumTime = 0.3f;
	if (m_bminVacuum == false && m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
	{
		m_fminVacuumTime += fTimeDelta;
		if (m_fminVacuumTime < fminVacuumTime)
		{
			m_bminVacuum = true;
		}
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_UP))
	{
		m_fminVacuumTime = 0.f;
	}

	// 0.4초 후, 최소 흡수 시간을 관여하는 부울 값이 false가 된다. 만약 키까지 때고 있었으면 완전히 흡수를 그만둔다.
	if (m_fVacuumTime > fminVacuumTime)
		m_bminVacuum = false;
	

	// 흡수 시작!!!!!!!!!!! (흡수 중이 아닐떄만 발동)
	if (m_fVacuumDelay > 0.5f && (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) || m_bminVacuum == true))
	{
		// 흡수가 끝나고 아이들로 무조건 가기 때문에, 튀는것을 막기 위한 행위이다.
		m_pModelCom[BODY_DEFAULT]->Set_Animation(STATE_IDLE, 60.f, true, false);


		if (m_tKirbyInfo.m_isJump == true)
			m_bJumpVacuum = true;

		INFO(m_isVacuum) = true;

		// 착지중일때 강제로 흡수하게 만든다.
		INFO(m_isLanding) = false;

		m_fVacuumTime += fTimeDelta;

		if (m_VacuumControl == VACUUM_VACUUMSTART)
		{
			Change_State(STATE_INHALESTART, 60.f, false, false, BODY_VACUUM);

			if (m_pModelCom[INFO(m_eBodyState)]->IsFinished())
			{
				m_VacuumControl = VACUUM_VACUUM;
			}
		}
		else if (m_VacuumControl == VACUUM_VACUUM)
		{
			// 공중에서 흡수를 발동하였을 경우, 땅에 닿는 순간 JumpVacuum 은 쓸모없어지고 땅에 닿는 순간 m_bLandingVacuum이 활성화된다.
			if (m_bJumpVacuum == true)
			{
				Change_State(STATE_INHALEFALL, 50.f, true, true, BODY_VACUUM);

				if (m_pControllerCom->Is_Terrain())
				{
					m_bLandingVacuum = true;
					m_bJumpVacuum = false;
				}
			}
			// 땅에 닿았을 때, 잠깐 발동하는 Landing Anim함수이다.
			else if (m_bLandingVacuum == true)
			{
				Change_State(STATE_INHALELANDING, 50.f, false, true, BODY_VACUUM);

				if (m_pModelCom[INFO(m_eBodyState)]->IsFinished())
				{
					m_bLandingVacuum = false;
				}
			}
			// 조작중일땐, 걸어가자.
			else if (INFO(m_isController) == true)
				Change_State(STATE_INHALEWALK, 50.f, true, true, BODY_VACUUM);
			else
				Change_State(STATE_INHALE, 50.f, true, true, BODY_VACUUM);

			// 만약, 1.2초 이상 흡수를 시도한다면, SUPERVACUUM으로 바뀐다.
			if (m_fVacuumTime > 1.2f)
			{
				if (INFO(m_isController) == true)
					m_VacuumControl = VACUUM_SUPERVACUUM;
				else
					m_VacuumControl = VACUUM_SUPERVACUUMSTART;
			}
		}
		else if (m_VacuumControl == VACUUM_SUPERVACUUMSTART)
		{
			INFO(m_eEyeState) = EYE_IDLE;

			// 우와아아아 하는거
			if (INFO(m_isController) == false)
			{
				Change_State(STATE_SUPERINHALESTART, 60.f, false, true, BODY_VACUUM);
			}

			// 조작하거나 하면 바로 SUPERVACUUM으로 넘어감.
			if (INFO(m_isController) == true || m_pModelCom[INFO(m_eBodyState)]->IsFinished())
			{
				m_VacuumControl = VACUUM_SUPERVACUUM;
			}
		}
		else if (m_VacuumControl == VACUUM_SUPERVACUUM)
		{
			INFO(m_eEyeState) = EYE_CLOSE;

			// 조작중이면
			if (INFO(m_isController) == true)
			{
				Change_State(STATE_SUPERINHALEWALK, 50.f, true, true, BODY_VACUUM);
			}
			else
			{
				Change_State(STATE_SUPERINHALE, 50.f, true, true, BODY_VACUUM);
			}

			if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_UP) ||
				m_pGameInstance->Get_DIKeyState(DIK_DOWN, KEY_UP) || 
				m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_UP) || 
				m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_UP))
			{
				m_VacuumControl = VACUUM_SUPERVACUUMSTART;
			}
		}
	}
	// 흡수 종료 (Vacuum 중이였다!)
	else if (INFO(m_isVacuum) == true)
	{
		Change_State(STATE_INHALEEND, 100.f, false, false, BODY_VACUUM);

		if (m_pModelCom[INFO(m_eBodyState)]->IsFinished())
		{
			m_VacuumControl = VACUUM_VACUUMSTART;
			m_fVacuumTime = 0.f;
			m_fVacuumDelay = 0.f;
			// isVacuum이 꺼짐으로서, 비로소 IDLE 상태로 복귀한다. (or) Eat일수도 있다.
			INFO(m_isVacuum) = false;

			//IDLE로 가면서 방향 보간되는거도 막기
			INFO(m_vTargetDir) = INFO(m_vMoveDir);

			// 최소로 흡수해주는 부울 값
			m_bminVacuum = false;

			if (m_bJumpVacuum == true)
			{
				m_bJumpVacuum = false;
			}

		}
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
		(_eBodyState == BODY_VACUUM && _iMeshIndex == 2) ||
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
		m_iIdleChoose == 0 ? Change_State(STATE_IDLESTREACH, 60.f, false, true, BODY_DEFAULT) 
			: Change_State(STATE_IDLELOOKAROUND, 60.f, false, true, BODY_DEFAULT);

		if (m_pModelCom[INFO(m_eBodyState)]->IsFinished())
		{
			m_iIdleChoose == 0 ? m_iIdleChoose = 1 : m_iIdleChoose = 0;
			m_fIdleStreachTime = 0.f;
		}
	}
	else
	{
		Change_State(STATE_IDLE, 60.f, true, true, BODY_DEFAULT);
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
	m_pFSM = CFSM::Create();

	// Default
	m_pFSM->Add_State(STATE_IDLE, CKirbyDefault_Idle_State::Create());
	m_pFSM->Add_State(STATE_IDLESTREACH, CKirbyDefault_Idle_State::Create());
	m_pFSM->Add_State(STATE_IDLELOOKAROUND, CKirbyDefault_Idle_State::Create());

	m_pFSM->Add_State(STATE_RUN, CKirbyDefault_Run_State::Create());

	m_pFSM->Add_State(STATE_JUMPL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPR, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPEND, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPFALL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_LANDINGEND, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_LANDINGSMALL, CKirbyDefault_Jump_State::Create());
	m_pFSM->Add_State(STATE_FALL, CKirbyDefault_Jump_State::Create());

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

	// Vacuum
	m_pFSM->Add_State(STATE_SPIT, CKirbyVacuum_Spit_State::Create());

	m_pFSM->Add_State(STATE_INHALESTART, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALEEND, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALEFALL, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALE, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_INHALELANDING, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_SUPERINHALE, CKirbyVacuum_Vacuum_State::Create());
	m_pFSM->Add_State(STATE_SUPERINHALESTART, CKirbyVacuum_Vacuum_State::Create());

	m_pFSM->Add_State(STATE_INHALEWALK, CKirbyVacuum_VacuumWalk_State::Create());
	m_pFSM->Add_State(STATE_SUPERINHALEWALK, CKirbyVacuum_VacuumWalk_State::Create());


	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Default_Desc = {};
	FSM_Default_Desc.iState = STATE_IDLE;
	FSM_Default_Desc.pModel = m_pModelCom[BODY_DEFAULT];
	m_pFSM->Initialize(&FSM_Default_Desc);

	CFSM::FSM_INFO		FSM_Balloon_Desc = {};
	FSM_Balloon_Desc.iState = STATE_IDLE;
	FSM_Balloon_Desc.pModel = m_pModelCom[BODY_BALLOON];
	m_pFSM->Initialize(&FSM_Balloon_Desc);

	CFSM::FSM_INFO		FSM_Vacuum_Desc = {};
	FSM_Vacuum_Desc.iState = STATE_IDLE;
	FSM_Vacuum_Desc.pModel = m_pModelCom[BODY_VACUUM];
	m_pFSM->Initialize(&FSM_Vacuum_Desc);
}

void CKirby::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody)
{
	INFO(m_eBodyState) = eBody;
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation, INFO(m_eBodyState));
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
