#include "stdafx.h"
#include "KickableRock.h"
#include "HitBox.h"

CKickableRock::CKickableRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CRigidObject{ pDevice, pContext }
{
}

CKickableRock::CKickableRock(const CKickableRock& rhs)
	: CRigidObject( rhs )
{
}

HRESULT CKickableRock::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CKickableRock::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;

	HRESULT  hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if(pGameObjectDesc != nullptr)
		Add_Components(pGameObjectDesc->wstrModelName);

	m_eAbilityType = ABILITY_DEFAULT;
	m_bMotionBlur = true;

	return S_OK;
}

_int CKickableRock::Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	__super::Tick(m_fTimeDelta);

	Compute_MotionBlur();

	if (true == m_bDead)
		return Ready_Dead(0.9f);


	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	if (m_ePhyXState == PO_FLYAWAY)
	{
		_float3 vDamegeDir = m_vDamegeDir;
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDamegeDir * m_fTimeDelta * 30.f);
		m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), m_fTimeDelta, 360.f);
		m_fFlyTime += m_fTimeDelta;

		if (m_fFlyTime > 2.f)
			m_bDead = true;
	}
	else if (m_ePhyXState == PO_FLYDEADAWAY)
	{
		m_fDeadTime += m_fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = m_vDamegeDir;

		// 이제 날아가는 것을 구현해보자.
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDamegeDir * m_fTimeDelta * 10.f);

		// 점프되는 체공시간을 구현해보자.
		vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + _float4(0.f, m_fDamageJumpPower, 0.f, 0.f) * m_fTimeDelta);
		m_fDamageJumpPower -= GRAVITY * m_fTimeDelta * 3.f;

		if (m_fDeadTime > 0.45f)
			m_bDead = true;
	}

	return OBJ_NOEVENT;
}

void CKickableRock::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(m_fTimeDelta);

	// 커비 입 안에 있고, Fly가 아닐땐 입 안에 있는 상황이므로, Render되지않는다.
	if (m_ePhyXState == PO_KIRBYMOUTH)
		return;

	if (m_bLockCollision == true)
	{
		// 커비에게 빨려들어가지 않게 END로 한다.
		m_ePhyXState = PO_END;
		m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
		m_pRigidBodyCom->Add_Force(_float3(0.f, -0.5f, 0.f));

		m_fLifeTime += m_fTimeDelta;
		if (m_fLifeTime >= 1.5f)
		{
			m_fLifeTime = 0.f;
			m_bDead = true;
		}
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CKickableRock::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);
		
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CKickableRock::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CKickableRock::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}

#endif


void CKickableRock::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_KICK)
	{
		// 노말 상태일 경우에만 톡 쳤을때 반응하게 하고, 나머진 서로 충돌이 되면 안 된다.
		if (m_ePhyXState == PO_NORMAL && m_bLockCollision == false)
		{
			m_pRigidBodyCom->Activate(true);
			CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
			_float4 vPlayerPos = static_cast<CTransform*>(pPlayer->Get_TransformCom())->Get_State(CTransform::STATE_POSITION);
			_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

			_float3 vDir = vPos - vPlayerPos;
			vDir.y = 0.f;
			vDir.Normalize();
			vDir.y += 1.f;
			vDir.Normalize();
			_float3 force = vDir;
			m_pRigidBodyCom->Kick_RigidBody(XMVector3Normalize(force), 480.f);

			// 힘이 한번만 작용되게 한다.
			m_bLockCollision = true;
		}
		else if (m_ePhyXState == PO_VACUUMING)
		{
			// 빨리는 순간. 아무것도 안해도 됨 ㅋ
		}
	}
}

HRESULT CKickableRock::Add_Components(const wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + wstrModelName;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_RigidBody */
	CRigidBody::RIGIDBODY_DESC rigidDesc {};
	rigidDesc.bTrigger = false;
	rigidDesc.bDynamic = true;
	rigidDesc.bKinematic = false;
	rigidDesc.eShapeType = RIGID_SPHERE;
	rigidDesc.fOffsetSize = { 0.5f, 0.5f, 0.5f };
	rigidDesc.vMaterial = _float3(10.f, 1.f, 0.85f);
	rigidDesc.fDensity = 800.f;
	rigidDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	hr = __super::Add_Component(TEXT("Prototype_Component_RigidBody"),
		TEXT("Com_RigidBody"), (CComponent**)&m_pRigidBodyCom, &rigidDesc);
	CHECK_FAILED(hr);
	//m_pRigidBodyCom->Set_Object(this);
	m_pRigidBodyCom->Activate(false);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = OBJECT;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 0.f, 1.f);


	return S_OK;
}

HRESULT CKickableRock::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;

	_float fWhiteColor = 0.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColor, sizeof(_float))))
		return E_FAIL;


	return S_OK;
}

void CKickableRock::Compute_MotionBlur()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vMotionVelocity.z = m_ePhyXState != PO_NORMAL ? 1.f : 0.f;

	m_vPreScreenPos = vCurScreenPos;
}


CKickableRock* CKickableRock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKickableRock* pInstance = new CKickableRock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CKickableRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CKickableRock::Clone(void* pArg)
{
	CKickableRock* pInstance = new CKickableRock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CKickableRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CKickableRock::Free()
{
	__super::Free();
	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pModelCom);
} 

