#include "stdafx.h"
#include "UI_Editor.h"

CUI_Editor::CUI_Editor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CUI_Editor::CUI_Editor(const CUI_Editor& rhs)
	: CUIObject{ rhs }
{
}

HRESULT CUI_Editor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Editor::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;


	return S_OK;
}

_int CUI_Editor::Tick(_float fTimeDelta)
{
	//IMGUI 프레임 생성
	ImGui::NewFrame();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.5f);

	ImGui::SetNextWindowSize(ImVec2(250, 200));
	if (ImGui::Begin(u8"Editor 에디터", NULL, /*ImGuiWindowFlags_NoCollapse | */ ImGuiWindowFlags_NoResize)) //창 제목
	{
		if (ImGui::CollapsingHeader(u8"Mouse 마우스", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO& io = ImGui::GetIO();

			ImGui::SeparatorText(u8"Mouse");
			if (ImGui::IsMousePosValid())
				ImGui::Text("X : %.2f \t Y : %.2f", io.MousePos.x, io.MousePos.y);

			else
				ImGui::Text("Mouse InValid");
		}
	}
	ImGui::End(); //창 종료


	ImGui::PopStyleVar();
	ImGui::EndFrame(); //현재 프레임 종료

	return OBJ_NOEVENT;
}

void CUI_Editor::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_Editor::Render()
{
	//설정 값으로 렌더
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

#pragma region BIND SHADER & VIBUFFER

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#pragma endregion 

	return S_OK;
}

void CUI_Editor::Render_IMGUI()
{
}

HRESULT CUI_Editor::Add_Components()
{
	return S_OK;
}

HRESULT CUI_Editor::Bind_ShaderResources()
{
	HRESULT hr;

	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0);

	return S_OK;
}

CUI_Editor* CUI_Editor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Editor* pInstance = new CUI_Editor(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : UI_Editor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Editor::Clone(void* pArg)
{
	CUI_Editor* pInstance = new CUI_Editor(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : UI_Editor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Editor::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
