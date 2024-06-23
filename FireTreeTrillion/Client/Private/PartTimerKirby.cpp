#include "stdafx.h"
#include "PartTimerKirby.h"
#include "FSM.h"
#include "Camera.h"

#include "PartTimerKirby_State.h"
#include "HitBox.h"

CPartTimerKirby::CPartTimerKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
{
}

CPartTimerKirby::CPartTimerKirby(const CPartTimerKirby& rhs)
	: CCharacter{ rhs }
{
}

HRESULT CPartTimerKirby::Initialize_Prototype()
{
	m_eCollisionGroup = PLAYER;

	return S_OK;
}

HRESULT CPartTimerKirby::Initialize(void* pArg)
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

	// FSM에서 첫 애니메이션 돌아가게 하는 구조
	m_pModelCom->Set_Animation(FOODSHOP_SELECT, 50.f, true, true);

	if(*m_pCurrentLevelID == LEVEL_TOWN)
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(2.f, 15.f, 3.f, 1.f));

	m_fScore = 10.f;

	Set_Slope(false);

	// 타겟 카메라를 만들어준다.
	if (FAILED(Make_TargetToCams()))
		return E_FAIL;

	return S_OK;
}

_int CPartTimerKirby::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_FirstTimer();

	//if (FOODSHOP_CORRECT == Get_State())
	//{
	//	m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta, 6.f);
	//}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CPartTimerKirby::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pModelCom->Play_Animation(fTimeDelta);

		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_DEFERREDINFO, this);
	}
}

HRESULT CPartTimerKirby::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_KIRBY)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CPartTimerKirby::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CPartTimerKirby::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	//ImGui::Text("Input C? : %d", m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS));
	//ImGui::Text("FSM : %d", m_pFSM->Get_State());
	ImGui::Separator(); ImGui::NewLine();
	__super::Render_IMGUI();
}
#endif

void CPartTimerKirby::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
		/*	m_vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
			Change_State(KABU_DAMAGE, 50.f, false, true);*/
		}
	}
	else if (eContent == CCollisionCenter::CONTENT_VACUUMOBJECT)
	{

	}
	else if (eContent == CCollisionCenter::CONTENT_ATTACK)
	{
		if (m_ePhyXState == PO_NORMAL)
		{
			/*m_vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
			Change_State(KABU_DAMAGE, 50.f, false, true);*/
		}
	}
}

void CPartTimerKirby::Collision_Hitbox(CPhysXObject* pGameObject)
{
}

void CPartTimerKirby::Change_State(ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CPartTimerKirby::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
}

_float4 CPartTimerKirby::Compute_TerrainPosition()
{
	if (m_pControllerCom == nullptr)
		return _float4();

	return m_pControllerCom->Compute_TerrainPosition_Vector();
}

HRESULT CPartTimerKirby::Make_TargetToCams()
{
	// 첫 카메라 기준으로 움직이기에 미리 받아둔다.
	if (m_pCamera == nullptr)
	{
		//인트로, 게임플레이 스테이지라면 카메라로 main camera를 저장한다.
		(LEVEL_INTRO <= *m_pCurrentLevelID && *m_pCurrentLevelID < LEVEL_END) ?
			m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Main"))) :
			m_pCamera = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free"))); //나머지 레벨이라면 다른 카메라를 저장한다.

		if (m_pCamera == nullptr)
		{
			ALARM_FAIL(TEXT("망했어 카메라 없다"));
			return E_FAIL;
		}
		Safe_AddRef(m_pCamera);
	}

	m_pCamera->Set_Target(m_pTransformCom);

	//게임 레벨에 free camera 있다면 그놈에게도 타겟 등록해 준다.
	if (LEVEL_INTRO <= *m_pCurrentLevelID && *m_pCurrentLevelID < LEVEL_END)
	{
		CCamera* pCameraFree = static_cast<CCamera*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Free")));
		if (pCameraFree != nullptr)
			pCameraFree->Set_Target(m_pTransformCom);
	}

	return S_OK;
}

HRESULT CPartTimerKirby::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_KirbyPartTimer"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);
	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;


	/* For.Com_CharacterController */
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 0.5f;
	desc.tCapsuleShape.fHeight = 0.4f;// 1.f;
	desc.tCapsuleShape.fRadius = 0.4f;// 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);


	/* For.HitBox */
	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = PLAYER;
	hr = m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox);
	CHECK_FAILED(hr);
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.f, 0.85f);


	/* For. FSM */
	SetUp_FSM();

	return S_OK;
}

HRESULT CPartTimerKirby::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
	//	return E_FAIL;

	return S_OK;
}

void CPartTimerKirby::SetUp_FSM()
{
	// FSM 상태 초기화
	m_pFSM = CFSM::Create();
	m_pFSM->Add_State(FOODSHOP_SELECT,	CPartTimerKirby_Idle_State::Create());
	m_pFSM->Add_State(FOODSHOP_MOVEL,	CPartTimerKirby_Move_State::Create());
	m_pFSM->Add_State(FOODSHOP_MOVER,	CPartTimerKirby_Move_State::Create());
	m_pFSM->Add_State(FOODSHOP_CORRECT, CPartTimerKirby_Grab_State::Create());
	
	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = FOODSHOP_SELECT;
	FSM_Desc.pModel = &m_pModelCom;
	m_pFSM->Initialize(&FSM_Desc);
}

CPartTimerKirby* CPartTimerKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPartTimerKirby* pInstance = new CPartTimerKirby(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPartTimerKirby"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPartTimerKirby::Clone(void* pArg)
{
	CPartTimerKirby* pInstance = new CPartTimerKirby(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CPartTimerKirby"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPartTimerKirby::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);

	for (auto& pEyeTexture : m_pEyeTexture)
		Safe_Release(pEyeTexture);
	for (auto& pMouthTexture : m_pMouthTexture)
		Safe_Release(pMouthTexture);

	Safe_Release(m_pCamera);
}
