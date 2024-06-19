#include "stdafx.h"
#include "Editor_UI.h"
#include "ImGUI_Manager.h"

#ifdef _DEBUG
#include "ImGuizmo.h"
#include "LayerUI.h"
#include "ImSequencer.h"
//#include "ImGuiFileDialog.h"
//#include "ImGuiFileDialogConfig.h" //현재 사용 안함
#endif


CEditor_UI::CEditor_UI(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CEditor_UI::CEditor_UI(const CEditor_UI& _rhs)
	: CUIObject{ _rhs }
{
}

static _int g_iSelectUI, g_iSelectTex = -1;
static vector<_int> g_SelectUIs;

enum POPUP_TYPE { POPUP_CREATE, POPUP_DELETE, POPUP_GROUP, POPUP_SAVE, POPUP_LOAD, POPUP_MODIFY, POPUP_NONE };
enum POPUP_DETAIL {
	NEED_CREATE, NEED_SELECT,
	FILE_OPEN, FILE_COPY, FILE_READ,
	DETAIL_NONE
};

static POPUP_TYPE g_eOpenPopup = { POPUP_NONE };
static POPUP_DETAIL g_ePopupDetail = { DETAIL_NONE };

static _int g_IsSuccessed = { -1 };
string g_strResult, g_strPopupTag, g_strMessage, g_strDetail = { u8" " };
string g_strUITag = { "LayerUI" };

static _bool g_IsOrthoProj = { TRUE };
_float g_fFOV = XMConvertToRadians(30.0f);
_float g_fNear = { 0.1f };
_float g_fFar = { 1000.f };

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

HRESULT CEditor_UI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEditor_UI::Initialize(void* _pArg)
{
	if (FAILED(__super::Initialize(_pArg)))
		return E_FAIL;

#pragma region LOAD_FILEDATA

	//에디터 첫 실행 시, 파일데이터 읽고 로드
	//string strFilePath = "../Bin/Resources/Data/UI/";
	//Load_FileData(strFilePath + "Test_Origin.txt");

#pragma endregion

	return S_OK;
}

_int CEditor_UI::Tick(_float _fTimeDelta)
{
	__super::Tick(_fTimeDelta);

	if (!m_LayerUIs.empty())
	{
		for (auto& pUIObj : m_LayerUIs)
			pUIObj->Tick(_fTimeDelta);
	}

#pragma region KEY_INPUT

	string strFilePath = { "../../../UI_txt/" };

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_N, KEY_DOWN))
			Create_UIObject(UI_LAYER, UI_TEXTURE);

		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
			Delete_UIObject(UI_LAYER);

		if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_DOWN))
		{	g_IsSuccessed = -1; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = DETAIL_NONE;	}//저장팝업 출력

		if (m_pGameInstance->Get_DIKeyState(DIK_D, KEY_DOWN))
		{	g_IsSuccessed = -1; g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = DETAIL_NONE; }//로드팝업 출력

		if (m_pGameInstance->Get_DIKeyState(DIK_G, KEY_DOWN))
			Group_UIObject(GROUP_SELECT);
	}

#pragma endregion

	return OBJ_NOEVENT;
}

void CEditor_UI::Late_Tick(_float _fTimeDelta)
{
	if (!m_LayerUIs.empty())
	{
		for (auto& pUIObj : m_LayerUIs)
			pUIObj->Late_Tick(_fTimeDelta);
	}

	Set_GizmoGrid(); //IMGUI GIZMO GRID CUSTOM

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CEditor_UI::Render()
{
	ImGuizmo::BeginFrame(); //기즈모 생성 및 초기화

	Window_PopupAlert();
	Set_DockSpace(); //IMGUI DOCKSPACE

	Window_Directories();
	Window_Textures();
	Window_Properties();
	Window_Tools();

	return S_OK;
}

void CEditor_UI::Render_IMGUI()
{
}

_bool CEditor_UI::Set_DockSpace()
{
	// 도킹 모드는 크기/위치 고정 시 도킹 불가
	//ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));
	//ImGui::SetNextWindowSize(ImVec2(iSizeX, iSizeY));
	ImGuiWindowFlags Window_Flags{};/*ImGuiWindowFlags_MenuBar* ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;*/
	ImGuiDockNodeFlags Dockspace_Flags = ImGuiDockNodeFlags_None;
	Dockspace_Flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;

	/*ImGuiWindowFlags_NoCollapse | ImGuiTreeNodeFlags_DefaultOpen;*/
	if (ImGui::Begin(u8"UI Editor 에디터", 0, Window_Flags))
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID Dockspace_ID = ImGui::GetID("##UI Editor Dockspace");
			ImGui::DockSpace(Dockspace_ID, ImVec2(0.0f, 0.0f), Dockspace_Flags);
		}

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(u8"File 파일"))
			{
				if (ImGui::BeginMenu(u8"New/Create 생성"))
				{
					if (ImGui::BeginMenu(u8"Layer 레이어"))
					{
						if (ImGui::MenuItem(u8"Texture 텍스처", "Ctrl+N"))
							Create_UIObject(UI_LAYER, UI_TEXTURE);

						if (ImGui::MenuItem(u8"Font 폰트"))
							Create_UIObject(UI_LAYER, UI_FONT);

						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}

				string strFilePath = { "../../../UI_txt/" };
				//string strFileTag = CUtils::WstrToStr(m_LayerUIs.front()->Get_UIObj_Desc().wstrUITag);

				if (ImGui::BeginMenu(u8"Save 저장"))
				{
					if (ImGui::MenuItem(u8"Data 데이터", "Ctrl+S"))
						Save_FileData(strFilePath);

					if (ImGui::MenuItem(u8"Texture 텍스처")) {}
						//Save_Texture("../../../UI_dds/LayerUI.dds", m_pRTV); //추후 작업 예정

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem(u8"Load 로드", "Ctrl+D"))
				{	g_IsSuccessed = -1; g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = DETAIL_NONE;	}

				ImGui::EndMenu();
			}

#pragma region FILEDIALOG_사용안함

			// 파일 다이얼로그 (ImGUI_Manager::Set_FileDialog() 참고)
			//CImGUI_Manager::FILE_MODE eFileMode = m_pGameInstance->Set_FileDialog();
			//switch (eFileMode)
			//{
			//case CImGUI_Manager::FILE_MODE::FILE_SAVE:
			//	Save_FileData(strFilePath);
			//	break;

			//case CImGUI_Manager::FILE_MODE::FILE_LOAD:
			//	Load_FileData(strFilePath + strUITag + "_Orig.txt");
			//	break;
			//}

#pragma endregion

			ImGui::EndMainMenuBar();
		}
	}
	ImGui::End(); //창 종료

	return TRUE;
}

_bool CEditor_UI::Window_Directories()
{
	ImGuiWindowFlags Dirwindow_Flags{}; /*= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;*/
	if (ImGui::Begin(u8"Directories 디렉토리", 0, Dirwindow_Flags))
	{
		if (ImGui::BeginTabBar(u8"##Directories"))
		{
			Tab_LayerList();
			Tab_GroupList();

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	return FALSE;
}

_bool CEditor_UI::Tab_LayerList()
{
	if (ImGui::BeginTabItem(u8"Layer 레이어"))
	{
		ImGui::SeparatorText(u8"Layer List 레이어 목록");
		//ImGui::Text(u8"Shift+Click 다중 선택");
		ImGui::SameLine();  HelpMarker(u8"Shift 다중 선택");

		if (ImGui::BeginListBox(u8"##UI List", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			//static _int iSelect = -1;
			for (size_t iUI = 0; iUI < m_LayerUIs.size(); ++iUI)
			{
				string strUITag = CUtils::WstrToStr(m_LayerUIs[iUI]->Get_UIObj_Desc().wstrUITag);
				strUITag += "_" + to_string(iUI);

				if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
					strUITag = "##";

#pragma region LISTBOX
				const _bool IsSelected = find(g_SelectUIs.begin(), g_SelectUIs.end(), iUI) != g_SelectUIs.end();
				if (ImGui::Selectable(strUITag.c_str(), IsSelected)) //리스트박스 항목 선택 여부 확인
				{
					if (ImGui::GetIO().KeyShift) //다중 선택
					{
						if (!IsSelected)
							g_SelectUIs.push_back(iUI);
					}
					else //단일 선택
					{
						g_SelectUIs.clear();
						g_SelectUIs.push_back(iUI);
					}
				}
				if (IsSelected) //목록 선택할 경우, 선택 대상에게 기즈모 동기화
				{
					ImGui::SetItemDefaultFocus();

					if (!m_LayerUIs.empty() && !g_SelectUIs.empty())
					{
						//Set_GizmoSync(m_LayerUIs[iUI]);
						Set_GizmoSync(m_LayerUIs[g_SelectUIs.front()]); //기즈모와 위젯, 오브젝트 동기화 작업
					}
				}
#pragma endregion

				if (ImGui::BeginPopupContextItem()) // 우클릭하면 세부 메뉴 표시
				{
					if (ImGui::BeginMenu(u8"Group 그룹"))
					{
						if (ImGui::MenuItem(u8"All 전체"))
							Group_UIObject(GROUP_ALL);

						if (ImGui::MenuItem(u8"Select 선택", "Ctrl+G"))
							Group_UIObject(GROUP_SELECT);

						ImGui::EndMenu();
					}

					if (ImGui::MenuItem(u8"Delete 삭제", "Ctrl+Z"))
						Delete_UIObject(UI_LAYER);

					if (ImGui::MenuItem(u8"Modify 변경")) //레이어Tag 변경
					{
						g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_MODIFY;
					}

					ImGui::EndPopup();
				}
			}

			ImGui::EndListBox();
		}

		ImGui::EndTabItem();
	}

	return TRUE;
}

_bool CEditor_UI::Tab_GroupList()
{
	if (ImGui::BeginTabItem(u8"Group 레이어 그룹"))
	{
		ImGui::SeparatorText(u8"Layer Group 레이어그룹 목록");

#pragma region COMBO
		string strUITag = {};

		if (!m_GroupUIs.empty())
		{
			for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
			{
				for (auto& GroupUI : m_GroupUIs)
				{
					strUITag = CUtils::WstrToStr(GroupUI[iGroupIx]->Get_UIObj_Desc().wstrUITag);
					strUITag += "_Group_[" + to_string(iGroupIx) + "]";
				}
			}
		}

		ImGui::PushItemWidth(265.f);
		if (ImGui::BeginCombo(u8"##", (g_iSelectUI >= 0 && g_iSelectUI < m_GroupUIs.size()) 
			? strUITag.c_str() : u8"그룹을 선택해주세요."))
		{
			for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
			{
				for (auto& GroupUI : m_GroupUIs)
				{
					string strUITag = CUtils::WstrToStr(GroupUI[iGroupIx]->Get_UIObj_Desc().wstrUITag);
					strUITag += "_Group_[" + to_string(iGroupIx) + "]";

					if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
						strUITag = "##";

					const _bool IsSelected = g_iSelectUI == iGroupIx;
					if (ImGui::Selectable(strUITag.c_str(), IsSelected))
						g_iSelectUI = iGroupIx;
				}
			}
			ImGui::EndCombo();
		}
#pragma endregion
		
#pragma region LISTBOX

		if (ImGui::BeginListBox(u8"##", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
			{
				for (size_t iLayerIx = 0; iLayerIx < m_GroupUIs[iGroupIx].size(); ++iLayerIx)
				{
					auto& GroupUI = m_GroupUIs[iGroupIx][iLayerIx];
					string strUITag = CUtils::WstrToStr(GroupUI->Get_UIObj_Desc().wstrUITag);
					strUITag += "_Group_[" + to_string(iGroupIx) + "]_" + to_string(iLayerIx);

					if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
						strUITag = "##";

					const _bool IsSelected = find(g_SelectUIs.begin(), g_SelectUIs.end(), iGroupIx) != g_SelectUIs.end();
					if (ImGui::Selectable(strUITag.c_str(), IsSelected)) //리스트박스 항목 선택 여부 확인
					{
						if (ImGui::GetIO().KeyShift) //다중 선택
						{
							if (!IsSelected)
								g_SelectUIs.push_back(iLayerIx);
						}
						else //단일 선택
						{
							g_SelectUIs.clear();
							g_SelectUIs.push_back(iLayerIx);
						}
					}
					if (IsSelected) //목록 선택할 경우, 선택 대상에게 기즈모 동기화
					{
						ImGui::SetItemDefaultFocus();
						if (!m_GroupUIs.empty()) //그룹 상속관계 변환 동기화
						{
							for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
							{
								for (size_t iLayerIx = 0; iLayerIx < m_GroupUIs[iGroupIx].size(); ++iLayerIx)
								{
									auto& LayerUI = m_GroupUIs[iGroupIx][iLayerIx];

									//그룹 요소의 디스크립션을 첫번째 요소기준 증감 보정
									//UIOBJ_DESC FstLayerDesc = LayerUI[0].Get_UIObj_Desc(); //그룹의 첫번째 요소
									//UIOBJ_DESC LayerDesc = LayerUI[iLayerIx].Get_UIObj_Desc();
									//LayerDesc.vPos = FstLayerDesc.vPos - LayerDesc.vPos;

									//LayerUI[iLayerIx].Set_UIObj_Desc(LayerDesc);

									//for (auto& GroupUI : m_GroupUIs[iGroupIx])
									Set_GizmoSync(LayerUI); //기즈모와 위젯, 오브젝트 동기화 작업
								}
							}
						}
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::PopItemWidth();

		ImGui::EndTabItem();
	}

#pragma endregion

	return TRUE;
}

//아니 이거 왜이럼;;
_bool CEditor_UI::Window_Textures()
{
	ImGuiWindowFlags TexWindow_Flags = {}; /* ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;*/
	ImGuiComboFlags TexCombo_Flags = {};

	if (ImGui::Begin(u8"Texture 텍스처", 0, TexWindow_Flags))
	{
		ImGui::SeparatorText(u8"Texture List 텍스처 목록");

		//vecUI의 UIObj 순회하며 해당 오브젝트가 가진 텍스처 목록을 출력 
		if (ImGui::BeginListBox(u8"##Texture List", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
			{
				//해당 오브젝트의 텍스처 정보
				CTexture* pUITex = dynamic_cast<CTexture*>(m_LayerUIs[g_iSelectUI]->Get_Component(TEXT("Com_Texture")));
				if (!pUITex)
					return FALSE;

				_uint iMaxTex = pUITex->Get_NumTexture();
				for (size_t iTex = 0; iTex < iMaxTex; ++iTex)
				{
					_uint iTexIndex = m_LayerUIs[g_iSelectUI]->Get_UIObj_Desc().iTexIndex;
					string strTexTag = CUtils::WstrToStr(m_LayerUIs[g_iSelectUI]->Get_UIObj_Desc().wstrUITag);
					strTexTag += "_" + to_string(iTex);

					if (strTexTag.empty()) //strTexTag 값에 대한 예외처리
						strTexTag = "##";

					const _bool IsSelected = (g_iSelectTex == iTex);
					if (ImGui::Selectable(strTexTag.c_str(), IsSelected))
						g_iSelectTex = iTex;

					if (IsSelected)
					{	ImGui::SetItemDefaultFocus();		}

					if (ImGui::BeginPopupContextItem()) // 우클릭하면 컨텍스트 메뉴 표시
					{
						if (ImGui::MenuItem(u8"Modify 변경"))
						{
							if (!m_LayerUIs.empty())
								m_LayerUIs[g_iSelectUI]->Set_TexIndex(g_iSelectTex); //선택 텍스처로 변경
						}
						ImGui::EndPopup();
					}
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::End();
	return TRUE;
}

_bool CEditor_UI::Window_Properties()
{
	if (ImGui::Begin(u8"Properties 속성" /*, 0, ImGuiWindowFlags_NoCollapse */))
	{
		if (ImGui::BeginTabBar(u8"##")) //탭 바
		{
			if (ImGui::BeginTabItem(u8"Transform 변환"))
			{
				ImGui::SeparatorText(u8"Transform Edit 상태 편집");

				if (!g_SelectUIs.empty())
				{
					g_iSelectUI = g_SelectUIs.front();
					if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
					{
						Edit_Transform(m_LayerUIs[g_iSelectUI]);
						Edit_Projection(m_LayerUIs[g_iSelectUI]);
					}
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End(); //창 종료

	return TRUE;
}

_bool CEditor_UI::Window_Tools()
{
	if (ImGui::Begin(u8"Tool 도구"))
	{
		if (ImGui::BeginTabBar(u8"##"))
		{
			if (ImGui::BeginTabItem(u8"Color 색상 편집"))
			{
				ImGui::SeparatorText(u8"Color Edit 색상 편집");
				if (!g_SelectUIs.empty())
				{
					//g_iSelectUI = g_SelectUIs.front();
					if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
						Edit_RGBAColor(m_LayerUIs[g_iSelectUI]);
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(u8"Text 텍스트 편집"))
			{
				ImGui::SeparatorText(u8"Text Edit 텍스트 편집");

				/*
				string strUITag = {};
				ImGui::PushItemWidth(265.f);
				if (ImGui::BeginCombo(u8"##", (g_iSelectUI >= 0 && g_iSelectUI < m_GroupUIs.size())
					? strUITag.c_str() : u8"폰트를 선택해주세요."))
				{
					//for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
					//{
					//	for (auto& GroupUI : m_GroupUIs)
					//	{
					//		string strUITag = CUtils::WstrToStr(GroupUI[iGroupIx]->Get_UIObj_Desc().wstrUITag);
					//		strUITag += "_Group_[" + to_string(iGroupIx) + "]";
					//		if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
					//			strUITag = "##";
					//		const _bool IsSelected = g_iSelectUI == iGroupIx;
					//		if (ImGui::Selectable(strUITag.c_str(), IsSelected))
					//			g_iSelectUI = iGroupIx;
					//	}
					//}
					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
				*/

				Edit_Text();

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End(); //창 종료

	return TRUE;
}

//보류) 애니메이션 키프레임 제어 및 설정 기능
_bool CEditor_UI::Window_Sequencer()
{
	if (ImGui::Begin(u8"애니메이션 Animation"))
	{
		// Create ImSequencer instance
		//static ImSequencer::Sequence<float> sequence;
		//static ImSequencer::State<float> state;

		//// Set up the sequencer
		//static ImSequencer::SequenceConfig<float> config;
		//config.timeScaleMin = 0.1f;
		//config.timeScaleMax = 4.0f;
		//config.orthoHeight = 100.0f;
		//config.splitRatio = 0.2f;

		//// Render the sequencer
		//ImSequencer::Sequencer(config, &sequence, &state);

	}
	ImGui::End();

	return TRUE;
}

//완료) 전반적인 안내팝업 설정
void CEditor_UI::Window_PopupAlert()
{
	if (!g_SelectUIs.empty())
	{
		g_iSelectUI = g_SelectUIs.front();
		if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
		{
			wstring wstrUITag = m_LayerUIs[g_iSelectUI]->Get_UIObj_Desc().wstrUITag;
			g_strUITag = CUtils::WstrToStr(wstrUITag); //리스트박스의 선택항목 태그를 저장
		}
	}

	if (g_IsSuccessed) g_strResult = { "Successed" };
	else g_strResult = { "Failed" };
	//{
	switch (g_eOpenPopup)
	{
	case POPUP_CREATE:	g_strPopupTag = { u8"Create" }; g_strUITag = { "LayerUI" }; //생성할 땐 태그를 기본값으로
		//switch (g_ePopupDetail)
		//{	default: g_strDetail = { u8" " };	break;	}
		g_strDetail = { u8"" };
		ImGui::OpenPopup(g_strPopupTag.c_str());
		break;

	case POPUP_DELETE:	g_strPopupTag = { u8"Delete" };
		//switch (g_ePopupDetail)
		//{	default: g_strDetail = { u8" " };	break;	}
		g_strDetail = { u8"" };
		ImGui::OpenPopup(g_strPopupTag.c_str());
		break;

	case POPUP_GROUP:	g_strPopupTag = { u8"Group" };
		switch (g_ePopupDetail)
		{
		case NEED_CREATE:	g_strDetail = { u8"Need to Create Layer 생성한 레이어가 없습니다" }; break;
		case NEED_SELECT:	g_strDetail = { u8"Need to Select Layer 레이어를 선택해주세요" }; break;
		default: g_strDetail = { u8" " }; break;
		}
		ImGui::OpenPopup(g_strPopupTag.c_str());
		break;

	case POPUP_SAVE:	g_strPopupTag = { u8"Save" };
		switch (g_ePopupDetail)
		{
		case NEED_CREATE:	g_strDetail = { u8"Need to Create Layer 생성한 레이어가 없습니다" }; break;
		case FILE_OPEN:		g_strDetail = { u8"Check the File Path & Name 파일 경로와 이름을 확인해주세요" }; break;
		case FILE_COPY:		g_strDetail = { u8"원본 파일 복사에 실패했습니다" }; break;
		case DETAIL_NONE: default: g_strDetail = { u8" " }; break;
		}
		ImGui::OpenPopup(g_strPopupTag.c_str());
		break;

	case POPUP_LOAD:	g_strPopupTag = { u8"Load" };
		switch (g_ePopupDetail)
		{
		case FILE_OPEN:		g_strDetail = { u8"Check the File Path & Name 파일 경로와 이름을 확인해주세요" }; break;
		case FILE_READ:		g_strDetail = { u8"파일 데이터를 다시 확인해주세요" }; break;
		case DETAIL_NONE: default: g_strDetail = { u8" " }; break;
		}
		ImGui::OpenPopup(g_strPopupTag.c_str());
		break;

	case POPUP_MODIFY:	g_strPopupTag = { u8"Modify" };
		//switch (g_ePopupDetail)
		//{	default: g_strDetail = { u8" " };	break;	}
		g_strDetail = { u8"" };
		ImGui::OpenPopup(g_strPopupTag.c_str());
		break;
	}

#pragma region POPUP ALERT UI
	ImVec2 ViewCenter = ImGui::GetMainViewport()->GetCenter();
	ImGuiWindowFlags Popup_Flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowPos(ViewCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ViewCenter * 0.3f);

	static string strInput(1024 * 16, '\0');
	if (ImGui::BeginPopupModal(g_strPopupTag.c_str(), NULL, Popup_Flags))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN))
			ImGui::CloseCurrentPopup();

		if (ImGui::IsWindowAppearing()) //팝업 오픈 시점에 선택 항목의 태그를 인풋박스에 출력
			strInput = g_strUITag;

		ImVec2 WindowSize = ImGui::GetWindowSize();
		ImVec2 MessageSize = ImGui::CalcTextSize(g_strMessage.c_str());
		_float MessagePosX = (WindowSize.x - MessageSize.x) * 0.5f;
		ImGui::SetCursorPos(ImVec2(MessagePosX, WindowSize.y * 0.5f - 20.f));
		ImGui::Text(g_strMessage.c_str());

		ImVec2 DetailSize = ImGui::CalcTextSize(g_strDetail.c_str());
		_float DetailPosX = (WindowSize.x - DetailSize.x) * 0.5f;

		ImVec2 InputSize = { 200.f, 0.f };
		_float InputPosX = (WindowSize.x - InputSize.x) * 0.5f;

		string strFilePath = { "../../../UI_txt/" };
		string strFileExt = { "_Orig.txt" };
		if (u8"Modify" == g_strPopupTag)
		{
			ImGui::SetNextItemWidth(200.f);
			ImGui::SetCursorPos(ImVec2(InputPosX, WindowSize.y * 0.5f + 3.5f));
			ImGui::InputText(u8"##", &strInput[0], strInput.capacity());

			m_UIObjDesc.wstrUITag = Edit_LayerUITag(strInput.c_str());
		}

		if (u8"Save" == g_strPopupTag || u8"Load" == g_strPopupTag)
		{
			ImGui::SetNextItemWidth(200.f);
			ImGui::SetCursorPos(ImVec2(InputPosX, WindowSize.y * 0.5f + 3.5f));
			ImGui::InputText(u8"##", &strInput[0], strInput.capacity());
		}

		else
		{
			ImGui::SetCursorPos(ImVec2(DetailPosX, WindowSize.y * 0.5f - 5.f));
			ImGui::Text(g_strDetail.c_str());
		
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		}

		ImVec2 ButtonSize = ImVec2(120, 0);
		_float ButtonPosX = (WindowSize.x - ButtonSize.x) * 0.5f;
		ImGui::SetCursorPos(ImVec2(ButtonPosX, 100.f));
		if (ImGui::Button(u8"OK 확인", ButtonSize) || m_pGameInstance->Get_DIKeyState(DIK_RETURN, KEY_DOWN))
		{
			if (u8"Modify" == g_strPopupTag)
			{
				m_LayerUIs[g_iSelectUI]->Set_LayerUITag(m_UIObjDesc.wstrUITag);
				ImGui::CloseCurrentPopup(); 
			}

			if (u8"Save" == g_strPopupTag)
			{			
				strFilePath += strInput.c_str();// + strFileExt;
				if (TRUE == Save_FileData(strFilePath))
				{	g_IsSuccessed = TRUE;  g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = DETAIL_NONE;	
					ImGui::CloseCurrentPopup();
				}

				else
				{	g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = FILE_OPEN;	}
			}

			if (u8"Load" == g_strPopupTag)
			{			
				strFilePath += strInput.c_str() + strFileExt;
				if (TRUE == Load_FileData(strFilePath))
				{	g_IsSuccessed = TRUE;  g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = DETAIL_NONE;	
					ImGui::CloseCurrentPopup();
				}

				else
				{	g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = FILE_OPEN;	}
			}
			
			else
				ImGui::CloseCurrentPopup();	
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

#pragma endregion

	if ("Modify" == g_strPopupTag)
		g_strMessage = { u8"변경할 레이어명을 입력해주세요" };

	if ("Save" == g_strPopupTag)
	{
		if ( -1 == g_IsSuccessed)
		g_strMessage = { u8"저장할 파일명을 입력해주세요" };

		else
			g_strMessage = g_strResult + " to " + g_strPopupTag + " : " + g_strUITag;
	}

	else if ("Load" == g_strPopupTag)
	{
		if ( -1 == g_IsSuccessed)
		g_strMessage = { u8"로드할 파일명을 입력해주세요" };

		else
			g_strMessage = g_strResult + " to " + g_strPopupTag + " : " + g_strUITag;
	}

	else
		g_strMessage = g_strResult + " to " + g_strPopupTag + " : " + g_strUITag;

	g_eOpenPopup = POPUP_NONE;

}

//완료) 객체에 대한 변환(크기, 회전, 이동, 직교&원근) 편집
_bool CEditor_UI::Edit_Transform(CUIObject* _pUIObj)
{
	const char* DragTag = { "Translate 위치" };
	_float fTextWidth = ImGui::CalcTextSize(DragTag).x;
	_float Translate[3]{}, Rotate[3]{}, Scale[3]{};
	
	UIOBJ_DESC LayerUIDesc = _pUIObj->Get_UIObj_Desc();
	(_float3)Translate = LayerUIDesc.vPos;
	(_float3)Rotate = LayerUIDesc.vDegree;
	(_float3)Scale = LayerUIDesc.vSize;

	if (nullptr == _pUIObj)
		return FALSE;

	CTransform* pUITrans = (CTransform*)_pUIObj->Get_Component(g_strTransformTag);
	if (nullptr == pUITrans)
		return FALSE;

	_float4x4 UIWorldMat = pUITrans->Get_WorldFloat4x4();

	// 기즈모 연동
	ImGuizmo::DecomposeMatrixToComponents(UIWorldMat.m[0], Translate, Rotate, Scale);

	ImGui::PushItemWidth(175.f/*ImGui::GetColumnOffset()*/);
	ImGui::Text(u8"Translate 위치");
	ImGui::SameLine(); HelpMarker(u8"Ctrl+T");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::DragFloat3("##Translate", (_float*)&Translate, 1.f, (_float)-0.1 * g_iWinSizeX, (_float)g_iWinSizeX, "%.2f");

	ImGui::Text(u8"Rotate 회전");
	ImGui::SameLine(); HelpMarker(u8"Ctrl+R");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::DragFloat3("##Rotate", (_float*)&Rotate, 0.1f, (_int)-360, (_int)360, "%.2f");

	ImGui::Text(u8"Size 크기");
	ImGui::SameLine(); HelpMarker(u8"Ctrl+E");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::DragFloat3("##Size", (_float*)&Scale, 1.f, 0.f, g_iWinSizeX, "%.2f");

	LayerUIDesc.vPos = (_float3)Translate;
	LayerUIDesc.vDegree = (_float3)Rotate;
	LayerUIDesc.vSize = (_float3)Scale;

	_pUIObj->Set_UIObj_Desc(LayerUIDesc);

	ImGuizmo::RecomposeMatrixFromComponents(Translate, Rotate, Scale, UIWorldMat.m[0]);

	//월드행렬 세팅
	pUITrans->Set_WorldMatrix(UIWorldMat);

	return TRUE;
}

//완료) 객체의 색상, 알파 값 편집
_bool CEditor_UI::Edit_RGBAColor(CUIObject* _pUIObj)
{
	if (nullptr == _pUIObj)
		return FALSE;

	//static _bool saved_palette_init = true;
	//static ImVec4 saved_palette[32] = {};
	//if (saved_palette_init)
	//{
	//	for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
	//	{
	//		ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
	//			saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
	//		saved_palette[n].w = 1.0f; // Alpha
	//	}
	//	saved_palette_init = FALSE;
	//}
	ImGuiColorEditFlags ColorButton_Flags = ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_AlphaPreviewHalf /*| ImGuiColorEditFlags_NoTooltip*/;
	ImGuiColorEditFlags ColorEditRGBA_Flags = ImGuiColorEditFlags_NoSmallPreview;
	ImGuiColorEditFlags ColorEditHex_Flags = ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_DisplayHex;
	ImGuiColorEditFlags ColorPicker_Flags = { ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar };
	
	UIOBJ_DESC LayerUIDesc = _pUIObj->Get_UIObj_Desc();
	ImVec4 vColorRGBA = ImVec4(LayerUIDesc.vColorRGB.x, LayerUIDesc.vColorRGB.y, LayerUIDesc.vColorRGB.z, LayerUIDesc.fAlpha);

	ImGui::ColorButton("##ColorButton", *(ImVec4*)&vColorRGBA, ColorButton_Flags, ImVec2(50, 45));
	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::PushItemWidth(ImGui::GetColumnOffset());
	ImGui::ColorEdit4("##ColorEdit_RGBA", (_float*)&vColorRGBA, ColorEditRGBA_Flags);
	//ImGui::PopItemWidth();

	//ImGui::PushItemWidth(ImGui::GetColumnOffset());
	ImGui::ColorEdit4("##ColorEdit_HEX", (_float*)&vColorRGBA, ColorEditHex_Flags);
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	//ImGui::NewLine();

	ImGui::PushItemWidth(245.f);
	ImGui::ColorPicker4("##ColorPicker", (_float*)&vColorRGBA, ColorPicker_Flags);
	ImGui::PopItemWidth();

	//위젯 값 객체에 다시 적용
	LayerUIDesc.vColorRGB.x = vColorRGBA.x;
	LayerUIDesc.vColorRGB.y = vColorRGBA.y;
	LayerUIDesc.vColorRGB.z = vColorRGBA.z;
	LayerUIDesc.fAlpha = vColorRGBA.w;
		
	_pUIObj->Set_UIObj_Desc(LayerUIDesc);

	return TRUE;
}

//완료) 텍스트 편집
_bool CEditor_UI::Edit_Text()
{				
	//폰트 편집 동기화
	static string strInput(1024 * 16, '\0');
	//ImGuiInputTextFlags InputText_flags{}; //= ImGuiInputTextFlags_AllowTabInput;

	ImGui::InputTextMultiline(u8"##", &strInput[0],
		strInput.capacity(), ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()));

	wstring wstrText{};
	UIOBJ_DESC LayerUIDesc{};

	if (!g_SelectUIs.empty())
	{
		//g_iSelectUI = g_SelectUIs.front();
		if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
		{
			//UTF-8 인코딩 변환 작업
			LayerUIDesc = m_LayerUIs[g_iSelectUI]->Get_UIObj_Desc();

			wstrText = CUtils::StrToWstrUTF8(strInput);
			LayerUIDesc.wstrText = wstrText;

			m_LayerUIs[g_iSelectUI]->Set_UIObj_Desc(LayerUIDesc);
		}		
	}

	return TRUE;
}

//완료) 레이어명 편집
wstring CEditor_UI::Edit_LayerUITag(string _strInput)
{
	wstring wstrUITag{};
	UIOBJ_DESC LayerUIDesc{};

	if (!g_SelectUIs.empty())
	{
		g_iSelectUI = g_SelectUIs.front();
		if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
		{
			LayerUIDesc = m_LayerUIs[g_iSelectUI]->Get_UIObj_Desc();
			wstrUITag = CUtils::StrToWstr(_strInput);
			LayerUIDesc.wstrUITag = wstrUITag;

			return LayerUIDesc.wstrUITag;
		}
	}

	return wstring();
}

//완료) 직교/원근 투영 조정
_bool CEditor_UI::Edit_Projection(CUIObject* _pUIObj)
{	
	//뷰 행렬
	//_float4x4 WorldMatrix, ViewMatrix, ProjMatrix;
	//ViewMatrix = m_pTransformCom->Get_WorldMatrix_Inverse();
	//m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, ViewMatrix);

	const char* DragTag = { "Translate 위치" };
	_float fTextWidth = ImGui::CalcTextSize(DragTag).x;
	ImGuiSliderFlags ProjFlag = { ImGuiSliderFlags_AlwaysClamp };
	
	ImGui::SeparatorText(u8"Projection 투영");

	if (ImGui::RadioButton(u8"Ortho 직교", g_IsOrthoProj)) { g_IsOrthoProj = TRUE; }
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"Perspect 원근", !g_IsOrthoProj)) { g_IsOrthoProj = FALSE; }

	UIOBJ_DESC LayerUIDesc = _pUIObj->Get_UIObj_Desc();
	m_UIObjDesc.eUIProj = LayerUIDesc.eUIProj;

	_float fFOVDegree = XMConvertToDegrees(g_fFOV);

	ImGui::Text(u8"FOV 시야각");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::SliderFloat("##FOV", &fFOVDegree, 0.1f, 179.f, "%.2f", ProjFlag);

	ImGui::Text(u8"NearZ");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::SliderFloat("##NearZ", &g_fNear, 0.1f, 1000.f, "%.2f", ProjFlag);

	ImGui::Text(u8"FarZ");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::SliderFloat("##FarZ", &g_fFar, 0.2f, 1000.f, "%.2f", ProjFlag);

	_float4x4 ProjMatrix = _pUIObj->Get_ProjMatrix();

	//투영행렬 받아와서 변경 값 적용
	if (!g_IsOrthoProj)
	{
		m_UIObjDesc.eUIProj = PROJ_PERSPEC;
		XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(g_fFOV, (_float) g_iWinSizeX / g_iWinSizeY, g_fNear, g_fFar));
	}

	else
	{
		m_UIObjDesc.eUIProj = PROJ_ORTHO;
		XMStoreFloat4x4(&ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	}
	
	LayerUIDesc.eUIProj = m_UIObjDesc.eUIProj;

	_pUIObj->Set_ProjMatrix(ProjMatrix);
	_pUIObj->Set_UIObj_Desc(LayerUIDesc);

	return TRUE;
}

//완료) 기즈모 동기화
_bool CEditor_UI::Set_GizmoSync(CUIObject* _pUIObj)
{
	static ImGuizmo::OPERATION eCurGizmoOper(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE eCurGizmoMode(ImGuizmo::WORLD);

	//크기,회전,이동 변경 키
	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_E, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::SCALE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_R, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::ROTATE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_T, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::TRANSLATE;
	}

	//오브젝트의 트랜스폼/매트릭스 정보 저장
	CTransform* pUITrans = (CTransform*)_pUIObj->Get_Component(g_strTransformTag);
	_float4x4 UIWorldMat = pUITrans->Get_WorldFloat4x4();
	if (nullptr == pUITrans)
		return FALSE;

	//static _bool useSnap(false);
	_float fGizmoSpeed[3] = {
		0.001f,		//Translate
		0.001f,		//Rotate
		0.001f };		//Scale

	// 뷰, 투영 행렬 정보 로드
	_float4x4 ViewMatrix, ProjMatrix{};
	if (g_IsOrthoProj) //== TRUE
	{
		XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
		XMStoreFloat4x4(&ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
		ImGuizmo::SetOrthographic(TRUE);
	}
	else //g_IsOrthoProj //== FALSE
	{
		XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
		XMStoreFloat4x4(&ProjMatrix, XMMatrixPerspectiveFovLH(g_fFOV, (_float)g_iWinSizeX / g_iWinSizeY, g_fNear, g_fFar));
		ImGuizmo::SetOrthographic(FALSE);
	}

	//기즈모로 변환 값 적용
	ImGuizmo::Manipulate(ViewMatrix.m[0], ProjMatrix.m[0], eCurGizmoOper, eCurGizmoMode, UIWorldMat.m[0], NULL, fGizmoSpeed); /*useSnap ? &snap.x : NULL*/

	//월드행렬 세팅
	pUITrans->Set_WorldMatrix(UIWorldMat);

	return TRUE;
}

//완료) 그리드 생성 및 세팅
_bool CEditor_UI::Set_GizmoGrid()
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

	ImGuizmo::DrawGrid(ViewMatrix.m[0], ProjMatrix.m[0], MatGridX, 10000.f);

	// 기즈모 영역 세팅
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y);

	return TRUE;
}

//추가 필요) 레이어그룹 생성
void CEditor_UI::Create_UIObject(UI_STATE _eUIState, UI_TYPE _eUIType)
{
	string strProtoTag = { "Prototype_GameObject_" };

	if (UI_LAYER == _eUIState) //레이어 생성
	{
#pragma region UI INFO

		UIOBJ_DESC LayerUI_Desc{};
		LayerUI_Desc.wstrUITag = { TEXT("LayerUI") };

		LayerUI_Desc.eUIProj = { PROJ_ORTHO }; //직교투영 기본값
		LayerUI_Desc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
		LayerUI_Desc.vSize = { 100.f, 100.f };
		LayerUI_Desc.vPos = { 0.f, 0.f, 1.f };
		LayerUI_Desc.vDegree = { 0.f, 0.f, 0.f };
		LayerUI_Desc.vColorRGB = { 1.f, 1.f, 1.f };
		LayerUI_Desc.fAlpha = { 1.f };

#pragma endregion

#pragma region  UI_ANIMINFO

		/*
		UIANIM_DESC LayerUIAnim_Desc{};
		LayerUIAnim_Desc.eUIAnimState = { ANIM_END };
		LayerUIAnim_Desc.fAnimFPS = { 0.f };

		LayerUIAnim_Desc.iPreFrame = { 0 };
		LayerUIAnim_Desc.iCurrFrame = { 0 };
		LayerUIAnim_Desc.iStartFrame = { 0 };
		LayerUIAnim_Desc.iEndFrame = { 0 };
		LayerUIAnim_Desc.fFrameAcc = { 0.f };

		LayerUIAnim_Desc.eUIAnimType = { ANIM_NONE };
		LayerUIAnim_Desc.vScale = { 0.f, 0.f, 0.f };
		LayerUIAnim_Desc.vTrans = { 0.f, 0.f, 0.f };
		LayerUIAnim_Desc.vRotate = { 0.f, 0.f, 0.f };
		LayerUIAnim_Desc.fDuration = { 0.f };

		LayerUIAnim_Desc.strAnimTag = { "" };;
		*/

#pragma endregion
		
		strProtoTag += CUtils::WstrToStr(LayerUI_Desc.wstrUITag);
		
		if (UI_TEXTURE == _eUIType)
		{
			LayerUI_Desc.eUIType = UI_TEXTURE;
			LayerUI_Desc.iTexIndex = { 0 };
		}

		if (UI_FONT == _eUIType)
		{
			LayerUI_Desc.eUIType = UI_FONT;
			LayerUI_Desc.wstrText = { TEXT("") };
		}

		CUIObject* pLayerUI = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(strProtoTag), &LayerUI_Desc));
		if (nullptr == pLayerUI)
		{
			g_IsSuccessed = FALSE;	g_eOpenPopup = POPUP_CREATE;
			CHECK_NULLPTR(pLayerUI);
		}
		m_LayerUIs.push_back(pLayerUI);

		g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_CREATE;
	}

	if (UI_GROUP == _eUIState) //레이어그룹(캔버스) 생성
	{
	}
}

//추가 필요) 레이어그룹 삭제 
void CEditor_UI::Delete_UIObject(UI_STATE _eUIState)
{
	if (m_LayerUIs.empty())
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_DELETE; g_ePopupDetail = NEED_CREATE;

	if (UI_LAYER == _eUIState) //레이어 삭제
	{
		//ListBox에서 선택한 레이어를 삭제
		if (g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size())
		{
			m_LayerUIs.erase(m_LayerUIs.begin() + g_iSelectUI);
			g_iSelectUI = -1; // 삭제 후 선택한 UI 인덱스 초기화
			g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_DELETE;
		}
	}

	if (UI_GROUP == _eUIState) //레이어그룹(캔버스) 삭제
	{
	}
}

//보류) 그룹 선택 후 기즈모 상속 동기화
void CEditor_UI::Group_UIObject(GROUP_TYPE _eUIGroup)
{
	if (m_LayerUIs.empty()) //레이어가 없음
	{
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_GROUP; g_ePopupDetail = NEED_CREATE;
		return;
	}
	if (g_SelectUIs.empty()) //선택 레이어가 없음
	{
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_GROUP; g_ePopupDetail = NEED_SELECT;
		return;
	}

	if (GROUP_ALL == _eUIGroup) //전체 레이어 그룹화
	{
		m_GroupUIs.push_back(m_LayerUIs);

		//g_strDetail = { "Group All" };
		g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_GROUP; g_ePopupDetail = DETAIL_NONE;
	}

	vector<CUIObject*> TpSelectUIs; //임시 벡터

	if (GROUP_SELECT == _eUIGroup) //선택 레이어 그룹화
	{
		for (auto& SelectIndex : g_SelectUIs) //선택한 항목의 벡터 인덱스 검색
		{
			if ((g_iSelectUI >= 0 && g_iSelectUI < m_LayerUIs.size()))
			{
				TpSelectUIs.push_back(m_LayerUIs[SelectIndex]);
			}
		}

		if (!TpSelectUIs.empty()) //선택 레이어가 존재할 경우
		{
			m_GroupUIs.push_back(TpSelectUIs); //선택 항목의 요소를 벡터에 추가
			g_SelectUIs.clear(); //선택 항목 해제
		}

		//g_strDetail = { "Group Select" };
		g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_GROUP; g_ePopupDetail = DETAIL_NONE;
	}

}

//보류) 텍스처화 :: 엔진에서 렌더한 RTV 정보를 Texture2D로 받아 저장하는 방식
void CEditor_UI::Save_Texture(const string& _strFilePath, ID3D11RenderTargetView* _pRTV)
{
	string strFilePath = { "../Bin/Resources/Textures/UI/DDS/" };
	string strUITag = {};
	for (auto& iUI : m_LayerUIs)
		strUITag = CUtils::WstrToStr(iUI->Get_UIObj_Desc().wstrUITag);

	strFilePath += strUITag + ".dds";
	wstring wstrFilePath = CUtils::StrToWstr(strFilePath);

	if (m_LayerUIs.empty())
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = NEED_CREATE;

	//텍스처 저장
	if (FAILED(DirectX::SaveDDSTextureToFile(m_pContext, m_pTexture2D, wstrFilePath.c_str())))
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = FILE_OPEN;

	g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_SAVE;

#pragma region READY_UI

	//06.04) UI 렌더타겟 뷰 생성 및 준비
	//렌더할 뷰포트 세팅
	//if (FAILED(m_pGameInstance->Ready_RTVDebug(TEXT("Target_UI"), 50.f, 50.f, 100.f, 100.f)))
	//	return E_FAIL;

	//ViewportDesc.Width * 0.25f, ViewportDesc.Height * 0.25f

	//ID3D11Texture2D* pCopyTex2D = { nullptr };
	//if (FAILED(m_pGameInstance->Copy_Resource(TEXT("Target_UI"), &pCopyTex2D)))
	//	return E_FAIL;
	//
	//D3D11_TEXTURE2D_DESC CopyTexDesc{};
	//ZeroMemory(&CopyTexDesc, sizeof(D3D11_TEXTURE2D_DESC));

	//pCopyTex2D->GetDesc(&CopyTexDesc);

	//RTV 직접 접근하면 IMGUI에러로 크래시 발생 (사용안함)
	//D3D11_RENDER_TARGET_VIEW_DESC CopyRTVDesc = {};
	//CopyRTVDesc.Format = CopyTexDesc.Format;
	//CopyRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//CopyRTVDesc.Texture2D.MipSlice = 0;
	//m_pDevice->CreateRenderTargetView(pCopyTex2D, &CopyRTVDesc, &m_pUIRTV);

	//생성한 RTV 데이터 SRV에 복사
	//D3D11_SHADER_RESOURCE_VIEW_DESC		CopySRVDesc = {};
	//CopySRVDesc.Format = CopyTexDesc.Format;
	//CopySRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//CopySRVDesc.Texture2D.MostDetailedMip = 0;
	//CopySRVDesc.Texture2D.MipLevels = 1;

	//복사한 데이터를 기반으로 SRV 생성
	//m_pDevice->CreateShaderResourceView(pCopyTex2D, &CopySRVDesc, &m_pUISRV);
	//Safe_Release(pCopyTex2D);

#pragma endregion

#pragma region RENDER_RTV IMGUI

	//레벨 별 옵션 ON/OFF
	//_uint* iCurrLevel = m_pGameInstance->Get_CurrentLevelID();
	//if (5 == *iCurrLevel) //LEVEL_TOOL_UI 만
	//{
	//	ImGuiWindowFlags Dirwindow_Flags{}; /*= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;*/
	//	if (ImGui::Begin(u8"Preview 미리보기", 0, Dirwindow_Flags))
	//	{
	//		D3D11_VIEWPORT		ViewportDesc{};
	//		_uint				iNumViewports = { 1 };
	//		m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	//		ImVec2 vWinSize = { ViewportDesc.Width * 0.1f, ViewportDesc.Height * 0.1f }; //ImGui::GetWindowSize();
	//		//06.05) UI 렌더타겟 뷰 렌더
	//			//m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("UI Default"),
	//			//	_float2(5.f, 10.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);
	//			//이미 생성한 RTV 데이터 저장 및 DESC에 복사

	//		XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	//		m_WorldMatrix._11 = ViewportDesc.Width;
	//		m_WorldMatrix._22 = ViewportDesc.Height;
	//		m_WorldMatrix._41 = 0.f;
	//		m_WorldMatrix._42 = 0.f;

	//		XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	//		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f));

	//		ImGui::Image((void*)m_pUISRV, vWinSize);

	//	}
	//	ImGui::End();
	//}

#pragma endregion
}

//완료) 데이터 저장
_bool CEditor_UI::Save_FileData(const string& _strFilePath)
{
#pragma region FileName

	//벡터가 비었을 경우 
	if (m_LayerUIs.empty())
	{
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = NEED_CREATE;
		return FALSE;
	}
	string strOriginName = _strFilePath + "_Orig.txt";;
	string strTempName = _strFilePath + "_Temp.txt"; //임시 파일

#pragma endregion

	std::ofstream OutputFile(strTempName, ios::out | std::ios::binary);

	if (!OutputFile.is_open())
	{
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = FILE_OPEN;
		return FALSE;
	}

	size_t size = m_LayerUIs.size();
	OutputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

	//for (size_t i = 0; i < m_LayerUIs.size(); ++i)
	for (auto& pUIObj : m_LayerUIs)
	{	
		//ProtoType Tag
		wstring wstProtoTag = pUIObj->Get_PrototypeTag();
		string strProtoTag = CUtils::WstrToStr(wstProtoTag);
		_uint iProtoTagLen = strProtoTag.length();
		OutputFile.write(reinterpret_cast<const char*>(&iProtoTagLen), sizeof(iProtoTagLen));
		OutputFile.write(strProtoTag.c_str(), iProtoTagLen);

		//eUIType
		UIOBJ_DESC UIobj_Desc = pUIObj->Get_UIObj_Desc();
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.eUIType), sizeof(UIobj_Desc.eUIType));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.eUIProj), sizeof(UIobj_Desc.eUIProj));

		//UITag
		wstring wstrUITag = pUIObj->Get_UIObj_Desc().wstrUITag;//UIobj_Desc.wstrUITag;
		string strUITag = CUtils::WstrToStr(wstrUITag);
		_uint iUITagLen = strUITag.length();
		OutputFile.write(reinterpret_cast<const char*>(&iUITagLen), sizeof(iUITagLen));
		OutputFile.write(strUITag.c_str(), iUITagLen);

		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vCenter), sizeof(UIobj_Desc.vCenter));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vSize), sizeof(UIobj_Desc.vSize));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vPos), sizeof(UIobj_Desc.vPos));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vDegree), sizeof(UIobj_Desc.vDegree));

		_uint iTexIndex = pUIObj->Get_TexIndex();
		OutputFile.write(reinterpret_cast<const char*>(&iTexIndex), sizeof(iTexIndex));

		wstring wstrText = pUIObj->Get_UIObj_Desc().wstrText;
		string strText = CUtils::WstrToStr(wstrText);
		_uint iUIextLen = strText.length();

		OutputFile.write(reinterpret_cast<const char*>(&iUIextLen), sizeof(iUIextLen));
		OutputFile.write(strText.c_str(), iUIextLen);

		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vColorRGB), sizeof(UIobj_Desc.vColorRGB));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.fAlpha), sizeof(UIobj_Desc.fAlpha));
	}

	OutputFile.close();

	remove(strOriginName.c_str()); 	//기존 파일 삭제

	if (rename(strTempName.c_str(), strOriginName.c_str()) != 0) //임시 파일 > 최종 파일
	{
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = FILE_COPY;
		remove(strOriginName.c_str());
		return FALSE;
	}

	g_IsSuccessed = TRUE; g_eOpenPopup = POPUP_SAVE; g_ePopupDetail = DETAIL_NONE;
	return TRUE;
}

//완료) 데이터 로드
_bool CEditor_UI::Load_FileData(const string& _strFilePath)
{
	std::ifstream InputFile(_strFilePath, ios::in | std::ios::binary);

	if (!InputFile.is_open()) //==FALSE 
	{
		g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = FILE_OPEN;
		return FALSE;
	}

	size_t size = 0;
	InputFile.read(reinterpret_cast<char*>(&size), sizeof(size));
	m_LayerUIs.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		string strProtoTag = {};
		_uint iProtoTagLen = {};
		InputFile.read(reinterpret_cast<char*>(&iProtoTagLen), sizeof(iProtoTagLen));
		strProtoTag.resize(iProtoTagLen);
		InputFile.read(&strProtoTag[0], iProtoTagLen);

		if (0 == strProtoTag.size()) //프로토타입 태그 읽기 불가한 경우
		{
			g_IsSuccessed = FALSE; g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = FILE_READ;
			return FALSE;
		}

		UIOBJ_DESC UIobj_Desc{};
		string strUITag = {};
		_uint iUITagLen = {};
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.eUIType), sizeof(UIobj_Desc.eUIType));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.eUIProj), sizeof(UIobj_Desc.eUIProj));

		InputFile.read(reinterpret_cast<char*>(&iUITagLen), sizeof(iUITagLen));
		strUITag.resize(iUITagLen);
		InputFile.read(&strUITag[0], iUITagLen);
		UIobj_Desc.wstrUITag = CUtils::StrToWstr(strUITag);

		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vCenter), sizeof(UIobj_Desc.vCenter));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vSize), sizeof(UIobj_Desc.vSize));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vPos), sizeof(UIobj_Desc.vPos));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vDegree), sizeof(UIobj_Desc.vDegree));

		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.iTexIndex), sizeof(UIobj_Desc.iTexIndex));

		string strText = {};
		_uint iUIextLen = {};
		InputFile.read(reinterpret_cast<char*>(&iUIextLen), sizeof(iUIextLen));
		strText.resize(iUIextLen);
		InputFile.read(&strText[0], iUIextLen);
		UIobj_Desc.wstrText = CUtils::StrToWstr(strText);

		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vColorRGB), sizeof(UIobj_Desc.vColorRGB));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.fAlpha), sizeof(UIobj_Desc.fAlpha));

		CUIObject* pUIObject = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(strProtoTag), &UIobj_Desc));
		m_LayerUIs.push_back(pUIObject);
	}
	g_IsSuccessed = TRUE;

	if (g_IsSuccessed)
	{
		InputFile.close();
		g_eOpenPopup = POPUP_LOAD; g_ePopupDetail = DETAIL_NONE;
	}

	return TRUE;
}

CEditor_UI* CEditor_UI::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
{
	CEditor_UI* pInstance = new CEditor_UI(_pDevice, _pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : UI_Editor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEditor_UI::Clone(void* _pArg)
{
	CEditor_UI* pInstance = new CEditor_UI(*this);

	if (FAILED(pInstance->Initialize(_pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : UI_Editor"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEditor_UI::Free()
{
	__super::Free();
}