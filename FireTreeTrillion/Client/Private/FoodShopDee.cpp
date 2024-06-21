#include "stdafx.h"
#include "FoodShopDee.h"
#include "FSM.h"
#include "Dee_State.h"
#include "HitBox.h"

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
	CHECK_FAILED_MSG(hr, "¿Íµéµð »ý¼º ¸ÁÇß¾î");


	hr = Add_Components();
	CHECK_FAILED_MSG(hr, "¿Íµéµð »ý¼º ¸ÁÇß¾î");

	m_pTransformCom->Rotation(_float3{ 0.f, 1.f, 0.f }, ToRadian(180.f));
	m_pModelCom->Set_Animation(0, 50.f, true, true);

	return S_OK;
}

_int CFoodShopDee::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	__super::Tick(m_fTimeDelta);

	m_pControllerCom->FreeFall(m_pTransformCom, m_fTimeDelta);
	return OBJ_NOEVENT;
}

void CFoodShopDee::Late_Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	m_pModelCom->Play_Animation(m_fTimeDelta);

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
		//if (Custom_Face(i) == true)
		//	continue;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;


		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
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

void CFoodShopDee::Render_IMGUI()
{
	__super::Render_IMGUI();
}

void CFoodShopDee::Add_AnimEvent()
{
	__super::Add_AnimEvent();

}

void CFoodShopDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if(m_pGameInstance->Get_KeyState(DIK_LCONTROL, KEY_PRESS) && m_pGameInstance->Get_KeyState(DIK_1, KEY_DOWN))
	m_pModelCom->Set_Animation(27, 50.f, true, true);
}

void CFoodShopDee::Change_State(DEE_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

HRESULT CFoodShopDee::Add_Components()
{
	HRESULT hr;


	//½¦ÀÌ´õ
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	//¸ðµ¨
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_WaddleDeeBase"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	// FOR ANIMTOOL
	m_ppModelForAnimTool = &m_pModelCom;

	//´« ÅØ½ºÃÄ
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Dee_Eye"),
		TEXT("Com_Texture"), (CComponent**)&m_pEyeTextureCom);
	CHECK_FAILED(hr);


	//ÄÁÆ®·Ñ·¯
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
	return S_OK;
}

HRESULT CFoodShopDee::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		ALARM_FAIL("½¦ÀÌ´õ°¡ À¾¼­");

	HRESULT hr;

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");


	hr = m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");
	hr = m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float));
	CHECK_FAILED_MSG(hr, "¹ÙÀÎµù ¸ÁÇÔ");


	return S_OK;
}

void CFoodShopDee::SetUp_FSM()
{
	m_pFSM = CFSM::Create();


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
	Safe_Release(m_pEyeTextureCom);
	__super::Free();
}
