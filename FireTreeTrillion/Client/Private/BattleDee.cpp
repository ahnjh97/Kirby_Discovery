#include "stdafx.h"
#include "HitBox.h"
#include "FSM.h"
#include "BattleDee.h"
#include "Dee_Part.h"
#include "Dee_State.h"


_float3 CBattleDee::Make_DestPos()
{
	CTransform* pDeeDeeDeeTransform = m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_DeeDeeDee"), 0)->Get_TransformCom();
	_float3 vDestPos = pDeeDeeDeeTransform->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_LOOK) * 2.f;

	return vDestPos;
}

pair<DEE_ANIM, _bool> CBattleDee::Make_WhatToDo()
{
	DEE_ANIM eDeeState = DEEANIM_END;

	switch (CUtils::Make_RandomInt(0, 2))
	{
	case 0:
		eDeeState = DEEANIM_ENEMYRUN;
		break;
	case 1:
		eDeeState = DEEANIM_ANGERRUN;
		break;
	case 2:
		eDeeState = DEEANIM_ANGERRUN;
		break;
	default:
		eDeeState = DEEANIM_ANGERRUN;
		break;
	};

	Set_DeeEyeState(DEEEYE_SADNESS);
	return { eDeeState, true };
}

CBattleDee::CBattleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CWaddleDee{ pDevice, pContext }
{
}

CBattleDee::CBattleDee(const CBattleDee& rhs)
	:CWaddleDee{ rhs }
{

}

HRESULT CBattleDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBattleDee::Initialize(void* pArg)
{
	DEE_DESC pDeeDesc{};

	if (nullptr != pArg)
		pDeeDesc = *(DEE_DESC*)pArg;

	pDeeDesc.fSpeedPerSec = 5.f;
	pDeeDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	HRESULT hr;

	hr = __super::Initialize(&pDeeDesc);
	CHECK_FAILED(hr);

	hr = Add_Components();
	CHECK_FAILED(hr);

	Make_InitialState(pDeeDesc.eCharacter);

	hr = Add_PartObjects();
	CHECK_FAILED(hr);

	//m_pTransformCom->Rotation({ 0.f, 1.f, 0.f, 0.f }, ToRadian(180.f));

	m_eAbilityType = ABILITY_DEFAULT;

	return S_OK;
}



_int CBattleDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (m_ePhyXState == PO_VACUUMING || m_ePhyXState == PO_FLYDEADAWAY)
		Change_State(DEEANIM_DAMAGE, 120.f, true, false);

	//__super::Tick(m_fTimeDelta);
	// 모션블러 계산
	Compute_MotionBlur();

	// FSM 제어
	if (m_pFSM != nullptr)
		m_pFSM->Update(this, fTimeDelta);

	// 날아가는 도중엔 경사면 보간 제어가 필요없다.
	if (Get_State() != DEEANIM_DAMAGE)
	{
		SetOn_Slope(fTimeDelta);
	}

	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(m_fTimeDelta);

	//공통된 디 관련 변수를 업데이트 - 초기화한다
	Dee_SystemTick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CBattleDee::Late_Tick(_float fTimeDelta)
{
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(m_fTimeDelta);

	if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
		m_pModelCom->Play_Animation(m_fAccTime);


	//시야 벗어나면 컬링
	if (!m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 2.0f))
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

}

HRESULT CBattleDee::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{

		if (Custom_Face(i) == true)
			continue;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;


		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;

}

HRESULT CBattleDee::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CBattleDee::Add_AnimEvent()
{
	__super::Add_AnimEvent();
}

void CBattleDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		Change_State(DEEANIM_DAMAGE, 60.f, true, true);
		Set_DeeEyeState(DEEEYE_SMILE);
	}
}

#ifdef _DEBUG

void CBattleDee::Render_IMGUI()
{
	__super::Render_IMGUI();

	ImGui::Text(u8"현재 애님 인덱스 : %d", m_pFSM->Get_State());
}

#endif

HRESULT CBattleDee::Add_Components()
{
	HRESULT hr;

	//쉐이더
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	//모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_WaddleDeeBase"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	//눈 텍스쳐
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Dee_Eye"),
		TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	CHECK_FAILED(hr);


	//컨트롤러
	CCharacterController::CONTROLLER_DESC ControllerDesc{};
	ControllerDesc.vInitialPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	ControllerDesc.tCapsuleShape.fRadius = 0.1f;
	ControllerDesc.tCapsuleShape.fHeight = 0.2f;
	ControllerDesc.fOffset = 0.2f;
	ControllerDesc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &ControllerDesc);
	CHECK_FAILED(hr);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = BATTLEDEE;
	hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	CHECK_FAILED(hr);

	Set_BodyCollider(COLLIDER_CYLINDER, 0.6f, 1.2f, 1.2f);

	SetUp_FSM();

	return S_OK;
}

HRESULT CBattleDee::Add_PartObjects()
{

	return S_OK;
}

HRESULT CBattleDee::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		ALARM_FAIL("쉐이더가 읍서");

	HRESULT hr;

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));
	CHECK_FAILED(hr);


	hr = m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4));
	CHECK_FAILED(hr);
	hr = m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));
	CHECK_FAILED(hr);

	return S_OK;
}

void CBattleDee::SetUp_FSM()
{
	m_pFSM = CFSM::Create();

	m_pFSM->Add_State(DEEANIM_TROUBLE, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_ANGERRUN, CDee_Run_State::Create());
	m_pFSM->Add_State(DEEANIM_ENEMYRUN, CDee_Panic_State::Create());
	m_pFSM->Add_State(DEEANIM_CHEERINGA, CDee_Emotion_State::Create());

	m_pFSM->Add_State(DEEANIM_DAMAGE, CDee_FlyStun_State::Create());
	m_pFSM->Add_State(DEEANIM_MOVEFALL, CDee_Interact_State::Create());



}

void CBattleDee::Make_InitialState(DEE_CHARACTER iDeeCharacter)
{
	DEE_ANIM eAnim = DEEANIM_ANGERRUN;


	switch (iDeeCharacter)
	{
	case DEECHARACTER_TROUBLE:
		eAnim = DEEANIM_TROUBLE;
		break;
	case DEECHARACTER_RUN:
		eAnim = DEEANIM_ANGERRUN;
		break;
	case DEECHARACTER_ANGRY:
		eAnim = DEEANIM_CHEERINGA;
		break;
	default:
		eAnim = DEEANIM_ANGERRUN;
		break;
	}

	CFSM::FSM_INFO	FSMDesc = {};
	FSMDesc.iState = eAnim;
	FSMDesc.pModel = &m_pModelCom;

	m_pFSM->Initialize(&FSMDesc);

	m_pModelCom->Set_Animation(eAnim, 60.f, true, true);
	Set_DeeEyeState(DEEEYE_SADNESS);

}

_bool CBattleDee::Custom_Face(_uint iMeshIndex)
{
	if (iMeshIndex == 2)
	{
		HRESULT hr;

		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iMeshIndex, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIndex);
		CHECK_FAILED(hr);

		hr = m_pEyeTextureCom->Bind_ShaderResource(m_pShaderCom, "g_KirbyEyeTexture", (_uint)m_eEyeState);
		CHECK_FAILED(hr);

		_bool bStencil = true;
		_bool bRimLight = true;
		_bool bMotionBlur = true;
		m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool));

		m_pShaderCom->Begin(ANIMMODEL_EYE);
		m_pModelCom->Render(iMeshIndex);

		return true;
	}

	return false;
}

CBattleDee* CBattleDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBattleDee* pInstance = new CBattleDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CBattleDee"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBattleDee::Clone(void* pArg)
{
	CBattleDee* pInstance = new CBattleDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CBattleDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBattleDee::Free()
{
	Safe_Release(m_pEyeTextureCom);

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);

	m_PartObjects.clear();

	__super::Free();
}
