#include "stdafx.h"
#include "Dialog.h"
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
	CUIObject::UIOBJ_DESC MessageWindowDesc{};
	MessageWindowDesc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f };
	MessageWindowDesc.vPos = { 0.f, -325.f, 1.f, 1.f };
	MessageWindowDesc.vSize = { 1300.f * 0.8f, 288.f * 0.8f, 1.f };
	
	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_UI_Dialog"), 
		TEXT("Prototype_GameObject_UI_MessageWindow"), &MessageWindowDesc)))
		return E_FAIL;
	
	//json or csv 파일 파싱 및 로드를 여기에서. 파싱 데이터는 사실상 Message(Text) 내용만 파싱하면 됨
	//Load_FileData();


	MESSAGE_DESC tFontDesc{};
	tFontDesc.wstrFontTag = { TEXT("Font_HUDSub_KR15") };
	tFontDesc.wstrMessage = { TEXT("고마워~ 덕분에 살았어~!") };
	tFontDesc.fFontPos = { 410.f, 725.f };
	tFontDesc.fFontRGBA = { 0.f / 255.f, 138.f / 255.f, 121.f / 255.f, 1.f };
	tFontDesc.fFontSize = { 1.f, 1.f };
	tFontDesc.fFontScale = { 1.2f, 1.2f };
	tFontDesc.fRadian = { XMConvertToRadians(0.f) };

	tFontDesc.fDisplayTime = 0.1f;
	tFontDesc.fElapsedyTime = 0.f;

	Add_Message(&tFontDesc);

	return S_OK;
}

_int CDialog::Tick(_float fTimeDelta)
{
	Display_Message(fTimeDelta);

	if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN)) //테스트용
		Load("");

	//Save();
	return OBJ_NOEVENT;
}

void CDialog::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CDialog::Render()
{
	if (!m_vecMessage.empty())
	{
		for (auto& Message : m_vecMessage)
			Render_Message(m_tMessage_Desc);
	}

	return S_OK;
}

// 파싱해서 가져온 대화내용들을 vector안에 넣어줍니다.
HRESULT CDialog::Add_Message(void* _pArg)
{
	MESSAGE_DESC* tMessageDesc{};
	if (_pArg != nullptr)
		tMessageDesc = (MESSAGE_DESC*)_pArg;

	m_tMessage_Desc = *tMessageDesc;

	m_vecMessage.push_back(m_tMessage_Desc);

	m_fDisplayTime = m_tMessage_Desc.fDisplayTime;
	m_fElapsedTime = m_tMessage_Desc.fElapsedyTime;

	return S_OK;
}

// 다이얼로그 호출 : vec에 담아둔 메세지를 출력합니다.
HRESULT CDialog::Display_Message(_float _fTimeDelta)
{
	if (!m_vecMessage.empty())
	{
		m_fElapsedTime += _fTimeDelta;
		if (m_fElapsedTime >= m_fDisplayTime) //경과시간 대비 출력시간 체크
		{
			m_fElapsedTime = 0.f;

			if (m_iCurMessageIndex < m_vecMessage.size()) //벡터에 담긴 메시지들의 크기를 체크
			{
				DialogMessage tMessage = m_vecMessage[m_iCurMessageIndex];
				if (m_iCurCharIndex < tMessage.wstrMessage.length()) //메시지 길이 체크
					m_iCurCharIndex++;

				else //메시지 길이를 넘겼을 경우, 초기화
				{
					m_iCurMessageIndex = 0.f;
					m_iCurCharIndex = 0.f;
				}
			}

		}
	}

	//추후, A버튼 입력 전까지 대기하는 로직 필요
	//iCurCharIndex 비교 체크하여 동일/초과할 경우 대기.
	//A버튼 입력 상태를 확인할 경우, 다음 iCurMeesageIndex로 변경하여 스크립트 문단을 넘긴다.
	
	return S_OK;
}

// 다이얼로그 메시지 출력
HRESULT CDialog::Render_Message(MESSAGE_DESC _tMessageDesc)
{
	//Actor(NPC) 대상 별 SpriteFont 폰트 수정 필요
	wstring wstrFontTag = m_tMessage_Desc.wstrFontTag;
	_float2 vFontPos = m_tMessage_Desc.fFontPos;
	_float4 vFontRGBA = m_tMessage_Desc.fFontRGBA;

	_float2 vFontSize = m_tMessage_Desc.fFontSize;
	_float2 vFontScale = m_tMessage_Desc.fFontScale;
	_float fRadian = m_tMessage_Desc.fRadian;

	if (m_iCurMessageIndex < m_vecMessage.size())
	{
		DialogMessage tMessage = m_vecMessage[m_iCurMessageIndex];
		wstring& wstrSubstrMessage = tMessage.wstrMessage.substr(0, m_iCurCharIndex);

		m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrMessage, vFontPos, vFontRGBA, fRadian, vFontSize, vFontScale);
	}

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
	strPath = "../Bin/Resources/Data/Dialog.json";
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

	MESSAGE_DESC msgDesc{};
	msgDesc.uLevel  = data.value("Level", 0);
	//msgDesc.wstrNPC = data.value("NPC", 0);

	string npc = data.value("NPC", "");
	msgDesc.wstrNPC = utf8_decode(npc);
	for (auto& msg : data["Messages"]) 
	{
		m_vecMsg.push_back(utf8_decode(msg));
	}
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

	if (!m_vecMessage.empty())
	{
		for (auto& Message : m_vecMessage)
			m_vecMessage.clear();
	//	//	Safe_Release(Message);
	}
}

