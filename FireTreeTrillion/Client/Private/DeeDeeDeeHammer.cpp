#include "stdafx.h"
#include "DeeDeeDeeHammer.h"
#include "Bone.h"


CDeeDeeDeeHammer::CDeeDeeDeeHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{
}

CDeeDeeDeeHammer::CDeeDeeDeeHammer(const CDeeDeeDeeHammer& rhs)
	: CPartObject{ rhs }
{
}

HRESULT CDeeDeeDeeHammer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Initialize(void* pArg)
{
	DEEDEEDEEHAMMER_DESC* pWeaponDesc = (DEEDEEDEEHAMMER_DESC*)pArg;

	m_pBoneMatrix = pWeaponDesc->pBoneMatrix;
	m_pWhiteColorDiffuse = pWeaponDesc->pWhite;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	return S_OK;
}

_int CDeeDeeDeeHammer::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	Compute_MotionBlur();

	m_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * *m_pBoneMatrix * *m_pParentMatrix;

	return OBJ_NOEVENT;
}

void CDeeDeeDeeHammer::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, m_WorldMatrix._44), 5.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CDeeDeeDeeHammer::Render()
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
		if (FAILED(m_pShaderCom->Begin(13)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_PartObject(m_pShaderCom, &m_WorldMatrix, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DeeDeeDeeHammer"),
		TEXT("Com_Model_Sword"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CDeeDeeDeeHammer::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 추후 변경
	_bool bStencil = true;
	_bool bRimLight = true;
	_bool bMotionBlur = true;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", m_pWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CDeeDeeDeeHammer::Compute_MotionBlur()
{
	_vector vPos = CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vPreScreenPos = vCurScreenPos;
}

CDeeDeeDeeHammer* CDeeDeeDeeHammer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDeeDeeDeeHammer* pInstance = new CDeeDeeDeeHammer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDeeDeeDeeHammer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDeeDeeDeeHammer::Clone(void* pArg)
{
	CDeeDeeDeeHammer* pInstance = new CDeeDeeDeeHammer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDeeDeeDeeHammer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDeeDeeDeeHammer::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
