#include "stdafx.h"
#include "HUD.h"

CHUD::CHUD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CUIObject{ pDevice, pContext }
{
}

CHUD::CHUD(const CHUD& rhs)
    : CUIObject(rhs)
{
}

HRESULT CHUD::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CHUD::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* HUDUI_Desc{};
	if (nullptr != _pArg)
		HUDUI_Desc = (UIOBJ_DESC*)_pArg;

	m_UIObjDesc = (*HUDUI_Desc);

	return S_OK;
}

_int CHUD::Tick(_float _fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CHUD::Late_Tick(_float _fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD::Render()
{
	return S_OK;
}

#ifdef _DEBUG
void CHUD::Render_IMGUI()
{
}
#endif

CHUD* CHUD::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD* pInstance = new CHUD(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CHUD"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD::Clone(void* pArg)
{
	CHUD* pInstance = new CHUD(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD::Free()
{
	__super::Free();	
}


