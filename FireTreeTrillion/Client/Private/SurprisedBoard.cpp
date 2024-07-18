#include "stdafx.h"
#include "SurprisedBoard.h"

#include "FSM.h"
#include "SurprisedBoard_State.h"

#include "MultiEffect.h"
#include "HitBox.h"
#include "Kirby.h"
#include "Gm_DynamicField.h"

CSurprisedBoard::CSurprisedBoard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMonster{ pDevice, pContext }
{
}

CSurprisedBoard::CSurprisedBoard(const CSurprisedBoard& rhs)
	: CMonster(rhs)
	, m_arrModelCom(rhs.m_arrModelCom)
	, m_arrNonModelCom(rhs.m_arrNonModelCom)
{
}

HRESULT CSurprisedBoard::Initialize_Prototype()
{
	m_eCollisionGroup = TRIGGER_FOR_NOT_PHYSX;

	fill(m_arrModelCom.begin(), m_arrModelCom.end(), nullptr);
	fill(m_arrNonModelCom.begin(), m_arrNonModelCom.end(), nullptr);

	return S_OK;
}

HRESULT CSurprisedBoard::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	SURPRISED_DESC tDesc = *(SURPRISED_DESC*)pArg;
	m_eModelColor = tDesc.eColor;
	if (FAILED(Add_Components()))
		return E_FAIL;
	
	if (tDesc.vPosition != _float3())
	{
		_float3 vPos = tDesc.vPosition;	//15.f, 5.f, -159.f
		Add_HitBoxes(vPos);
	}

	/* FSM */
	SetUp_FSM(tDesc.eStartState);
	m_arrModelCom[m_eModelColor]->Set_Animation(tDesc.eStartState, 45.f, true, true);
	
	// m_matWorld 초기화
	m_matWorld = m_pTransformCom->Get_WorldFloat4x4();
	m_fTimeDelta = 1.f;
	if (tDesc.eStartState == WAIT_L)
		Go_Left_Rigid();
	else if (tDesc.eStartState == WAIT_R)
		Go_Right_Rigid();

	m_fAttack = 8.f;
	Add_AnimEvent();

	return S_OK;
}

_int CSurprisedBoard::Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	//__super::Tick(m_fTimeDelta);
	if (*m_pCurrentLevelID != LEVEL_TOOL_ANIM)
	{
		// FSM 제어
		if (m_pFSM != nullptr)
			m_pFSM->Update(this, fTimeDelta);
	}

	if (false == m_bNotified && nullptr != m_pDynamicField)
	{
		_uint iAnimIndex = Get_State();
		if (POP_OUT_L == iAnimIndex || POP_OUT_R == iAnimIndex)
		{
			if (0.45f < m_arrModelCom[m_eModelColor]->Get_AnimRatio())
			{
				m_bNotified = true;
				m_bReturnMove = TRUE;
				CGm_DynamicField* pDynamicField = dynamic_cast<CGm_DynamicField*>(m_pDynamicField);
				pDynamicField->Set_Interaction(true);
			}
		}
		
	}

	if (m_bReturnMove)
	{
		_uint iAnimIndex = Get_State();
		if (RETURN_L == iAnimIndex || RETURN_R == iAnimIndex)
		{
			m_bReturnMove = FALSE;
			CGm_DynamicField* pDynamicField = dynamic_cast<CGm_DynamicField*>(m_pDynamicField);
			pDynamicField->Set_ReturnMove(TRUE);
		}
	}
	return OBJ_NOEVENT;
}

void CSurprisedBoard::Late_Tick(_float fTimeDelta)
{
	m_arrModelCom[m_eModelColor]->Play_Animation(m_fTimeDelta);

	m_pRigidBodyCom->Update(m_matWorld);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CSurprisedBoard::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_arrModelCom[m_eModelColor]->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_arrModelCom[m_eModelColor]->Bind_ShaderResource(m_pShaderCom,	"g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_arrModelCom[m_eModelColor]->Bind_ShaderResource(m_pShaderCom,	"g_NormalTexture",	i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_arrModelCom[m_eModelColor]->Bind_ShaderResource(m_pShaderCom,	"g_MRATexture",		i, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_arrModelCom[m_eModelColor]->Bind_ShaderResource(m_pShaderCom, "g_EmissiveTexture", i, TextureType_EMISSIVE)))
			return E_FAIL;
		if (FAILED(m_arrModelCom[m_eModelColor]->Bind_BoneMatrices(m_pShaderCom,	"g_BoneMatrices",	i)))
			return E_FAIL;

		m_pShaderCom->Begin(ANIMMODEL_EMISSIVE);
		m_arrModelCom[m_eModelColor]->Render(i);
	}

	return S_OK;
}

HRESULT CSurprisedBoard::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_arrModelCom[m_eModelColor])))
		return E_FAIL;

	return S_OK;
}

void CSurprisedBoard::Add_AnimEvent()
{
	__super::Add_AnimEvent();
	
	// 1. 한 애니메이션에서 같은 이름의 이벤트 가능
	// 2. 재생 기준은 애님툴에서 지정한 애니메이션인지 + 시작 프레임이 애니메이션 프레임안에 들어가는 지
	// 3. 두번째 인자로 넣어준 람다가 시작 프레임 한번만 실행된다.
	m_arrModelCom[m_eModelColor]->Add_Event("Bboong", [this]() {
		
		});

	m_arrModelCom[m_eModelColor]->Add_Event("PlaySound", [this]() {
		// 사운드 처리
		//m_pGameInstance->PlaySound_Free(L"TakeItem01.wav", 0.5f);
		});

	m_arrModelCom[m_eModelColor]->Add_Event("ApplyDamage", [this]() {
		//데미지 처리
		});
}

#ifdef _DEBUG
void CSurprisedBoard::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	//ImGui::Text("RePress : %d", m_bRePressBlock);
	//ImGui::Text("Land : %d", INFO(m_isLanding));

	//ImGui::Text("JUMP : %d", INFO(m_isJump));
	//ImGui::Text("Velocity : %.2f", INFO(m_fJumpVelocity));
	//ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	//ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();

	//	ImGui::Text("MoveDir X : %.2f \tMoveDir Y : %.2f \tMoveDir Z : %.2f ", INFO(m_vMoveDir).x, INFO(m_vMoveDir).y, INFO(m_vMoveDir).z); ImGui::NewLine();
	//	ImGui::Text("TargetDir X : %.2f \tTargetDir Y : %.2f \tTargetDir Z : %.2f ", INFO(m_vTargetDir).x, INFO(m_vTargetDir).y, INFO(m_vTargetDir).z);
	__super::Render_IMGUI();
}
#endif

void CSurprisedBoard::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (true == m_bActivated)
		return;

	m_bActivated = true;

	if (eContent == CCollisionCenter::CONTENT_TRIGGER)
	{
		_uint uState = __super::Get_State();
		if (uState == WAIT_L)
			Change_State(CSurprisedBoard::PREPOP_OUT_L, 60.f, false, false);
			
		else if (uState == WAIT_R)
			Change_State(CSurprisedBoard::PREPOP_OUT_R, 60.f, false, false);
	}
}

void CSurprisedBoard::Change_State(ANIM_STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CSurprisedBoard::IsAnimFinished()
{
	return m_arrModelCom[m_eModelColor]->IsFinished();
}

_bool CSurprisedBoard::IsAnimFinished(_uint iCurrentAnimIndex)
{
	return m_arrModelCom[m_eModelColor]->IsFinished(iCurrentAnimIndex);
}

void CSurprisedBoard::Go_Left_Rigid(_float fOffset)
{
	_vector		vPosition = m_matWorld.Translation();
	_vector		vRight = m_matWorld.Right();

	_float fSpeed = m_pTransformCom->Get_SpeedPerSec();
	vPosition -= XMVector3Normalize(vRight) * fSpeed * m_fTimeDelta * fOffset;

	memcpy(&m_matWorld.m[3], &vPosition, sizeof(_float4));
}

void CSurprisedBoard::Go_Right_Rigid(_float fOffset)
{
	_vector		vPosition = m_matWorld.Translation();
	_vector		vRight	  = m_matWorld.Right();
	
	_float fSpeed = m_pTransformCom->Get_SpeedPerSec();
	vPosition += XMVector3Normalize(vRight) * fSpeed * m_fTimeDelta * fOffset;
	
	memcpy(&m_matWorld.m[3], &vPosition, sizeof(_float4));
}

void CSurprisedBoard::RegisterSurpriseBoardAndDynamicField(CGameObject* pDynamicField)
{
	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(LEVEL_PARK, TEXT("Layer_Player")));
	if (nullptr == pKirby || nullptr == pDynamicField)
		return;

	pKirby->RegisterActorToPlayer_ForSurpriseBoard(m_pRigidBodyCom->Get_RigidDynamic(), this, pDynamicField);
	m_pDynamicField = pDynamicField;
	Safe_AddRef(m_pDynamicField);
}

HRESULT CSurprisedBoard::Add_Components()
{
	HRESULT hr(S_OK);

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);
	
	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SurprisedBoardRed"),
								TEXT("Com_Model_RED"), (CComponent**)&m_arrModelCom[RED]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SurprisedBoardGreen"),
								TEXT("Com_Model_GREEN"), (CComponent**)&m_arrModelCom[GREEN]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_SurprisedBoardBlue"),
								TEXT("Com_Model_BLUE"), (CComponent**)&m_arrModelCom[BLUE]);
	CHECK_FAILED(hr);

	/* For.Com_NonAnim_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_NonAnim_SurprisedBoardRed"),
								TEXT("Com_NonAnimModel_RED"), (CComponent**)&m_arrNonModelCom[RED]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_NonAnim_SurprisedBoardGreen"),
								TEXT("Com_NonAnimModel_GREEN"), (CComponent**)&m_arrNonModelCom[GREEN]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_NonAnim_SurprisedBoardBlue"),
								TEXT("Com_NonAnimModel_BLUE"), (CComponent**)&m_arrNonModelCom[BLUE]);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_arrModelCom[m_eModelColor];

	/* For.Com_RigidBody */
	CRigidBody::RIGIDBODY_DESC rigidDesc{};
	rigidDesc.bTrigger = false;
	rigidDesc.bDynamic = true;
	rigidDesc.bKinematic = true;
	rigidDesc.eShapeType = RIGID_BOX;
	rigidDesc.fOffsetSize = { 5.f, 10.f, 0.5f };
	rigidDesc.vMaterial = _float3(10.f, 1.f, 0.85f);
	rigidDesc.fDensity = 800.f;
	rigidDesc.matWorld = m_matWorld;
	hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
								TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
	CHECK_FAILED(hr);
	m_pRigidBodyCom->Activate(true);


	return S_OK;
}

HRESULT CSurprisedBoard::Add_HitBoxes(_float3 vPos)
{
	HRESULT hr(S_OK);

	/* For.HitBox */
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = TRIGGER_FOR_NOT_PHYSX;
	HitBox.matObjectPosition = XMMatrixTranslation(vPos.x , vPos.y, vPos.z);
	hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	CHECK_FAILED(hr);
	Set_BodyCollider(COLLIDER_CYLINDER, 0.f, 4.f, 5.f);

	return S_OK;
}

HRESULT CSurprisedBoard::Bind_ShaderResources()
{
	CHECK_NULLPTR(m_pShaderCom);

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

void CSurprisedBoard::SetUp_FSM(ANIM_STATE eStartState)
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(ARM_MOVE_L,	CSurprisedBoard_Popout_State::Create());
	m_pFSM->Add_State(ARM_MOVE_R,	CSurprisedBoard_Popout_State::Create());
	m_pFSM->Add_State(POP_OUT_L,	CSurprisedBoard_Popout_State::Create());
	m_pFSM->Add_State(POP_OUT_R,	CSurprisedBoard_Popout_State::Create());
	m_pFSM->Add_State(PREPOP_OUT_L, CSurprisedBoard_Popout_State::Create());
	m_pFSM->Add_State(PREPOP_OUT_R, CSurprisedBoard_Popout_State::Create());

	m_pFSM->Add_State(RETURN_L,		CSurprisedBoard_Return_State::Create());
	m_pFSM->Add_State(RETURN_R,		CSurprisedBoard_Return_State::Create());

	m_pFSM->Add_State(WAIT_L,		CSurprisedBoard_Wait_State::Create());
	m_pFSM->Add_State(WAIT_R,		CSurprisedBoard_Wait_State::Create());

	// 상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = eStartState;
	FSM_Desc.pModel = &m_arrModelCom[m_eModelColor];
	m_pFSM->Initialize(&FSM_Desc);
}

CSurprisedBoard* CSurprisedBoard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSurprisedBoard* pInstance = new CSurprisedBoard(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSurprisedBoard"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSurprisedBoard::Clone(void* pArg)
{
	CSurprisedBoard* pInstance = new CSurprisedBoard(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSurprisedBoard"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSurprisedBoard::Free()
{
	__super::Free();
	//m_pGameInstance->ReleaseActor(m_pDynamicActor);

	Safe_Release(m_pDynamicField);
	Safe_Release(m_pRigidBodyCom);

	for (auto& pModel : m_arrModelCom)
		Safe_Release(pModel);
	for (auto& pNonAnimModel : m_arrNonModelCom)
		Safe_Release(pNonAnimModel);
}

