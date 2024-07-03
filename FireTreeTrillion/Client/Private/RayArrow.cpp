#include "stdafx.h"
#include "RayArrow.h"
#include "FinalBoss.h"
#include "Kirby.h"
#include "HitBox.h"

CRayArrow::CRayArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CRayArrow::CRayArrow(const CRayArrow& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CRayArrow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRayArrow::Initialize(void* pArg)
{
	RAYARROW_DESC* pRayArrowDesc = nullptr;

	if (nullptr != pArg)
	{
		pRayArrowDesc = (RAYARROW_DESC*)pArg;

		pRayArrowDesc->fSpeedPerSec = 7.f;
		pRayArrowDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pRayArrowDesc->vPosition;
		m_fAngle = pRayArrowDesc->fAngle;
		m_fHeight = pRayArrowDesc->fHeight;
		m_vSide = pRayArrowDesc->vSide;
		m_fDelayTime = pRayArrowDesc->fDelayTime;
		m_fSpeedWeight = pRayArrowDesc->fSpeedWeight;
	}

	if (FAILED(__super::Initialize(pRayArrowDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Monster")));
	CTransform* pTransform = pFinalBoss->Get_TransformCom();
	_vector vLookAt = pTransform->Get_State_Vector(CTransform::STATE_LOOK);

	m_pTransformCom->Look_At_Axis(vLookAt);

	_float3		vScaled = m_pTransformCom->Get_Scaled();

	//m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	//m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
	//m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), 1.f);
	//m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), -0.5f);

	m_vRight = m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
	m_vUp = m_pTransformCom->Get_State_Vector(CTransform::STATE_UP);
	m_vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	m_fRotateSpeed = 10.f;

	return S_OK;
}

_int CRayArrow::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Monster")));
	if (CFinalBoss::FINALBOSS_RAYARROWEND == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_RAYARROWENDAIR == pFinalBoss->Get_State())
	{
		m_bActive = true;
		m_bFireActive = true;
	}

	if(true == m_bFireActive)
		m_fFireTime += fTimeDelta;

	// 화살 발사
	if (true == m_bFire)
	{
		if(m_fDelayTime < m_fFireTime)
		{
			m_fRotateTime += m_fTimeDelta;
			if (0.5f > m_fRotateTime)
			{
				m_fRotateSpeed += m_fTimeDelta * 20.f;
				m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), m_fTimeDelta * m_fRotateSpeed);
			}
			else
			{
				// 베지에 곡선 계산
				_vector vNewPosition = {};
				m_fElapsedTime += m_fTimeDelta * m_fSpeedWeight;
				if (1.f > m_fElapsedTime)
				{
					vNewPosition.m128_f32[0] = (1.f - m_fElapsedTime) * (1.f - m_fElapsedTime) * m_vPosition.m128_f32[0] + 2.f * (1.f - m_fElapsedTime) * m_fElapsedTime * m_vControllPos.m128_f32[0] + m_fElapsedTime * m_fElapsedTime * m_vKirbyPos.m128_f32[0];
					vNewPosition.m128_f32[1] = (1.f - m_fElapsedTime) * (1.f - m_fElapsedTime) * m_vPosition.m128_f32[1] + 2.f * (1.f - m_fElapsedTime) * m_fElapsedTime * m_vControllPos.m128_f32[1] + m_fElapsedTime * m_fElapsedTime * m_vKirbyPos.m128_f32[1];
					vNewPosition.m128_f32[2] = (1.f - m_fElapsedTime) * (1.f - m_fElapsedTime) * m_vPosition.m128_f32[2] + 2.f * (1.f - m_fElapsedTime) * m_fElapsedTime * m_vControllPos.m128_f32[2] + m_fElapsedTime * m_fElapsedTime * m_vKirbyPos.m128_f32[2];
					m_pTransformCom->Look_At_Axis(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - XMVectorSetW(vNewPosition, 1.f));
					m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetW(vNewPosition, 1.f));
					m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), ToRadian(-m_fAngle));
				}
				else
				{
					// 땅에 박히고 n초 후 dead처리
					m_fDeadTime += m_fTimeDelta;
					if (1.f < m_fDeadTime)
						m_bDead = true;
				}
			}
		}
		else
		{
			CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
			CTransform* pKirbyTransform = pKirby->Get_TransformCom();
			m_vKirbyPos = pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION);
		}
	}
	// 화살 활성화
	else if(true == m_bActive)
	{
		m_fTurnTime += m_fTimeDelta;
		{
			if (0.25f > m_fTurnTime)
			{
				m_pTransformCom->Turn(m_vRight, m_fTimeDelta * 16.f);
				m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), ToRadian(m_fAngle) * m_fTimeDelta * 4.f);
				_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				vPos.m128_f32[1] += m_fTimeDelta * 2.f;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
			}
			else
			{
				m_bFire = true;
				m_bActive = false;
				
				m_vControllPos.m128_f32[0] = (m_vPosition.m128_f32[0] + m_vKirbyPos.m128_f32[0]) * 0.5f + m_vSide.m128_f32[0];
				m_vControllPos.m128_f32[1] = (m_vPosition.m128_f32[1] + m_vKirbyPos.m128_f32[1]) * 0.5f + m_fHeight; // Y축 높이 조정
				m_vControllPos.m128_f32[2] = (m_vPosition.m128_f32[2] + m_vKirbyPos.m128_f32[2]) * 0.5f + m_vSide.m128_f32[2];
			}
		}
	}

	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CRayArrow::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 100.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CRayArrow::Render()
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

HRESULT CRayArrow::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CRayArrow::Render_IMGUI()
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

void CRayArrow::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

#endif

HRESULT CRayArrow::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_RayArrow"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTER;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 1.5f, 1.f);

	return S_OK;
}

HRESULT CRayArrow::Bind_ShaderResources()
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

void CRayArrow::Compute_MotionBlur()
{
}

CRayArrow* CRayArrow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRayArrow* pInstance = new CRayArrow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CRayArrow"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRayArrow::Clone(void* pArg)
{
	CRayArrow* pInstance = new CRayArrow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CRayArrow"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRayArrow::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
