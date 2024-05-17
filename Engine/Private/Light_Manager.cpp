#include "Light_Manager.h"
#include "Light.h"

CLight_Manager::CLight_Manager()
{

}

const LIGHT_DESC * CLight_Manager::Get_LightDesc(_uint iIndex)
{
	if (iIndex >= m_Lights.size())
		return nullptr;

	auto	iter = m_Lights.begin();

	for (_uint i = 0; i < iIndex; ++i)
		++iter;

	return (*iter)->Get_LightDesc();	
}

HRESULT CLight_Manager::Initialize()
{
	return S_OK;
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC & LightDesc)
{
	CLight*		pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.emplace_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	// ºû ¼øÈ¸¸¦ µ·´Ù.
	for (auto Light = m_Lights.begin(); Light != m_Lights.end();)
	{
		_bool	LightDead = { false };
		LightDead = static_cast<CLight*>(*Light)->Get_DeadLight();

		// ºû ÄÃ¸µ
		if ((*Light)->Compute_RenderCull())
			(*Light)->Render(pShader, pVIBuffer);

		// ºû »çÇü ¼±°í
		if (LightDead == true)
		{
			Safe_Release(*Light);
			Light = m_Lights.erase(Light);
		}
		else
			Light++;
	}
	return S_OK;
}

void CLight_Manager::Clear_Light()
{
	// ¸ðµç ºûÀ» Á×ÀÎ´Ù.
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();
}

CLight_Manager * CLight_Manager::Create()
{
	CLight_Manager*		pInstance = new CLight_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CLight_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLight_Manager::Free()
{
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();
}



