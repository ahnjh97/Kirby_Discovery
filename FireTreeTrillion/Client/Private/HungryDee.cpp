#include "stdafx.h"
#include "HitBox.h"
#include "FSM.h"
#include "HungryDee.h"
#include "Dee_Part.h"
#include "Dee_State.h"
#include "PartTimeHelper.h"

pair<_float3, vector<WAITING_INFO>> CHungryDee::m_WaitingList =
{
	_float3{0.f, 0.f, 0.f},
	{
		//앞으로 빠지는 자리
		WAITING_INFO{ {15.f, 2.f, 0.f}, 0.f},
		WAITING_INFO{ {14.f, 2.f, 0.f}, 0.f},

		//맨 앞자리
		WAITING_INFO{ {0.f, 2.f, 0.f}, 0.f},

		WAITING_INFO{ {-1.4f, 2.f, -2.f}, 0.1f},
		WAITING_INFO{ {-2.f, 2.f, -4.f}, 0.15f},
		WAITING_INFO{ {-3.5f, 2.f, -6.f}, 0.08f},
		WAITING_INFO{ {-5.f, 2.f, -8.f}, 0.1f},
		WAITING_INFO{ {-4.f, 2.f, -10.f}, 0.1f},
		WAITING_INFO{ {-2.5f, 2.f, -12.f}, 0.1f},
		WAITING_INFO{ {-3.f, 2.f, -14.f}, 0.1f},
		WAITING_INFO{ {-4.5f, 2.f, -16.f}, 0.1f},
		WAITING_INFO{ {-3.f, 2.f, -18.f}, 0.1f},
		WAITING_INFO{ {-1.4f, 2.f, -20.f}, 0.1f},
		WAITING_INFO{ {-2.f, 2.f, -22.f}, 0.15f},
		WAITING_INFO{ {-3.5f, 2.f, -24.f}, 0.08f},
		WAITING_INFO{ {-5.f, 2.f, -26.f}, 0.1f},
		WAITING_INFO{ {-4.f, 2.f, -28.f}, 0.1f},
		WAITING_INFO{ {-2.5f, 2.f, -30.f}, 0.1f},
		WAITING_INFO{ {-3.f, 2.f, -32.f}, 0.1f},
		WAITING_INFO{ {-4.5f, 2.f, -34.f}, 0.1f},
		WAITING_INFO{ {-3.f, 2.f, -36.f}, 0.1f},
		WAITING_INFO{ {-1.f, 2.f, -38.f}, 0.1f},
	}
};

_int CHungryDee::m_iWatingNum = { 0 };
_float CHungryDee::m_fWaitingTime = { 30.f };

CHungryDee::CHungryDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CWaddleDee{ pDevice, pContext }
{
}

CHungryDee::CHungryDee(const CHungryDee& rhs)
	:CWaddleDee{ rhs }
{
}

HRESULT CHungryDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHungryDee::Initialize(void* pArg)
{
	HUNGRYDEE_DESC pDeeDesc{};

	if (nullptr != pArg)
		pDeeDesc = *(HUNGRYDEE_DESC*)pArg;

	pDeeDesc.fSpeedPerSec = 2.f;
	pDeeDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	HRESULT hr;

	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	m_WaitingList.first = pDeeDesc.matWorld.Translation();

	m_iMyIdx = pDeeDesc.iIdx;
	_float4 vDir = Dir(m_WaitingList.second[m_iMyIdx].vPos);
	vDir += _float4{18.f, 0.f, -6.f, 0.f};
	m_pTransformCom->Move(vDir);
	++m_iWatingNum;

	hr = Add_Components();
	CHECK_FAILED(hr);

	//마을에 있을 때만 파트 오브젝트 로드하기
	hr = Add_PartObjects();
	CHECK_FAILED(hr);



	m_pModelCom->Set_Animation(DEESHOPANIM_GUESTNORMAL, CUtils::Make_RandomFloat(45.f, 60.f), true, true);

	return S_OK;
}

_int CHungryDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	//디버깅용
	if (m_iMyIdx == 2)
		m_fWaitingTime -= m_fTimeDelta;


	if (m_pGameInstance->Get_KeyState(DIK_S, KEY_DOWN) && m_iMyIdx == 0)
	{
		CHungryDee::HUNGRYDEE_DESC HungryDeeDesc{};
		HungryDeeDesc.fSpeedPerSec = 5.f;
		HungryDeeDesc.fRotationPerSec = ToRadian(90.f);
		_float4x4 InitMat = _float4x4::Identity;
		InitMat.Translation(m_WaitingList.first);
		HungryDeeDesc.matWorld = InitMat;

		_int iStartIdx = m_iWatingNum;

		for (_int i = 0; i < 10; ++i)
		{
			HungryDeeDesc.iIdx = iStartIdx + i;
			m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_NPC"), TEXT("Prototype_GameObject_HungryDee"), &HungryDeeDesc);
		}
	}

	if (m_pGameInstance->Get_KeyState(DIK_W, KEY_DOWN))
	{
		if (m_iMyIdx == 2)
			CUtils::Make_RandomInt(0, 1) == 1 ? Bring_Food(PARTTIME_ITEM::DRINK) : Bring_Food(PARTTIME_ITEM::ITEM_END);

		m_iMyIdx = (m_iMyIdx + m_iWatingNum - 1) % m_iWatingNum;

		if (m_iMyIdx == m_iWatingNum - 1)
		{
			_float3 vDestPos = m_WaitingList.first + m_WaitingList.second[m_iMyIdx].vPos + _float3{ -10.f, 0.f, 0.f };
			m_pControllerCom->Set_Position(m_pTransformCom, Pos(vDestPos));
			Set_DeeEyeState(DEEEYE_IDLE);

			for (auto& partObj : m_PartObjects)
				Safe_Release(partObj.second);
			m_PartObjects.clear();
			Set_RenderPartObj(false);

			Change_State((DEE_ANIM)DEESHOPANIM_WALK, 60.f, true, true);
		}
	}


	//나머지 슈퍼틱, 파트 틱 처리
	__super::Tick(m_fTimeDelta);
	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(m_fTimeDelta);

	//공통된 디 관련 변수를 업데이트 - 초기화한다
	Dee_SystemTick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CHungryDee::Late_Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	m_pModelCom->Play_Animation(m_fTimeDelta);

	//시야 벗어나면 컬링
	if (!m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 2.0f))
		return;


	if (m_bRenderPartObj)
	{
		for (auto& Pair : m_PartObjects)
			Pair.second->Late_Tick(m_fTimeDelta);
	}


	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CHungryDee::Render()
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

HRESULT CHungryDee::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CHungryDee::Add_AnimEvent()
{
	__super::Add_AnimEvent();
}

void CHungryDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CHungryDee::Render_IMGUI()
{
	__super::Render_IMGUI();

	ImGui::Text(u8"현재 애님 인덱스 : %d", m_pFSM->Get_State());

	ImGui::Spacing();
	ImGui::Text(u8"대기 인덱스: %d", m_iMyIdx);
	_float4x4 WorldMat = m_pTransformCom->Get_WorldMatrix();
	ImGui::Text(u8"위치: %.2f\t%.2f\t%.2f\t%.2f", WorldMat._41, WorldMat._42, WorldMat._43, WorldMat._44);
}

void CHungryDee::Bring_Food(PARTTIME_ITEM eITEM)
{
	if (eITEM == PARTTIME_ITEM::ITEM_END)
	{
		Change_State((DEE_ANIM)DEESHOPANIM_INCORRECT, 60.f, false, true);
		Set_DeeEyeState(DEEEYE_ANGER);
		return;
	}

	CPartObject* pPartObj = { nullptr };
	CDee_Part::DEEPART_DESC	PartDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));

	PartDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	PartDesc.pSocket = pModel->Get_BonePtr("RHaveL");


	switch (eITEM)
	{
	case PARTTIME_ITEM::CAKE:
		PartDesc.wstrModelName = TEXT("Item_EnergyDrink");
		break;
	case PARTTIME_ITEM::BURGER:
		PartDesc.wstrModelName = TEXT("Item_EnergyDrink");
		break;
	case PARTTIME_ITEM::TOMATO:
		PartDesc.wstrModelName = TEXT("Item_EnergyDrink");
		break;
	case PARTTIME_ITEM::DRINK:
		PartDesc.wstrModelName = TEXT("Item_EnergyDrink");
		break;
	default:
		PartDesc.wstrModelName = TEXT("Item_EnergyDrink");
		break;
	}

	pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
	if (nullptr == pPartObj)
		return;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pPartObj);
	Change_State((DEE_ANIM)DEESHOPANIM_CLERKCORRECT, 60.f, false, true);
	Set_DeeEyeState(DEEEYE_SMILE);
}

HRESULT CHungryDee::Add_Components()
{

	HRESULT hr;


	//쉐이더
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	//모델
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_WaddleDeeHungry"),
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


	//CHitBox::HITBOX_DESC HitBox{};
	//HitBox.pOwner = this;
	//HitBox.pDesc = &m_tColliderDesc[BODY];
	//HitBox.pCollisionType = NPC;
	//hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	//CHECK_FAILED(hr);

	//Set_BodyCollider(COLLIDER_CYLINDER, 0.6f, 1.2f, 1.2f);

	SetUp_FSM();

	return S_OK;
}

HRESULT CHungryDee::Add_PartObjects()
{
	if (*m_pCurrentLevelID != LEVEL_TOWN)
		return S_OK;

	CPartObject* pPartObj = { nullptr };
	CDee_Part::DEEPART_DESC	PartDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));

	PartDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	PartDesc.pSocket = pModel->Get_BonePtr("HatL");


	switch (CUtils::Make_RandomInt(0, 4))
	{
	case 0:
		PartDesc.wstrModelName = TEXT("DeePart_Delivery");
		break;
	case 1:
		PartDesc.wstrModelName = TEXT("DeePart_Pharmacy");
		break;
	case 2:
		PartDesc.wstrModelName = TEXT("DeePart_RollingBall");
		break;
	case 3:
		PartDesc.wstrModelName = TEXT("DeePart_Theater");
		break;
	case 4:
		PartDesc.wstrModelName = TEXT("DeePart_Knowledge");
		break;
	}

	pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
	if (nullptr == pPartObj)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pPartObj);

	return S_OK;
}

HRESULT CHungryDee::Bind_ShaderResources()
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

void CHungryDee::SetUp_FSM()
{
	m_pFSM = CFSM::Create();

	//그냥 서서 하는 스테이트
	m_pFSM->Add_State(DEESHOPANIM_GUESTNORMAL, CDee_Hungry_State::Create());
	m_pFSM->Add_State(DEESHOPANIM_GUESTANGER, CDee_Hungry_State::Create());
	m_pFSM->Add_State(DEESHOPANIM_GUESTFIDGET, CDee_Hungry_State::Create());
	m_pFSM->Add_State(DEESHOPANIM_ORDERNORMAL, CDee_Hungry_State::Create());


	m_pFSM->Add_State(DEESHOPANIM_WALK, CDee_Hungry_State::Create());

	m_pFSM->Add_State(DEESHOPANIM_CLERKCORRECT, CDee_Hungry_State::Create());
	m_pFSM->Add_State(DEESHOPANIM_INCORRECT, CDee_Hungry_State::Create());

	m_pFSM->Add_State(DEESHOPANIM_CORRECTMOVE, CDee_Hungry_State::Create());
	m_pFSM->Add_State(DEESHOPANIM_INCORRECTMOVE, CDee_Hungry_State::Create());

	CFSM::FSM_INFO	FSMDesc = {};
	FSMDesc.iState = DEESHOPANIM_GUESTNORMAL;
	FSMDesc.pModel = &m_pModelCom;

	m_pFSM->Initialize(&FSMDesc);

}

_bool CHungryDee::Custom_Face(_uint iMeshIndex)
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

CHungryDee* CHungryDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHungryDee* pInstance = new CHungryDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CHungryDee"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHungryDee::Clone(void* pArg)
{
	CHungryDee* pInstance = new CHungryDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CHungryDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHungryDee::Free()
{
	Safe_Release(m_pEyeTextureCom);

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);

	m_PartObjects.clear();

	__super::Free();
}
