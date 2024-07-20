#include "stdafx.h"
#include "SpikeSpear.h"
#include "FinalBoss.h"
#include "HitBox.h"
#include "Ability.h"

CSpikeSpear::CSpikeSpear(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CSpikeSpear::CSpikeSpear(const CSpikeSpear& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CSpikeSpear::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSpikeSpear::Initialize(void* pArg)
{
	SPIKESPEAR_DESC* pSpikeSpearDesc = nullptr;

	if (nullptr != pArg)
	{
		pSpikeSpearDesc = (SPIKESPEAR_DESC*)pArg;

		pSpikeSpearDesc->fSpeedPerSec = 7.f;
		pSpikeSpearDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pSpikeSpearDesc->vPosition;
	}

	if (FAILED(__super::Initialize(pSpikeSpearDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_vPosition.m128_f32[1] += 15.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_pTransformCom->Set_Scaled(2.f, 2.f, 2.f);
	m_pTransformCom->Turn(XMVectorSet(-1.f, 0.f, 0.f, 0.f), 1.f);

	m_bNonDead = true;

	return S_OK;                                                                                                                                                                                                                          
}

_int CSpikeSpear::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Monster")));

	m_fSpikeTime += m_fTimeDelta;
	m_fLifeTime += m_fTimeDelta;

	if (CFinalBoss::FINALBOSS_DIMENSIONSPIKE == pFinalBoss->Get_State())
	{

		if(0.23f < pFinalBoss->Get_AnimRatio() && 0.3f > pFinalBoss->Get_AnimRatio())
		{
			m_vPosition.m128_f32[1] -= m_fTimeDelta * 27.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
		}
	}

	if (6.f < m_fLifeTime)
	{
		if (25.f > m_vPosition.m128_f32[1])
		{
			m_vPosition.m128_f32[1] += m_fTimeDelta * 35.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
		}
		else
			m_bDead = true;
	}
	else if (3.5f < m_fSpikeTime)
	{
		if (12.f < m_vPosition.m128_f32[1])
		{
			m_vPosition.m128_f32[1] -= m_fTimeDelta * 80.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
		}
		else
		{
			if(false == m_bItem)
			{
				m_bItem = true;

				_float4 vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				vPos.y -= 10.f;
				for (_uint i = 0; i < 3; ++i)
				{
					HRESULT hr;
					// 별 아이템 떨굼
					CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
					AbilityItemDesc.fRotateDir = 1.f;																	// 별 회전 방향 오른쪽															// 별 회전 방향 왼쪽
					AbilityItemDesc.fAngle = 360.f / 3.f * i;													// 별의 진행 방향의 각도
					AbilityItemDesc.vDir = XMVectorSet(1.f, 0.f, 0.f, 0.f);							// 별의 진행 방향
					AbilityItemDesc.vPosition = vPos;	// 별의 생성 위치
					AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
					hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
					CHECK_FAILED(hr);

					//효선아 여기야
					vPos.m128_f32[1] -= 1.f;
					CMultiEffect::MULTI_FX_DESC FXDesc{};
					FXDesc.vInitPos = (_float3)vPos;
					FXDesc.vInitScale = { 5.f, 5.f, 5.f };

					//FXDesc.fStartDelay = 1.f;

					Add_Effect("HS_FB down spear circle", FXDesc);

				}
			}
		}
	}
	else if (3.35f < m_fSpikeTime && 3.45f > m_fSpikeTime)
	{
		m_vPosition.m128_f32[1] += m_fTimeDelta * 35.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	}

	//m_fLifeTime += m_fTimeDelta;
	//if (2.f < m_fLifeTime)
	//	m_bDead = true;

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CSpikeSpear::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 12.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CSpikeSpear::Render()
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

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CSpikeSpear::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CSpikeSpear::Render_IMGUI()
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

void CSpikeSpear::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CSpikeSpear::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FinalBossSpear"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTERBULLET;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, -11.f, 1.f, 2.f);

	return S_OK;
}

HRESULT CSpikeSpear::Bind_ShaderResources()
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

void CSpikeSpear::Compute_MotionBlur()
{
}

CSpikeSpear* CSpikeSpear::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSpikeSpear* pInstance = new CSpikeSpear(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSpkieSpear"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSpikeSpear::Clone(void* pArg)
{
	CSpikeSpear* pInstance = new CSpikeSpear(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSpkieSpear"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSpikeSpear::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
