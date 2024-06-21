#include "stdafx.h"
#include "GameInstance.h"
#include "SkySphere.h"


CSkySphere::CSkySphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CSkySphere::CSkySphere(const CSkySphere& rhs)
	:CGameObject{ rhs }
{
}

HRESULT CSkySphere::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkySphere::Initialize(void* pArg)
{

	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	hr = Add_Components();
	CHECK_FAILED(hr);


	m_pTransformCom->Set_Scaled(_float3{ .6f, .6f, .6f });
	return S_OK;
}

_int CSkySphere::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CSkySphere::Late_Tick(_float fTimeDelta)
{
	_float4 vCamPos = m_pGameInstance->Get_CamPosition();
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vCamPos);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_PRIORITY, this);
}

HRESULT CSkySphere::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	HRESULT hr;

	for (_uint i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(MODEL_SKY);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CSkySphere::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Model_SkySphere_Stage1_Day"), TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CSkySphere::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	return S_OK;
}

CSkySphere* CSkySphere::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSkySphere* pInstance = new CSkySphere(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSkySphere"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSkySphere::Clone(void* pArg)
{
	CSkySphere* pInstance = new CSkySphere(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CSkySphere"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkySphere::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	__super::Free();
}
