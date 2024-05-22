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

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;


	_float4 vPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	m_eBodyState = BODY_DEFAULT;
	m_eMouthState = MONTH_IDLE;
	m_eEyeState = EYE_IDLE;

	//m_eCurrentState = STATE_IDLE;

	// 카메라 기준으로 움직이기에 미리 받아둔다.
	if (m_pCamera == nullptr)
	{
		m_pCamera = static_cast<CCamera_Free*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Camera"), 0));
		Safe_AddRef(m_pCamera);
	}

	m_pCamera->Set_Target(m_pTransformCom);


	_float4 m_pCameraLook = m_pCamera->Get_TransformCom()->Get_State_Vector(CTransform::STATE_LOOK);
	m_pCameraLook.y = 0.f;
	m_pCameraLook = XMVector4Normalize(m_pCameraLook);
	m_tKirbyInfo.m_vMoveDir = -1.f * m_pCameraLook;
	m_tKirbyInfo.m_vTargetDir = m_tKirbyInfo.m_vMoveDir;

	m_pTransformCom->Look_At_ForLandObject(vPos + m_tKirbyInfo.m_vMoveDir);

	m_pModelCom[m_eBodyState]->Set_Animation(STATE_IDLE, 60.f, true, true);

	return S_OK;
}

_int CKirby::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	// 커비의 기본적인 축 보정, 밸런스 보정을 담당한다.
	Setting_KirbyBalance();
	// 키 입력에 대한 상태처리
	Key_Input(fTimeDelta);

	// 점프용 velocity(속도)
	m_fJumpVelocity -= GRAVITY * fTimeDelta;
	if (true == m_isJump)
		m_isJump = m_pControllerCom->Jump(m_pTransformCom, m_fJumpVelocity, fTimeDelta);
	else
		m_pControllerCom->FreeFall(m_pTransformCom, fTimeDelta);


	// FSM 제어
	if (m_pFSM != nullptr)
		m_pFSM->Update(this, fTimeDelta);

	return OBJ_NOEVENT;
}

void CKirby::Late_Tick(_float fTimeDelta)
{
	m_pModelCom[m_eBodyState]->Play_Animation(fTimeDelta);

	SetOn_Slope(fTimeDelta);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

	// 리지드바디

}

HRESULT CKirby::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom[m_eBodyState]->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (Kirby_FaceCustom(m_eBodyState, i) == true)
			continue;

		if (FAILED(m_pModelCom[m_eBodyState]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom[m_eBodyState]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;

		m_pModelCom[m_eBodyState]->Render(i);
	}

	return S_OK;
}

HRESULT CKirby::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom[m_eBodyState])))
		return E_FAIL;

	return S_OK;
}

void CKirby::Render_IMGUI()
{
	__super::Render_IMGUI();

	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	ImGui::Text("MoveDir X : %.2f \tMoveDir Y : %.2f \tMoveDir Z : %.2f ", m_tKirbyInfo.m_vMoveDir.x, m_tKirbyInfo.m_vMoveDir.y, m_tKirbyInfo.m_vMoveDir.z); ImGui::NewLine();
	ImGui::Text("TargetDir X : %.2f \tTargetDir Y : %.2f \tTargetDir Z : %.2f ", m_tKirbyInfo.m_vTargetDir.x, m_tKirbyInfo.m_vTargetDir.y, m_tKirbyInfo.m_vTargetDir.z);

}

_uint CKirby::Get_State()
{
	return m_pFSM->Get_State();
}

void CKirby::Setting_KirbyBalance()
{
	_float4 vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	m_pTransformCom->Look_At_ForLandObject(vPos + m_tKirbyInfo.m_vMoveDir);

	// 보정
	_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vEditRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	vEditRight = XMVector3Normalize(vEditRight);
	_vector vEditLook = XMVector3Cross(vEditRight, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	vEditLook = XMVector3Normalize(vEditLook);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vEditLook));
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vEditRight));
	m_pTransformCom->Set_State(CTransform::STATE_UP, XMVectorSet(0.f, 1.f, 0.f, 0.f));
}

void CKirby::Key_Input(_float fTimeDelta)
{
	//test
	if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_LF);
		else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_RF);
		else
			m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_FRONT);

		m_pFSM->ChangeState(STATE_RUN, 60.f, true, true);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_DOWN, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_LB);
		else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_RB);
		else
			m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_BACK);

		m_pFSM->ChangeState(STATE_RUN, 60.f, true, true);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_LEFT);
		m_pFSM->ChangeState(STATE_RUN, 60.f, true, true);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		m_tKirbyInfo.m_vTargetDir = Make_TargetDir(DIR_RIGHT);
		m_pFSM->ChangeState(STATE_RUN, 60.f, true, true);
	}
	else
	{
		m_pFSM->ChangeState(STATE_IDLE, 60.f, true, true);
	}



	//Test
	if (m_pGameInstance->Get_DIKeyState(DIK_P, KEY_DOWN))
	{
		m_iTestAnim++;
		m_pModelCom[m_eBodyState]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[m_eBodyState]->Set_TickPerSecond(60.f);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_O, KEY_DOWN))
	{
		m_iTestAnim--;
		if (m_iTestAnim < 0)
			m_iTestAnim = 0;
		m_pModelCom[m_eBodyState]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[m_eBodyState]->Set_TickPerSecond(60.f);

	}

	if (m_pGameInstance->Get_DIKeyState(DIK_0, KEY_DOWN))
	{
		m_eBodyState = BODY_DEFAULT;
		m_pModelCom[m_eBodyState]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[m_eBodyState]->Set_TickPerSecond(60.f);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_9, KEY_DOWN))
	{
		m_eBodyState = BODY_BALLOON;
		m_pModelCom[m_eBodyState]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[m_eBodyState]->Set_TickPerSecond(60.f);

	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_8, KEY_DOWN))
	{
		m_eBodyState = BODY_VACUUM;
		m_pModelCom[m_eBodyState]->Set_Animation(m_iTestAnim, true, true);
		m_pModelCom[m_eBodyState]->Set_TickPerSecond(60.f);
	}

	// JUMP TEST
	if (m_pGameInstance->Get_DIKeyState(DIK_SPACE, KEY_DOWN))
	{
		m_isJump = true;
		m_fJumpVelocity = 5.f;
	}
}


// ==================================== 커비 전용 ===================================
void CKirby::SetOn_Slope(_float fTimeDelta)
{
	// 지면의 up벡터
	PxVec3 slope = m_pControllerCom->Compute_Slope(m_pTransformCom);
	_vector vTerrainNormal = CUtils::To_Vector(slope);

	Lerp_UpVector(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), vTerrainNormal, 10.f, fTimeDelta);
}

/// <summary> 객체와 지면의 up벡터를 비교하여 객체의 각도를 보간한다. </summary>
/// <param name="_vOriginUp"> 객체의 up 벡터 </param>
/// <param name="_vTargetUp"> 지면의 노말 벡터 </param>
/// <param name="_maxAngle"> 해당 각도보다 크면 각도 보간이 된다. </param>
void CKirby::Lerp_UpVector(_fvector _vOriginUp, _fvector _vTargetUp, _float _maxAngle, _float fTimeDelta)
{
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(_vTargetUp, _vOriginUp));

	if (fAngle >= XMConvertToRadians(_maxAngle))
	{
		_vector vRight = XMVector3Cross(XMVector3Normalize(_vOriginUp), XMVector3Normalize(_vTargetUp));
		m_pTransformCom->Turn(vRight, fTimeDelta * fAngle * m_fOffsetTurn);
	}
}
// ==========================================================================

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
		m_pModelCom[m_eBodyState]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
		m_pModelCom[m_eBodyState]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
		m_pMouthTexture[m_eMouthState]->Bind_ShaderResource(m_pShaderCom, "g_KirbyMouthTexture", 0);
		m_pShaderCom->Begin(3);
		m_pModelCom[m_eBodyState]->Render(_iMeshIndex);
		return true;
	}
	// Default 상태의 눈 부위 // Vacuum 상태의 눈 부위 // Balloon 상태의 눈 부위
	else if ((_eBodyState == BODY_DEFAULT && _iMeshIndex == 3) ||
		(_eBodyState == BODY_VACUUM && _iMeshIndex == 0) ||
		(_eBodyState == BODY_BALLOON && _iMeshIndex == 3))
	{
		m_pModelCom[m_eBodyState]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", _iMeshIndex, TextureType_DIFFUSE);
		m_pModelCom[m_eBodyState]->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", _iMeshIndex);
		m_pEyeTexture[m_eEyeState]->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", 0);
		m_pShaderCom->Begin(4);
		m_pModelCom[m_eBodyState]->Render(_iMeshIndex);
		return true;
	}

	return false;
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
	m_pFSM->Add_State(STATE_RUN, CKirbyDefault_Run_State::Create());

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
