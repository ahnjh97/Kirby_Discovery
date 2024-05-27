#include "stdafx.h"
#include "UI_Editor.h"

#include "ImGuizmo.h"
#include "ImGuiFileDialog.h"
#include "ImGuiFileDialogConfig.h"

CUI_Editor::CUI_Editor(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_Editor::CUI_Editor(const CUI_Editor& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CUI_Editor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Editor::Initialize(void* _pArg)
{
	if (FAILED(__super::Initialize(_pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

#pragma region HUD_Kirby

	UIOBJ_DESC HUD_KirbyDESC{};
	HUD_KirbyDESC.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	HUD_KirbyDESC.vSize = { 100.f, 100.f };
	HUD_KirbyDESC.vPos = { HUD_KirbyDESC.vCenter.x/* - 200.f*/, HUD_KirbyDESC.vCenter.y/* - 200.f */ };

	CUIObject* pUIObject = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_HUD_Kirby"), &HUD_KirbyDESC));
	CHECK_NULLPTR(pUIObject);
	m_vecUIObj.push_back(pUIObject);

#pragma endregion

	return S_OK;
}

_int CUI_Editor::Tick(_float _fTimeDelta)
{
	__super::Tick(_fTimeDelta);

	//Set_OrthoProj();
	for (auto& pUIObj : m_vecUIObj)
		pUIObj->Tick(_fTimeDelta);

	return OBJ_NOEVENT;
}

void CUI_Editor::Late_Tick(_float _fTimeDelta)
{
	for (auto& pUIObj : m_vecUIObj)
		pUIObj->Late_Tick(_fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_Editor::Render()
{
	return S_OK;
}

void CUI_Editor::Render_IMGUI()
{
	Set_GizmoGrid(); //IMGUI GIZMO GRID CUSTOM
	Set_DockSpace(); //IMGUI DOCKSPACE

	//CUIObject* pUIObj = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_HUD_Kirby"), nullptr));
	//CHECK_NULLPTR(pUIObj);
	//m_vecUIObj.emplace_back(pUIObj);

	ImGuiWindowFlags Dirwindow_Flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	if (ImGui::Begin(u8"Directory 경로 ", 0, Dirwindow_Flags))
	{
		if (ImGui::BeginTabBar(u8"##Directory"))
		{
			if (ImGui::BeginTabItem(u8"List 목록"))
			{
				ImGui::SeparatorText(u8"HUD");
				
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	if (ImGui::Begin(u8"Properties 속성", 0, ImGuiWindowFlags_NoCollapse))
	{
		if (ImGui::BeginTabBar(u8"Test TabBar 1")) //탭 바
		{
			if (ImGui::BeginTabItem(u8"Transform 변환"))
			{
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

	//ImGui::PopStyleVar();
}

HRESULT CUI_Editor::Add_Components()
{
	return S_OK;
}

HRESULT CUI_Editor::Bind_ShaderResources()
{
	return S_OK;
}

_bool CUI_Editor::Edit_Transform()
{
	ImGui::SeparatorText(u8"Transform Edit 상태 편집");

	// 기즈모 영역 세팅
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y);

	//_bool IsUsingGizmo = FALSE;
	//if (IsUsingGizmo)
		Set_GizmoSync(); //기즈모와 위젯, 오브젝트 동기화 작업

	//m_UIObjDesc.vSize *= (_float2)Scale;
	//m_UIObjDesc.vPos *= (_float2)Translate;
	

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

	ProjMatrix = XMMatrixOrthographicLH(ViewVolume.x, ViewVolume.y, 0.0f, 1600.f);
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, ProjMatrix);

	//const CTransform* pUIEditorTrans = dynamic_cast<const CTransform*>(m_pGameInstance->
	//	Get_Component(LEVEL_TOOL_UI, TEXT("Layer_UI"), g_strTransformTag));
	CTransform* pUIEditorTrans = dynamic_cast<CTransform*>(this->Get_Component(g_strTransformTag));

	WorldMatrix = pUIEditorTrans->Get_WorldMatrix();

	return TRUE;
}

_bool CUI_Editor::Set_GizmoSync()
{
	const char* DragTag = { "Translate 위치" };
	_float fTextWidth = ImGui::CalcTextSize(DragTag).x;

	ImGuizmo::BeginFrame(); //기즈모 생성
	_float4x4 WorldMatrix = m_pTransformCom->Get_WorldFloat4x4();
	if (nullptr == m_pTransformCom)
		return FALSE;

	static ImGuizmo::OPERATION eCurGizmoOper(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y);
	static ImGuizmo::MODE eCurGizmoMode(ImGuizmo::WORLD);

	// 기즈모 사용여부 텍스트
	if (!ImGuizmo::IsUsing())
		ImGui::Text(u8"Gizmo InValid");

	else
	{
		switch (eCurGizmoOper)
		{
		case ImGuizmo::TRANSLATE:
			ImGui::Text(u8"Translate Edit");
			break;

		case ImGuizmo::SCALE:
			ImGui::Text(u8"Scale Edit");
			break;

		case ImGuizmo::ROTATE_Z:
			ImGui::Text(u8"Rotate Edit");
			break;
		}
	}

	// 기즈모 키 입력 시 기능 스왑
	ImGui::Text(u8"Ctrl S : 크기 / Ctrl R : 회전 / Ctrl T : 위치");

	// 크기 회전 이동 변경 키
	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::SCALE/*ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y*/;

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
	ImGui::DragFloat3("##Scale", (_float*)Scale, 0.01f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Translate 위치");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Translate", (_float*)&Translate, 0.1f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Rotate 회전");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat("##Rotate", (_float*)&Rotate[2], 0.1f, (_int)-360, (_int)360, u8"Degree 각도 : %.1f");

	ImGuizmo::RecomposeMatrixFromComponents(Translate, Rotate, Scale, WorldMatrix.m[0]);

	// 뷰, 투영 행렬 정보 로드
	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	//static _bool useSnap(false);
	_float fGizmoSpeed[3] = {
		0.1f,		//Translate
		0.1f,		//Rotate
		0.01f };		//Scale

	//오브젝트 변환
	ImGuizmo::Manipulate(ViewMatrix.m[0], ProjMatrix.m[0], eCurGizmoOper, eCurGizmoMode, WorldMatrix.m[0], NULL, fGizmoSpeed);
	/*useSnap ? &snap.x : NULL*/

	//월드행렬 세팅
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);

	return TRUE;
}

_bool CUI_Editor::Set_GizmoGrid()
{	
	//IMGUI Gizmo Grid 커스텀 (X/Y 2D 좌표계용)
	static const float MatGridX[16] =
	{ 1.f, 0.f,  0.f, 0.f,
		0.f, 0.f, -1.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	ProjMatrix = CGameInstance::Get_Instance()->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	ImGuizmo::DrawGrid(ViewMatrix.m[0], ProjMatrix.m[0], MatGridX, 1000.f);

	return TRUE;
}

_bool CUI_Editor::Set_DockSpace()
{
	// 도킹 모드는 크기/위치 고정 시 도킹 불가
	//ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));
	//ImGui::SetNextWindowSize(ImVec2(iSizeX, iSizeY));
	ImGuiWindowFlags Window_Flags = /*ImGuiWindowFlags_MenuBar | */ ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
	ImGuiDockNodeFlags MainDockspace_Flags = ImGuiDockNodeFlags_None;
	MainDockspace_Flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;

	/*ImGuiWindowFlags_NoCollapse | ImGuiTreeNodeFlags_DefaultOpen;*/
	if (ImGui::Begin(u8"UI Editor 에디터", 0, Window_Flags))
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID Dockspace_ID = ImGui::GetID("##UI Editor Dockspace");
			ImGui::DockSpace(Dockspace_ID, ImVec2(0.0f, 0.0f), MainDockspace_Flags);
		}

		if (ImGui::BeginMainMenuBar())
		{
			// 파일 다이얼로그
			m_pGameInstance->Set_FileDialog();
			
			if (ImGui::Button(u8"Save 저장"))
			{
				if (Save_FileData())
					MSG_BOX(TEXT("Save Complete!"));
			}
			if (ImGui::Button(u8"Load 로드"))
			{
				if (Load_FileData("../Bin/Resources/Data/UI/Test.UIDAT"))
					MSG_BOX(TEXT("Load Complete!"));
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::End(); //창 종료
	}

	return TRUE;
}

_bool CUI_Editor::Set_FileDialog()
{
	// 다이얼로그 오픈
	return TRUE;
}

_bool CUI_Editor::Save_FileData()
{
	string FileName = "../Bin/Resources/Data/UI/Test.UIDAT";
	std::ofstream File(FileName, ios::out | std::ios::binary);

	if (!File.is_open()) 
	{
		MSG_BOX(TEXT("Save Failed : UI")); 
		return FALSE;
	}

	size_t size = m_vecUIObj.size();
	File.write(reinterpret_cast<const char*>(&size), sizeof(size));

	for (auto iter : m_vecUIObj)
	{
		wstring ProtoTag = iter->Get_PrototypeTag();
		UIOBJ_DESC ProtoUI_Desc = iter->Get_UIObj_Desc();
		//_float2 size2D = _float2();//iter->Get_size2D();
		//_float2 position2D = _float2();//iter->Get_pos2D();

		string strProtoTag = CUtils::WstrToStr(ProtoTag);
		_uint ProtoTagLen = strProtoTag.length();

		File.write(reinterpret_cast<const char*>(&ProtoTagLen), sizeof(ProtoTagLen));
		File.write(strProtoTag.c_str(), /*sizeof(char) **/ ProtoTagLen);

		File.write(reinterpret_cast<const char*>(&ProtoUI_Desc), sizeof(ProtoUI_Desc));
		//File.write(reinterpret_cast<const char*>(&position2D), sizeof(position2D));
	}

	//쓰기
	//string strModelName = pModel->Get_ModelInfo().strModelName;
	//_float4x4 matWorld = pTransform->Get_WorldMatrix();
	//_uint iStrLength = strModelName.length();

	//outputFile.write(reinterpret_cast<const char*>(&iStrLength), sizeof(iStrLength));
	//outputFile.write(strModelName.c_str(), iStrLength);
	//outputFile.write(reinterpret_cast<const char*>(&matWorld), sizeof(_float4x4));

	File.close();

	return TRUE;
}

_bool CUI_Editor::Load_FileData(const string& _FilePath)
{
	std::ifstream File(_FilePath, ios::in | std::ios::binary);

	if (!File.is_open()) 
	{
		MSG_BOX(TEXT("Load Failed : UI"));
		return FALSE;
	}

	size_t size = 0;
	File.read(reinterpret_cast<char*>(&size), sizeof(size));
	m_vecUIObj.reserve(size);
	string ProtoTag;

	for (size_t i = 0; i < size; ++i)
	{
		//char ProtoTag[256];
		//size_t ProtoTagLen;
		_uint ProtoTagLen;
		File.read(reinterpret_cast<char*>(&ProtoTagLen), sizeof(ProtoTagLen));
		ProtoTag.resize(ProtoTagLen);

		File.read(&ProtoTag[0], /*sizeof(char) **/ ProtoTagLen);

		UIOBJ_DESC ProtoUI_Desc;
		File.read(reinterpret_cast<char*>(&ProtoUI_Desc), sizeof(ProtoUI_Desc));

		//UIOBJ_DESC	LoadUI_Desc{};
		//LoadUI_Desc.vCenter = ProtoUI_Desc.vCenter;
		//LoadUI_Desc.vSize = ProtoUI_Desc.vSize;
		//LoadUI_Desc.vPos = ProtoUI_Desc.vPos;
		//LoadUI_Desc.fFrame = ProtoUI_Desc.fFrame;

		CGameObject* pObj = m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(ProtoTag), &ProtoUI_Desc);
		CUIObject* pUIObject = dynamic_cast<CUIObject*>(pObj);

		// push_back 끝나면 크래시 현상 발생
		m_vecUIObj.push_back(pUIObject);
	}

	File.close();

	//읽기
	//string strModelName;
	//_float4x4 matWorld{};
	//_int iCamIndex{};

	//while (!fileStream.eof())
	//{
	//	_uint iStrLength;
	//	fileStream.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
	//	strModelName.resize(iStrLength);
	//	fileStream.read(&strModelName[0], iStrLength);
	//	fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(_float4x4))
	//}


	return TRUE;
}

CUI_Editor* CUI_Editor::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
{
	CUI_Editor* pInstance = new CUI_Editor(_pDevice, _pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : UI_Editor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Editor::Clone(void* _pArg)
{
	CUI_Editor* pInstance = new CUI_Editor(*this);

	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : UI_Editor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Editor::Free()
{
	for (auto& pUIObj : m_vecUIObj)
		Safe_Release(pUIObj);

	m_vecUIObj.clear();

	__super::Free();
}
