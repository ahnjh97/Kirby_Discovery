#include "stdafx.h"
#include "BreakableRockPartical.h"

CBreakableRockPartical::CBreakableRockPartical(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CRigidObject{ pDevice, pContext }
{
}

CBreakableRockPartical::CBreakableRockPartical(const CBreakableRockPartical& rhs)
	: CRigidObject(rhs)
{
}

HRESULT CBreakableRockPartical::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBreakableRockPartical::Initialize(void* pArg)
{
	BREAKABLEPARTICALDESC desc = *(BREAKABLEPARTICALDESC*)pArg;

	GAMEOBJECT_DESC		GameObjectDesc{};
	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(desc.matrix);

	if (FAILED(Add_Components(desc.wstrModelName)))
		return E_FAIL;

	desc.vMoveDir.Normalize();

	m_pRigidBodyCom->Kick_RigidBody(desc.vMoveDir, desc.fPower);

	m_bStencil = true;
	m_bRimLight = true;
	m_bMotionBlur = true;

	m_fLifeMaxTime = CUtils::Make_RandomFloat(1.2f, 2.5f);

	return S_OK;
}

_int CBreakableRockPartical::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Ready_Dead(0.9f);


	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	m_fLifeTime += m_fTimeDelta;
	Compute_MotionBlur();

	if (m_fLifeTime > m_fLifeMaxTime)
		m_bDead = true;

	return OBJ_NOEVENT;
}

void CBreakableRockPartical::Late_Tick(_float fTimeDelta)
{

	m_pRigidBodyCom->Update_PhysX(m_pTransformCom);
	m_pRigidBodyCom->Add_Force(_float3(0.f, -0.5f, 0.f));

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

}

HRESULT CBreakableRockPartical::Render()
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
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);
		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBreakableRockPartical::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG

void CBreakableRockPartical::Render_IMGUI()
{
}

#endif

HRESULT CBreakableRockPartical::Add_Components(const wstring& _wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelName, 
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
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
	m_pRigidBodyCom->Activate(true);

	return S_OK;
}

HRESULT CBreakableRockPartical::Bind_ShaderResources()
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

void CBreakableRockPartical::Compute_MotionBlur()
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

CBreakableRockPartical* CBreakableRockPartical::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBreakableRockPartical* pInstance = new CBreakableRockPartical(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBreakableRockPartical"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBreakableRockPartical::Clone(void* pArg)
{
	CBreakableRockPartical* pInstance = new CBreakableRockPartical(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBreakableRockPartical"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBreakableRockPartical::Free()
{
	__super::Free();
	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pModelCom);
}
