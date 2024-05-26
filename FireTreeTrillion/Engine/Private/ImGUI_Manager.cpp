#include "ImGUI_Manager.h"

//#include "imgui.h"
#include "GameInstance.h"
#include "ImGuizmo.h"
#include "PipeLine.h"


HRESULT CImGUI_Manager::Initialize(HWND hWnd, ID3D11Device* pGraphic_Device, ID3D11DeviceContext* pContext)
{
	m_pDevice = pGraphic_Device;
	Safe_AddRef(m_pDevice);

	m_pContext = pContext;
	Safe_AddRef(pContext);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	io.ConfigViewportsNoTaskBarIcon = true;

	// 05.21) 한글 폰트 적용
	io.Fonts->AddFontFromFileTTF("C://Windows/Fonts/malgun.ttf", 15.f, NULL, io.Fonts->GetGlyphRangesKorean());

//다른 폰트를 넣으려는 사투
	//ifstream file("C://Windows/Fonts/Pretendard.ttf");
	//if (!file.good())
	//{
	//	io.Fonts->AddFontFromFileTTF("C://Windows/Fonts/malgun.ttf", 16.f, NULL, io.Fonts->GetGlyphRangesKorean());
	//}

	// Setup Dear ImGui style
#pragma region IMGUI_STYLE
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImVec4 vPinkDark = { 0.6f, 0.18f, 0.37f, 1.0f };
	ImVec4 vPink = { 0.8f, 0.18f, 0.37f, 1.0f };
	ImVec4 vPinkLight = { 1.0f, 0.18f, 0.37f, 1.0f };

	//테두리
	style.Colors[ImGuiCol_Border] = vPink;

	//프레임
	style.Colors[ImGuiCol_FrameBg] = vPink;
	style.Colors[ImGuiCol_FrameBgHovered] = vPinkLight;
	style.Colors[ImGuiCol_FrameBgActive] = vPink;

	//타이틀
	style.Colors[ImGuiCol_TitleBg] = vPink;
	style.Colors[ImGuiCol_TitleBgActive] = vPinkLight;
	style.Colors[ImGuiCol_TitleBgCollapsed] = vPink;

	//메뉴 바
	style.Colors[ImGuiCol_MenuBarBg] = vPinkDark;
	style.Colors[ImGuiCol_ScrollbarBg] = vPink;

	//버튼
	style.Colors[ImGuiCol_Button] = vPinkDark;
	style.Colors[ImGuiCol_ButtonHovered] = vPinkLight;
	style.Colors[ImGuiCol_ButtonActive] = vPinkLight;

	//헤더
	style.Colors[ImGuiCol_Header] = vPinkDark;
	style.Colors[ImGuiCol_HeaderHovered] = vPinkLight;
	style.Colors[ImGuiCol_HeaderActive] = vPinkLight;

	//분리선
	style.Colors[ImGuiCol_Separator] = vPink;
	style.Colors[ImGuiCol_SeparatorHovered] = vPinkLight;
	style.Colors[ImGuiCol_SeparatorActive] = vPink;

	//리사이즈 그립
	style.Colors[ImGuiCol_ResizeGrip] = vPink;
	style.Colors[ImGuiCol_ResizeGripHovered] = vPinkLight;
	style.Colors[ImGuiCol_ResizeGripActive] = vPink;

	// 탭
	style.Colors[ImGuiCol_Tab] = vPink;
	style.Colors[ImGuiCol_TabHovered] = vPinkLight;
	style.Colors[ImGuiCol_TabActive] = vPink;

	style.Colors[ImGuiCol_TabUnfocused] = vPink;
	style.Colors[ImGuiCol_TabUnfocusedActive] = vPink;

	// 도킹 프리뷰
	style.Colors[ImGuiCol_DockingPreview] = vPink;
	style.Colors[ImGuiCol_DockingEmptyBg] = vPink;

#pragma endregion

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(pGraphic_Device, pContext);

	return S_OK;
}


void CImGUI_Manager::Tick(_float fTimeDelta)
{
	
}

void CImGUI_Manager::Late_Tick(_float fDeltaTime)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

HRESULT CImGUI_Manager::Render()
{
	// Rendering
	//ImGui::EndFrame();
	//ImGui::ShowDemoWindow();
	ImGui::Render();

	return S_OK;
}

HRESULT CImGUI_Manager::RenderUpdate()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	return S_OK;
}


void CImGUI_Manager::SetDockSpace()
{
	ImGuiWindowFlags         WindowFlag = ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* Viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(Viewport->WorkPos);
	ImGui::SetNextWindowSize(Viewport->WorkSize);
	ImGui::SetNextWindowViewport(Viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));

	WindowFlag |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	WindowFlag |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	WindowFlag |= ImGuiDockNodeFlags_PassthruCentralNode;
	WindowFlag |= ImGuiWindowFlags_NoBackground;

	_bool   bIsShow = true;

	ImGui::Begin("DockSpace", &bIsShow, WindowFlag);
	ImGui::PopStyleVar(1);
	ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID   DockSpaceID = ImGui::GetID("DockSpace");
		ImGuiDockNodeFlags Flag = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGui::DockSpace(DockSpaceID, ImVec2(0.f, 0.f), Flag);
	}

	ImGui::End();
}

void CImGUI_Manager::RenderGrid()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	const float identityMatrix[16] =
	{	1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	ImGuizmo::DrawGrid(ViewMatrix.m[0], ProjMatrix.m[0], identityMatrix, 100.f);
}

void CImGUI_Manager::EditTransform(_float4x4& matrix)
{
#ifndef _DEBUG
	return;
#endif

	ImGui::Separator();
	ImGui::NewLine();

	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

	if(CGameInstance::Get_Instance()->Get_DIKeyState(DIK_Q, KEY_DOWN))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_E, KEY_DOWN))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_R, KEY_DOWN))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	
	ImGui::SameLine();
	
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	
	ImGui::SameLine();
	
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix.m[0], matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix.m[0]);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		
		ImGui::SameLine();
		
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}

	static bool useSnap(false);
	ImGui::SameLine();
	
	_float3 snap = _float3();
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);
	ImGuizmo::Manipulate(ViewMatrix.m[0], ProjMatrix.m[0], mCurrentGizmoOperation, mCurrentGizmoMode, matrix.m[0], NULL, useSnap ? &snap.x : NULL);
}

CImGUI_Manager* CImGUI_Manager::Create(HWND hWnd, ID3D11Device* pGraphic_Device, ID3D11DeviceContext* pContext)
{
	CImGUI_Manager* pInstance = new CImGUI_Manager();

	if (FAILED(pInstance->Initialize(hWnd, pGraphic_Device, pContext)))
	{
		MSG_BOX(TEXT("Failed To Created : CImGUI_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CImGUI_Manager::Free()
{
	__super::Free();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

