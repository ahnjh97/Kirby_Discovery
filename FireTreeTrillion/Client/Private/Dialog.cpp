#include "stdafx.h"
#include "Dialog.h"

CDialog::CDialog(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CDialog::CDialog(const CDialog& _rhs)
	: CUIObject{ _rhs }
{
}

// 파싱해서 가져온 대화내용들을 vector안에 넣어줍니다.
HRESULT CDialog::Add_Message(const wstring& _wstrMessage, _float _fDisplayTime)
{
	m_vecMessage.push_back({ _wstrMessage, _fDisplayTime });
	return S_OK;
}

// 다이얼로그 호출 : vec에 담아둔 메세지를 출력합니다.
HRESULT CDialog::Start_Message()
{
	for (auto& Message : m_vecMessage)
		Display_Message(Message.wstrMessage, Message.fDisplayTime);
	
	return S_OK;
}

// 다이얼로그 출력합니다.
// ps. 폰트하다가 안되면 이미지
HRESULT CDialog::Display_Message(const wstring& _wstrMessage, _float _fDisplayTime)
{
	for (size_t iIndex = 0; iIndex < _wstrMessage.length(); ++iIndex)
	{
		_wstrMessage.substr(iIndex, 1);
	}

	//SpriteFont 폰트 수정 필요.
	wstring wstrFontTag = { TEXT("Font_HUDSub_KR15") };
	//wstring wstrText = { TEXT("???") };
	_float2 vFontPos = { 410.f, 725.f };
	//_float4 vFontRGBA = { m_UIObjDesc.vColorRGB };
	//_float4 vFontRGBA = { 176.f / 255.f, 12.f / 255.f, 24.f / 255.f, m_UIObjDesc.fAlpha };
	_float4 vFontRGBA = {	0.f / 255.f, 
							138.f / 255.f, 
							121.f / 255.f, 
							1.f };

	//vFontRGBA.w = m_UIObjDesc.fAlpha;

	_float2 vFontOrig = { 1.f, 1.f };
	_float2 vFontScale = { 1.2f, 1.2f };
	_float fRadian = { XMConvertToRadians(0.f) };

	m_pGameInstance->Render_Font(wstrFontTag, _wstrMessage, vFontPos, vFontRGBA, fRadian, vFontOrig, vFontScale);

	return S_OK;
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

	//for (auto& Message : m_vecMessage)
	//	Safe_Release(Message);
	//m_vecMessage.clear();
}

