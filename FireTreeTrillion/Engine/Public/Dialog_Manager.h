#pragma once

#include "Base.h"

BEGIN(Engine)

class CDialog_Manager final : public CBase
{
	struct DialogMessage {
		wstring wstrMessage = { L"" };
		_float fDisplayTime = { 0.f }; //메시지 표시 속도
		//size_t	iCurIndex = { 0 }; 
	};

private:
	CDialog_Manager();
	virtual ~CDialog_Manager() = default;

public:
	HRESULT Add_Message(const wstring& _wstrMessage, _float _fDisplayTime);
	HRESULT Start_Message();
	HRESULT	Display_Message(const wstring& _wstrMessage, _float _fDisplayTime);

private:
	vector<DialogMessage>	m_vecMessage;

//private:
//	class CCustomFont* Find_Font(const wstring& strFontTag);

public:
	static CDialog_Manager* Create();
	virtual void Free() override;
};

END