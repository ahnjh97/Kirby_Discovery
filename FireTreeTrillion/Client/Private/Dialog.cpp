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
	//for (auto& Message : m_vecMessage)
	//	Safe_Release(Message);
	//m_vecMessage.clear();
}

