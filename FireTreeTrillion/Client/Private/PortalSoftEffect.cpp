#include "stdafx.h"
#include "PortalSoftEffect.h"

CPortalSoftEffect::CPortalSoftEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPortalSoftEffect::CPortalSoftEffect(const CPortalSoftEffect& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CPortalSoftEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPortalSoftEffect::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;
	}

	if (FAILED(__super::Initialize(pGameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(10.f, 10.f, 1.f);
	if(LEVEL_TOWN == *m_pCurrentLevelID)
		m_pTransformCom->Set_Scaled(15.f, 15.f, 1.f);

	return S_OK;
}

_int CPortalSoftEffect::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CPortalSoftEffect::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CPortalSoftEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;
	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

	if (FAILED(m_pShaderCom->Begin(POSTEX_SOFTFX)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CPortalSoftEffect::Render_IMGUI()
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


HRESULT CPortalSoftEffect::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Portal_Soft_Black"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPortalSoftEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
		return E_FAIL;

	return S_OK;
}

CPortalSoftEffect* CPortalSoftEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPortalSoftEffect* pInstance = new CPortalSoftEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPortalSoftEffect"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPortalSoftEffect::Clone(void* pArg)
{
	CPortalSoftEffect* pInstance = new CPortalSoftEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPortalSoftEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPortalSoftEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
