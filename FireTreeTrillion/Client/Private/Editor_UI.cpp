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

	//Set_OrthoProj();
	if (!m_LayerUIs.empty())
	{
		for (auto& pUIObj : m_LayerUIs)
			pUIObj->Tick(_fTimeDelta);
	}

#pragma region KEY_INPUT

	string strFilePath = { "../../../UI_txt/" };
	string strUITag = {};
	for (auto& pUIObj : m_LayerUIs)
		strUITag = CUtils::WstrToStr(pUIObj->Get_UIObj_Desc().wstrUITag);

	if (m_pGameInstance->Get_DIKeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_N, KEY_DOWN))
			Create_UIObject(UI_LAYER, UI_TEXTURE);

		if (m_pGameInstance->Get_DIKeyState(DIK_Z, KEY_DOWN))
			Delete_UIObject(UI_LAYER);

		if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_DOWN))
			Save_FileData(strFilePath);

		if (m_pGameInstance->Get_DIKeyState(DIK_D, KEY_DOWN))
			Load_FileData(strFilePath + "LayerUI_Orig.txt");

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
	ImGuizmo::SetOrthographic(TRUE); //기즈모 직교기준

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
					if (ImGui::BeginMenu(u8"Layer 레이어", "Ctrl+N"))
					{
						if (ImGui::MenuItem(u8"Texture 텍스처"))
							Create_UIObject(UI_LAYER, UI_TEXTURE);

						if (ImGui::MenuItem(u8"Font 폰트"))
							Create_UIObject(UI_LAYER, UI_FONT);

						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}

				string strFilePath = { "../../../UI_txt/" };
				string strUITag = {};

				for (auto& pUIObj : m_LayerUIs)
					strUITag = CUtils::WstrToStr(pUIObj->Get_UIObj_Desc().wstrUITag);

				if (ImGui::BeginMenu(u8"Save 저장"))
				{
					if (ImGui::MenuItem(u8"Data 데이터", "Ctrl+S"))
						Save_FileData(strFilePath);

					if (ImGui::MenuItem(u8"Texture 텍스처"))
						Save_Texture("../../../UI_dds/LayerUI.dds", m_pRTV); //추후 작업 예정

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem(u8"Load 로드", "Ctrl+D"))
					//Load_FileData(strFilePath + strUITag + "_Orig.txt");
					Load_FileData(strFilePath + "LayerUI_Orig.txt");

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

static _int iSelectUI, iSelectTex = -1;
static vector<_int> SelectUIs;

enum POPUP_TYPE { POPUP_CREATE, POPUP_DELETE, POPUP_GROUP, POPUP_SAVE, POPUP_LOAD, POPUP_MODIFY, POPUP_NONE };
enum POPUP_DETAIL {
	NEED_CREATE, NEED_SELECT,
	FILE_OPEN, FILE_COPY,
	DETAIL_NONE
};

static POPUP_TYPE eOpenPopup = { POPUP_NONE };
static POPUP_DETAIL ePopupDetail = { DETAIL_NONE };

static _bool IsSuccessed = { FALSE };
string strResult, strPopupTag, strMessage, strDetail = { u8" " };
string strUITag = { "LayerUI" };

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

				//const _bool IsSelected = iSelectUI == iUI;
				//if (ImGui::Selectable(strUITag.c_str(), IsSelected)) //리스트박스 항목 단일 선택
				//	iSelectUI = iUI;
				//if (IsSelected) //목록 선택할 경우, 선택 대상에게 기즈모 동기화
				//{
				//	ImGui::SetItemDefaultFocus();
				//	if (!m_LayerUIs.empty())
				//		Set_GizmoSync(m_LayerUIs[iUI]); //기즈모와 위젯, 오브젝트 동기화 작업
				//}

#pragma region LISTBOX 다중선택
				const _bool IsSelected = find(SelectUIs.begin(), SelectUIs.end(), iUI) != SelectUIs.end();
				if (ImGui::Selectable(strUITag.c_str(), IsSelected)) //리스트박스 항목 선택 여부 확인
				{
					if (ImGui::GetIO().KeyShift) //다중 선택
					{
						if (!IsSelected)
							SelectUIs.push_back(iUI);
					}
					else //단일 선택
					{
						SelectUIs.clear();
						SelectUIs.push_back(iUI);
					}
				}
				if (IsSelected) //목록 선택할 경우, 선택 대상에게 기즈모 동기화
				{
					ImGui::SetItemDefaultFocus();

					if (!m_LayerUIs.empty() && !SelectUIs.empty())
					{
						Set_GizmoSync(m_LayerUIs[iUI]); //기즈모와 위젯, 오브젝트 동기화 작업
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
						IsSuccessed = TRUE; eOpenPopup = POPUP_MODIFY;
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
					strUITag += "_Group[" + to_string(iGroupIx) + "]";
				}
			}
		}

		ImGui::PushItemWidth(290.f);
		if (ImGui::BeginCombo(u8"##", (iSelectUI >= 0 && iSelectUI < m_GroupUIs.size()) 
			? strUITag.c_str() : u8"그룹을 선택해주세요."))
		{
			for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
			{
				for (auto& GroupUI : m_GroupUIs)
				{
					string strUITag = CUtils::WstrToStr(GroupUI[iGroupIx]->Get_UIObj_Desc().wstrUITag);
					strUITag += "_" + to_string(iGroupIx);

					if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
						strUITag = "##";

					const _bool IsSelected = iSelectUI == iGroupIx;
					if (ImGui::Selectable(strUITag.c_str(), IsSelected))
						iSelectUI = iGroupIx;
				}
			}
			ImGui::EndCombo();
		}
#pragma endregion
		
		if (ImGui::BeginListBox(u8"##", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
			{
				for (size_t iLayerIx = 0; iLayerIx < m_GroupUIs[iGroupIx].size(); ++iLayerIx)
				{
					auto& GroupUI = m_GroupUIs[iGroupIx][iLayerIx];
					string strUITag = CUtils::WstrToStr(GroupUI->Get_UIObj_Desc().wstrUITag);
					strUITag += "_Group_" + to_string(iGroupIx) + "_" + to_string(iLayerIx);

					if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
						strUITag = "##";

					const _bool IsSelected = find(SelectUIs.begin(), SelectUIs.end(), iGroupIx) != SelectUIs.end();
					if (ImGui::Selectable(strUITag.c_str(), IsSelected)) //리스트박스 항목 선택 여부 확인
					{
						if (ImGui::GetIO().KeyShift) //다중 선택
						{
							if (!IsSelected)
								SelectUIs.push_back(iLayerIx);
						}
						else //단일 선택
						{
							SelectUIs.clear();
							SelectUIs.push_back(iLayerIx);
						}
					}
					if (IsSelected) //목록 선택할 경우, 선택 대상에게 기즈모 동기화
					{
						ImGui::SetItemDefaultFocus();
						if (!m_GroupUIs.empty()) //그룹 상속관계 변환 동기화
						{
							//for (size_t iGroupIx = 0; iGroupIx < m_GroupUIs.size(); ++iGroupIx)
							//{
							//	for (size_t iLayerIx = 0; iLayerIx < m_GroupUIs[iGroupIx].size(); ++iLayerIx)
							//	{
							//		auto& LayerUI = m_GroupUIs[iGroupIx][iLayerIx];

							//		//그룹 요소의 디스크립션을 첫번째 요소기준 증감 보정
							//		UIOBJ_DESC FstLayerDesc = LayerUI[0].Get_UIObj_Desc(); //그룹의 첫번째 요소
							//		UIOBJ_DESC LayerDesc = LayerUI[iLayerIx].Get_UIObj_Desc();
							//		LayerDesc.vPos = FstLayerDesc.vPos - LayerDesc.vPos;

							//		LayerUI[iLayerIx].Set_UIObj_Desc(LayerDesc);

							//		//for (auto& GroupUI : m_GroupUIs[iGroupIx])
							//		Set_GizmoSync(LayerUI); //기즈모와 위젯, 오브젝트 동기화 작업
							//	}
							//}
						}
					}
				}
			}
			ImGui::EndListBox();
		}
		ImGui::PopItemWidth();

		ImGui::EndTabItem();
	}

	return TRUE;
}

_bool CEditor_UI::Window_Textures()
{
	ImGuiWindowFlags TexWindow_Flags = {}; /* ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;*/
	ImGuiComboFlags TexCombo_Flags = {};

	if (ImGui::Begin(u8"Texture 텍스처", 0, TexWindow_Flags))
	{
		ImGui::SeparatorText(u8"Texture List 텍스처 목록");
#pragma region LISTBOX

		//vecUI의 UIObj 순회하며 해당 오브젝트가 가진 텍스처 목록을 출력 
		if (ImGui::BeginListBox(u8"##Texture List", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			if ((iSelectUI >= 0 && iSelectUI < m_LayerUIs.size()))
			{
				//해당 오브젝트의 텍스처 정보
				CTexture* pUITex = dynamic_cast<CTexture*>(m_LayerUIs[iSelectUI]->Get_Component(TEXT("Com_Texture")));
				if (!pUITex)
					return FALSE;

				_uint iMaxTex = pUITex->Get_NumTexture();
				for (size_t iTex = 0; iTex < iMaxTex; ++iTex)
					{
						_uint iTexIndex = m_LayerUIs[iSelectUI]->Get_UIObj_Desc().iTexIndex;
						string strTexTag = CUtils::WstrToStr(m_LayerUIs[iSelectUI]->Get_UIObj_Desc().wstrUITag);
						strTexTag += "_" + to_string(iTex);

						if (strTexTag.empty()) //strTexTag 값에 대한 예외처리
							strTexTag = "##";

						const _bool IsSelected = (iSelectTex == iTex);
						if (ImGui::Selectable(strTexTag.c_str(), IsSelected))
							iSelectTex = iTex;

						if (IsSelected)
						{
							ImGui::SetItemDefaultFocus();

							//선택 오브젝트의 텍스처를 보여줌
							//if (!m_LayerUIs.empty())
							//	iSelectTex = m_LayerUIs[iSelectUI]->Get_TexIndex(); 
						}

						if (ImGui::BeginPopupContextItem()) // 우클릭하면 컨텍스트 메뉴 표시
						{
							if (ImGui::MenuItem(u8"Modify 변경"))
							{
								if (!m_LayerUIs.empty())
									//m_LayerUIs[iSelectUI]->Set_UIObj_Desc();
									m_LayerUIs[iSelectUI]->Set_TexIndex(iSelectTex); //선택 텍스처로 변경
							}

							ImGui::EndPopup();
						}
					}
			}
			ImGui::EndListBox();
		}

#pragma endregion
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

				if (!SelectUIs.empty())
				{
					iSelectUI = SelectUIs.front();
					if ((iSelectUI >= 0 && iSelectUI < m_LayerUIs.size()))
						Edit_Transform(m_LayerUIs[iSelectUI]);
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
				Edit_RGBAColor();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(u8"Text 텍스트 편집"))
			{
				ImGui::SeparatorText(u8"Text Edit 텍스트 편집");
				Edit_Text();

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End(); //창 종료

	return TRUE;
}

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

//전반적인 안내팝업 설정
void CEditor_UI::Window_PopupAlert()
{
	if (!SelectUIs.empty())
	{
		iSelectUI = SelectUIs.front();
		if ((iSelectUI >= 0 && iSelectUI < m_LayerUIs.size()))
		{
			wstring wstrUITag = m_LayerUIs[iSelectUI]->Get_UIObj_Desc().wstrUITag;
			strUITag = CUtils::WstrToStr(wstrUITag); //{ " LayerUI" };
		}
	}

	if (IsSuccessed) strResult = { "Successed" };
	else strResult = { "Failed" };
	//{
	switch (eOpenPopup)
	{
	case POPUP_CREATE:	strPopupTag = { "Create" };
		switch (ePopupDetail)
		{	default: strDetail = { u8" " };	break;	}
		ImGui::OpenPopup(strPopupTag.c_str());
		break;

	case POPUP_DELETE:	strPopupTag = { "Delete" };
		switch (ePopupDetail)
		{	default: strDetail = { u8" " };	break;	}
		ImGui::OpenPopup(strPopupTag.c_str());
		break;

	case POPUP_GROUP:	strPopupTag = { "Group" };
		switch (ePopupDetail)
		{
		case NEED_CREATE:	strDetail = { u8"Need to Create Layer 생성한 레이어가 없습니다" }; break;
		case NEED_SELECT:	strDetail = { u8"Need to Select Layer 레이어를 선택해주세요" }; break;
		default: strDetail = { u8" " }; break;
		}
		ImGui::OpenPopup(strPopupTag.c_str());
		break;

	case POPUP_SAVE:	strPopupTag = { "Save" };
		switch (ePopupDetail)
		{
		case NEED_CREATE:	strDetail = { u8"Need to Create Layer 생성한 레이어가 없습니다" }; break;
		case FILE_OPEN:		strDetail = { u8"Check the File Path & Name 파일 경로와 이름을 확인해주세요" }; break;
		case FILE_COPY:		strDetail = { u8"Copy Failed" }; break;
		case DETAIL_NONE: default: strDetail = { u8" " }; break;
		}
		ImGui::OpenPopup(strPopupTag.c_str());
		break;

	case POPUP_LOAD:	strPopupTag = { "Load" };
		switch (ePopupDetail)
		{
		case FILE_OPEN:		strDetail = { u8"Check the File Path & Name 파일 경로와 이름을 확인해주세요" }; break;
		case DETAIL_NONE: default: strDetail = { u8" " }; break;
		}
		ImGui::OpenPopup(strPopupTag.c_str());
		break;

	case POPUP_MODIFY:	strPopupTag = { "Modify" };
		switch (ePopupDetail)
		{	default: strDetail = { u8" " };	break;	}
		ImGui::OpenPopup(strPopupTag.c_str());
		break;
	}

#pragma region POPUP ALERT

	ImVec2 ViewCenter = ImGui::GetMainViewport()->GetCenter();
	ImGuiWindowFlags Popup_Flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowPos(ViewCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ViewCenter * 0.3f);
	if (ImGui::BeginPopupModal(strPopupTag.c_str(), NULL, Popup_Flags))
	{
		ImVec2 WindowSize = ImGui::GetWindowSize();
		ImVec2 MessageSize = ImGui::CalcTextSize(strMessage.c_str());
		_float MessagePosX = (WindowSize.x - MessageSize.x) * 0.5f;
		ImGui::SetCursorPos(ImVec2(MessagePosX, WindowSize.y * 0.5f - 20.f));
		ImGui::Text(strMessage.c_str());

		ImVec2 DetailSize = ImGui::CalcTextSize(strDetail.c_str());
		_float DetailPosX = (WindowSize.x - DetailSize.x) * 0.5f;
		if ("Modify" == strPopupTag)
		{
			static string strInput(1024 * 16, '\0');
			ImVec2 InputSize = { 200.f, 0.f };
			_float InputPosX = (WindowSize.x - InputSize.x) * 0.5f;

			ImGui::SetNextItemWidth(200.f);
			ImGui::SetCursorPos(ImVec2(InputPosX, WindowSize.y * 0.5f + 3.5f));
			ImGui::InputText(u8"##", &strInput[0], strInput.capacity());

			m_UIObjDesc = Edit_LayerUITag(strInput);
		}
		else
		{
			ImGui::SetCursorPos(ImVec2(DetailPosX, WindowSize.y * 0.5f - 5.f));
			ImGui::Text(strDetail.c_str());
		
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		}

		ImVec2 ButtonSize = ImVec2(120, 0);
		_float ButtonPosX = (WindowSize.x - ButtonSize.x) * 0.5f;
		ImGui::SetCursorPos(ImVec2(ButtonPosX, 100.f));
		if (ImGui::Button("OK", ButtonSize) || m_pGameInstance->Get_DIKeyState(DIK_RETURN, KEY_DOWN)) 
		{	
			if ("Modify" == strPopupTag)
				m_LayerUIs[iSelectUI]->Set_UIObj_Desc(m_UIObjDesc);
				
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

#pragma endregion

	if ("Modify" == strPopupTag)
		strMessage = { u8"변경할 레이어 이름을 입력해주세요" };

	else
		strMessage = strResult + " to " + strPopupTag + " : " + strUITag;

	eOpenPopup = POPUP_NONE;

}

//객체에 대한 변환(크기, 회전, 이동) 편집
_bool CEditor_UI::Edit_Transform(CUIObject* _pUIObj)
{
	const char* DragTag = { "Translate 위치" };
	_float fTextWidth = ImGui::CalcTextSize(DragTag).x;
	_float Translate[3], Rotate[3], Scale[3] = { 0.f, 0.f, 0.f };

	if (nullptr == _pUIObj) //|| UI_FONT == _pUIObj->Get_UIObj_Desc().eUIType)
		return FALSE;

	CTransform* pUITrans = (CTransform*)_pUIObj->Get_Component(g_strTransformTag);
	if (nullptr == pUITrans)
		return FALSE;

	_float4x4 UIWorldMat = pUITrans->Get_WorldFloat4x4();

	// 기즈모 연동
	ImGuizmo::DecomposeMatrixToComponents(UIWorldMat.m[0], Translate, Rotate, Scale);

	UIOBJ_DESC pUIObj_Desc = _pUIObj->Get_UIObj_Desc();
	pUIObj_Desc.vSize = (_float3)Scale;
	pUIObj_Desc.vPos = (_float3)Translate;
	pUIObj_Desc.fDegree = (_float)Rotate[2];

	ImGui::PushItemWidth(ImGui::GetColumnOffset());
	ImGui::Text(u8"Scale 크기");
	ImGui::SameLine(); HelpMarker(u8"Ctrl+E");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::DragFloat3("##Scale", (_float*)Scale, 1.f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Translate 위치");
	ImGui::SameLine(); HelpMarker(u8"Ctrl+T");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::DragFloat3("##Translate", (_float*)&Translate, 1.f, (_float)-0.1 * g_iWinSizeX, (_float)g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Rotate 회전");
	ImGui::SameLine(); HelpMarker(u8"Ctrl+R");
	ImGui::SameLine(fTextWidth + 35);
	ImGui::DragFloat("##Rotate", (_float*)&Rotate[2], 0.1f, (_int)-360, (_int)360, u8"Degree 각도 : %.1f");
	ImGui::PopItemWidth();

	_pUIObj->Set_UIObj_Desc(pUIObj_Desc);

	ImGuizmo::RecomposeMatrixFromComponents(Translate, Rotate, Scale, UIWorldMat.m[0]);

	//월드행렬 세팅
	pUITrans->Set_WorldMatrix(UIWorldMat);

	return TRUE;
}

_bool CEditor_UI::Edit_RGBAColor()
{
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
	ImGuiColorEditFlags ColorButton_Flags = ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_AlphaPreviewHalf /*| ImGuiColorEditFlags_NoTooltip*/;
	ImGuiColorEditFlags ColorEditRGBA_Flags = ImGuiColorEditFlags_NoSmallPreview;
	ImGuiColorEditFlags ColorEditHex_Flags = ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_DisplayHex;
	ImGuiColorEditFlags ColorPicker_Flags = { ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar };

	//컬러버튼
	ImGui::ColorButton("##ColorButton", *(ImVec4*)&color, ColorButton_Flags, ImVec2(50, 45));
	ImGui::SameLine();


	ImGui::BeginGroup();
	ImGui::PushItemWidth(ImGui::GetColumnOffset());
	ImGui::ColorEdit4("##ColorEdit_RGBA", (_float*)&color, ColorEditRGBA_Flags);
	//ImGui::PopItemWidth();

	//ImGui::PushItemWidth(ImGui::GetColumnOffset());
	ImGui::ColorEdit4("##ColorEdit_HEX", (_float*)&color, ColorEditHex_Flags);
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	//ImGui::NewLine();

	//ImGui::PushItemWidth(ImGui::GetColumnOffset());
	ImGui::ColorPicker4("##ColorPicker", (_float*)&color, ColorPicker_Flags);
	//ImGui::PopItemWidth();

	return TRUE;
}

//06.04) 글꼴 편집 기능 구현 
//추후 작업) 선택 개체에 대한 정보와 위젯에 대한 동기화 작업 필요 (UTF-8 변환 이슈로 보류)
_bool CEditor_UI::Edit_Text()
{				
	//폰트 편집 동기화
	static string strInput(1024 * 16, '\0');
	//ImGuiInputTextFlags InputText_flags{}; //= ImGuiInputTextFlags_AllowTabInput;

	ImGui::InputTextMultiline(u8"##", &strInput[0],
		strInput.capacity(), /*IM_ARRAYSIZE(strInputText.c_str()*/
		ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())); /*ImGui::GetTextLineHeight() * 10*/

#pragma region FONT_SYNC

	wstring wstrText{};
	UIOBJ_DESC LayerUIDesc{};

	if (!SelectUIs.empty())
	{
		iSelectUI = SelectUIs.front();
		if ((iSelectUI >= 0 && iSelectUI < m_LayerUIs.size()))
		{
			//UTF-8 인코딩 변환 작업
			LayerUIDesc = m_LayerUIs[iSelectUI]->Get_UIObj_Desc();

			wstrText = CUtils::StrToWstrUTF8(strInput);
			LayerUIDesc.wstrText = wstrText;

			//strInput = CUtils::WstrToStr(FontDesc.wstrText);
			//wstrText = CUtils::StrToWstr(strInput);
			//FontDesc.wstrText = wstrText;
			//strInputText = CUtils::StrToUTF8(strInputText); //strANSI > strUTF8 변환 (만약 이미 UTF8이면 변환 안해도됨)

			m_LayerUIs[iSelectUI]->Set_UIObj_Desc(LayerUIDesc);
			//strTempText = strInputText;

			//if (strInputText != strTempText) //이전에 입력한 값이랑 현재 값이랑 다를 경우
			//{
			//	FontDesc = m_LayerUIs[iSelectUI]->Get_UIObj_Desc();
			//	FontDesc.wstrText = wstrText;
			//	m_LayerUIs[iSelectUI]->Set_UIObj_Desc(FontDesc);
			//}
		}		
	}

#pragma endregion

	return TRUE;
}

CUIObject::UIOBJ_DESC CEditor_UI::Edit_LayerUITag(string _strInput)
{
	static wstring wstrUITag{};
	UIOBJ_DESC LayerUIDesc{};

	if (!SelectUIs.empty())
	{
		iSelectUI = SelectUIs.front();
		if ((iSelectUI >= 0 && iSelectUI < m_LayerUIs.size()))
		{
			LayerUIDesc = m_LayerUIs[iSelectUI]->Get_UIObj_Desc();

			wstrUITag = CUtils::StrToWstrUTF8(_strInput);
			LayerUIDesc.wstrUITag = wstrUITag;

			return LayerUIDesc;
			//m_LayerUIs[iSelectUI]->Set_UIObj_Desc(LayerUIDesc);
		}
	}

	return LayerUIDesc;
}

_bool CEditor_UI::Set_OrthoProj()
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

//기즈모 동기화
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

	// 뷰, 투영 행렬 정보 로드
	_float4x4 ViewMatrix, ProjMatrix;
	XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	//static _bool useSnap(false);
	_float fGizmoSpeed[3] = {
		0.01f,		//Translate
		0.01f,		//Rotate
		0.01f };		//Scale

	//기즈모로 변환 값 적용
	ImGuizmo::Manipulate(ViewMatrix.m[0], ProjMatrix.m[0], eCurGizmoOper, eCurGizmoMode,
		UIWorldMat.m[0], NULL, fGizmoSpeed); /*useSnap ? &snap.x : NULL*/

	//월드행렬 세팅
	pUITrans->Set_WorldMatrix(UIWorldMat);

	return TRUE;
}

//그리드 생성 및 세팅
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

//추가 필요) 레이어그룹(캔버스) 생성
void CEditor_UI::Create_UIObject(UI_STATE _eUIState, UI_TYPE _eUIType)
{
	string strProtoTag = { "Prototype_GameObject_" };

	if (UI_LAYER == _eUIState) //레이어 생성
	{
		UIOBJ_DESC LayerUI_Desc{};
		//LayerUI_Desc.eUIType = { TYPE_LAYER };
		LayerUI_Desc.wstrUITag = { TEXT("LayerUI") };
		LayerUI_Desc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
		LayerUI_Desc.vSize = { 100.f, 100.f };
		LayerUI_Desc.vPos = { 0.f, 0.f };
		LayerUI_Desc.fDegree = { 0.f };
		//LayerUI_Desc.iTexIndex = { 0 };

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
			LayerUI_Desc.vColorRGBA = { 1.f, 1.f, 1.f, 1.f };
		}

		CUIObject* pLayerUI = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(strProtoTag), &LayerUI_Desc));
		if (nullptr == pLayerUI)
		{
			IsSuccessed = FALSE;
			eOpenPopup = POPUP_CREATE;
			CHECK_NULLPTR(pLayerUI);
		}
		m_LayerUIs.push_back(pLayerUI);

		IsSuccessed = TRUE; eOpenPopup = POPUP_CREATE;
	}

	if (UI_GROUP == _eUIState) //레이어그룹(캔버스) 생성
	{
	}
}

//추가 필요) 레이어그룹 삭제 
void CEditor_UI::Delete_UIObject(UI_STATE _eUIState)
{
	if (m_LayerUIs.empty())
		IsSuccessed = FALSE; eOpenPopup = POPUP_DELETE; ePopupDetail = NEED_CREATE;

	if (UI_LAYER == _eUIState) //레이어 삭제
	{
		//ListBox에서 선택한 레이어를 삭제
		if (iSelectUI >= 0 && iSelectUI < m_LayerUIs.size())
		{
			m_LayerUIs.erase(m_LayerUIs.begin() + iSelectUI);
			iSelectUI = -1; // 삭제 후 선택한 UI 인덱스 초기화
			IsSuccessed = TRUE; eOpenPopup = POPUP_DELETE;
		}
	}

	if (UI_GROUP == _eUIState) //레이어그룹(캔버스) 삭제
	{
	}
}

//진행 보류) 그룹 선택 후 기즈모 상속 동기화
void CEditor_UI::Group_UIObject(GROUP_TYPE _eUIGroup)
{
	if (m_LayerUIs.empty()) //레이어가 없음
	{
		IsSuccessed = FALSE; eOpenPopup = POPUP_GROUP; ePopupDetail = NEED_CREATE;
		return;
	}
	if (SelectUIs.empty()) //선택 레이어가 없음
	{
		IsSuccessed = FALSE; eOpenPopup = POPUP_GROUP; ePopupDetail = NEED_SELECT;
		return;
	}

	if (GROUP_ALL == _eUIGroup) //전체 레이어 그룹화
	{
		m_GroupUIs.push_back(m_LayerUIs);

		//strDetail = { "Group All" };
		IsSuccessed = TRUE; eOpenPopup = POPUP_GROUP; ePopupDetail = DETAIL_NONE;
	}

	vector<CUIObject*> TpSelectUIs; //임시 벡터

	if (GROUP_SELECT == _eUIGroup) //선택 레이어 그룹화
	{
		for (auto& SelectIndex : SelectUIs) //선택한 항목의 벡터 인덱스 검색
		{
			if ((iSelectUI >= 0 && iSelectUI < m_LayerUIs.size()))
			{
				TpSelectUIs.push_back(m_LayerUIs[SelectIndex]);
			}
		}

		if (!TpSelectUIs.empty()) //선택 레이어가 존재할 경우
		{
			m_GroupUIs.push_back(TpSelectUIs); //선택 항목의 요소를 벡터에 추가
			SelectUIs.clear(); //선택 항목 해제
		}

		//strDetail = { "Group Select" };
		IsSuccessed = TRUE; eOpenPopup = POPUP_GROUP; ePopupDetail = DETAIL_NONE;
	}

}

//진행 보류) 텍스처화 :: RTV 기준으로 저장 (저장은 되나, RTV 세팅 필요(셰이더 담당자 협업))
//엔진에서 렌더한 RTV 정보를 받아 저장하는 방식
void CEditor_UI::Save_Texture(const string& _strFilePath, ID3D11RenderTargetView* _pRTV)
{
	string strFilePath = { "../Bin/Resources/Textures/UI/DDS/" };
	string strUITag = {};
	for (auto& iUI : m_LayerUIs)
		strUITag = CUtils::WstrToStr(iUI->Get_UIObj_Desc().wstrUITag);

	strFilePath += strUITag + ".dds";
	wstring wstrFilePath = CUtils::StrToWstr(strFilePath);

	if (m_LayerUIs.empty())
		IsSuccessed = FALSE; eOpenPopup = POPUP_SAVE; ePopupDetail = NEED_CREATE;

	//텍스처 저장
	if (FAILED(DirectX::SaveDDSTextureToFile(m_pContext, m_pTexture2D, wstrFilePath.c_str())))
		IsSuccessed = FALSE; eOpenPopup = POPUP_SAVE; ePopupDetail = FILE_OPEN;

	IsSuccessed = TRUE; eOpenPopup = POPUP_SAVE;
}

//데이터 저장
_bool CEditor_UI::Save_FileData(const string& _strFilePath)
{
	string strUITag = {};
	UI_TYPE eUIType = { UI_NONE };
	for (auto& iUI : m_LayerUIs)
	{
		strUITag = CUtils::WstrToStr(iUI->Get_UIObj_Desc().wstrUITag);
		eUIType = iUI->Get_UIObj_Desc().eUIType;
	}

	//벡터가 비었을 경우 
	if (m_LayerUIs.empty())
	{
		IsSuccessed = FALSE; eOpenPopup = POPUP_SAVE; ePopupDetail = NEED_CREATE;
		return FALSE;
	}

	string strOriginName = _strFilePath + strUITag + "_Orig.txt";
	string strTempName = _strFilePath + strUITag + "_Temp.txt"; //임시 파일

	std::ofstream OutputFile(strTempName, ios::out | std::ios::binary);

	if (!OutputFile.is_open())
	{
		IsSuccessed = FALSE; eOpenPopup = POPUP_SAVE; ePopupDetail = FILE_OPEN;
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

		//UITag
		wstring wstrUITag = UIobj_Desc.wstrUITag;
		string strUITag = CUtils::WstrToStr(wstrUITag);
		_uint iUITagLen = strUITag.length();
		OutputFile.write(reinterpret_cast<const char*>(&iUITagLen), sizeof(iUITagLen));
		OutputFile.write(strUITag.c_str(), iUITagLen);

		//size_t wstrPos = wstrUITag.find(L"-"); //문자열 위치
		//저장할 때 wstrUITag에 대한 예외처리
		//if (wstrUITag == m_UIObjDesc.wstrUITag)
		//{
		//}
		//if (wstrPos != wstring::npos) //문자열 위치에 언더바가 존재할 경우
		//{
		//	wstrUITag = wstrUITag.substr(0, wstrPos); //언더바 이전 문자열만 남김
		//	++i;
		//	wstrNum = to_wstring(i);
		//	wstrUITag += TEXT("-") + wstrNum;
		//}
		//wstrUITag += TEXT("-") + wstrNum;


		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vCenter), sizeof(UIobj_Desc.vCenter));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vSize), sizeof(UIobj_Desc.vSize));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vPos), sizeof(UIobj_Desc.vPos));
		OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.fDegree), sizeof(UIobj_Desc.fDegree));

		//if (UI_TEXTURE == eUIType)
		//{
			_uint iTexIndex = pUIObj->Get_TexIndex();
			OutputFile.write(reinterpret_cast<const char*>(&iTexIndex), sizeof(iTexIndex));
		//}

		//if (UI_FONT == eUIType)
		//{
			wstring wstrText = pUIObj->Get_UIObj_Desc().wstrText;
			string strText = CUtils::WstrToStr(wstrText);
			_uint iUIextLen = strText.length();

			OutputFile.write(reinterpret_cast<const char*>(&iUIextLen), sizeof(iUIextLen));
			OutputFile.write(strText.c_str(), iUIextLen);
			//OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.wstrText), sizeof(UIobj_Desc.wstrText));

			OutputFile.write(reinterpret_cast<const char*>(&UIobj_Desc.vColorRGBA), sizeof(UIobj_Desc.vColorRGBA));
		//}
	}

	OutputFile.close();

	//if (!OutputFile)
	//{
	//	IsSuccessed = FALSE; eOpenPopup = POPUP_SAVE; ePopupDetail = FILE_WRITE;
	//	remove(strTempName.c_str()); 	//기존 파일 삭제
	//	return FALSE;
	//}

	remove(strOriginName.c_str()); 	//기존 파일 삭제

	if (rename(strTempName.c_str(), strOriginName.c_str()) != 0) //임시 파일 > 최종 파일
	{
		IsSuccessed = FALSE; eOpenPopup = POPUP_SAVE; ePopupDetail = FILE_COPY;
		remove(strOriginName.c_str());
		return FALSE;
	}

	//MSG_BOX(TEXT("Successed to Save : FileData"));
	IsSuccessed = TRUE; eOpenPopup = POPUP_SAVE; ePopupDetail = DETAIL_NONE;
	return TRUE;
}

//데이터 로드
_bool CEditor_UI::Load_FileData(const string& _strFilePath)
{
	std::ifstream InputFile(_strFilePath, ios::in | std::ios::binary);

	if (!InputFile.is_open()) //==FALSE 
	{
		//MSG_BOX(TEXT("Failed to Open : FileData"));
		IsSuccessed = FALSE; eOpenPopup = POPUP_LOAD; ePopupDetail = FILE_OPEN;
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

		if (0 == strProtoTag.size())
			return FALSE;

		UIOBJ_DESC UIobj_Desc{};
		string strUITag = {};
		_uint iUITagLen = {};
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.eUIType), sizeof(UIobj_Desc.eUIType));

		InputFile.read(reinterpret_cast<char*>(&iUITagLen), sizeof(iUITagLen));
		strUITag.resize(iUITagLen);
		InputFile.read(&strUITag[0], iUITagLen);

		UIobj_Desc.wstrUITag = CUtils::StrToWstr(strUITag);

		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vCenter), sizeof(UIobj_Desc.vCenter));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vSize), sizeof(UIobj_Desc.vSize));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vPos), sizeof(UIobj_Desc.vPos));
		InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.fDegree), sizeof(UIobj_Desc.fDegree));

		//if (UI_TEXTURE == UIobj_Desc.eUIType)
			InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.iTexIndex), sizeof(UIobj_Desc.iTexIndex));

		//if (UI_FONT == UIobj_Desc.eUIType)
		//{
			string strText = {};
			_uint iUIextLen = {};
			InputFile.read(reinterpret_cast<char*>(&iUIextLen), sizeof(iUIextLen));
			strText.resize(iUIextLen);
			InputFile.read(&strText[0], iUIextLen);
			UIobj_Desc.wstrText = CUtils::StrToWstr(strText);

			InputFile.read(reinterpret_cast<char*>(&UIobj_Desc.vColorRGBA), sizeof(UIobj_Desc.vColorRGBA));
		//}


		CUIObject* pUIObject = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(strProtoTag), &UIobj_Desc));
		m_LayerUIs.push_back(pUIObject);
	}

	InputFile.close();

	//MSG_BOX(TEXT("Successed to Load : FileData"));
	IsSuccessed = TRUE; eOpenPopup = POPUP_LOAD; ePopupDetail = DETAIL_NONE;
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