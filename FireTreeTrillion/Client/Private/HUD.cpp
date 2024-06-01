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

HRESULT CHUD::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	//CUIObject::UIOBJ_DESC HUD_KirbyDESC{};
	//HUD_KirbyDESC.wstrUITag = { TEXT("HUD_Kirby") };
	//HUD_KirbyDESC.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	//HUD_KirbyDESC.vSize = { 100.f, 100.f };
	//HUD_KirbyDESC.vPos = { HUD_KirbyDESC.vCenter.x/* - 200.f*/,
	//						HUD_KirbyDESC.vCenter.y/* - 200.f */ };
	//HUD_KirbyDESC.fDegree = { 0.f };
	//HUD_KirbyDESC.iTexIndex = { 0 };

	//CUIObject* pUIObj = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_HUD_Kirby"), &HUD_KirbyDESC));
	//CHECK_NULLPTR(pUIObj);
	//m_vecSingleUI.push_back(pUIObj);

	return S_OK;
}

_int CHUD::Tick(_float _fTimeDelta)
{		
	return OBJ_NOEVENT;
}

void CHUD::Late_Tick(_float _fTimeDelta)
{
	//for (auto& pUIObj : m_vecSingleUI)
	//	pUIObj->Late_Tick(_fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD::Render()
{
	return S_OK;
}

void CHUD::Render_IMGUI()
{
}

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


