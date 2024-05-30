#include "stdafx.h"
#include "UI_Editor.h"
#include "ImGUI_Manager.h"

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

#pragma region GET_GAMEOBJ

	//CUIObject* pUIObj = dynamic_cast<CUIObject*>(m_pGameInstance->Get_GameObject(LEVEL_TOOL_UI, TEXT("Layer_UI"), 0));
	//CHECK_NULLPTR(pUIObj);
	//m_vecUIObj.push_back(pUIObj);

#pragma endregion

#pragma region LOAD_FILEDATA

	//에디터 첫 실행 시, 파일데이터 읽고 로드
	string strFilePath = "../Bin/Resources/Data/UI/";
	Load_FileData(strFilePath + "Test_Origin.txt");

#pragma endregion

	return S_OK;
}

_int CUI_Editor::Tick(_float _fTimeDelta)
{
	__super::Tick(_fTimeDelta);

	//Set_OrthoProj();
	if (!m_vecUIObj.empty())
	{
		for (auto& pUIObj : m_vecUIObj)
			pUIObj->Tick(_fTimeDelta);
	}

	return OBJ_NOEVENT;
}

void CUI_Editor::Late_Tick(_float _fTimeDelta)
{
	if (!m_vecUIObj.empty())
	{
		for (auto& pUIObj : m_vecUIObj)
			pUIObj->Late_Tick(_fTimeDelta);
	}

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

	ImGuiWindowFlags Dirwindow_Flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	if (ImGui::Begin(u8"Directory 경로 ", 0, Dirwindow_Flags))
	{
		if (ImGui::BeginTabBar(u8"##Directory"))
		{
			if (ImGui::BeginTabItem(u8"HUD"))
			{
				ImGui::SeparatorText(u8"UI List 목록");

				if (ImGui::BeginListBox(u8"##UI List", ImVec2(285, 200)))
				{
					_int iSelect = -1;
					for (size_t i = 0; i < m_vecUIObj.size(); ++i)
					{
 						string strUITag = CUtils::WstrToStr(m_vecUIObj[i]->Get_UIObj_Desc().wstrUITag);
						
						if (strUITag.empty()) //wstrUITag 값에 대한 예외처리
							strUITag = "##";

						if (ImGui::Selectable(strUITag.c_str(), iSelect == i))
						{
							iSelect = i;
						}

						if (iSelect == i) //목록 선택할 경우, 활성화
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndListBox();
				}

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
	ImGuiColorEditFlags ColorButton_Flags = ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoTooltip;
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
	ImGuizmo::SetOrthographic(TRUE); //기즈모 직교기준

	//오브젝트의 트랜스폼/매트릭스 정보 저장
	CTransform* UITrans = (CTransform*)(m_pGameInstance->Get_Component(LEVEL_TOOL_UI, TEXT("Layer_UI"), g_strTransformTag, 0));
	_float4x4 UIWorldMat = UITrans->Get_WorldFloat4x4();

	if (nullptr == UITrans)
		return FALSE;

	static ImGuizmo::OPERATION eCurGizmoOper(ImGuizmo::TRANSLATE);
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

		case ImGuizmo::ROTATE:
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
			eCurGizmoOper = ImGuizmo::SCALE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_R, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::ROTATE;

		else if (m_pGameInstance->Get_DIKeyState(DIK_T, KEY_DOWN))
			eCurGizmoOper = ImGuizmo::TRANSLATE;
	}

	// 기즈모 드래그 위젯 연동
	_float Translate[3], Rotate[3], Scale[3];	
	ImGuizmo::DecomposeMatrixToComponents(UIWorldMat.m[0], Translate, Rotate, Scale);

	ImGui::Text(u8"Scale 크기");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Scale", (_float*)Scale, 0.1f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Translate 위치");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat3("##Translate", (_float*)&Translate, 0.1f, 0.f, g_iWinSizeX, "%.1f");

	ImGui::Text(u8"Rotate 회전");
	ImGui::SameLine(fTextWidth + 20);
	ImGui::DragFloat("##Rotate", (_float*)&Rotate[2], 0.1f, (_int)-360, (_int)360, u8"Degree 각도 : %.1f");

	ImGuizmo::RecomposeMatrixFromComponents(Translate, Rotate, Scale, UIWorldMat.m[0]);

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
	UITrans->Set_WorldMatrix(UIWorldMat);

	//UIObj 순회하며 변경 값 적용
	for (auto& iter : m_vecUIObj)
	{
		m_UIObjDesc.wstrUITag = iter->Get_UIObj_Desc().wstrUITag;
		m_UIObjDesc.vPos = (_float2)Translate;
		m_UIObjDesc.fRaito = Rotate[2];
		m_UIObjDesc.vSize = (_float2)Scale;

		iter->Set_UIObj_Desc(m_UIObjDesc);
	}

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

	ImGuizmo::DrawGrid(ViewMatrix.m[0], ProjMatrix.m[0], MatGridX, 10000.f);

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

			if (ImGui::BeginMenu(u8"File 파일"))
			{
				if (ImGui::MenuItem(u8"Create 생성"))
				{
					if (Create_UIObject())
						MSG_BOX(TEXT("Successed to Create : UI Object"));

					else
						MSG_BOX(TEXT("Failed to Create : UI Object"));
				}

				if (ImGui::MenuItem(u8"Save 저장"))
				{
					if (Save_FileData("../Bin/Resources/Data/UI/")) //==TRUE
						MSG_BOX(TEXT("Successed to Save : FileData"));

					else
						MSG_BOX(TEXT("Failed to Save : FileData"));
				}

				if (ImGui::MenuItem(u8"Load 로드")) //==TRUE
				{
					string strFilePath = "../Bin/Resources/Data/UI/";

					if (Load_FileData(strFilePath + "Test_Origin.txt"))
						MSG_BOX(TEXT("Successed to Load : FileData"));

					else
						MSG_BOX(TEXT("Failed to Load : FileData"));
				}

				ImGui::EndMenu();
			}

			// 파일 다이얼로그 (ImGUI_Manager::Set_FileDialog() 참고)
			/*switch (m_pGameInstance->Set_FileDialog())
			{
			case CImGUI_Manager::FILE_MODE::FILE_SAVE:
				Save_FileData();
				break;

			case CImGUI_Manager::FILE_MODE::FILE_LOAD:
				Load_FileData("../Bin/Resources/Data/UI/Test.txt");
				break;
			}*/
			ImGui::EndMainMenuBar();
		}

		ImGui::End(); //창 종료
	}

	return TRUE;
}

_bool CUI_Editor::Create_UIObject()
{
	CUIObject* pUIObj = dynamic_cast<CUIObject*>(m_pGameInstance->Get_GameObject(LEVEL_TOOL_UI, TEXT("Layer_UI"), 0));
	if (nullptr == pUIObj)
	{ 
		CHECK_NULLPTR(pUIObj);
		return FALSE;
	}
	m_vecUIObj.push_back(pUIObj);

	return TRUE;
}

_bool CUI_Editor::Save_FileData(string _strFilePath)
{
	string strOriginName = _strFilePath + "Test_Origin.txt";
	string strTempName = _strFilePath + "Test_Temp.txt"; //임시

	std::ofstream OutputFile(strTempName, ios::out | std::ios::binary);

	if (!OutputFile.is_open()) //==FALSE
	{
		MSG_BOX(TEXT("Failed to Open : FileData"));
		return FALSE;
	}

	size_t size = m_vecUIObj.size();
	if (m_vecUIObj.empty())
	{
		OutputFile.close();
		return FALSE;
	}

	OutputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

	//for (auto& iter : m_vecUIObj)
	for (size_t i = 0; i < m_vecUIObj.size(); ++i)
	{
		//wstring wstProtoTag = iter->Get_PrototypeTag();
		wstring wstProtoTag = m_vecUIObj[i]->Get_PrototypeTag();
		string strProtoTag = CUtils::WstrToStr(wstProtoTag);
		_uint iProtoTagLen = strProtoTag.length();

		OutputFile.write(reinterpret_cast<const char*>(&iProtoTagLen), sizeof(iProtoTagLen));
		OutputFile.write(strProtoTag.c_str(), iProtoTagLen);

		//m_UIObjDesc = iter->Get_UIObj_Desc();
		m_UIObjDesc = m_vecUIObj[i]->Get_UIObj_Desc();
		wstring wstrUITag = m_UIObjDesc.wstrUITag;
		//wstring wstrNum = to_wstring(i);

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

		string strUITag = CUtils::WstrToStr(wstrUITag);
		_uint iUITagLen = strUITag.length();

		//OutputFile.write(reinterpret_cast<const char*>(&m_UIObjDesc.wstrUITag), sizeof(m_UIObjDesc.wstrUITag));
		OutputFile.write(reinterpret_cast<const char*>(&iUITagLen), sizeof(iUITagLen));
		OutputFile.write(strUITag.c_str(), iUITagLen);

		OutputFile.write(reinterpret_cast<const char*>(&m_UIObjDesc.vCenter), sizeof(m_UIObjDesc.vCenter));
		OutputFile.write(reinterpret_cast<const char*>(&m_UIObjDesc.vSize), sizeof(m_UIObjDesc.vSize));
		OutputFile.write(reinterpret_cast<const char*>(&m_UIObjDesc.vPos), sizeof(m_UIObjDesc.vPos));
		OutputFile.write(reinterpret_cast<const char*>(&m_UIObjDesc.fRaito), sizeof(m_UIObjDesc.fRaito));

		//CUIObject* pUIObject = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(strProtoTag), &m_UIObjDesc));
		//m_vecUIObj.push_back(pUIObject);
	}

	OutputFile.close();

	if (!OutputFile)
	{
		MSG_BOX(TEXT("Failed to Write : FileData"));
		remove(strTempName.c_str()); 	//기존파일 삭제
		return FALSE;
	}

	remove(strOriginName.c_str()); 	//기존파일 삭제

	if (rename(strTempName.c_str(), strOriginName.c_str()) != 0) //임시 파일 > 최종 파일
	{
		MSG_BOX(TEXT("Failed to Rename Temp > Origin"));
		remove(strOriginName.c_str());
		return FALSE;
	}

	return TRUE;
}

_bool CUI_Editor::Load_FileData(const string& _strFilePath)
{
	std::ifstream InputFile(_strFilePath, ios::in | std::ios::binary);

	if (!InputFile.is_open()) //==FALSE 
	{
		MSG_BOX(TEXT("Load Failed : UI"));
		return FALSE;
	}

	size_t size = 0;
	InputFile.read(reinterpret_cast<char*>(&size), sizeof(size));
	m_vecUIObj.reserve(size);

	for (size_t i = 0; i < size; ++i)
	{
		string strProtoTag = {};
		_uint iProtoTagLen = {};
		InputFile.read(reinterpret_cast<char*>(&iProtoTagLen), sizeof(iProtoTagLen));
		strProtoTag.resize(iProtoTagLen);
		InputFile.read(&strProtoTag[0], iProtoTagLen);

		if (0 == strProtoTag.size())
			return FALSE;

		string strUITag;
		_uint iUITagLen; 
		InputFile.read(reinterpret_cast<char*>(&iUITagLen), sizeof(iUITagLen));
		strUITag.resize(iUITagLen);
		InputFile.read(&strUITag[0], iUITagLen);
		
		//InputFile.read(reinterpret_cast<char*>(&m_UIObjDesc.wstrUITag), sizeof(m_UIObjDesc.wstrUITag));
		InputFile.read(reinterpret_cast<char*>(&m_UIObjDesc.vCenter), sizeof(m_UIObjDesc.vCenter));
		InputFile.read(reinterpret_cast<char*>(&m_UIObjDesc.vSize), sizeof(m_UIObjDesc.vSize));
		InputFile.read(reinterpret_cast<char*>(&m_UIObjDesc.vPos), sizeof(m_UIObjDesc.vPos));
		InputFile.read(reinterpret_cast<char*>(&m_UIObjDesc.fRaito), sizeof(m_UIObjDesc.fRaito));

		//list box용 태그 연동
		m_UIObjDesc.wstrUITag = CUtils::StrToWstr(strUITag);

		CUIObject* pUIObject = dynamic_cast<CUIObject*>(m_pGameInstance->Clone_GameObject(CUtils::StrToWstr(strProtoTag), &m_UIObjDesc));
		m_vecUIObj.push_back(pUIObject);
	}

	InputFile.close();

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
	if (!m_vecUIObj.empty())
	{
		for (auto& pUIObj : m_vecUIObj)
			Safe_Release(pUIObj);

		m_vecUIObj.clear();
	}

	__super::Free();
}
;