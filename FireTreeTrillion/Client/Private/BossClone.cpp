#include "stdafx.h"
#include "BossClone.h"
#include "FinalBoss.h"
#include "HitBox.h"

CBossClone::CBossClone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CBossClone::CBossClone(const CBossClone& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CBossClone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossClone::Initialize(void* pArg)
{
	BOSSCLONE_DESC* pBossCloneDesc = nullptr;

	if (nullptr != pArg)
	{
		pBossCloneDesc = (BOSSCLONE_DESC*)pArg;

		pBossCloneDesc->fSpeedPerSec = 7.f;
		pBossCloneDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pBossCloneDesc->vPosition;
		m_vTargetPos = pBossCloneDesc->vTargetPos;
		m_vLook = pBossCloneDesc->vLook;
	}

	if (FAILED(__super::Initialize(pBossCloneDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_pTransformCom->Look_At_Axis(m_vLook);

	m_pModelCom->Set_Animation(78, 50.f, true, false);

	return S_OK;
}

_int CBossClone::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Monster")));
	if (CFinalBoss::FINALBOSS_DAMAGE == pFinalBoss->Get_State())
		m_bDead = true;

	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, m_vTargetPos)));
	if(false == m_bMove)
	{
		if (15.f < fDistance)
		{
			if (0.f < m_fSpeed)
				m_fSpeed -= m_fTimeDelta * 1.1f;
			else
				m_fSpeed = 0.f;

			vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * 22.f * m_fSpeed;
			vPos.m128_f32[1] = m_vPosition.m128_f32[1];
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
		else
		{
			m_fDelayTime += m_fTimeDelta;
			if(0.1f < m_fDelayTime)
				m_bMove = true;
		}
	}
	else if (true == m_bMove)
	{
		if (0.5f < fDistance)
		{
			if (0.f < m_fSecondSpeed)
				m_fSecondSpeed -= m_fTimeDelta * 0.8f;
			else
				m_fSecondSpeed = 0.f;

			vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * 22.f * m_fSecondSpeed;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
	}

	//if (14.6f < fDistance)
	//{
	//	if (0.f < m_fSpeed)
	//		m_fSpeed -= m_fTimeDelta;
	//	else
	//		m_fSpeed = 0.f;

	//	vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * 22.f * m_fSpeed;
	//	vPos.m128_f32[1] = m_vPosition.m128_f32[1];
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	//}
	//else if (0.5f < fDistance)
	//{
	//	if (0.f < m_fSecondSpeed)
	//		m_fSecondSpeed -= m_fTimeDelta * 0.8f;
	//	else
	//		m_fSecondSpeed = 0.f;

	//	vPos += XMVector3Normalize(m_vTargetPos - vPos) * m_fTimeDelta * 22.f * m_fSecondSpeed;
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	//}

	return OBJ_NOEVENT;
}

void CBossClone::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 100.0f))
	{
		m_pModelCom->Play_Animation(m_fTimeDelta);

		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CBossClone::Render()
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

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBossClone::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBossClone::Render_IMGUI()
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

void CBossClone::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CBossClone::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FinalBoss"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 2.5f, 1.f, 2.5f);

	return S_OK;
}

HRESULT CBossClone::Bind_ShaderResources()
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
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
	//	return E_FAIL;


	return S_OK;
}

CBossClone* CBossClone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossClone* pInstance = new CBossClone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBossClone"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBossClone::Clone(void* pArg)
{
	CBossClone* pInstance = new CBossClone(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBossClone"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossClone::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
