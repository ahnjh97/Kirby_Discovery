#include "stdafx.h"
#include "SingleEffect.h"

CSingleEffect::CSingleEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEffect{ pDevice, pContext }
{
}

CSingleEffect::CSingleEffect(const CSingleEffect& rhs)
	:CEffect{ rhs }
	, m_FXDesc{ rhs.m_FXDesc }
{
}

HRESULT CSingleEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSingleEffect::Initialize_Prototype(FX_DESC FXDesc)
{
	return S_OK;
}

HRESULT CSingleEffect::Initialize(void* pArg)
{
	FX_DESC FXDesc{};

	if (pArg != nullptr)
	{
		FXDesc = *(FX_DESC*)pArg;
	}

	HRESULT hr;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);

	hr = Add_Components(FXDesc);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CSingleEffect::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CSingleEffect::Late_Tick(_float fTimeDelta)
{
	if (m_bIsColorRender)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	if (m_bIsBloom)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);

}

HRESULT CSingleEffect::Render()
{
	HRESULT hr;

	if (m_pModelCom == nullptr)
	{
		hr = Bind_ShaderResources(m_iTexIdx, m_iMaskTexIdx);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(m_iPassIdx);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}
	else
	{
		hr = Bind_ShaderResources(m_iTexIdx, m_iMaskTexIdx);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(m_iPassIdx);
		CHECK_FAILED(hr);

		hr = m_pModelCom->Render(0);
		CHECK_FAILED(hr);
	}
	return S_OK;
}

HRESULT CSingleEffect::Add_Components(FX_DESC& FXDesc)
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strTexTag),
		TEXT("Com_DiffuseTexture"), (CComponent**)&m_pTextureCom[TEX_DIFFUSE]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strMaskTexTag),
		TEXT("Com_MaskTexture"), (CComponent**)&m_pTextureCom[TEX_MASK]);
	CHECK_FAILED(hr);


	if (FXDesc.strBufferTag == "Prototype_Component_VIBuffer_Rect")
	{
		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(FXDesc.strBufferTag),
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);

		CHECK_FAILED(hr);


		hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Shader_FXPosTex"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

		CHECK_FAILED(hr);
	}
	else
	{
		hr = __super::Add_Component(*m_pCurrentLevelID, CUtils::StrToWstr(FXDesc.strBufferTag),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);

		CHECK_FAILED(hr);


		hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Shader_FXModel"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

		CHECK_FAILED(hr);
	}


	return S_OK;
}

HRESULT CSingleEffect::Bind_ShaderResources(_int iTexIdx, _int iMaskTexIdx)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	//직교일 경우, 직교 행렬 바인딩
	if (!m_bIsOrthographic)
	{
		hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW));
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ));
		CHECK_FAILED(hr);
	}


	hr = m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iTexIdx);
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", iMaskTexIdx);
	CHECK_FAILED(hr);


	return S_OK;
}

CSingleEffect* CSingleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSingleEffect* pInstance = new CSingleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSingleEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CSingleEffect* CSingleEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, FX_DESC FXDesc)
{
	CSingleEffect* pInstance = new CSingleEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(FXDesc)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSingleEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSingleEffect::Clone(void* pArg)
{
	CSingleEffect* pInstance = new CSingleEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CSingleEffect"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSingleEffect::Free()
{
	__super::Free();
}
