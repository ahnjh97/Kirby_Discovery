#include "stdafx.h"
#include "Laser.h"
#include "Kirby.h"
#include "HitBox.h"



CLaser::CLaser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CLaser::CLaser(const CLaser& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CLaser::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLaser::Initialize(void* pArg)
{
	LASER_DESC* pLaserDesc = nullptr;

	if (nullptr != pArg)
	{
		pLaserDesc = (LASER_DESC*)pArg;

		pLaserDesc->fSpeedPerSec = 7.f;
		pLaserDesc->fRotationPerSec = XMConvertToRadians(90.0f);
		m_vPosition = pLaserDesc->vPosition;
		m_vPosition.m128_f32[1] += 3.f;
	}

	if (FAILED(__super::Initialize(pLaserDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPosition);

	//m_pTransformCom->Look_At(-pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
	m_pTransformCom->Set_Scaled(1.f, 1.f, 10.f);

	m_pModelCom->Set_Animation(1, 60.f, true, true);

	//m_pTransformCom->Look_At(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION));

	CMultiEffect::MULTI_FX_DESC FXDesc{};
	FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	FXDesc.vInitRot = { 180.f, 0.f, 0.f };
	//FXDesc.vInitPos
	Add_Effect("HS_FB dimension laser", FXDesc, false);

	m_fAttack = 5.f;
	m_bNonDead = true;

	return S_OK;
}

_int CLaser::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vKirbyPos.m128_f32[1] = 0.f;

	m_pTransformCom->Look_At_Interpolate(vKirbyPos, m_fTimeDelta * 0.2f);
	Activate_FrustumCollider(0.f, 200.f, 5.f);

	//레이저에 보스 건물 영역이 들어맞는지 판단, 그 영역에 자국을 남긴다.
	static _float fDecalTime{ 0.f };
	fDecalTime += m_fTimeDelta;
	
	if (.15f < fDecalTime)
	{
		fDecalTime = 0.f;

		_float3 vCollidingPoint =
			 CUtils::Compute_CollidingPoint(GET_POS, (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK), {0.f, 0.f, 0.f}, {21.f, 1.f, 21.f});

		if (!ISDEFAULTFLOAT3(vCollidingPoint))
		{
			if (m_pModelCom->Get_CurAnimIndex() != 0)
			{
				//레이저와의 충돌 자국
				CEffect::FX_DESC FXDesc{};
				FXDesc.vInitPos = vCollidingPoint;
				FXDesc.vInitScale = { 3.f, 3.f, 3.f };
				Add_Effect("HS_FB laser decal", FXDesc, false);

				//충돌 시 튀는 파티클
				CParticle::PARTICLE_DESC ParticleDesc{};
				ParticleDesc.vInitPos = vCollidingPoint;
				ParticleDesc.vInitScale = { 2.f, 2.f, 2.f };
				Add_Effect("HS_perfect laser collide particle", ParticleDesc);
			}

		}
	}

	if (true == m_bEnd)
	{
		m_bEnd = false;
		m_pModelCom->Set_Animation(0, 60.f, false, true);
	}

	if (m_pModelCom->IsFinished())
		m_bDead = true;

	return OBJ_NOEVENT;
}

void CLaser::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);


	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 100.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CLaser::Render()
{
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
		//if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
		//	return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CLaser::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CLaser::Render_IMGUI()
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

void CLaser::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

/*
_float3 CLaser::Compute_CollidingPoint(_float3 vLaserStart, _float3 vLaserDir, _float3 vLandCenter, _float3 vLandExtent)
{
	_float3 vResult{ -1.f, -1.f, -1.f };
	vLaserDir.Normalize();

	BoundingBox landBox;
	landBox.Center = vLandCenter;
	landBox.Extents = vLandExtent;

	_float fDist;
	if(landBox.Intersects(vLaserStart, vLaserDir, fDist))
	{
		vResult = vLaserStart + vLaserDir * fDist;
	}

	return vResult;
}
*/

HRESULT CLaser::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DimensionLaser"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	CHitBox::HITBOX_DESC HitBox{};
	//HitBox.pOwner = this;
	//HitBox.pDesc = &m_tColliderDesc[BODY];
	//HitBox.pCollisionType = MONSTER;
	//if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
	//	return E_FAIL;
	//Set_BodyCollider(COLLIDER_CYLINDER, 0.5f, 1.f, 0.85f);

	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[ATTACK];
	HitBox.pCollisionType = MONSTERBULLET;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	//Set_BodyCollider(COLLIDER_FRUSTUM, 0.5f, 1.5f, 0.85f);

	return S_OK;
}

HRESULT CLaser::Bind_ShaderResources()
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

void CLaser::Compute_MotionBlur()
{
}

CLaser* CLaser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLaser* pInstance = new CLaser(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLaser"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLaser::Clone(void* pArg)
{
	CLaser* pInstance = new CLaser(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLaser"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLaser::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
