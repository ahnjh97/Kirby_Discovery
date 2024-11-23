#include "stdafx.h"
#include "HitBox.h"
#include "FSM.h"
#include "FoodShopDee.h"
#include "Dee_Part.h"
#include "Dee_State.h"
#include "UI_MessageWindow.h"
#include "Camera_Main.h"
#include "UI_Interactable.h"
#include "Kirby.h"

CFoodShopDee::CFoodShopDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CWaddleDee{ pDevice, pContext }
{
}

CFoodShopDee::CFoodShopDee(const CFoodShopDee& rhs)
	:CWaddleDee{ rhs }
{
}

HRESULT CFoodShopDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFoodShopDee::Initialize(void* pArg)
{
	DEE_DESC pDeeDesc{};

	if (nullptr != pArg)
		pDeeDesc = *(DEE_DESC*)pArg;

	pDeeDesc.fSpeedPerSec = 5.f;
	pDeeDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	HRESULT hr;

	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	hr = Add_Components();
	CHECK_FAILED(hr);


	hr = Add_PartObjects();
	CHECK_FAILED(hr);

	m_pTransformCom->Rotation(_float3{ 0.f, 1.f, 0.f }, ToRadian(180.f));
	m_pModelCom->Set_Animation(DEEANIM_WAIT, 60.f, true, true);

	m_pUI_Interactable = dynamic_cast<CUI_Interactable*>(m_pGameInstance->Add_CloneReturn(*m_pCurrentLevelID, L"Layer_UI", L"Prototype_GameObject_UI_Interactable"));
	m_pUI_Interactable->Set_Owner(this);
	m_pUI_Interactable->Set_Offset(2.f);

	return S_OK;
}

_int CFoodShopDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	__super::Tick(m_fTimeDelta);

	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(m_fTimeDelta);

	//공통된 디 관련 변수를 업데이트 - 초기화한다
	Dee_SystemTick(m_fTimeDelta);

	static _float fAccTime = 0.f;
	fAccTime += m_fTimeDelta;
	if (fAccTime >= 1.f)
	{
		bOpenEffect = false;
		fAccTime = 0.f;
	}

	if (m_bIsInteractKirby) //07.22) 상호작용 할 경우, 커비를 바라보게 세팅
	{
		CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Kirby")));
		CHECK_NULLPTR(pKirby);

		CTransform* pKirbyTrans = pKirby->Get_TransformCom();
		CTransform* pNPCDeeTrans = this->Get_TransformCom();

		pNPCDeeTrans->Look_At_Interpolate(pKirbyTrans->Get_State_Vector(CTransform::STATE_POSITION), m_fTimeDelta);
		
		pKirbyTrans->Look_At_Interpolate(pNPCDeeTrans->Get_State_Vector(CTransform::STATE_POSITION), m_fTimeDelta);
		pKirby->DialogOn(pKirbyTrans->Get_State_Float4(CTransform::STATE_LOOK));
	}


	m_bCheckCollision = false;
	return OBJ_NOEVENT;
}

void CFoodShopDee::Late_Tick(_float fTimeDelta)
{

	if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
		m_pModelCom->Play_Animation(m_fAccTime);

	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(m_fTimeDelta);

	// tick-collision_tick을 거쳐서 충돌처리가 안되었다고 판단되면 InteractableUI를 띄우지 않습니다.
	if (false == m_bCheckCollision)
		m_pUI_Interactable->Set_IsRender(false);

	//시야 벗어나면 컬링
	if (!m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 2.0f))
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CFoodShopDee::Render()
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

HRESULT CFoodShopDee::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG

void CFoodShopDee::Render_IMGUI()
{
	__super::Render_IMGUI();

	ImGui::Text(u8"현재 애님 인덱스 : %d", m_pFSM->Get_State());
}

#endif

void CFoodShopDee::Add_AnimEvent()
{
	__super::Add_AnimEvent();

}

void CFoodShopDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	m_bIsKirbyInZone = true;
	m_fResetHiTime = 5.f;

	//DEE NPC 상호작용 시, MessageWindow UI 출력
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_DOWN))
	{
		m_pUI_Interactable->Set_IsRender(false);
		m_bIsInteractKirby = TRUE;

		// 07.14) 크래시 버그 수정 (Layer 명확하게 검색)
		CUI_MessageWindow* pMWindow =  dynamic_cast<CUI_MessageWindow*>
			(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_UI_Msg_Parttimer_Dee")));
		CHECK_NULLPTR(pMWindow);
		pMWindow->Show_DialogMessage();

		CCamera_Main* pCameraMain = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main")));
		CHECK_NULLPTR(pCameraMain);
		pCameraMain->Lock_All({ 12.f, 30.f, 1.9f }, { 0.09f, -0.18f, 0.98f }, true);
	}

	// Interactable UI 처리
	m_pUI_Interactable->Set_IsRender(true);
	m_bCheckCollision = true;
}

HRESULT CFoodShopDee::Add_Components()
{
	HRESULT hr(S_OK);


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
	ControllerDesc.fOffset = 1.f;
	ControllerDesc.uCollisionType = m_eCollisionGroup;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &ControllerDesc);
	CHECK_FAILED(hr);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = NPC;
	hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	CHECK_FAILED(hr);
	Set_BodyCollider(COLLIDER_CYLINDER, 0.6f, 1.2f, 5.f);

	SetUp_FSM();

	return S_OK;
}

HRESULT CFoodShopDee::Add_PartObjects()
{
	if (*m_pCurrentLevelID != LEVEL_TOWN && *m_pCurrentLevelID != LEVEL_PARTTIME)
		return S_OK;

	CPartObject* pPartObj = { nullptr };
	CDee_Part::DEEPART_DESC	PartDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));

	PartDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	PartDesc.pSocket = pModel->Get_BonePtr("HatL");
	PartDesc.wstrModelName = TEXT("DeePart_FoodShop");

	pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
	if (nullptr == pPartObj)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pPartObj);

	return S_OK;
}

HRESULT CFoodShopDee::Bind_ShaderResources()
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

void CFoodShopDee::SetUp_FSM()
{
	m_pFSM = CFSM::Create();

	m_pFSM->Add_State(DEEANIM_LOOKAROUND, CDee_NPC_State::Create());

	m_pFSM->Add_State(DEEANIM_CLERKWAVEHAND, CDee_NPC_State::Create());
	m_pFSM->Add_State(DEEANIM_CLERKTALK, CDee_NPC_State::Create());

	m_pFSM->Add_State(DEEANIM_TALK1, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TALK2, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TALK3A, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TALK3B, CDee_Emotion_State::Create());

	m_pFSM->Add_State(DEEANIM_ANGER, CDee_Emotion_State::Create());



	CFSM::FSM_INFO	FSMDesc = {};
	FSMDesc.iState = DEEANIM_LOOKAROUND;
	FSMDesc.pModel = &m_pModelCom;

	m_pFSM->Initialize(&FSMDesc);

}

_bool CFoodShopDee::Custom_Face(_uint iMeshIndex)
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

CFoodShopDee* CFoodShopDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFoodShopDee* pInstance = new CFoodShopDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CFoodShopDee"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFoodShopDee::Clone(void* pArg)
{
	CFoodShopDee* pInstance = new CFoodShopDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CFoodShopDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFoodShopDee::Free()
{

	__super::Free();
}
