#include "stdafx.h"
#include "SpikeSpear.h"
#include "FinalBoss.h"
#include "HitBox.h"
#include "Ability.h"
#include "Camera.h"
#include "Kirby.h"
#include "Camera_Main.h"

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

	m_fRingInnerRadius = 0;
	m_fRingOuterRadius = 0;

	m_fAttack = 10.f;

	return S_OK;                                                                                                                                                                                                                          
}

_int CSpikeSpear::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_BossMonster")));

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

				m_pGameInstance->PlaySound_Free(L"BossChimera_Spike2.wav", 0.5f);

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


				}

					//효선아 여기야
					vPos.y = 0.f; 
					CMultiEffect::MULTI_FX_DESC FXDesc{};
					FXDesc.vInitPos = (_float3)vPos;
					FXDesc.vInitScale = { 5.f, 5.f, 5.f };

					//FXDesc.fStartDelay = 1.f;
					m_pGameInstance->Get_CurCameraPtr()->Make_Shake(.2f, .5f);
					Add_Effect("HS_FB down spear circle", FXDesc, false);

					CParticle::PARTICLE_DESC FXPDesc{};
					FXPDesc.vInitPos = (_float3)vPos;
					FXPDesc.vInitScale = { 1.f, 1.f, 1.f };
					pFinalBoss->Add_Effect("YW Final Boss Wiggle B", FXPDesc, false);

					m_vRingPos = (_float3)vPos; 

			}
		}
	}
	else if (3.35f < m_fSpikeTime && 3.45f > m_fSpikeTime)
	{
		m_vPosition.m128_f32[1] += m_fTimeDelta * 35.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);
	}

	if(true == m_bItem)
	{
		m_fEffectTime += m_fTimeDelta;
		if(2.f > m_fEffectTime)
			CheckFinalCrusherRingCollision(m_fTimeDelta);
		else
		{
			m_fRingInnerRadius = 0;
			m_fRingOuterRadius = 0;
		}
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
		if (FAILED(m_pShaderCom->Begin(MODEL_FOR_SPIKE)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	RenderRing();
#endif

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

void CSpikeSpear::CheckFinalCrusherRingCollision(_float fTimeDelta)
{
	m_fRingOuterRadius += fTimeDelta * 7.4f;
	m_fRingInnerRadius = m_fRingOuterRadius * 0.92f;
	if (m_fRingInnerRadius < 1.f)
		m_fRingInnerRadius = 1.f;

	m_bRenderRing = true;
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();
	_float3 vKirbyPos = pKirbyTransformCom->Get_State(CTransform::STATE_POSITION);
	_float fDis = (vKirbyPos - m_vRingPos).Length();

	// 바깥원과 안쪽 원 사이에 있고, 높이 차이가 1 미만일때 충돌
	if (fDis > m_fRingOuterRadius || fDis < m_fRingInnerRadius || vKirbyPos.y > m_vRingPos.y + 1)
		return;

	if (true == CCollisionCenter::Get_Instance()->Kirby_Dodge_SlowMotionSystem(pKirby))
		return;

	if (pKirby->isOverPower() == false) // 무적이 아닐 경우
	{
		_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_float4 vDistance = vKirbyPos - GET_POS;
		vDistance.y = 0.f;
		vDistance.Normalize();

		_float4 vNewDir{};
		_float4 vRight = m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
		vRight.Normalize();

		if (true == IsKirbyOnMyLeft(pKirby))
			vNewDir = vDistance + vRight * 2.5f;
		else
			vNewDir = vDistance - vRight * 2.5f;
		vNewDir.Normalize();
		_vector vKnockbackDir = vNewDir;

		pKirby->Set_DamageMoving(vKnockbackDir * 1.8f, 8.f); // 심바 전용 넉백

		_float fMonsterAttack = Get_Attack();
		pKirby->Minus_Hp(fMonsterAttack);
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		pCamera->Make_Shake(1.2f, 0.5f, _float2(0.f, -1.f));

		pKirby->Collision(CCollisionCenter::CONTENT_ATTACK, this);
	}
}

_bool CSpikeSpear::IsKirbyOnMyLeft(CKirby* pKirby)
{
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	CTransform* pKirbyTransform = pKirby->Get_TransformCom();
	if (nullptr == pKirbyTransform)
		return false;
	_vector vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);

	_vector vDir = vKirbyPos - vPos;
	vDir = XMVector3Normalize(XMVectorSetY(vDir, 0));

	_vector crossProduct = XMVector3Cross(vLook, vDir);
	_float fCrossResultY = XMVectorGetY(crossProduct);

	if (fCrossResultY > 0.f)
		return true;
	else
		return false;
}

#ifdef _DEBUG
void CSpikeSpear::RenderRing()
{
	vector<_vector> vecOuterRingPoints;
	vector<_vector> vecInnerRingPoints;
	for (_uint i = 0; i < 36; i++)
	{
		_float4 vDir = CUtils::TurnDirectionVector(XMVectorSet(1, 0, 0, 0), _float3(0, 1, 0), i * 10.f);
		_float4 vOuterPos = m_vRingPos + vDir * m_fRingOuterRadius;
		vOuterPos.w = 1.f;
		vecOuterRingPoints.push_back(vOuterPos);

		_float4 vInnerPos = m_vRingPos + vDir * m_fRingInnerRadius;
		vInnerPos.w = 1.f;
		vecInnerRingPoints.push_back(vInnerPos);
	}

	RenderPolygon(vecOuterRingPoints);
	RenderPolygon(vecInnerRingPoints);
}

void CSpikeSpear::RenderPolygon(vector<_vector>& worldPoints)
{
	if (m_pGameInstance->Get_HitBoxRender() == false)
		return;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	_matrix ViewMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW); // CPipeLine::D3DTS_VIEW
	_matrix ProjMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ); // CPipeLine::D3DTS_PROJ
	_matrix VPMatrix = XMMatrixMultiply(ViewMatrix, ProjMatrix);

	auto TransformToScreen = [&](_vector worldPos)
	{
		_vector screenPos = XMVector3TransformCoord(worldPos, VPMatrix);
		screenPos = XMVectorMultiplyAdd(screenPos, XMVectorSet(0.5f, -0.5f, 1.0f, 0.0f), XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f));
		screenPos = XMVectorMultiply(screenPos, XMVectorSet(g_iWinSizeX, g_iWinSizeY, 1.f, 0.f));
		return ImVec2(XMVectorGetX(screenPos), XMVectorGetY(screenPos));
	};

	// 월드 좌표를 화면 좌표로 변환
	vector<ImVec2> screenPoints;
	for (const auto& point : worldPoints)
		screenPoints.push_back(TransformToScreen(point));

	// 점들을 이어서 다각형 그리기
	for (size_t i = 0; i < screenPoints.size(); ++i)
	{
		const ImVec2& p1 = screenPoints[i];
		const ImVec2& p2 = screenPoints[(i + 1) % screenPoints.size()]; // 마지막 점은 첫 번째 점과 연결
		drawList->AddLine(p1, p2, IM_COL32(255, 255, 0, 255), 2.0f);
	}
}
#endif

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
