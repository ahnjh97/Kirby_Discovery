#include "stdafx.h"
#include "Multi_UI.h"

CMulti_UI::CMulti_UI(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CMulti_UI::CMulti_UI(const CMulti_UI& _rhs)
    : CUIObject {_rhs}
{
}

HRESULT CMulti_UI::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMulti_UI::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC MultiUI_Desc{};
	MultiUI_Desc.eUIType = { TYPE_MULTI };
	MultiUI_Desc.wstrUITag = { TEXT("Multi_UI") };
	MultiUI_Desc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	MultiUI_Desc.vSize = { 100.f, 100.f };
	MultiUI_Desc.vPos = { MultiUI_Desc.vCenter.x/* - 200.f*/,
							MultiUI_Desc.vCenter.y/* - 200.f */ };
	MultiUI_Desc.fDegree = { 0.f };
	MultiUI_Desc.iTexIndex = { 0 };

	CUIObject* pUIObj = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Single_UI"), &MultiUI_Desc));
	CHECK_NULLPTR(pUIObj);
	m_MultiUIs.push_back(pUIObj);

#pragma endregion

	return S_OK;
}

_int CMulti_UI::Tick(_float _fTimeDelta)
{	
	__super::Tick(_fTimeDelta);

	if (!m_MultiUIs.empty())
	{
		for (auto& pUIObj : m_MultiUIs)
			pUIObj->Tick(_fTimeDelta);
	}

	return OBJ_NOEVENT;
}

void CMulti_UI::Late_Tick(_float _fTimeDelta)
{
	if (!m_MultiUIs.empty())
	{
		for (auto& pUIObj : m_MultiUIs)
			pUIObj->Late_Tick(_fTimeDelta);
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CMulti_UI::Render()
{
	//for (auto& pUIObj : m_MultiUIs)
	//	pUIObj->Render();

	return S_OK;
}

void CMulti_UI::Render_IMGUI()
{
}

HRESULT CMulti_UI::Add_Components()
{
	return S_OK;
}

HRESULT CMulti_UI::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex)
{
	return S_OK;
}

HRESULT CMulti_UI::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	return S_OK;
}

CMulti_UI* CMulti_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMulti_UI* pInstance = new CMulti_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CMulti_UI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMulti_UI::Clone(void* pArg)
{
	CMulti_UI* pInstance = new CMulti_UI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CMulti_UI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMulti_UI::Free()
{
	__super::Free();

	for (auto& pUIObj : m_MultiUIs)
		Safe_Release(pUIObj);

	m_MultiUIs.clear();
}


