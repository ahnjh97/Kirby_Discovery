#include "stdafx.h"
#include "LoadingStart.h"

CLoadingStart::CLoadingStart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIObject{ pDevice, pContext }
{
}

CLoadingStart::CLoadingStart(const CLoadingStart& rhs)
    : CUIObject{ rhs }
{
}

HRESULT CLoadingStart::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLoadingStart::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_fAlpha = 0.f;

	return S_OK;
}

_int CLoadingStart::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CLoadingStart::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SUPERUI, this);
}

HRESULT CLoadingStart::Render()
{
	HRESULT hr;
	hr = Bind_ShaderResources(m_fAlpha);
	CHECK_FAILED(hr);

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_DIFFUSE_1]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture");
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_MASK_1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture");
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(26);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoadingStart::Render(_float fTimeDelta)
{
	m_fAlpha += fTimeDelta * 2.f;
	if (1.f < m_fAlpha)
	{
		m_fAlpha = 1.f;
	}

	Render_Again(327.f, 45.f, 0.f, -50.f);

	m_pTransformCom->Set_Scaled(396.f, 396.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, 0.1f, 1.f));

	HRESULT hr;
	hr = Bind_ShaderResources(m_fAlpha);
	CHECK_FAILED(hr);

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_DIFFUSE_1]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iTexIndex);
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_MASK_1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture");
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(26);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CLoadingStart::Render_IMGUI()
{

}
#endif

HRESULT CLoadingStart::Add_Components()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Texture_Diffuse */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_LoadingStart"),
		TEXT("Com_Texture_Diffuse_1"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE_1]);
	CHECK_FAILED(hr);

	/* For.Com_Texture_Diffuse */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_LoadingStartSide"),
		TEXT("Com_Texture_Diffuse_2"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE_2]);
	CHECK_FAILED(hr);

	/* For.Com_Texture_Mask */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Mask_RoundMask"),
		TEXT("Com_Texture_Mask_1"), (CComponent**)&m_pTextureCom[TYPE_MASK_1]);
	CHECK_FAILED(hr);

	/* For.Com_Texture_Mask */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_Mask_PillarMask"),
		TEXT("Com_Texture_Mask_2"), (CComponent**)&m_pTextureCom[TYPE_MASK_2]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoadingStart::Bind_ShaderResources(_float fAlpha)
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);
	CHECK_FAILED(hr);

	//hr = m_pShaderCom->Bind_RawValue("g_fTimeDelta", &m_fTimeDelta, sizeof(_float));
	//CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoadingStart::Render_Again(_float fSizeX, _float fSizeY, _float fPosX, _float fPosY)
{
	m_pTransformCom->Set_Scaled(fSizeX * 3.f, fSizeY * 2.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, -50.f, 0.1f, 1.f));

	HRESULT hr;
	hr = Bind_ShaderResources(m_fAlpha);
	CHECK_FAILED(hr);

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_DIFFUSE_2]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iTexIndex);
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TYPE_MASK_2]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 2);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(26);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

CLoadingStart* CLoadingStart::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLoadingStart* pInstance = new CLoadingStart(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLoadingStart"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLoadingStart::Clone(void* pArg)
{
	CLoadingStart* pInstance = new CLoadingStart(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLoadingStart"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoadingStart::Free()
{
	__super::Free();

	for (size_t i = 0; i < TYPE_END; i++)
		Safe_Release(m_pTextureCom[i]);
}
