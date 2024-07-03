#include "..\Public\Dialog_Manager.h"
#include "CustomFont.h"

CDialog_Manager::CDialog_Manager()
{

}

// 파싱해서 가져온 대화내용들을 vector안에 넣어줍니다.
HRESULT CDialog_Manager::Add_Message(const wstring& _wstrMessage, _float _fDisplayTime)
{
	m_vecMessage.push_back({ _wstrMessage, _fDisplayTime });
	return S_OK;
}

// 다이얼로그 호출 : vec에 담아둔 메세지를 출력합니다.
HRESULT CDialog_Manager::Start_Message()
{
	for (auto& Message : m_vecMessage)
		Display_Message(Message.wstrMessage, Message.fDisplayTime);
	
	return S_OK;
}

// 다이얼로그 출력합니다.
// ps. 폰트하다가 안되면 이미지
HRESULT CDialog_Manager::Display_Message(const wstring& _wstrMessage, _float _fDisplayTime)
{
	for (size_t iIndex = 0; iIndex < _wstrMessage.length(); ++iIndex)
	{
		_wstrMessage.substr(iIndex, 1);
	}

	return S_OK;
}

CDialog_Manager * CDialog_Manager::Create()
{
	return new CDialog_Manager();
}

void CDialog_Manager::Free()
{
	//for (auto& Message : m_vecMessage)
	//	Safe_Release(Message);

	//m_vecMessage.clear();
}

