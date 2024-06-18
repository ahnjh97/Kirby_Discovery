#include "UIObject.h"
#include "GameInstance.h"

CUIObject::CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CUIObject::CUIObject(const CUIObject& rhs)
	: CGameObject(rhs)
	, m_ViewMatrix(rhs.m_ViewMatrix)
	, m_ProjMatrix(rhs.m_ProjMatrix)
	, m_bIsRender(rhs.m_bIsRender)
{
}

HRESULT CUIObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUIObject::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CUIObject::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CUIObject::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CUIObject::Render()
{
	return S_OK;
}

#ifdef _DEBUG
void CUIObject::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif

void CUIObject::Free()
{
	if (!m_GroupUIs.empty())
	{
		for (auto& pGroupUI : m_GroupUIs)
		{
			for (auto& pUIObj : pGroupUI)
				Safe_Release(pUIObj);
		}
		m_GroupUIs.clear();
	}

	if (!m_LayerUIs.empty())
	{
		for (auto& pUIObj : m_LayerUIs)
			Safe_Release(pUIObj);

		m_LayerUIs.clear();
	}

	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}

