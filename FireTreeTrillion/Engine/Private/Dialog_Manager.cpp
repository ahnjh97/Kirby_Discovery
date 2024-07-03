#include "..\Public\Dialog_Manager.h"
#include "CustomFont.h"

CDialog_Manager::CDialog_Manager()
{

}

HRESULT CDialog_Manager::Add_Message(const wstring& _wstrMessage, _float _fDisplayTime)
{
	m_vecMessage.push_back({ _wstrMessage, _fDisplayTime });
	return S_OK;
}

HRESULT CDialog_Manager::Start_Message()
{
	for (auto& Message : m_vecMessage)
		Display_Message(Message.wstrMessage, Message.fDisplayTime);
	
	return S_OK;
}

HRESULT CDialog_Manager::Display_Message(const wstring& _wstrMessage, _float _fDisplayTime)
{
	for (size_t iIndex = 0; iIndex < _wstrMessage.length(); ++iIndex)
	{
		_wstrMessage.substr(0, 1);
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

