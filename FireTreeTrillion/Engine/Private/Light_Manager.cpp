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
	return;

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
		{
			ImGui::SeparatorText("Directional Light");
		}

		// light numbering
		string strLightName = "LIGHT_" + to_string(iIDX);
		ImGui::SeparatorText(strLightName.c_str());

		if (pLightDesc->eType != LIGHT_DESC::TYPE_DIRECTIONAL)
		{
			// POSITION
			string strLightPos = "POSITION_" + to_string(iIDX);
			ImGui::DragFloat3(strLightPos.c_str(), &pLightDesc->vPosition.x, .05f, -200.f, 500.f, "%.2f");
			// RANGE
			string strLightRange = "RANGE_" + to_string(iIDX);
			ImGui::DragFloat(strLightRange.c_str(), &pLightDesc->fRange, .05f, 0.f, 200.f, "%.2f");
		}

		// DIFFUSE
		string strLightDiffuse = "DIFFUSE_" + to_string(iIDX);
		ImGui::DragFloat3(strLightDiffuse.c_str(), &pLightDesc->vDiffuse.x, .01f, 0.f, 1.f, "%.2f");
		// AMBIENT
		string strLightAmbient = "AMBIENT_" + to_string(iIDX);
		ImGui::DragFloat3(strLightAmbient.c_str(), &pLightDesc->vAmbient.x, .01f, 0.f, 1.f, "%.2f");
		// SPECULAR
		string strLightSpecular = "SPECULAR_" + to_string(iIDX);
		ImGui::DragFloat3(strLightSpecular.c_str(), &pLightDesc->vSpecular.x, .01f, 0.f, 1.f, "%.2f");
		ImGui::NewLine(); ImGui::Separator();

		++iIDX;
	}

	ImGui::End();
}
#endif

void CLight_Manager::Blink_Light(_float fTimeDelta, _uint iLightNum, _float fRandomSpeed)
{
	auto& iter = m_Lights.begin();
	_int index = 0;
	for (; iter != m_Lights.end(); ++iter)
	{
		if (index == iLightNum)
		{
			break;
		}
		++index;
	}

	LIGHT_DESC* pDesc = (*iter)->Get_LightDesc();
	static _float fOriginRange = pDesc->fRange;
	static _bool bOnce = false;
	
	if (false == bOnce)
	{
		m_eState = SIZE_DOWN;
		bOnce = true;
	}

	switch (m_eState)
	{
	case SIZE_UP:
	{
		if (pDesc->fRange < fOriginRange)
			pDesc->fRange += fTimeDelta * fRandomSpeed;
		else
		{
			pDesc->fRange = fOriginRange;
			m_eState = SIZE_DOWN;
		}
	}
	break;
	case SIZE_DOWN:
	{
		if (pDesc->fRange <= 0.5f)
		{
			pDesc->fRange = 0.5f;
			m_eState = SIZE_UP;
		}
		else
			pDesc->fRange -= fTimeDelta * fRandomSpeed;
	}
	break;
	}
}

void CLight_Manager::Set_CurLightRange(_uint iLightNum, _float fRange)
{
	auto& iter = m_Lights.begin();
	_int index = 0;
	for (; iter != m_Lights.end(); ++iter)
	{
		if (index == iLightNum)
		{
			break;
		}
		++index;
	}

	LIGHT_DESC* pDesc = (*iter)->Get_LightDesc();
	pDesc->fRange = fRange;
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



