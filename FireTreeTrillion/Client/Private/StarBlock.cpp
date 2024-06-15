#include "stdafx.h"
#include "StarBlock.h"

CStarBlock::CStarBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMapObject{ pDevice, pContext }
{
}

CStarBlock::CStarBlock(const CStarBlock& rhs)
	: CMapObject( rhs )
{
}

HRESULT CStarBlock::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStarBlock::Initialize(void* pArg)
{
	STARBLOCK_DESC tStarBlcokDesc{};
	if (nullptr != pArg) {
		tStarBlcokDesc = *(STARBLOCK_DESC*)pArg;
		m_eSize = tStarBlcokDesc.eSize;
	}

	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	//fill(m_arrModelCom.begin(), m_arrModelCom.end(), nullptr);

	Add_Components();
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(-3.f, 7.f, -188.f, 1.f));

	return S_OK;
}

_int CStarBlock::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CStarBlock::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CStarBlock::Render()
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

HRESULT CStarBlock::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CStarBlock::Render_IMGUI()
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

HRESULT CStarBlock::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	switch (m_eSize)
	{
	case SMALL:
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockS"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	break;
	case MEDIUM:
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockM"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	break;
	case LARGE:
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockL"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	break;
	}

	return S_OK;
}

HRESULT CStarBlock::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CStarBlock* CStarBlock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStarBlock* pInstance = new CStarBlock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CStarBlock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStarBlock::Clone(void* pArg)
{
	CStarBlock* pInstance = new CStarBlock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CStarBlock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStarBlock::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}

