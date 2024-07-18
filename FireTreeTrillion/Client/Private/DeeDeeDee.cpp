#include "stdafx.h"
#include "DeeDeeDee.h"
#include "DeeDeeDee_State.h"
#include "FSM.h"

#include "HitBox.h"
#include "Bone.h"
#include "DeeDeeDeeHammer.h"
#include "Camera_Main.h"
#include "EventCenter.h"

#include "Ability.h"
#include "UI_MessageWindow.h"

#include "UI_Interactable.h"

#define INFO(Dst) m_tInfo.Dst

CDeeDeeDee::CDeeDeeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CDeeDeeDee::CDeeDeeDee(const CDeeDeeDee& rhs)
	: CMonster{ rhs }
{
}

HRESULT CDeeDeeDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDeeDeeDee::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_tInfo.m_vOriginPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	m_fMaxHp = 200.f;
	m_fHp = 200.f;
	//m_fHp = 10.f;
	m_fAttack = 15.f;
	m_eVacuumSize = SIZE_BIG;
	m_eAbilityType = ABILITY_DEFAULT;

	Add_AnimEvent();

	if (*m_pCurrentLevelID == LEVEL_TOWN)
	{
		m_pModelCom->Set_Animation(STATE_WAIT, 20.f, true, false);
		m_pTransformCom->Turn(_float4(0.f, 1.f, 0.f, 0.f), 1.f, 160.f);
	}
	else
	{
		m_pModelCom->Set_Animation(STATE_WAIT, 60.f, true, false);
	}
	 
	_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	m_vNeckLook = m_vLEyeLook = m_vREyeLook = m_tInfo.m_vMoveDir = m_tInfo.m_vTargetDir = vLook;

	m_pUI_Interactable = dynamic_cast<CUI_Interactable*>(m_pGameInstance->Add_CloneReturn(*m_pCurrentLevelID, L"Layer_UI", L"Prototype_GameObject_UI_Interactable"));
	m_pUI_Interactable->Set_Owner(this);
	m_pUI_Interactable->Set_Offset(7.f);

	return S_OK;
}

_int CDeeDeeDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	// 죽었다면, 데드 애니메이션 재생 (얘는 정상시간 받음)
	Dead_Animation(fTimeDelta);

	// 망치 뼈 업데이트
	Hammer_BoneUpdate();

	if (*m_pCurrentLevelID != LEVEL_TOWN)
	{
		// 디디디 시스템 틱
		DeeDeeDee_SystemTick(m_fTimeDelta);
	}
	// 모션블러 계산
	Compute_MotionBlur();


	// FSM 제어
	if (m_pFSM != nullptr)
		m_pFSM->Update(this, m_fTimeDelta);

	if (m_fWhiteColorDiffuse > 0.f)
	{
		// 0.2초만에 다시 원래 색상으로 복귀한다.
		m_fWhiteColorDiffuse -= fTimeDelta * 5.f;

		if (m_fWhiteColorDiffuse < 0.f)
			m_fWhiteColorDiffuse = 0.f;
	}

	Damage_Delay(m_fTimeDelta);

	if (m_pWeapons != nullptr)
		m_pWeapons->Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CDeeDeeDee::Late_Tick(_float fTimeDelta)
{
	// 뼈를 꺾는다.
	Look_Player(m_fTimeDelta);

	if (m_pWeapons != nullptr)
		m_pWeapons->Late_Tick(m_fTimeDelta);

	if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
		m_ePhyXState == PO_FLYDEADAWAY ? m_pModelCom->Play_Animation(m_fAccTime * 0.3f) : m_pModelCom->Play_Animation(m_fAccTime);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 6.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}


}

HRESULT CDeeDeeDee::Render()
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

		if (i == 0 || i == 6)
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
				return E_FAIL;
		}

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;

}

HRESULT CDeeDeeDee::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG

void CDeeDeeDee::Render_IMGUI()
{
	ImGui::Text("Pattern : %d", (_int)INFO(m_ePattern));
	ImGui::Separator(); ImGui::NewLine();
	__super::Render_IMGUI();
}

#endif

void CDeeDeeDee::Add_AnimEvent()
{
	__super::Add_AnimEvent();

	// 1. 한 애니메이션에서 같은 이름의 이벤트 가능
	// 2. 재생 기준은 애님툴에서 지정한 애니메이션인지 + 시작 프레임이 애니메이션 프레임안에 들어가는 지
	// 3. 두번째 인자로 넣어준 람다가 시작 프레임 한번만 실행된다.
	m_pModelCom->Add_Event("ApplyDamage", [this]() {
		// 커비의 히트박스를 실시간으로 변화시킨다.
		HitBoxChanger(m_pFSM->Get_State());
		});
	// 사운드 처리
}

void CDeeDeeDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	static _bool bOnce = false;

	// PARK로 이동하기 위한 다이얼로그 출력
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_DOWN)) //07.14) 키 변경 C > A
	{
		m_pUI_Interactable->Set_IsRender(false);

		CUI_MessageWindow* pMWindow = dynamic_cast<CUI_MessageWindow*>
			(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_UI_Msg_DeeDeeDee")));
		CHECK_NULLPTR(pMWindow);
		pMWindow->Show_DialogMessage();

		CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
		CHECK_NULLPTR(pCameraMain);
		pCameraMain->Lock_All({ -5.f, 39.f, 30.f }, { -0.3f, -0.2f, 0.93f }, true);

		bOnce = true;
	}

	if (bOnce == false)
		m_pUI_Interactable->Set_IsRender(true);
}

void CDeeDeeDee::Change_State(STATE_TYPE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CDeeDeeDee::Look_Player(_float fTimeDelta)
{
	CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));

	if (pPlayer == nullptr)
		return;

#pragma region 모가지
	////// 목을 돌린다.

	CBone* pBone = m_pModelCom->Get_BonePtr("C_FaceJ");
	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vLook.y = 0.f;
	vLook.Normalize();

	_float4 vTargetPos = pPlayer->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vTargetDir = vTargetPos - vPos;
	vTargetDir.y = 0.f;
	vTargetDir.Normalize();

	_float fNeckAngle = ToDegree(acos(vLook.Dot(vTargetDir)));

	_bool ONEye = { true };
	if (fNeckAngle > 40.f || m_tInfo.m_isBattle == false)
	{
		vTargetDir = vLook;
		ONEye = false;
	}

	Bone_Turn_Interpolate(m_vNeckLook, vTargetDir, fTimeDelta);
	Quaternion vStartQuat = CUtils::Make_Quat_FromDir(vLook);
	Quaternion vDestQuat = CUtils::Make_Quat_FromDir(m_vNeckLook);
	vStartQuat.Inverse(vStartQuat);
	Quaternion vResultQuat = vDestQuat * vStartQuat;
	_matrix RotationMatrix = _float4x4::CreateFromQuaternion(vResultQuat);
	_float4x4* pEditMatrix = pBone->Get_EditMatrixPtr();
	*pEditMatrix = RotationMatrix;

	if (m_tInfo.m_isBattle == false)
	{
		*pEditMatrix = _float4x4::Identity;
	}
	////////
#pragma endregion

	CBone* pLeftEyeBone = m_pModelCom->Get_BonePtr("L_EyeBallJ");
	CBone* pRightEyeBone = m_pModelCom->Get_BonePtr("R_EyeBallJ");

	_float4x4 LeftEyeMatrix = *pLeftEyeBone->Get_CombinedTransformationMatrix() * m_pTransformCom->Get_WorldFloat4x4();
	_float4x4 RightEyeMatrix = *pRightEyeBone->Get_CombinedTransformationMatrix() * m_pTransformCom->Get_WorldFloat4x4();

	vPos.y += 3.5f;
	_float4 vLeftEyePos = vPos; // CUtils::Get_State_Vector_Matrix(LeftEyeMatrix, CUtils::STATE_POSITION);
	_float4 vRightEyePos = vPos; // CUtils::Get_State_Vector_Matrix(RightEyeMatrix, CUtils::STATE_POSITION);

	_float4 vPlayerPos = pPlayer->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

	_float4 vEyeDir = m_vNeckLook; // m_pTransformCom->Get_State(CTransform::STATE_LOOK);



	_float4 vTargetLeftEyeDir = XMVector3Normalize(vPlayerPos - vLeftEyePos);
	vTargetLeftEyeDir.y *= -1.f;
	_float4 vTargetRightEyeDir = XMVector3Normalize(vPlayerPos - vRightEyePos);

	_vector		vLeftAxis = XMVector3Cross(XMVector3Normalize(vEyeDir), XMVector3Normalize(vTargetLeftEyeDir));
	_vector		vRightAxis = XMVector3Cross(XMVector3Normalize(vTargetRightEyeDir), XMVector3Normalize(vEyeDir));

	//	_float3 vLeftAxis = static_cast<_float3>(vEyeDir).Cross((_float3)vTargetLeftEyeDir);
	//	_float3 vRightAxis = static_cast<_float3>(vEyeDir).Cross((_float3)vTargetRightEyeDir);
	//_float3 vLeftLocalAxis = Vector3::TransformNormal(vLeftAxis, m_pTransformCom->Get_WorldMatrix_Inv());
	//	Quaternion LeftquatRotation = Quaternion::CreateFromAxisAngle(vLeftLocalAxis, fAngle);
	_float fAngle = acos(vEyeDir.Dot(vTargetLeftEyeDir));

	_vector		vQuaternionLeft = XMQuaternionRotationAxis(XMVector3TransformNormal(vLeftAxis, m_pTransformCom->Get_WorldMatrix_Inverse()), fAngle);

	_float3 vRightLocalAxis = Vector3::TransformNormal(vRightAxis, m_pTransformCom->Get_WorldMatrix_Inv());
	fAngle = acos(vEyeDir.Dot(vTargetRightEyeDir));
	//	Quaternion RightquatRotation = Quaternion::CreateFromAxisAngle(vRightLocalAxis, fAngle);

	_vector		vQuaternionRight = XMQuaternionRotationAxis(XMVector3TransformNormal(vRightAxis, m_pTransformCom->Get_WorldMatrix_Inverse()), fAngle);

	//	_float4x4 LeftRotMat = _float4x4::CreateFromQuaternion(LeftquatRotation);
	//	_float4x4 RightRotMat = _float4x4::CreateFromQuaternion(RightquatRotation);

	_matrix		LeftRotMat = XMMatrixRotationQuaternion(vQuaternionLeft);
	_matrix		RightRotMat = XMMatrixRotationQuaternion(vQuaternionRight);

	pEditMatrix = pLeftEyeBone->Get_EditMatrixPtr();
	*pEditMatrix = LeftRotMat;

	pEditMatrix = pRightEyeBone->Get_EditMatrixPtr();
	*pEditMatrix = RightRotMat;

	if (ONEye == false/*ToDegree(acos(vEyeDir.Dot(vTargetLeftEyeDir))) > 40.f || m_tInfo.m_isBattle == false*/)
	{
		pBone = m_pModelCom->Get_BonePtr("L_EyeBallJ");
		pEditMatrix = pBone->Get_EditMatrixPtr();
		*pEditMatrix = _float4x4::Identity;
		pBone = m_pModelCom->Get_BonePtr("R_EyeBallJ");
		pEditMatrix = pBone->Get_EditMatrixPtr();
		*pEditMatrix = _float4x4::Identity;
	}


}

void CDeeDeeDee::Bone_Turn_Interpolate(_float4& vMoveDir, const _float4& vTargetDir, _float fTimeDelta)
{
	if (vMoveDir == vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 4.f;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);

	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));

	if (fcosTheta < -0.9995f || fcosTheta > 0.9995f)
	{
		//// 180도로 NaN 방지 랜덤으로 -1, 1도 틀어줌
		//_float4x4 rotationMatrix;
		//XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
		//CUtils::Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomInt(0, 1) == 1 ? 1.f : -1.f);
		//vMoveDir = XMVector3Transform(vMoveDir, XMLoadFloat4x4(&rotationMatrix));
		//vMoveDir = XMVectorSetW(vMoveDir, 0.0f);
		return;
	}
	else
	{
		_float ftheta = acos(fcosTheta);
		_float fAngleDegrees = XMConvertToDegrees(ftheta);

		if (fAngleDegrees < 1.0f)
		{
			vMoveDir = vTargetDir;
		}
		else
		{
			_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
			_float fAlpha = sin((1 - fInterpolate) * ftheta) / fsinTheta;
			_float fBeta = sin(fInterpolate * ftheta) / fsinTheta;
			_float4 vResult = vMoveDirXZ * fAlpha + vTargetDirXZ * fBeta;
			vMoveDir = XMVector4Normalize(vResult);
			vMoveDir = XMVector3Normalize(vMoveDir);
		}
	}
}

_bool CDeeDeeDee::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

_float CDeeDeeDee::IsAnimRatio()
{
	return m_pModelCom->Get_AnimRatio();
}

HRESULT CDeeDeeDee::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DeeDeeDee"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	m_ppModelForAnimTool = &m_pModelCom;

	/* For.Com_CharacterController */
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	desc.tCapsuleShape.fRadius = 3.f;
	desc.fOffset = 3.5f;
	//desc.fOffset = 1.f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);

	/* FSM */
	SetUp_FSM();

	// 마을 말곤 망치를 들고 있다.
	if (*m_pCurrentLevelID != LEVEL_TOWN)
	{
		CHitBox::HITBOX_DESC HitBox{};
		HitBox.pOwner = this;
		HitBox.pDesc = &m_tColliderDesc[BODY];
		HitBox.pCollisionType = BOSS_DEEDEEDEE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
			return E_FAIL;
		Set_BodyCollider(COLLIDER_CYLINDER, 1.5f, 3.f, 3.f);


		HitBox.pDesc = &m_tColliderDesc[ATTACK];
		HitBox.pCollisionType = HITBOX_DEEDEEDEE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
			return E_FAIL;

		CDeeDeeDeeHammer::DEEDEEDEEHAMMER_DESC WeaponDesc{};
		WeaponDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		WeaponDesc.pBoneMatrix = &m_WeaponMatrix;
		WeaponDesc.pWhite = &m_fWhiteColorDiffuse;
		m_pWeapons = static_cast<CDeeDeeDeeHammer*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeeDeeDeeHammer"), &WeaponDesc));
		CHECK_NULLPTR(m_pWeapons);
	}
	// 마을엔 히트박스를 갖고 있다.
	else
	{
		CHitBox::HITBOX_DESC HitBox{};
		HitBox.pOwner = this;
		HitBox.pDesc = &m_tColliderDesc[BODY];
		HitBox.pCollisionType = NPC;
		hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
		CHECK_FAILED(hr);
		Set_BodyCollider(COLLIDER_SPHERE, 3.f, 0.f, 10.f);
	}

	return S_OK;
}

HRESULT CDeeDeeDee::Bind_ShaderResources()
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

void CDeeDeeDee::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();

	m_pFSM->Add_State(STATE_WAIT, CDeeDeeDee_Idle_State::Create());
	m_pFSM->Add_State(STATE_RUN, CDeeDeeDee_Run_State::Create());
	m_pFSM->Add_State(STATE_WALK, CDeeDeeDee_Run_State::Create());

	m_pFSM->Add_State(STATE_SHOUTSTART, CDeeDeeDee_Shout_State::Create());
	m_pFSM->Add_State(STATE_SHOUT, CDeeDeeDee_Shout_State::Create());
	m_pFSM->Add_State(STATE_SHOUTEND, CDeeDeeDee_Shout_State::Create());

	m_pFSM->Add_State(STATE_SLIDINGSTART, CDeeDeeDee_Slide_State::Create());
	m_pFSM->Add_State(STATE_SLIDING, CDeeDeeDee_Slide_State::Create());
	m_pFSM->Add_State(STATE_SLIDINGEND, CDeeDeeDee_Slide_State::Create());

	m_pFSM->Add_State(STATE_JUMPSTART, CDeeDeeDee_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMP, CDeeDeeDee_Jump_State::Create());
	m_pFSM->Add_State(STATE_JUMPEND, CDeeDeeDee_Jump_State::Create());
	m_pFSM->Add_State(STATE_FALL, CDeeDeeDee_Jump_State::Create());
	m_pFSM->Add_State(STATE_LANDING, CDeeDeeDee_Jump_State::Create());

	m_pFSM->Add_State(STATE_HAMMERSIDE, CDeeDeeDee_SideAttack_State::Create());
	m_pFSM->Add_State(STATE_HAMMERSIDECHARGE, CDeeDeeDee_SideAttack_State::Create());
	m_pFSM->Add_State(STATE_HAMMERSIDEWALK, CDeeDeeDee_SideAttack_State::Create());
	m_pFSM->Add_State(STATE_HAMMERSIDEATTACK, CDeeDeeDee_SideAttack_State::Create());

	m_pFSM->Add_State(STATE_HAMMERATTACK, CDeeDeeDee_HammerAttack_State::Create());
	m_pFSM->Add_State(STATE_HAMMERATTACKCHARGE, CDeeDeeDee_HammerAttack_State::Create());
	m_pFSM->Add_State(STATE_HAMMERATTACKHIT, CDeeDeeDee_HammerAttack_State::Create());

	m_pFSM->Add_State(STATE_DEATH, CDeeDeeDee_Death_State::Create());
	m_pFSM->Add_State(STATE_DEATHWAIT, CDeeDeeDee_Death_State::Create());
	m_pFSM->Add_State(STATE_DEATHWAITEND, CDeeDeeDee_Death_State::Create());

	m_pFSM->Add_State(STATE_COMMAND, CDeeDeeDee_Initialize_State::Create());


	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.pModel = &m_pModelCom;
	FSM_Desc.iState = STATE_WAIT;
	m_pFSM->Initialize(&FSM_Desc);
}

void CDeeDeeDee::HitBoxChanger(_uint eState)
{
	_uint iItemCnt = { 0 };
	HRESULT hr;
	_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	vLook.m128_f32[1] = 0.f;

	switch (eState)
	{
	case STATE_SLIDING:
		Activate_SphereCollider(0.5f, 5.f);
		break;
	case STATE_LANDING:
		// 별 아이템 떨굼
		iItemCnt = 5;
		for (_uint i = 0; i < iItemCnt; ++i)
		{
			CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
			if (i < iItemCnt / 2)
				AbilityItemDesc.fRotateDir = 1.f;															// 별 회전 방향 오른쪽
			else
				AbilityItemDesc.fRotateDir = -1.f;															// 별 회전 방향 왼쪽
			AbilityItemDesc.fAngle = 360.f / (_float)iItemCnt * i;											// 별의 진행 방향의 각도
			AbilityItemDesc.vDir = XMVector3Normalize(vLook) * 2.f;					// 별의 진행 방향
			AbilityItemDesc.vPosition = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + vLook * 2.f;		// 별의 생성 위치
			AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
			CHECK_FAILED(hr);
		}
		Activate_SphereCollider(0.5f, 7.f);
		break;
	case STATE_HAMMERSIDEATTACK:
		// 별 아이템 떨굼
		iItemCnt = 5;
		for (_uint i = 0; i < iItemCnt; ++i)
		{
			CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
			if (i < iItemCnt / 2)
				AbilityItemDesc.fRotateDir = 1.f;															// 별 회전 방향 오른쪽
			else
				AbilityItemDesc.fRotateDir = -1.f;															// 별 회전 방향 왼쪽
			AbilityItemDesc.fAngle = 360.f / (_float)iItemCnt * i;											// 별의 진행 방향의 각도
			AbilityItemDesc.vDir = XMVector3Normalize(vLook) * 2.f;					// 별의 진행 방향
			AbilityItemDesc.vPosition = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + vLook * 2.f;		// 별의 생성 위치
			AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
			CHECK_FAILED(hr);
		}
		Activate_SphereCollider(0.5f, 10.f);
		break;
	case STATE_HAMMERATTACKHIT:
		// 별 아이템 떨굼
		iItemCnt = 1;
		for (_uint i = 0; i < iItemCnt; ++i)
		{
			CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
			if (i < iItemCnt / 2)
				AbilityItemDesc.fRotateDir = 1.f;															// 별 회전 방향 오른쪽
			else
				AbilityItemDesc.fRotateDir = -1.f;															// 별 회전 방향 왼쪽
			AbilityItemDesc.fAngle = 360.f / (_float)iItemCnt * i;											// 별의 진행 방향의 각도
			AbilityItemDesc.vDir = XMVector3Normalize(vLook) * 2.f;					// 별의 진행 방향
			AbilityItemDesc.vPosition = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + vLook * 10.f;		// 별의 생성 위치
			AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
			CHECK_FAILED(hr);
		}
		Activate_FrustumCollider(0.5f, 10.f, 90.f);
		break;
	case STATE_SHOUTSTART:
		Activate_FrustumCollider(0.5f, 12.f, 90.f);
		break;

	default:
		break;
	}
}

//카메라 메인에 자기 자신을 두 번째 타겟으로 등록한다.
HRESULT CDeeDeeDee::Make_TargetToCams()
{
	if (*m_pCurrentLevelID == LEVEL_TOOL_ANIM)
		return S_OK;

	CCamera_Main* pCameraMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
	CHECK_NULLPTR(pCameraMain);

	pCameraMain->Set_Target(m_pTransformCom, CCamera::TARGET_SECOND, CCamera::FOCUS_BOTH);

	return S_OK;
}

void CDeeDeeDee::DeeDeeDee_SystemTick(_float fTimeDelta)
{
	if (*m_pCurrentLevelID == LEVEL_TOOL_ANIM)
		return;

	// 디디디는 항상 m_vMoveDir를 바라본다.
	_float4 vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	m_pTransformCom->Look_At_ForLandObject(vPos + m_tInfo.m_vMoveDir);

	// 맨처음에만!! 발동하는 애니메이션!!
	if (m_bInitializeAnim == true)
	{
		CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
		_float4 vPlayerPos = pPlayer->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

		if ((INFO(m_vOriginPos) - vPlayerPos).Length() < 30.f)
		{
			if (Get_State() == STATE_WAIT)
			{
				if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_BossUI"), TEXT("Prototype_GameObject_HUD_BossHpBar"), this)))
					return;
				Change_State(STATE_COMMAND, 60.f, false, true);
				m_bInitializeAnim = false;

				//전투 시작 이벤트 호출
				CEventCenter::Get_Instance()->Notify(KEVENT_DDD_BATTLESTART, this);
			}
		}
	}


	// 맨 처음 발동하는 애니메이션이 끝나고, 진정한 패턴들이 시작된다.
	if (m_bInitializeAnim == false)
	{
		CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
		_float4 vPlayerPos = pPlayer->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

		if ((INFO(m_vOriginPos) - vPlayerPos).Length() < 30.f)
		{
			INFO(m_bInRangePlayer) = true;
		}
		else
		{
			INFO(m_bInRangePlayer) = false;
		}


		// 내 근처에 있어도 플레이어가 범위 안에 들어있는 것으로 판단한다.
		if ((vPos - vPlayerPos).Length() < 5.f)
			INFO(m_bInRangePlayer) = true;
	}

	// 디디디는 멀리가면 돌아가고 싶어한다.
	if ((vPos - INFO(m_vOriginPos)).Length() > 30.f)
	{
		INFO(m_bBackToOriginPos) = true;
	}


	INFO(m_ePattern) = Now_Pattern();


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

CDeeDeeDee::MYPATTERN CDeeDeeDee::Now_Pattern()
{
	if (INFO(m_bInRangePlayer) == true && INFO(m_bBackToOriginPos) == false) {
		return PATTERN_BATTLE;
	}
	if (INFO(m_bInRangePlayer) == true && INFO(m_bBackToOriginPos) == true) {
		INFO(m_bBackToOriginPos) = false;
		return PATTERN_BATTLE;
	}
	if (INFO(m_bInRangePlayer) == false && INFO(m_bBackToOriginPos) == false) {
		return PATTERN_CENTER;
	}
	if (INFO(m_bInRangePlayer) == false && INFO(m_bBackToOriginPos) == true) {
		return PATTERN_CENTER;
	}

	return INFO(m_ePattern);
}

void CDeeDeeDee::Hammer_BoneUpdate()
{
	m_WeaponMatrix = *(m_pModelCom->Get_BonePtr("RHaveL")->Get_CombinedTransformationMatrix());
}

void CDeeDeeDee::Dead_Animation(_float fTimeDelta)
{
	if (m_isDead == false)
	{
		if (m_fHp <= 0.f)
		{
			m_isDead = true;
			_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			Change_State(STATE_DEATH, 60.f, false, true);
			m_pGameInstance->Setting_RadialBlur(vPos, 20.f, 10.f);
			m_pGameInstance->Set_FirstTimerRatio(0.2f);
			m_pGameInstance->Set_SecondTimerRatio(0.2f);
			m_pGameInstance->Set_BlackBackGround(true);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Zoom(-5.f);
		}
	}

	if (m_isDead == true)
	{
		if (m_bRestoreTrigger == true)
		{
			m_fDeadTime += fTimeDelta;
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Zoom(-5.f + (m_fDeadTime * 2.5f));

			if (m_fDeadTime > 2.f)
			{
				m_pGameInstance->Restore_FirstTimer();
				m_pGameInstance->Restore_SecondTimer();
				m_pGameInstance->Set_BlackBackGround(false);
				m_bRestoreTrigger = false;
				pCamera->Zoom(0.f);
			}
		}
	}
}

CDeeDeeDee* CDeeDeeDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDeeDeeDee* pInstance = new CDeeDeeDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDeeDeeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDeeDeeDee::Clone(void* pArg)
{
	CDeeDeeDee* pInstance = new CDeeDeeDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDeeDeeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDeeDeeDee::Free()
{
	__super::Free();
	Safe_Release(m_pWeapons);

}
