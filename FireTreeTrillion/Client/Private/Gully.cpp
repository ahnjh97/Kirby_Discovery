#include "stdafx.h"
#include "Gully.h"
#include "HitBox.h"
#include "Camera_Main.h"
#include "Fire.h"

CGully::CGully(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGully::CGully(const CGully& rhs)
	: CPhysXObject{ rhs }
{
}



HRESULT CGully::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGully::Initialize(void* pArg)
{
	GULLY_DESC* pGullyDesc = { nullptr };

	if (nullptr != pArg)
	{
		pGullyDesc = (GULLY_DESC*)pArg;

		pGullyDesc->fSpeedPerSec = 7.f;
		pGullyDesc->fRotationPerSec = XMConvertToRadians(90.0f);

	}

	if (FAILED(__super::Initialize(pGullyDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_vPosition.m128_f32[1] -= 5.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_pTransformCom->Turn(CUtils::Make_Random_Vector(1.f), 1.f);

	m_bPoolingDead = true;
	m_bDead = false;

	return S_OK;
}

_int CGully::Tick(_float fTimeDelta)
{
	if (true == m_bPoolingDead)
		return OBJ_NOEVENT;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (0.f < m_fLifeTime)
	{
		m_fLifeTime -= m_fTimeDelta;
	}
	else
	{
		if (0.2f < m_fScale)
		{
			m_fScale -= m_fTimeDelta;
			m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
		}
		else
		{
			m_vPosition.m128_f32[1] -= 5.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
			m_fLifeTime = 0.f;
			m_bPoolingDead = true;
			m_bFireGully = false;
		}
	}

	if (m_bFireGully == true && m_fLifeTime > 0.f)
	{
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos.y += 0.5f;
		vPos.x += CUtils::Make_RandomFloat(-1.f, 1.f);
		vPos.z += CUtils::Make_RandomFloat(-1.f, 1.f);

		CFire::FIREDESC Firedesc = {};
		Firedesc.vFirePos = vPos;
		Firedesc.fUpRange = { 100.f };
		Firedesc.vFirstColor = { 1.f, .5f, .2f , 1.f };
		Firedesc.vTargetColor = { .7f, 0.f, 1.f, 1.f };
		Firedesc.fScale = { 2.7f };
		Firedesc.fTimeRatio = { 0.8f };
		Firedesc.vMoveDir = { 0.f, 1.f, 0.f };
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Fire"), TEXT("Prototype_GameObject_Fire"), &Firedesc)))
			return OBJ_NOEVENT;
	}
	
	return OBJ_NOEVENT;
}

void CGully::Late_Tick(_float fTimeDelta)
{
	if (true == m_bPoolingDead)
		return;

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 100.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CGully::Render()
{
	if (true == m_bPoolingDead)
		return S_OK;

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

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CGully::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGully::Render_IMGUI()
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

void CGully::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CGully::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MoundPiece"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTERBULLET;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.5f, 0.85f);

	return S_OK;
}

HRESULT CGully::Bind_ShaderResources()
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

	return S_OK;
}

CGully* CGully::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGully* pInstance = new CGully(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGully"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGully::Clone(void* pArg)
{
	CGully* pInstance = new CGully(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGully"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGully::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
