#include "stdafx.h"
#include "UI_Editor.h"

#include "ImGuizmo.h"

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
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CUI_Editor::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_Editor::Render()
{

#pragma region BIND SHADER & VIBUFFER

	/*
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;
	*/

#pragma endregion 

	return S_OK;
}

void CUI_Editor::Render_IMGUI()
{
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.5f);
	m_pGameInstance->RenderGrid();

	//ImGui::SetNextWindowPos(ImVec2(0, 100.f));
	//ImGui::SetNextWindowSize(ImVec2(250, 200));
	if (ImGui::Begin(u8"UI Editor 에디터", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse)) 
		//ImGuiWindowFlags_NoResize
	{
		// 테스트용
		/*
		if (ImGui::CollapsingHeader(u8"Mouse 마우스", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGuiIO& io = ImGui::GetIO();

			ImGui::SeparatorText(u8"Mouse");
			if (ImGui::IsMousePosValid())
				ImGui::Text("X : %.2f \t Y : %.2f", io.MousePos.x, io.MousePos.y);

			else
				ImGui::Text("Mouse InValid");
		}
		*/

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu(u8"File 파일"))
			{
				if (ImGui::MenuItem(u8"Load 로드"))
				{

				}

				if (ImGui::MenuItem(u8"Save 저장"))
				{

				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (ImGui::BeginTabBar(u8"Test TabBar 1"))
		{
			if (ImGui::BeginTabItem(u8"Test Tab Item 1"))
			{
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End(); //창 종료
	}

	if (ImGui::Begin(u8"Test Tab 테스트 탭", 0, ImGuiWindowFlags_NoCollapse))
	{
		if (ImGui::BeginTabBar(u8"Test TabBar 1")) //탭 바
		{
			if (ImGui::BeginTabItem(u8"Test Tab Item 1"))
			{
				ImGui::SeparatorText(u8"Transform 오브젝트 변환");
				
				ImGui::Text(u8"Ctrl S : 크기 | Ctrl R : 회전 | Ctrl T : 위치");
				Edit_Transform();

				//텍스처 선택 시 기즈모 활성화
				//_bool IsUsingGizmo = { FALSE };
				//if (TRUE == IsUsingGizmo)
				//{
				//}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(u8"Test Tab Item 2"))
			{
				ImGui::SeparatorText(u8"Test SeparatorText");
				ImGui::Text(u8"Test Text");


				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End(); //창 종료
	}

	//if (ImGui::Begin(u8"Viewport"))
	//{
	//	ImGui::End();
	//}

	if (ImGui::Begin(u8"Texture Preview 텍스처 미리보기", 0, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
	}

	//ImGui::PopStyleVar();
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

_bool CUI_Editor::Edit_Transform()
{
	const char* DragTag = { "Translate 위치" };
	_float fTextWidth = ImGui::CalcTextSize(DragTag).x;

	ImGuizmo::BeginFrame(); //기즈모 생성
	_float4x4 WorldMatrix = m_pTransformCom->Get_WorldFloat4x4();
	if (nullptr == m_pTransformCom)
		return FALSE;

	static ImGuizmo::OPERATION eCurGizmoOper(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE eCurGizmoMode(ImGuizmo::WORLD);

	//크기 회전 이동 변경 키
	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::SCALE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_R, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::ROTATE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_T, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::TRANSLATE;
	}

	if (!ImGuizmo::IsUsing())
		ImGui::Text(u8"Gizmo InValid");

	else
	{
		switch (eCurGizmoOper)
		{
		case ImGuizmo::SCALE:
			ImGui::Text(u8"Scale Edit");
			break;

		case ImGuizmo::ROTATE:
			ImGui::Text(u8"Rotate Edit");
			break;

		case ImGuizmo::TRANSLATE:
			ImGui::Text(u8"Translate Edit");
			break;
		}
	}

	//행렬 분해 후 재구성
	_float Translate[3], Rotate[3], Scale[3];

	ImGuizmo::DecomposeMatrixToComponents(WorldMatrix.m[0], Translate, Rotate, Scale);
	ImGui::Text(u8"Scale 크기");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Scale", (_float*)&Scale);


	ImGui::Text(u8"Rotate 회전");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Rotate", (_float*)&Rotate);

	ImGui::Text(u8"Translate 위치");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Translate", (_float*)&Translate);
	ImGuizmo::RecomposeMatrixFromComponents(Translate, Rotate, Scale, WorldMatrix.m[0]);

	//기즈모 영역 세팅
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	//뷰, 투영 행렬 정보 로드
	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	static _bool useSnap(false);
	_float3 snap = _float3();


	//오브젝트 변환
	ImGuizmo::Manipulate(ViewMatrix.m[0], ProjMatrix.m[0], eCurGizmoOper, eCurGizmoMode,
		WorldMatrix.m[0], useSnap ? &snap.x : NULL);

	//월드행렬 세팅
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);

	return TRUE;
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

	//Safe_Release(m_pShaderCom);
	//Safe_Release(m_pTextureCom);
	//Safe_Release(m_pVIBufferCom);
}
