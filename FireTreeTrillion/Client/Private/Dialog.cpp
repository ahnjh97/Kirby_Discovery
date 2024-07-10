#include "stdafx.h"
#include "Dialog.h"
#include "UI_MessageWindow.h"

#include <codecvt>
#include <locale>

CDialog::CDialog(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CDialog::CDialog(const CDialog& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CDialog::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDialog::Initialize(void* pArg)
{
	DIALOG_DESC desc = *(DIALOG_DESC*)pArg;

	// 레벨마다 다른 path를 받아 해당 정보에 맞는 Messgae Window를 생성합니다.
	Load(desc.strPath);

	return S_OK;
}

_int CDialog::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CDialog::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CDialog::Render()
{
	return S_OK;
}

// UTF-16 문자열을 UTF-8 문자열로 변환하는 함수
string CDialog::utf8_encode(const wstring& wstr) 
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// UTF-8 문자열을 UTF-16 문자열로 변환하는 함수
wstring CDialog::utf8_decode(const string& str) 
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void CDialog::Save()
{
	json data;
	data["Level"] = LEVEL_DEEDEEDEE;
	data["NPC"] = "Parttimer_Dee";

	vector<wstring> messages = {
		L"안녕 나는 위지영이야.",
		L"오늘의 점심은 무얼까.",
		L"두부일까 짬뽕일까?"
	};

	for (const auto& message : messages) 
	{
		data["Messages"].push_back(utf8_encode(message));
	}

	wstring	wstrFont = L"Font_HUDSub_KR15";
	data["FontTag"] = utf8_encode(wstrFont);
	data["FontPos"] = { 410.f, 725.f};
	data["FontRGBA"] = { 0.f / 255.f, 138.f / 255.f, 121.f / 255.f, 1.f };
	data["FontSize"] = { 1.f, 1.f };
	data["FontScale"] = { 1.2f, 1.2f };
	data["Radian"] = 0.f;
	data["DisplayTime"] = 0.1f;
	data["ElapsedyTime"] = 0.f;

	string file_path = "../Bin/Resources/Data/Dialog.json";
	ofstream output_file(file_path);
	if (!output_file.is_open())
	{
		MSG_BOX(TEXT("파일을 열 수 없습니다: Save"));
		return;
	}

    // JSON 데이터를 파일에 저장
	try {
		output_file << data.dump(4); // .dump(4) == JSON을 예쁘게 출력하기 위한 들여쓰기 설정
	}
	catch (const exception& e) 
	{
		MSG_BOX(TEXT("JSON 파일 쓰기 중 에러 발생: "));
		return;
	}

	output_file.close();
	MSG_BOX(TEXT("JSON 파일이 생성되었습니다: "));
}

void CDialog::Load(string strPath)
{
	HRESULT hr(S_OK);
	ifstream input_file(strPath);
	if (!input_file.is_open()) 
	{
		MSG_BOX(TEXT("파일을 열 수 없습니다: Load"));
		return;
	}

	json data;
	try 
	{
		input_file >> data;
	}
	catch (const exception& e) 
	{
		MSG_BOX(TEXT("JSON 파일 읽기 중 에러 발생: "));
		return;
	}
	input_file.close();

	m_tMessage_Desc.uLevel = data.value("Level", 0);
	string npc = data.value("NPC", "");
	m_tMessage_Desc.wstrNPC = utf8_decode(npc);
	vector<wstring>	vecMsg;
	for (auto& msg : data["Messages"])
	{
		vecMsg.push_back(utf8_decode(msg));
	}

	CUI_MessageWindow::MESSAGE_DESC tMessage_Desc = {};
	tMessage_Desc.wstrFontTag   = utf8_decode(data.value("FontTag", ""));
	tMessage_Desc.fFontPos      = { data["FontPos"][0], data["FontPos"][1] };
	tMessage_Desc.fFontRGBA     = { data["FontRGBA"][0], data["FontRGBA"][1], data["FontRGBA"][2], data["FontRGBA"][3] };
	tMessage_Desc.fFontSize     = { data["FontSize"][0], data["FontSize"][1] };
	tMessage_Desc.fFontScale    = { data["FontScale"][0], data["FontScale"][1] };
	tMessage_Desc.fRadian	    = data.value("Radian", 0.f);
	tMessage_Desc.fDisplayTime  = data.value("DisplayTime", 0.f);
	tMessage_Desc.fElapsedyTime = data.value("ElapsedyTime", 0.f);

	//Title
	//tMessage_Desc.fElapsedyTime = data.value("ElapsedyTime", 0.f); //TitleTag
	//tMessage_Desc.fElapsedyTime = data.value("ElapsedyTime", 0.f); //TitlePos
	//tMessage_Desc.fElapsedyTime = data.value("ElapsedyTime", 0.f); //TitleRGBA
	//tMessage_Desc.fElapsedyTime = data.value("ElapsedyTime", 0.f); //TitleSize

	//Highlight

	
	tMessage_Desc.vecMsg		= vecMsg;
	hr = m_pGameInstance->Add_Clone(m_tMessage_Desc.uLevel, TEXT("Layer_UI_Dialog"), TEXT("Prototype_GameObject_UI_MessageWindow"), &tMessage_Desc);
	CHECK_FAILED(hr);
}

CDialog* CDialog::Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
{
	CDialog* pInstance = new CDialog(_pDevice, _pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CDialog"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDialog::Clone(void* pArg)
{
	CDialog* pInstance = new CDialog(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDialog"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDialog::Free()
{
	__super::Free();

}

