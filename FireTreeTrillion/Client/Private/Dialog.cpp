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
	
	//json or csv 파일 파싱 및 로드를 여기에서.
	//Load_FileData();


	//Font에 대한 정보를 던짐
	MESSAGE_DESC FontDesc{};
	FontDesc.wstrFontTag = { TEXT("Font_HUDSub_KR15") };
	FontDesc.wstrMessage = { TEXT("고마워~ 덕분에 살았어~!") };
	FontDesc.fFontPos = { 410.f, 725.f };
	FontDesc.fFontRGBA = { 0.f / 255.f, 138.f / 255.f, 121.f / 255.f, 1.f };
	FontDesc.fFontSize = { 1.f, 1.f };
	FontDesc.fFontScale = { 1.2f, 1.2f };
	FontDesc.fRadian = { XMConvertToRadians(0.f) };

	FontDesc.fDisplayTime = 0.1f;
	FontDesc.fElapsedyTime = 0.f;

	Add_Message(/*FontDesc.wstrMessage, 0.1f, */&FontDesc);

	return S_OK;
}

_int CDialog::Tick(_float fTimeDelta)
{
	Display_Message(fTimeDelta);

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
			Render_Message(Message.wstrMessage);
	}

	return S_OK;
}

// 파싱해서 가져온 대화내용들을 vector안에 넣어줍니다.
HRESULT CDialog::Add_Message(/*const wstring& _wstrMessage, _float _fDisplayTime, */void* _pArg)
{
	MESSAGE_DESC* pMessage_Desc = (MESSAGE_DESC*)_pArg;

	m_vecMessage.push_back({ pMessage_Desc->wstrMessage});
	m_fDisplayTime = pMessage_Desc->fDisplayTime;
	m_fElapsedTime = pMessage_Desc->fElapsedyTime;

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

// 다이얼로그 출력합니다.
// ps. 폰트하다가 안되면 이미지
HRESULT CDialog::Render_Message(const wstring& _wstrMessage)
{
	//SpriteFont 폰트 수정 필요.
	wstring wstrFontTag = { TEXT("Font_HUDSub_KR15") };
	_float2 vFontPos = { 410.f, 725.f };
	_float4 vFontRGBA = {	0.f / 255.f, 138.f / 255.f, 121.f / 255.f, 1.f };

	_float2 vFontOrig = { 1.f, 1.f };
	_float2 vFontScale = { 1.2f, 1.2f };
	_float fRadian = { XMConvertToRadians(0.f) };

	if (m_iCurMessageIndex < m_vecMessage.size())
	{
		DialogMessage tMessage = m_vecMessage[m_iCurMessageIndex];
		wstring& wstrSubstrMessage = tMessage.wstrMessage.substr(0, m_iCurCharIndex);

		m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrMessage, vFontPos, vFontRGBA, fRadian, vFontOrig, vFontScale);
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
	__super::Free();

	if (!m_vecMessage.empty())
	{
		for (auto& Message : m_vecMessage)
			m_vecMessage.clear();
	//	//	Safe_Release(Message);
	}
}

