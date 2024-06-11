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
	//현재는 level에서 map으로 담아 객체를 추가하고 있으므로 해당 클래스는 사용하지 않음
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* HUDUI_Desc{};
	if (nullptr != _pArg)
		HUDUI_Desc = (UIOBJ_DESC*)_pArg;

	m_UIObjDesc = (*HUDUI_Desc);

	CUIObject* pUIObj = { nullptr };

	pUIObj = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_HUD_KirbyStatus"), &m_UIObjDesc));
	m_HUDs.push_back(pUIObj);

	return S_OK;
}

_int CHUD::Tick(_float _fTimeDelta)
{
	__super::Tick(_fTimeDelta);

	if (!m_HUDs.empty())
	{
		for (auto& pUIObj : m_HUDs)
			pUIObj->Tick(_fTimeDelta);
	}

	return OBJ_NOEVENT;
}

void CHUD::Late_Tick(_float _fTimeDelta)
{
	if (!m_HUDs.empty())
	{
		for (auto& pUIObj : m_HUDs)
			pUIObj->Late_Tick(_fTimeDelta);
	}

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
	if (!m_HUDs.empty())
	{
		for (auto& pHUD : m_HUDs)
			Safe_Release(pHUD);

		m_HUDs.clear();
	}

	__super::Free();	
}


