#include "..\Public\Level.h"
#include "GameInstance.h"

CLevel::CLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);

}

HRESULT CLevel::Initialize()
{
	m_iLevel = *m_pGameInstance->Get_CurrentLevelID();
	return S_OK;
}

void CLevel::Tick(_float fTimeDelta)
{
}

HRESULT CLevel::Render()
{
	return S_OK;
}

HRESULT CLevel::Add_Component(const wstring& strPrototypeTag, const wstring& strComponentTag, CComponent** ppOut)
{
	CComponent* pComponent = m_pGameInstance->Clone_Component(*m_pGameInstance->Get_CurrentLevelID(), strPrototypeTag);
	if (nullptr == pComponent)
		return E_FAIL;

	*ppOut = pComponent;

	return S_OK;
}

void CLevel::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
