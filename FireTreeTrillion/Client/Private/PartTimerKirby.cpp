#include "stdafx.h"
#include "PartTimerKirby.h"

#include "FSM.h"
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

	if(*m_pCurrentLevelID == LEVEL_TOWN)
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(2.f, 24.f, 3.f, 1.f));
	m_pModelCom->Set_Animation(FOODSHOP_CORRECT, 50.f, true, true);
	m_fScore = 10.f;

	Set_Slope(false);

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
		//if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
		//	return E_FAIL;
		//if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
		//	return E_FAIL;

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

void CPartTimerKirby::Change_State(PARTTIMER_KIRBY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

_bool CPartTimerKirby::IsAnimFinished()
{
	return m_pModelCom->IsFinished();
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
	desc.fOffset = 1.f;
	desc.strProtoObjName = CUtils::WstrToStr(m_wstrPrototypeTag);
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);
	//m_pControllerCom->Set_Object(this);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
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
	//m_pFSM->Add_State(KABU_WAIT, CKabu_Idle_State::Create());
	//m_pFSM->Add_State(KABU_DAMAGE, CKabu_Damage_State::Create());
	//m_pFSM->Add_State(KABU_WARP1, CKabu_Warp_State::Create());

	//상태 Initialize
	CFSM::FSM_INFO		FSM_Desc = {};
	FSM_Desc.iState = 0;
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

}
