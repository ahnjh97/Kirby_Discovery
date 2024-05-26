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

	UIOBJ_DESC LOGO_DESC{};
	LOGO_DESC.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	LOGO_DESC.vSize = { 10.f, 10.f };
	LOGO_DESC.vPos = { LOGO_DESC.vCenter.x /*- 200.f*/, LOGO_DESC.vCenter.y /*- 200.f*/ };

	m_pTransformCom->Set_Scaled(LOGO_DESC.vSize.x, LOGO_DESC.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
					LOGO_DESC.vCenter.x - LOGO_DESC.vPos.x,
					-LOGO_DESC.vCenter.y + LOGO_DESC.vPos.y, 0.f, 1.f));

	//XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	//XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	return S_OK;
}

_int CUI_Editor::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	Set_OrthoProj();

	return OBJ_NOEVENT;
}

void CUI_Editor::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_Editor::Render()
{

#pragma region BIND SHADER & VIBUFFER

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

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
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.5f);
	// m_pGameInstance->RenderGrid();

#pragma region IMGUI GIZMO CUSTOM

	// IMGUI Gizmo Grid 커스텀 (X/Y 2D 좌표계용)
	static const float MatGridX[16] =
	{ 1.f, 0.f,  0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	ImGuizmo::DrawGrid(ViewMatrix.m[0], ProjMatrix.m[0], MatGridX, 1000.f);

#pragma endregion
	// 도킹 모드는 크기/위치 고정 시 도킹 불가
	//ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));
	//ImGui::SetNextWindowSize(ImVec2(iSizeX, iSizeY));
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
			if (ImGui::TreeNode(u8"Test TreeNode"))
			{
				for (int i = 0; i < 3; i++)
				{
					if (i == 0)
						ImGui::SetNextItemOpen(true, ImGuiCond_Once);

					ImGui::PushID(i);
					if (ImGui::TreeNode("", "Child %d", i))
					{
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
				ImGui::TreePop();
			}

			if (ImGui::BeginTabItem(u8"Test Tab Item 1"))
			{
				ImGui::PushItemWidth(200);

				ImGui::Text("Lists:");
				const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD" };
				static int selection[4] = { 0, 1, 2, 3 };

				//ImGui::PushID();
				ImGui::ListBox("", &selection[0], items, IM_ARRAYSIZE(items));
				//ImGui::PopID();
				ImGui::SetItemTooltip("ListBox %d hovered", 0);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End(); //창 종료
	}

	if (ImGui::Begin(u8"Properties 속성", 0, ImGuiWindowFlags_NoCollapse))
	{
		if (ImGui::BeginTabBar(u8"Test TabBar 1")) //탭 바
		{
			if (ImGui::BeginTabItem(u8"Transform 변환"))
			{
				//텍스처 선택 시 기즈모 활성화
				//_bool IsUsingGizmo = { FALSE };
				//if (TRUE == IsUsingGizmo)
				//{
				//}

				Edit_Transform();
				Edit_RGBAColor();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(u8"Animation 애니메이션"))
			{
				ImGui::SeparatorText(u8"애니메이션 제어");
				ImGui::Text(u8"Test Text");

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End(); //창 종료
	}

	if (ImGui::Begin(u8"ViewPort", 0, ImGuiWindowFlags_NoCollapse))
	{
		// 샘플 코드
		static ImVector<ImVec2> points;
		static ImVec2 scrolling(0.0f, 0.0f);
		static bool opt_enable_grid = true;
		static bool opt_enable_context_menu = true;
		static bool adding_line = false;

		// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
		ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
		ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
		if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
		if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
		ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

		// Draw border and background color
		ImGuiIO& io = ImGui::GetIO();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
		draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

		// Draw grid + all lines in the canvas
		draw_list->PushClipRect(canvas_p0, canvas_p1, true);
		if (opt_enable_grid)
		{
			const float GRID_STEP = 64.0f;
			for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
				draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
			for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
				draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
		}
		draw_list->PopClipRect();

		ImGui::End();
	}

	if (ImGui::Begin(u8"Preview 미리보기", 0, ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
	}

	//ImGui::PopStyleVar();
}

HRESULT CUI_Editor::Add_Components()
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Texture */
	hr = __super::Add_Component(LEVEL_TOOL_UI, TEXT("Prototype_Component_Texture_Logo"),
		//hr = __super::Add_Component(LEVEL_TOOL_UI, TEXT("Prototype_Component_Texture_Logo"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
	CHECK_FAILED(hr);

	/* For.Com_VIBuffer */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CUI_Editor::Bind_ShaderResources()
{
	HRESULT hr = S_OK;

	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	// 05.24) 카메라 줌인/아웃용 Matrix 처리
	_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix();
	_float4x4 ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	_float4x4 ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjMatrix)))
		return E_FAIL;

	m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);

	return S_OK;
}

_bool CUI_Editor::Edit_Transform()
{
	ImGui::SeparatorText(u8"Transform Edit 상태 편집");

#pragma region IMGUI_GIZMO

	const char* DragTag = { "Translate 위치" };
	_float fTextWidth = ImGui::CalcTextSize(DragTag).x;

	ImGuizmo::BeginFrame(); //기즈모 생성
	_float4x4 WorldMatrix = m_pTransformCom->Get_WorldFloat4x4();
	if (nullptr == m_pTransformCom)
		return FALSE;

	static ImGuizmo::OPERATION eCurGizmoOper(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y);
	static ImGuizmo::MODE eCurGizmoMode(ImGuizmo::WORLD);

	
	// 기즈모 사용여부 텍스트
	if (!ImGuizmo::IsUsing())
		ImGui::Text(u8"Gizmo InValid");

	else
	{
		switch (eCurGizmoOper)
		{
		case ImGuizmo::SCALE:
			ImGui::Text(u8"Scale Edit");
			break;

		case ImGuizmo::ROTATE_Z:
			ImGui::Text(u8"Rotate Edit");
			break;

		case ImGuizmo::TRANSLATE: case ImGuizmo::TRANSLATE_X: case ImGuizmo::TRANSLATE_Y:
			ImGui::Text(u8"Translate Edit");
			break;
		}
	}

	// 기즈모 키 입력 시 기능 스왑
	ImGui::Text(u8"Ctrl S : 크기 / Ctrl R : 회전 / Ctrl T : 위치");

	// 크기 회전 이동 변경 키
	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::SCALE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_R, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::ROTATE_Z;

		else if (m_pGameInstance->Get_DIKeyState(DIK_T, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y;
	}
	
	// 기즈모 드래그 위젯 연동
	_float Translate[3], Rotate[3], Scale[3];
	ImGuizmo::DecomposeMatrixToComponents(WorldMatrix.m[0], Translate, Rotate, Scale);

	ImGui::Text(u8"Scale 크기");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Scale", (_float*)Scale, 1.f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Translate 위치");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Translate", (_float*)&Translate, 1.f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Rotate 회전");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat("##Rotate", (_float*)&Rotate[2], 1.f, (_int)-360, (_int)360, u8"Degree 각도 : %.0f");

	ImGuizmo::RecomposeMatrixFromComponents(Translate, Rotate, Scale, WorldMatrix.m[0]);

	// 기즈모 영역 세팅
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y);

	// 뷰, 투영 행렬 정보 로드
	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	//static _bool useSnap(false);
	_float fGizmoSpeed[3] = { 
		0.1f,		//Translate
		0.1f,		//Rotate
		0.1f };	//Scale

	//오브젝트 변환
	ImGuizmo::Manipulate(ViewMatrix.m[0], ProjMatrix.m[0], eCurGizmoOper, eCurGizmoMode, WorldMatrix.m[0], NULL, fGizmoSpeed);
	/*useSnap ? &snap.x : NULL*/

	//월드행렬 세팅
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);

	m_UIObjDesc.vSize *= (_float2)Scale;
	m_UIObjDesc.vPos *= (_float2)Translate;
	

#pragma endregion

	//m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_UIObjDesc.vPos.x, m_UIObjDesc.vPos.y, 0.f, 1.f));

	return TRUE;
}

_bool CUI_Editor::Edit_RGBAColor()
{
	ImGui::SeparatorText(u8"Color Edit 색상 편집");

	static ImVec4 color = ImVec4(
		(127.0f / 255.0f) / 1.f, 
		127.0f / 255.0f, 
		127.0f / 255.0f, 
		127.0f / 255.0f);

	// Generate a default palette. The palette will persist and can be edited.
	static _bool saved_palette_init = true;
	static ImVec4 saved_palette[32] = {};
	if (saved_palette_init)
	{
		for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
		{
			ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
				saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
			saved_palette[n].w = 1.0f; // Alpha
		}
		saved_palette_init = FALSE;
	}
	ImGuiColorEditFlags ColorButton_Flags = ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip ;
	ImGuiColorEditFlags ColorEdit_Flags = ImGuiColorEditFlags_NoSmallPreview;

	ImGui::ColorButton("MyColor##3c", *(ImVec4*)&color, ColorButton_Flags, ImVec2(50, 50));
	ImGui::SameLine();

	ImGui::PushItemWidth(225.f);
	ImGui::ColorEdit4("##ColorEdit", (_float*)&color, ColorEdit_Flags);
	ImGui::PopItemWidth();


	return TRUE;
}

_bool CUI_Editor::Set_OrthoProj()
{
	// 05.24) 직교투영 스페이스 변환
	_float4x4 WorldMatrix, ViewMatrix, ProjMatrix;
	ViewMatrix = m_pTransformCom->Get_WorldMatrix_Inverse();
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, ViewMatrix);

	//XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f);

	// 뷰볼륨 조정
	_float2 ViewVolume;
	ViewVolume.x -= g_iWinSizeX * 0.01f;
	ViewVolume.y -= g_iWinSizeY * 0.01f;
	
	if (g_iWinSizeX <= ViewVolume.x || g_iWinSizeY <= ViewVolume.y)
		return FALSE;

	ProjMatrix = XMMatrixOrthographicLH(ViewVolume.x, ViewVolume.y, 0.0f, 1000.f);
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, ProjMatrix);

	//const CTransform* pUIEditorTrans = dynamic_cast<const CTransform*>(m_pGameInstance->
	//	Get_Component(LEVEL_TOOL_UI, TEXT("Layer_UI"), g_strTransformTag));
	CTransform* pUIEditorTrans = dynamic_cast<CTransform*>(this->Get_Component(g_strTransformTag));

	WorldMatrix = pUIEditorTrans->Get_WorldMatrix();


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

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
