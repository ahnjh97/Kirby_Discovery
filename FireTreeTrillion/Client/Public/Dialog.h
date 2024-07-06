#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

// 0. 이친구를 클라로 옮겨주세요.
// 1. 스크립트 내용을 저장한다. //스크립트 내용은 파싱할 것입니다.
// 2. 어떻게 RENDER ON-OFF할 것인지
// 3. 어떻게 스크립트 내용물이 다 끝났다고 판단할 것인지
	// [EX] 벡터에 있는 내용물들을 다 사용하고 empty()인지 확인하거나 벡터에 있는 내용물들을 다 순회해서 사용했는 지 판단하기
BEGIN(Client)
class CDialog : public CUIObject
{
	typedef struct DialogMessage {
		wstring wstrFontTag = { TEXT("") };
		wstring wstrMessage = { TEXT("") };
		_float2	fFontPos = { 0.f, 0.f };
		_float4	fFontRGBA = { 0.f, 0.f, 0.f, 0.f };

		_float2 fFontSize = { 0.f, 0.f }; //원본 사이즈
		_float2 fFontScale = { 0.f, 0.f }; //원본대비 키울 스케일 비율
		_float fRadian = { XMConvertToRadians(0.f) };
		
		_float fDisplayTime = { 0.f }; //출력 시간
		_float fElapsedyTime = { 0.f }; //경과 시간
		//size_t	iCurIndex = { 0 }; 
	}MESSAGE_DESC;

private:
	CDialog(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext);
	CDialog(const CDialog& _rhs);
	virtual ~CDialog() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

public:
	HRESULT Add_Message(/*const wstring& _wstrMessage, _float _fDisplayTime, */void* _pArg);
	HRESULT Display_Message(_float _fTimeDelta);
	HRESULT	Render_Message(const wstring& _wstrMessage);

private:
	vector<DialogMessage>	m_vecMessage;

public:
	static CDialog* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

private:
	MESSAGE_DESC*			m_pMessage_Desc{};

	_float					m_fElapsedTime = { 0.f }; //경과 시간
	_float					m_fDisplayTime = { 0.f }; //출력 시간
	_uint					m_iCurMessageIndex = { 0 }; 
	_uint					m_iCurCharIndex = { 0 };
};
END