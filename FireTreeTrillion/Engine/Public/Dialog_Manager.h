#pragma once

#include "Base.h"

BEGIN(Engine)


// 0. 이친구를 클라로 옮겨주세요.
// 1. 스크립트 내용을 저장한다. //스크립트 내용은 파싱할 것입니다.
// 2. 어떻게 RENDER ON-OFF할 것인지
// 3. 어떻게 스크립트 내용물이 다 끝났다고 판단할 것인지
	// [EX] 벡터에 있는 내용물들을 다 사용하고 empty()인지 확인하거나 벡터에 있는 내용물들을 다 순회해서 사용했는 지 판단하기
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