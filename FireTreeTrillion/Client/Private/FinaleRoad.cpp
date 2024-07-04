#include "stdafx.h"
#include "FinaleRoad.h"

CFinaleRoad::CFinaleRoad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPhysXObject{ pDevice ,pContext }
{
}

CFinaleRoad::CFinaleRoad(const CFinaleRoad& rhs)
	:CPhysXObject{ rhs }
{
}

void CFinaleRoad::OnCollision()
{
}

HRESULT CFinaleRoad::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleRoad::Initialize(void* pArg)
{
	return S_OK;
}

_int CFinaleRoad::Tick(_float fTimeDelta)
{
	return _int();
}

void CFinaleRoad::Late_Tick(_float fTimeDelta)
{
}

HRESULT CFinaleRoad::Render()
{
	return S_OK;
}

HRESULT CFinaleRoad::Render_LightDepth()
{
	return S_OK;
}

void CFinaleRoad::Render_IMGUI()
{
}

HRESULT CFinaleRoad::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);




	return S_OK;
}

HRESULT CFinaleRoad::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CFinaleRoad::Make_Particles()
{
}

CFinaleRoad* CFinaleRoad::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinaleRoad* pInstance = new CFinaleRoad(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinaleRoad"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinaleRoad::Clone(void* pArg)
{
	CFinaleRoad* pInstance = new CFinaleRoad(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinaleRoad"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinaleRoad::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
