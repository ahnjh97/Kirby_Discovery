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

HRESULT CLight_Manager::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, _bool bForTool)
{
	// ºû ¼øÈ¸¸¦ µ·´Ù.
	for (auto Light = m_Lights.begin(); Light != m_Lights.end();)
	{
		_bool	LightDead = { false };
		LightDead = static_cast<CLight*>(*Light)->Get_DeadLight();

		// ºû ÄÃ¸µ
		if ((*Light)->Compute_RenderCull())
			(*Light)->Render(pShader, pVIBuffer, bForTool);

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


#ifdef _DEBUG
void CLight_Manager::IMGUI_Tick()
{
	if (m_Lights.empty()) return;

	ImGui::Begin("LIGHT Editor");
	_int iIDX = 0;
	auto it = m_Lights.begin();
	for (auto& light : m_Lights)
	{
		LIGHT_DESC* pLightDesc;
		pLightDesc = const_cast<LIGHT_DESC*>(light->Get_LightDesc());

		// ÅÂ¾ç±¤Àº IMGUI¿¡¼­ Á¶ÀýÇÏÁö ¾Ê½À´Ï´Ù.
		if (pLightDesc->eType == LIGHT_DESC::TYPE_DIRECTIONAL)
			continue;

		// light numbering
		string strLightName = "LIGHT_" + to_string(iIDX);
		ImGui::SeparatorText(strLightName.c_str());

		// POSITION
		string strLightPos = "POSITION_" + to_string(iIDX);
		ImGui::DragFloat3(strLightPos.c_str(), &pLightDesc->vPosition.x, -200.f, 500.f);
		// RANGE
		string strLightRange = "RANGE_" + to_string(iIDX);
		ImGui::SliderFloat(strLightRange.c_str(), &pLightDesc->fRange, 0.f, 200.f);
		// DIFFUSE
		string strLightDiffuse = "DIFFUSE_" + to_string(iIDX);
		ImGui::SliderFloat3(strLightDiffuse.c_str(), &pLightDesc->vDiffuse.x, 0.f, 1.f);
		// AMBIENT
		string strLightAmbient = "AMBIENT_" + to_string(iIDX);
		ImGui::SliderFloat3(strLightAmbient.c_str(), &pLightDesc->vAmbient.x, 0.f, 1.f);
		// SPECULAR
		string strLightSpecular = "SPECULAR_" + to_string(iIDX);
		ImGui::SliderFloat3(strLightSpecular.c_str(), &pLightDesc->vSpecular.x, 0.f, 1.f);
		ImGui::NewLine(); ImGui::Separator();

		++iIDX;
	}

	ImGui::End();
}
#endif

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



