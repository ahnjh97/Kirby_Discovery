#include "stdafx.h"
#include "LoadingFont.h"
#include "TransingStar.h"

CLoadingFont::CLoadingFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CLoadingFont::CLoadingFont(const CLoadingFont& rhs)
	: CUIObject{ rhs }
{
}

HRESULT CLoadingFont::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLoadingFont::Initialize(void* pArg)
{
	LOADINGFONT_DESC* pLoadingFontDesc = nullptr;

	if (nullptr != pArg)
	{
		pLoadingFontDesc = (LOADINGFONT_DESC*)pArg;

		m_bDeadRender = pLoadingFontDesc->bDeadRender;
		m_iTexIndex = pLoadingFontDesc->iTexIndex;
		m_fPosX = pLoadingFontDesc->fPosX;
		m_fEndPos = pLoadingFontDesc->fEndPos;
		m_fDisappearPos = pLoadingFontDesc->fDisappearPos;
		m_strTag = pLoadingFontDesc->strTag;
	}

	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_pTransformCom->Set_Scaled(128.f * 0.5f, 128.f * 0.5f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fPosX, -50.f, 0.f, 1.f));

	m_fAlpha = 0.f;

	return S_OK;
}

_int CLoadingFont::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	CTransingStar* pTransingStar = static_cast<CTransingStar*>(m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_TransingStar")));

	_float fPos = { 0.f };
	if (true == pTransingStar->Get_FontRender())
	{
		if (nullptr != pTransingStar)
		{
			m_fRatio -= fTimeDelta * 1.5f;
			if (0.f < m_fRatio)
			{
				fPos = Compute_Easing(m_fEndPos, m_fDisappearPos, 1.f - m_fRatio);
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(fPos, -50.f, 0.f, 1.f));
			}
			else
			{
				if(true == m_bDeadRender)
					pTransingStar->Set_FontRender(false);
				m_fRatio = 0.f;
				m_bDead = true;
			}
		}
	}
	else if (1.f > m_fRatio)
	{
		m_fRatio += fTimeDelta * 0.5f;
		fPos = Compute_Easing(m_fPosX, m_fEndPos, m_fRatio);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(fPos, -50.f, 0.f, 1.f));
	}
	else
		m_fRatio = 1.f;

	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CLoadingFont::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SUPERUI, this);
}

HRESULT CLoadingFont::Render()
{
	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	hr = m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iTexIndex);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(12);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoadingFont::Render(_float fTimeDelta)
{
	return S_OK;
}

#ifdef _DEBUG
void CLoadingFont::Render_IMGUI()
{
}
#endif

HRESULT CLoadingFont::Add_Components()
{
	HRESULT hr(S_OK);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Texture_Diffuse */
	hr = __super::Add_Component(LEVEL_STATIC, m_strTag/*TEXT("Prototype_Component_Texture_UI_Forest_Font")*/,
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
	CHECK_FAILED(hr);


	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoadingFont::Bind_ShaderResources()
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	hr = m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);
	CHECK_FAILED(hr);

	_float3 vColor = _float3( 1.f, 1.f ,1.f );
	hr = m_pShaderCom->Bind_RawValue("g_vRColor", &vColor, sizeof(_float3));
	CHECK_FAILED(hr);

	_float fAlpha = m_fRatio + 0.1f;
	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	return S_OK;
}

_float CLoadingFont::Compute_Easing(_float vStartPos, _float vEndPos, _float fRatio)
{

	_float t = 1 - (_float)pow(1 - fRatio, 3);

	_float vResult = vStartPos + (vEndPos - vStartPos) * t;

	return vResult;
}

CLoadingFont* CLoadingFont::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLoadingFont* pInstance = new CLoadingFont(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLoadingFont"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLoadingFont::Clone(void* pArg)
{
	CLoadingFont* pInstance = new CLoadingFont(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLoadingFont"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoadingFont::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
}
