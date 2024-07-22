#include "stdafx.h"
#include "UI_Fading.h"


CUI_Fading::CUI_Fading(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_Fading::CUI_Fading(const CUI_Fading& rhs)
	: CUIObject{ rhs }
{
}

HRESULT CUI_Fading::Initialize_Prototype()
{
	

	return S_OK;
}
HRESULT CUI_Fading::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_pTransformCom->Set_Scaled(2000.f, 1000.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.f, 0.2f, 1.f));

	m_bIsRender = false;

	return S_OK;
}

_int CUI_Fading::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	m_fTimeDelta = fTimeDelta;

	return OBJ_NOEVENT;
}

void CUI_Fading::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SUPERUI, this);
}

HRESULT CUI_Fading::Render()
{
	if (m_bIsRender == false) return S_OK;

	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	_int iFade = 1;
	m_pShaderCom->Bind_RawValue("g_iFade", &iFade, sizeof(_int));

	if (m_eState == FADEOUT)
	{
		m_fFadeOutRatio -= m_fTimeDelta;
		_float fRatio = EASE_IN_SINE(m_fFadeOutRatio);
		m_pShaderCom->Bind_RawValue("g_fFadeRatio", &fRatio, sizeof(_float));

		if (m_fFadeOutRatio <= 0.f)
			m_fFadeOutRatio = 0.f;
	}
	else
	{
		m_fFadeOutRatio += m_fTimeDelta;
		_float fRatio = EASE_IN_SINE(m_fFadeOutRatio);
		m_pShaderCom->Bind_RawValue("g_fFadeRatio", &fRatio, sizeof(_float));

		if (m_fFadeOutRatio >= 1.f)
			m_fFadeOutRatio = 1.f;

	}
	//return S_OK;

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	hr = m_pMaskTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
	CHECK_FAILED(hr);
	
	hr = m_pShaderCom->Begin(POSTEX_FADEINOUT);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render(); 
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CUI_Fading::Render_IMGUI()
{
	//char ratio[16];
	////ImGui::DragFloat(ratio, (_float*)&m_fRatioTimeBar, 0.01f, 0.01f, 1.f);
	//ImGui::Separator(); ImGui::NewLine();

	//for (_int i = 0; i < m_arrPosition.size(); ++i)
	//{
	//	char name[16], size[16], color[16];
	//	sprintf_s(name,  "pos%d",   i);
	//	//sprintf_s(size,  "size%d",  i);

	//	ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
	//	//ImGui::DragFloat(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 0.1f, 2.f);

	//	m_arrSize[i].x = m_arrOriginalSize[i].x * m_arrSizeRatio[i];
	//	m_arrSize[i].y = m_arrOriginalSize[i].y * m_arrSizeRatio[i];

	//	m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
	//		XMVectorSet(m_arrPosition[i].x   - g_iWinSizeX * 0.5f,
	//					- m_arrPosition[i].y + g_iWinSizeY * 0.5f,
	//					0.f,
	//					1.f));

	//	ImGui::NewLine();
	//}

	//char test[16], test2[16];
	//ImGui::DragFloat3(test, (_float*)&m_vTESTCOLOR, 0.01f, 0.f, 1.f);
	//ImGui::DragFloat3(test2, (_float*)&m_vTESTCOLOR2, 0.01f, 0.f, 1.f);
}
#endif

HRESULT CUI_Fading::Add_Components()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
											  TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	// 마스킹 텍스쳐
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Fade"),
											  TEXT("Com_Texture_Mask"), (CComponent**)&m_pMaskTextureCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
											  TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CUI_Fading::Bind_ShaderResources()
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

CUI_Fading* CUI_Fading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Fading* pInstance = new CUI_Fading(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_Fading"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Fading::Clone(void* pArg)
{
	CUI_Fading* pInstance = new CUI_Fading(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_Fading"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Fading::Free()
{
	__super::Free();

	Safe_Release(m_pMaskTextureCom);
}

