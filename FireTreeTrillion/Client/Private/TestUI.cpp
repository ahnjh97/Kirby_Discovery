#include "stdafx.h"
#include "TestUI.h"

#include "Utils.h"

CTestUI::CTestUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIObject{ pDevice, pContext }
{
}

CTestUI::CTestUI(const CTestUI& rhs)
    : CUIObject(rhs)
{
}

HRESULT CTestUI::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTestUI::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_size2D		= _float2(100.f, 100.f);
	m_position2D	= _float2(100.f, 100.f);
	m_WindowSize2D	= _float2(g_iWinSizeX, g_iWinSizeY);

	m_pTransformCom->Set_Scaled(m_size2D.x, m_size2D.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_position2D.x - m_WindowSize2D.x * 0.5f,
			-m_position2D.y + m_WindowSize2D.y * 0.5f,
			0.f,
			1.f));

	
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_WindowSize2D.x, m_WindowSize2D.y, 0.f, 1.f));

	return S_OK;
}

_int CTestUI::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CTestUI::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
	
}

HRESULT CTestUI::Render()
{
	//if (!m_bIsRender)
	//	return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CTestUI::Render_IMGUI()
{
	char name[12], name2[12];
	sprintf_s(name, "pos");
	ImGui::DragFloat2(name, (_float*)&m_position2D);
	sprintf_s(name2, "size");
	ImGui::DragFloat2(name2, (_float*)&m_size2D);

	m_pTransformCom->Set_Scaled(m_size2D.x, m_size2D.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_position2D.x - m_WindowSize2D.x * 0.5f,
			-m_position2D.y + m_WindowSize2D.y * 0.5f,
			0.f,
			1.f));

	// 여기서 회전하는 부분 수정해야합니다.
	//static _float  Z_radian = 0.f;
	//ImGui::DragFloat("Z_radian", &Z_radian, 0.f, 360.f);
	//m_pTransformCom->Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMConvertToRadians(Z_radian));
}
#endif

HRESULT CTestUI::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Texture */

	// 05.20) 테스트
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Logo"),
	//hr = __super::Add_Component(LEVEL_TOOL_UI, TEXT("Prototype_Component_Texture_Logo"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
	CHECK_FAILED(hr);

	/* For.Com_VIBuffer */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CTestUI::Bind_ShaderResources()
{
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);

	return S_OK;
}

CTestUI* CTestUI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTestUI* pInstance = new CTestUI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTestUI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTestUI::Clone(void* pArg)
{
	CTestUI* pInstance = new CTestUI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTestUI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTestUI::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}


