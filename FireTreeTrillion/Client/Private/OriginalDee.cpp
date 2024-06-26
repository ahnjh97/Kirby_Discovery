#include "stdafx.h"
#include "HitBox.h"
#include "FSM.h"
#include "OriginalDee.h"
#include "Dee_Part.h"
#include "Dee_State.h"

pair<_float3, vector<TOWN_POINT_INFO>> COriginalDee::m_TownPoints =
{
	//잘못했슴다 이 부분을 이동 오프셋으로 주겠음...
	_float3{1.7f, 22.f, 4.f},
	{
		TOWN_POINT_INFO
		{
			TOWNPOINT_0,
			{0.f, 0.f, 10.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_1, TOWNPOINT_7, TOWNPOINT_8}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_1,
			{1.6f - 1.7f, 0.f, 18.3f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_0, TOWNPOINT_2, TOWNPOINT_12}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_2,
			{-8.8f - 1.7f, 0.f, 19.f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_1, TOWNPOINT_3, TOWNPOINT_FLOWERUPLEFT}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_3,
			{-11.9f - 1.7f, 0.f, 7.6f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_2, TOWNPOINT_4, TOWNPOINT_5, TOWNPOINT_FLOWERDOWNLEFT}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_4,
			{-17.2f - 1.7f, 0.f, 2.f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_3}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_5,
			{-10.1f - 1.7f, 0.f, -8.f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_3, TOWNPOINT_6, TOWNPOINT_FLOWERDOWNLEFT}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_6,
			{2.8f - 1.7f, 0.f, -12.f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_5, TOWNPOINT_7, TOWNPOINT_8, TOWNPOINT_9}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_7,
			{-1.2f - 1.7f, 0.f, 1.6f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_0, TOWNPOINT_6, TOWNPOINT_8}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_8,
			{5.7f - 1.7f, 0.f, 2.8f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_0, TOWNPOINT_6, TOWNPOINT_7}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_9,
			{15.f - 1.7f, 0.f, -10.5f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_6, TOWNPOINT_10, TOWNPOINT_14}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_10,
			{20.7f - 1.7f, 0.f, 1.2f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_9, TOWNPOINT_11, TOWNPOINT_FLOWERDOWNRIGHT}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_11,
			{20.1f - 1.7f, 0.f, 16.1f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_10, TOWNPOINT_12, TOWNPOINT_1FLOOR, TOWNPOINT_FLOWERDOWNRIGHT}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_12,
			{12.7f - 1.7f, 0.f, 19.6f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_1, TOWNPOINT_11, TOWNPOINT_FOODSHOP, TOWNPOINT_FLOWERUPRIGHT}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_FOODSHOP,
			{13.8f - 1.7f, 0.f, 24.1f - 4.f},
			{DEEANIM_CHOOSE_START},
			{TOWNPOINT_12}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_14,
			{33.f - 1.7f, 0.f, -3.f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_9, TOWNPOINT_FISHING}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_FISHING,
			{44.f - 1.7f, 0.f, 8.5f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_14, TOWNPOINT_16}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_16,
			{41.4f - 1.7f, 0.f, 17.5f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_FISHING, TOWNPOINT_1FLOOR}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_1FLOOR,
			{31.8f - 1.7f, 0.f, 22.8f - 4.f},
			{DEEANIM_WALK, DEEANIM_MOVEFALL},
			{TOWNPOINT_11, TOWNPOINT_16, TOWNPOINT_UNDERSTAIRA, TOWNPOINT_UNDERSTAIRB}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_UNDERSTAIRA,
			{35.4f - 1.7f, 0.f, 37.2f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_1FLOOR, TOWNPOINT_MIDSTAIRA}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_MIDSTAIRA,
			{42.f - 1.7f, 0.f, 53.5f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_UNDERSTAIRA, TOWNPOINT_UPSTAIRA}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_UPSTAIRA,
			{45.7f - 1.7f, 0.f, 70.9f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_MIDSTAIRA, TOWNPOINT_2FLOOR}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_2FLOOR,
			{48.2f - 1.7f, 0.f, 76.8f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_UPSTAIRA,TOWNPOINT_UPSTAIRB }
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_UPSTAIRB,
			{50.8f - 1.7f, 0.f, 71.f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_2FLOOR, TOWNPOINT_MIDSTAIRB}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_MIDSTAIRB,
			{47.5f - 1.7f, 0.f, 51.f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_UPSTAIRB, TOWNPOINT_UNDERSTAIRB}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_UNDERSTAIRB,
			{40.7f - 1.7f, 0.f, 34.2f - 4.f},
			{DEEANIM_WALK},
			{TOWNPOINT_MIDSTAIRB, TOWNPOINT_1FLOOR}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_FLOWERUPRIGHT,
			{13.4f - 1.7f, 0.f, 12.f - 4.f},
			{DEEANIM_WATERING},
			{TOWNPOINT_12}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_FLOWERDOWNRIGHT,
			{14.9f - 1.7f, 0.f, -1.f - 4.f},
			{DEEANIM_WATERING},
			{TOWNPOINT_10, TOWNPOINT_11}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_FLOWERUPLEFT,
			{-10.f - 1.7f, 0.f, 10.6f - 4.f},
			{DEEANIM_WATERING},
			{TOWNPOINT_2}
		},
		TOWN_POINT_INFO
		{
			TOWNPOINT_FLOWERDOWNLEFT,
			{-10.f - 1.7f, 0.f, -1.f - 4.f},
			{DEEANIM_WATERING},
			{TOWNPOINT_3, TOWNPOINT_5}
		},
	}
};


COriginalDee::COriginalDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CWaddleDee{ pDevice, pContext }
{
}

COriginalDee::COriginalDee(const COriginalDee& rhs)
	:CWaddleDee{ rhs }
{
}

_float3 COriginalDee::Make_DestPos()
{
	return m_TownPoints.first + m_TownPoints.second[m_eDestPoint].vPosOffset;
}

pair<DEE_ANIM, _bool> COriginalDee::Make_WhatToDo()
{
	_int iStateSize = m_TownPoints.second[m_eDestPoint].StateOffset.size();
	DEE_ANIM eDeeState = m_TownPoints.second[m_eDestPoint].StateOffset[CUtils::Make_RandomInt(0, iStateSize - 1)];

	_int iPointSize = m_TownPoints.second[m_eDestPoint].NearPoint.size();

	if(m_ePrePoint != TOWNPOINT_END)
		m_TownPoints.second[m_ePrePoint].bIsUsing = false;

	TOWN_POINT eArrivedPoint = m_eDestPoint;
	TOWN_POINT eDestPoint = TOWNPOINT_END;

	vector<TOWN_POINT> PointList = m_TownPoints.second[m_eDestPoint].NearPoint;

	//갔던 곳 빼고 다시 만들기
	while (true)
	{
		_int iDestIndex = CUtils::Make_RandomInt(0, PointList.size() - 1);

		eDestPoint = PointList[iDestIndex];

		if (PointList.size() == 1)
			break;


		//누가 그 곳을 목적지로 한다면! 가지마.
		if (m_TownPoints.second[eDestPoint].bIsUsing)
		{
			auto iter = PointList.begin() + iDestIndex;
			PointList.erase(iter);
			continue;
		}

		//내가 갔던 곳과 동일하다면! 가지마.
		if (m_ePrePoint == eDestPoint)
		{
			auto iter = PointList.begin() + iDestIndex;
			PointList.erase(iter);
			continue;
		}

		break;
	}

	m_eDestPoint = eDestPoint;
	m_TownPoints.second[eArrivedPoint].bIsUsing = true;
	m_ePrePoint = eArrivedPoint;

	return { eDeeState, (eDeeState == DEEANIM_WALK) };
}

HRESULT COriginalDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT COriginalDee::Initialize(void* pArg)
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

	hr = Add_PartObjects(pDeeDesc.eCharacter);
	CHECK_FAILED(hr);


	return S_OK;
}

_int COriginalDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	__super::Tick(m_fTimeDelta);

	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(m_fTimeDelta);

	//공통된 디 관련 변수를 업데이트 - 초기화한다
	Dee_SystemTick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void COriginalDee::Late_Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	m_pModelCom->Play_Animation(m_fTimeDelta);

	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(m_fTimeDelta);


	//시야 벗어나면 컬링
	if (!m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 2.0f))
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT COriginalDee::Render()
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

HRESULT COriginalDee::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void COriginalDee::Add_AnimEvent()
{
	__super::Add_AnimEvent();
}

void COriginalDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	m_bIsKirbyInZone = true;
	m_fResetHiTime = 5.f;
}

void COriginalDee::Render_IMGUI()
{
	__super::Render_IMGUI();

	ImGui::Text(u8"현재 애님 인덱스 : %d", m_pFSM->Get_State());

	//Draw_TownPoints();

}

HRESULT COriginalDee::Add_Components()
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

	Set_BodyCollider(COLLIDER_CYLINDER, 0.6f, 1.2f, 1.2f);

	SetUp_FSM();

	return S_OK;
}

HRESULT COriginalDee::Add_PartObjects(DEE_CHARACTER eCharacter)
{

	CPartObject* pPartObj = { nullptr };
	CDee_Part::DEEPART_DESC	PartDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));
	PartDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	PartDesc.pSocket = pModel->Get_BonePtr("RHaveL");
	PartDesc.pCurAnimState = &m_eMyState;
	
	if (eCharacter == DEECHARACTER_FRONTMAN)
	{
		PartDesc.wstrModelName = TEXT("DeePart_Guitar");

		pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
		if (nullptr == pPartObj)
			return E_FAIL;

		m_PartObjects.emplace(TEXT("Part_Instrument"), pPartObj);
	}
	else if (eCharacter == DEECHARACTER_GUITAR)
	{
		PartDesc.wstrModelName = TEXT("DeePart_Bass");

		pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
		if (nullptr == pPartObj)
			return E_FAIL;

		m_PartObjects.emplace(TEXT("Part_Instrument"), pPartObj);
	}
	else if (eCharacter == DEECHARACTER_DRUM)
	{
		PartDesc.wstrModelName = TEXT("DeePart_DrumStick");

		pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
		if (nullptr == pPartObj)
			return E_FAIL;

		m_PartObjects.emplace(TEXT("Part_InstrumentR"), pPartObj);

		PartDesc.pSocket = pModel->Get_BonePtr("RHaveL");

		pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
		if (nullptr == pPartObj)
			return E_FAIL;
		m_PartObjects.emplace(TEXT("Part_InstrumentL"), pPartObj);

	}
	else
	{
		PartDesc.wstrModelName = TEXT("DeePart_WateringCan");

		pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
		if (nullptr == pPartObj)
			return E_FAIL;

		m_PartObjects.emplace(TEXT("Part_Instrument"), pPartObj);

	}
	/*if (*m_pCurrentLevelID != LEVEL_TOWN && *m_pCurrentLevelID != LEVEL_PARTTIME)
		return S_OK;

	CPartObject* pPartObj = { nullptr };
	CDee_Part::DEEPART_DESC	PartDesc{};

	CModel* pModel = (CModel*)Get_Component(TEXT("Com_Model"));

	PartDesc.wstrModelName = TEXT("DeePart_FoodShop");

	pPartObj = static_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_DeePart"), &PartDesc));
	if (nullptr == pPartObj)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pPartObj);*/

	return S_OK;
}

HRESULT COriginalDee::Bind_ShaderResources()
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

void COriginalDee::SetUp_FSM()
{
	m_pFSM = CFSM::Create();

	m_pFSM->Add_State(DEEANIM_WAIT, CDee_Idle_State::Create());
	m_pFSM->Add_State(DEEANIM_TOWNWAIT, CDee_Idle_State::Create());

	m_pFSM->Add_State(DEEANIM_WALK, CDee_Walk_State::Create());
	m_pFSM->Add_State(DEEANIM_ENEMYWALK, CDee_Walk_State::Create());

	m_pFSM->Add_State(DEEANIM_SITWAIT, CDee_Sit_State::Create());
	m_pFSM->Add_State(DEEANIM_SITTALKA, CDee_Emotion_State::Create());


	m_pFSM->Add_State(DEEANIM_TALK1, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TALK2, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TALK3A, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TALK3B, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_ANGER, CDee_Emotion_State::Create());
	m_pFSM->Add_State(DEEANIM_TROUBLE, CDee_Emotion_State::Create());


	m_pFSM->Add_State(DEEANIM_SITSLEEP, CDee_Sleep_State::Create());

	m_pFSM->Add_State(DEEANIM_WATERING, CDee_Interact_State::Create());
	m_pFSM->Add_State(DEEANIM_MOVEFALL, CDee_Interact_State::Create());
	m_pFSM->Add_State(DEEANIM_LOOKAROUND, CDee_Interact_State::Create());

	m_pFSM->Add_State(DEEANIM_CHOOSE_START, CDee_Interact_State::Create());
	m_pFSM->Add_State(DEEANIM_CHOOSE_WAIT, CDee_Interact_State::Create());


}

void COriginalDee::Make_InitialState(DEE_CHARACTER eCharacter)
{
	DEE_ANIM eAnim = DEEANIM_SITSLEEP;


	switch (eCharacter)
	{
	case DEECHARACTER_IDLE:
		eAnim = DEEANIM_WAIT;
		break;
	case DEECHARACTER_WALK:
		eAnim = DEEANIM_WALK;
		break;
	case DEECHARACTER_SIT:
		eAnim = DEEANIM_SITWAIT;
		break;
	case DEECHARACTER_SITTALK:
		eAnim = DEEANIM_SITTALKA;
		break;
	case DEECHARACTER_TROUBLE:
		eAnim = DEEANIM_TROUBLE;
		break;
	case DEECHARACTER_ANGRY:
		eAnim = DEEANIM_ANGER;
		break;
	case DEECHARACTER_SLEEPY:
		eAnim = DEEANIM_SITSLEEP;
		break;
	default:
		eAnim = DEEANIM_WAIT;
		break;
	}


	CFSM::FSM_INFO	FSMDesc = {};
	FSMDesc.iState = eAnim;
	FSMDesc.pModel = &m_pModelCom;

	m_pFSM->Initialize(&FSMDesc);

	m_pModelCom->Set_Animation(eAnim, 60.f, true, true);


	//만약 걷는 와들디였다면?!
	if (eAnim == DEEANIM_WALK)
	{
		TOWN_POINT	iMinPoint;
		_float		fMinDist{999.f};

		_float3 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		for (auto& tPoint : m_TownPoints.second)
		{
			_float3 vTargetPos = m_TownPoints.first + tPoint.vPosOffset;
			_float fDist = (XZVec(vPos) - XZVec(vTargetPos)).Length();
			if (fDist < fMinDist)
			{
				iMinPoint = tPoint.MyPoint;
				fMinDist = fDist;
			}
		}

		m_eDestPoint = iMinPoint;
	}
	else if (eAnim == DEEANIM_TROUBLE)
	{
		Set_DeeEyeState(DEEEYE_SMILE);
	}

}

_bool COriginalDee::Custom_Face(_uint iMeshIndex)
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

void COriginalDee::Draw_TownPoints()
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	for (const auto& point : m_TownPoints.second)
	{
		_float3 vWorldPos = m_TownPoints.first + point.vPosOffset;
		CUtils::Make_World_ToScreen(vWorldPos);
		ImVec2 vMyPos = { vWorldPos.x, vWorldPos.y };
		drawList->AddCircleFilled(vMyPos, 5.0f, IM_COL32(255, 0, 0, 255));

		// 연결된 점들 사이에 선 그리기
		for (const auto& conn : point.NearPoint)
		{
			_float3 vTargetWorldPos = m_TownPoints.first + m_TownPoints.second[conn].vPosOffset;
			CUtils::Make_World_ToScreen(vTargetWorldPos);
			ImVec2 vTargetPos = { vTargetWorldPos.x, vTargetWorldPos.y };

			drawList->AddLine(vMyPos, vTargetPos, IM_COL32(255, 255, 255, 255), 2.0f);
		}
	}
}

COriginalDee* COriginalDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COriginalDee* pInstance = new COriginalDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : COriginalDee"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* COriginalDee::Clone(void* pArg)
{
	COriginalDee* pInstance = new COriginalDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : COriginalDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COriginalDee::Free()
{
	Safe_Release(m_pEyeTextureCom);

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);

	m_PartObjects.clear();

	__super::Free();
}
