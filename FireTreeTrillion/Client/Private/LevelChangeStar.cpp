#include "stdafx.h"
#include "LevelChangeStar.h"

CLevelChangeStar::CLevelChangeStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CLevelChangeStar::CLevelChangeStar(const CLevelChangeStar& rhs)
	: CUIObject{ rhs }
{
}

HRESULT CLevelChangeStar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLevelChangeStar::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	fill(m_arrTextures.begin(), m_arrTextures.end(), nullptr);
	hr = Add_Components();
	CHECK_FAILED(hr);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	return S_OK;
}

_int CLevelChangeStar::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CLevelChangeStar::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CLevelChangeStar::Render()
{
	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(0);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CLevelChangeStar::Render_IMGUI()
{
}
#endif

HRESULT CLevelChangeStar::Add_Components()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

#pragma region 텍스쳐 컴포넌트
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SimpleStar"),
		TEXT("Com_Texture_AlphaStar"), (CComponent**)&m_arrTextures[0]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SimpleStar"),
		TEXT("Com_Texture_SubStar"), (CComponent**)&m_arrTextures[1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_SimpleStar"),
		TEXT("Com_Texture_LastStar"), (CComponent**)&m_arrTextures[2]);
	CHECK_FAILED(hr);
#pragma endregion

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLevelChangeStar::Bind_ShaderResources()
{
	HRESULT hr(S_OK);

	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	for (size_t i = 0; i < m_arrTextures.size(); ++i)
	{
		hr = m_arrTextures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

CLevelChangeStar* CLevelChangeStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevelChangeStar* pInstance = new CLevelChangeStar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevelChangeStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLevelChangeStar::Clone(void* pArg)
{
	CLevelChangeStar* pInstance = new CLevelChangeStar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLevelChangeStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevelChangeStar::Free()
{
	__super::Free();
	for (auto& texure : m_arrTextures)
		Safe_Release(texure);
}

