#include "stdafx.h"
#include "UI_MessageWindow.h"
#include "UI_BtnIcon.h"
#include "Kirby.h"
#include "Level_Loading.h"
#include "TransingStar.h"
#include "EventCenter.h"
#include "UI_Fading.h"

CUI_MessageWindow::CUI_MessageWindow(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject { _pDevice, _pContext }
{
}

CUI_MessageWindow::CUI_MessageWindow(const CUI_MessageWindow& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CUI_MessageWindow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_MessageWindow::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	MESSAGE_DESC* MessageWindowDesc{};
	m_tMessageDesc = *(MESSAGE_DESC*)_pArg;

	if (FAILED(Add_Transform(_pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
#pragma region MESSAGEWINDOW BASE

	m_UIObjDesc.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f, 0.f };
	m_UIObjDesc.vPos = { 0.f, -325.f, 1.f, 1.f };

	m_UIObjDesc.vSize = { 1360.f * 0.8f, 288.f * 0.8f, 1.f };

	_float4 vBaseTrans = { m_UIObjDesc.vPos };
	vBaseTrans.w = 1.f;
	m_pTransCom[TEXMW_BASE]->Set_State(CTransform::STATE_POSITION, vBaseTrans);
	m_pTransCom[TEXMW_BASE]->Set_Scaled(m_UIObjDesc.vSize);

#pragma endregion

#pragma region MESSAGEWINDOW BTN BASE

	_float3 vScale = { 76.f * 0.8f, 76.f * 0.8f, 1.f };
	m_pTransCom[TEXMW_BTNBASE]->Set_Scaled(vScale);
	m_vBtnScale = m_pTransCom[TEXMW_BTNBASE]->Get_Scaled();

	_float4 vBtnTrans{};
	switch (*m_pCurrentLevelID)
	{
	case LEVEL_TOWN: case LEVEL_DEEDEEDEE:
		vBtnTrans = { 503.f, -394.f, 1.f, 1.f };
		break;

	case LEVEL_SIMBA: case LEVEL_FINALBOSS: case LEVEL_FINALE: default:
		vBtnTrans = { 509.f, -394.f, 1.f, 1.f };
		break;
	}

	m_pTransCom[TEXMW_BTNBASE]->Set_State(CTransform::STATE_POSITION, vBtnTrans);

#pragma endregion

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	m_vBaseScale = m_pTransformCom->Get_Scaled();

	m_UIObjDesc.fAlpha = 0.f;
	m_eCurState = WINDOW_IDLE;


#pragma region UI_BUTTON

	m_pUIBtn = static_cast<CUI_BtnIcon*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_UI_BtnIcon")));
	if (nullptr == m_pUIBtn)
		return E_FAIL;

#pragma endregion

	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();
	// 하이라이트 처리를 하기 위한 문자열 정리하는 함수
	Split_Message();

	function<void(CGameObject*)> func{};
	CEventCenter* pEventCenter = CEventCenter::Get_Instance();

	if (LEVEL_SIMBA == *m_pCurrentLevelID)
	{
		func = bind(&CUI_MessageWindow::Start_Message, this, placeholders::_1);
		pEventCenter->Subscribe(KEVENT_SIMBA_APPEAR_START, this, func);
	}

	m_bEventCall = false;
	return S_OK;
}

_int CUI_MessageWindow::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_pUIBtn->Tick(fTimeDelta);

	//테스트용
	//if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN) && LEVEL_DEEDEEDEE == *m_pCurrentLevelID)
	//	Show_DialogMessage();
	
	//A 버튼 입력 시, 다음 스크립트 문단을 준비하여 출력
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_DOWN) && WINDOW_SHOW == m_eCurState)
	{
		m_pUIBtn->Set_BtnState(CUI_BtnIcon::BTN_STATE::BTN_SELECT); //버튼 상태 동기화

		m_iCurMessageIndex += 1; //벡터의 다음 문단 줄로 넘김
		m_iCurCharIndex = m_iCurCharIndexHightlight = m_iCurCharIdxPostHightlight = 0; //글자 수는 초기화
		m_bSignalHightlight = m_bSignalPostHightlight = false;
		
		if (m_iCurMessageIndex == m_tMessageDesc.vecMsg.size()) //벡터에 담긴 메시지들의 크기를 체크
		{
			m_eCurState = WINDOW_HIDE;
			m_pUIBtn->Set_BtnState(CUI_BtnIcon::BTN_STATE::BTN_HIDE);
			OnEvent(); //모든 스크립트 재생 종료 시, 해당 이벤트를 수행
			m_bEventCall = true;
		}

		if(LEVEL_SIMBA == *m_pCurrentLevelID)
		{ 
			if (3 == m_iCurMessageIndex)
				CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_NEXT_DIALOG1);
			if (7 == m_iCurMessageIndex)
				CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_NEXT_DIALOG2);
			if (m_iCurMessageIndex == m_tMessageDesc.vecMsg.size() - 1)
				CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_LAST_DIALOG);
			if (m_iCurMessageIndex == m_tMessageDesc.vecMsg.size())
				CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_APPEAR_END);
		}
	}

	if (m_bEventCall && *m_pCurrentLevelID == LEVEL_DEEDEEDEE)
		Event_Tick(fTimeDelta);

	_float3 vOffset = { 0.9f, 0.9f, 1.f };
	_float3 vShowScale{};
	switch (m_eCurState)
	{
	case WINDOW_IDLE: 
		m_UIObjDesc.fAlpha = 0.f;
		break;

	case WINDOW_HIDE: //알파 값 및 스케일 감소
		m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;	

		//vOffset.y -= EASE_OUT(fTimeDelta * 2.5f);
		//m_pTransCom[TEXMW_BASE]->Set_Scaled(m_vBaseScale * vOffset);
		//m_pTransCom[TEXMW_BTNBASE]->Set_Scaled(m_vBtnScale * vOffset);
		break;

	case WINDOW_SHOW: //알파 값 및 스케일 증가
		m_UIObjDesc.fAlpha += fTimeDelta * 5.f;	
		Display_Message(fTimeDelta);
		break;

	default:	break;
	}

	if (m_UIObjDesc.fAlpha >= 1.f)
		m_UIObjDesc.fAlpha = 1.f;

	if (m_UIObjDesc.fAlpha <= 0.f) //알파 값 보정 및 업데이트 중지
	{
		m_UIObjDesc.fAlpha = 0.f;
		return OBJ_NOEVENT;
	}	


	return OBJ_NOEVENT;
}

void CUI_MessageWindow::Late_Tick(_float fTimeDelta)
{
	m_pUIBtn->Late_Tick(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);

}

HRESULT CUI_MessageWindow::Render()
{
	if (WINDOW_HIDE == m_eCurState && 0.f == m_UIObjDesc.fAlpha)
		return S_OK;

#pragma region RENDER_BINDSET

	HRESULT hr;
	for (_uint iTEXIx = 0; iTEXIx < TEXMW_NONE; ++iTEXIx)
	{
		TEX_MWTYPE eTexType = { TYPE_DEFAULT };
		switch (*m_pCurrentLevelID)
		{
		case LEVEL_TOWN: case LEVEL_DEEDEEDEE: 
				eTexType = TYPE_DEFAULT;
			break;

		case LEVEL_SIMBA: case LEVEL_FINALBOSS: case LEVEL_FINALE:
			if (TEXMW_BTNBASE == iTEXIx) //해당 레벨에서는 출력x
				continue;

			else
				eTexType = TYPE_BOSS;
			break;

		default: break;
		}
	
		if (FAILED(m_pTransCom[iTEXIx]->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		//셰이더 파일의 매트릭스 정보를 가져와 바인딩
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
			return E_FAIL;

		PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };
		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTexCom[iTEXIx], eTexType);
		CHECK_FAILED(hr);
	}

#pragma endregion

#pragma region BASE_CLAW
	
	switch (*m_pCurrentLevelID)
	{
	case LEVEL_SIMBA: case LEVEL_FINALBOSS: case LEVEL_FINALE:
		if (FAILED(m_pTransCom[TEXMW_BASE]->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
			return E_FAIL;

		hr = Bind_ShaderResources(m_pShaderCom, POSTEX_UIMWBASE_CLAW, m_pTexClaw, 0);
		CHECK_FAILED(hr);
	break;

	default: break;
	}

#pragma endregion
	
	//버튼 렌더링
	m_pUIBtn->Render();
	
	//폰트 렌더링
	for (auto& Message : m_tMessageDesc.vecMsg)
		Render_Message();

	return S_OK;
}

#ifdef DEBUG
void CUI_MessageWindow::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case WINDOW_IDLE:			ImGui::Text(u8"MWINDOW_IDLE"); break;
	case WINDOW_HIDE:		ImGui::Text(u8"MWINDOW_HIDE"); break;
	case WINDOW_SHOW:		ImGui::Text(u8"MWINDOW_SHOW"); break;
	case WINDOW_NONE:	default: ImGui::Text(u8"MWINDOW_NONE"); break;
	}
}
#endif // DEBUG

void CUI_MessageWindow::Show_DialogMessage()
{
	//특정 트리거가 발동할 경우, 해당 Window UI를 출력
	//Window UI 출력은 스크립트가 종료될때까지 유지
	if (WINDOW_HIDE == m_eCurState) //단, idle 상태일 경우는 트리거 시점에 show해야하므로 hide만 처리
		return;
	
	m_eCurState = WINDOW_SHOW;
	m_pUIBtn->Set_BtnState(CUI_BtnIcon::BTN_STATE::BTN_BLINK);	//버튼 상태 동기화
}

HRESULT CUI_MessageWindow::Add_Transform(void* _pArg)
{
	for (_uint iTrans = 0; iTrans < TEXMW_NONE; ++iTrans)
	{
		m_pTransCom[iTrans] = CTransform::Create(m_pDevice, m_pContext);

		if (nullptr == m_pTransCom[iTrans])
			return E_FAIL;

		if (FAILED(m_pTransCom[iTrans]->Initialize(_pArg)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CUI_MessageWindow::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MessageWindow_Base"),
		TEXT("Com_TexBase"), (CComponent**)&m_pTexCom[TEXMW_BASE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MessageWindow_Base_Claw"),
		TEXT("Com_TexBase_Claw"), (CComponent**)&m_pTexClaw)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MessageWindow_BtnBase"),
		TEXT("Com_TexBtnBase"), (CComponent**)&m_pTexCom[TEXMW_BTNBASE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_MessageWindow::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
{
	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//셰이더의 원시데이터 가져와 저장
	_pShaderCom->Bind_RawValue("g_vRColor", &m_UIObjDesc.vColorRGB, sizeof(_float3));

	_pShaderCom->Bind_RawValue("g_fAlpha", &m_UIObjDesc.fAlpha, sizeof(_float));

	//Begin() > Apply() 함수 호출 전 셰이더 전역 데이터를 저장해야함
	if (FAILED(_pShaderCom->Begin(_iPassIndex)))
		return E_FAIL;

	if (FAILED(Bind_VIBuffer(m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_MessageWindow::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

//다이얼로그 메시지 글자/문단 출력 로직
HRESULT CUI_MessageWindow::Display_Message(_float _fTimeDelta)
{
	if (m_tMessageDesc.wstrFontTag.empty())
		return S_OK;

	m_fElapsedTime += _fTimeDelta;
	if (m_fElapsedTime >= m_tMessageDesc.fDisplayTime) //경과시간 대비 출력시간 체크
	{
		m_fElapsedTime = 0.f;

		if (m_iCurMessageIndex < m_tMessageDesc.vecMsg.size()) //벡터에 담긴 메시지들의 크기를 체크
		{
			wstring wstrMsg = m_tMessageDesc.vecMsg[m_iCurMessageIndex];

			if (m_iCurCharIndex < wstrMsg.length()) //메시지 길이 체크
				m_iCurCharIndex++;
		}

		// for hight-light
		const auto& wstrHighlightMsg = get<1>(m_vecSplitMsg[m_iCurMessageIndex]);
		if (m_bSignalHightlight)
		{
			if (m_iCurCharIndexHightlight < wstrHighlightMsg.length()) //메시지 길이 체크
				m_iCurCharIndexHightlight++;
		}

		// for post hight-light
		const auto& wstrPostHighlightMsg = get<2>(m_vecSplitMsg[m_iCurMessageIndex]);
		if (m_bSignalPostHightlight)
		{
			if (m_iCurCharIdxPostHightlight < wstrPostHighlightMsg.length()) //메시지 길이 체크
				m_iCurCharIdxPostHightlight++;
		}
	}

	return S_OK;
}

// 다이얼로그 메시지 렌더
HRESULT CUI_MessageWindow::Render_Message()
{
	if (WINDOW_HIDE == m_eCurState || WINDOW_IDLE == m_eCurState)
		return S_OK;

	wstring wstrFontTag = m_tMessageDesc.wstrFontTag;
	_float2 vFontPos = m_tMessageDesc.vFontPos;
	_float4 vFontRGBA = m_tMessageDesc.vFontRGBA;

	_float2 vFontSize = m_tMessageDesc.vFontSize;
	_float2 vFontScale = m_tMessageDesc.vFontScale;
	_float fRadian = XMConvertToRadians(m_tMessageDesc.fRadian);

	//스크립트
	if (m_iCurMessageIndex < m_tMessageDesc.vecMsg.size())
	{
		wstring wstrMsg = m_tMessageDesc.vecMsg[m_iCurMessageIndex];

		//스크립트 그림자
		_float2 vOffset[] = { {-3.f, 0.f},
							{3.f, 0.f},
							{0.f, -3.f} ,
							{0.f, 3.f} };

		_float4 vMessageShadowRGBA = { 0.1f, 0.1f, 0.1f, 0.1f };
		_float2 vMessageShadowScale = { 1.0f, 1.0f };

		// 스크립트 그림자
		wstring& wstrSubstrMessage = wstrMsg.substr(0, m_iCurCharIndex);
		for (_uint i = 0; i < 4; ++i)
		{
			_float2 vMessageShadowPos = { vFontPos.x + vOffset[i].x, vFontPos.y + vOffset[i].y };
			m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrMessage, vMessageShadowPos, vMessageShadowRGBA, fRadian, vFontSize, vMessageShadowScale);
		}

		const auto& wstrPreHighlightMsg  = get<0>(m_vecSplitMsg[m_iCurMessageIndex]);
		const auto& wstrHighlightMsg	 = get<1>(m_vecSplitMsg[m_iCurMessageIndex]);
		const auto& wstrPostHighlightMsg = get<2>(m_vecSplitMsg[m_iCurMessageIndex]);

		if (m_iCurCharIndex < wstrPreHighlightMsg.size())	//하이라이트 포지션전까지
			m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrMessage, vFontPos, vFontRGBA, fRadian, vFontSize, vFontScale);
		else
		{
			m_pGameInstance->Render_Font(wstrFontTag, wstrPreHighlightMsg, vFontPos, vFontRGBA, fRadian, vFontSize, vFontScale);
			// 다음 튜플 원소 내용 시그널 부울 값
			m_bSignalHightlight = true;
		}

		// 하이라이트 스크립트
		if (m_bSignalHightlight)
		{
			if (m_iCurCharIndexHightlight < wstrHighlightMsg.size())	// 포스트 하이라이트 포지션전까지
			{
				wstring& wstrSubstrHightlightMsg = wstrHighlightMsg.substr(0, m_iCurCharIndexHightlight);
				m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrHightlightMsg, vFontPos, m_tMessageDesc.vHighlightRGBA, fRadian, vFontSize, vFontScale);
			}
			else
			{
				m_pGameInstance->Render_Font(wstrFontTag, wstrHighlightMsg, vFontPos, m_tMessageDesc.vHighlightRGBA, fRadian, vFontSize, vFontScale);
				m_bSignalPostHightlight = true;
			}
		}

		// 하이라이트 하고 난 이후의 스크립트
		if (m_bSignalPostHightlight)
		{
			_float2 pos2D = Repose_Fonts(vFontPos, wstrHighlightMsg);
			//_float2 pos2D = _float2(vFontPos.x + wstrHighlightMsg.size() * 18.4, vFontPos.y + 36.8f);
			if (m_iCurCharIdxPostHightlight < wstrPostHighlightMsg.size())
			{
				wstring& wstrSubstrPostHightlightMsg = wstrPostHighlightMsg.substr(0, m_iCurCharIdxPostHightlight);
				m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrPostHightlightMsg, pos2D, vFontRGBA, fRadian, vFontSize, vFontScale);
			}
			else
				m_pGameInstance->Render_Font(wstrFontTag, wstrPostHighlightMsg, pos2D, vFontRGBA, fRadian, vFontSize, vFontScale);
		}
	}

	//타이틀 (스크립트 대화 대상) 출력
	if (LEVEL_DEEDEEDEE == *m_pCurrentLevelID || LEVEL_TOWN == *m_pCurrentLevelID)
	{
		wstring wstrTitleTag = m_tMessageDesc.wstrTitleTag;
		wstring wstrTitleText = m_tMessageDesc.wstrTitleText;
		_float2	vTitlePos = m_tMessageDesc.vTitlePos;
		_float4	vTitleRGBA = { 1.f, 1.f, 1.f, 1.f };

		_float2 vTitleSize = m_tMessageDesc.vTitleSize;
		_float2 vTitleScale = m_tMessageDesc.vTitleScale;

		//타이틀 그림자
		_float2 vOffset[] = { {-3.f, 0.f},
							{3.f, 0.f},
							{0.f, -3.f} ,
							{0.f, 3.f} };

		_float4 vTitleShadowRGBA = m_tMessageDesc.vTitleRGBA;
		_float2 vTitleShadowScale = { 1.0f, 1.0f };
		for (_uint i = 0; i < 4; ++i)
		{
			_float2 vTitleShadowPos = { vTitlePos.x + vOffset[i].x, vTitlePos.y + vOffset[i].y };
			m_pGameInstance->Render_Font(wstrTitleTag, wstrTitleText, vTitleShadowPos, vTitleShadowRGBA, fRadian, vTitleSize, vTitleShadowScale);
		}
		//타이틀
		m_pGameInstance->Render_Font(wstrTitleTag, wstrTitleText, vTitlePos, vTitleRGBA, 0.f, vTitleSize, vTitleScale);
	}

	return S_OK;
}

void CUI_MessageWindow::Split_Message()
{
	for (_int i = 0; i < m_tMessageDesc.vecMsg.size();++i)
	{
		// 하이라이트를 확인하고자 하는 메세지
		wstring wstrMsg = m_tMessageDesc.vecMsg[i];

		// 해당 메세지(스크립트)에서 하이라이트 하기전/ 하이라이트/ 하이라이트 하고난 이후 메세지 3개로 쪼갠다.
		wstring wstrHighlight = m_tMessageDesc.vecHighlight[i];
		size_t FindHighlightPos = wstrMsg.find(wstrHighlight); // 하이라이트 포지션
		size_t wstrHighlightLength = wstrHighlight.length();   // 하이라이트 길이

		tuple<wstring, wstring, wstring> tupleMsgs;
		if (FindHighlightPos != string::npos) // 하이라이트 문자열이 존재할 경우
		{
			//substr(start, length)
			wstring wstrPreHighlightMsg		= wstrMsg.substr(0, FindHighlightPos); //하이라이트 전 부분
			wstring wstrHighlightMsg		= wstrMsg.substr(FindHighlightPos, wstrHighlightLength); //하이라이트 부분
			wstring wstrPostHighlightMsg	= wstrMsg.substr(FindHighlightPos + wstrHighlightLength); //하이라이트 이후 부분
			tupleMsgs = { wstrPreHighlightMsg, wstrHighlightMsg, wstrPostHighlightMsg };
		}
		else
			tupleMsgs = { wstrMsg, L"", L"" };

		// 실질적으로 나눠진 message를 출력하는 vector
		m_vecSplitMsg.push_back(tupleMsgs);
	}
}

// 글꼴별 위치 재조정
_float2 CUI_MessageWindow::Repose_Fonts(_float2 fontPos, wstring wstrHighlightMsg)
{
	_float2 pos2D = _float2();
	if (m_tMessageDesc.wstrFontTag == L"Font_Dialog_SubKR18spac10")
	{
		if(*m_pCurrentLevelID == LEVEL_DEEDEEDEE)
			pos2D = _float2(fontPos.x + wstrHighlightMsg.size() * 18.4f, fontPos.y + 44.8f);
		else if (*m_pCurrentLevelID == LEVEL_TOWN)
		{
			if(wstrHighlightMsg.size() == 9)
				pos2D = _float2(fontPos.x + wstrHighlightMsg.size() * 22.1f, fontPos.y + 44.8f);
			if(wstrHighlightMsg.size() == 6)
				pos2D = _float2(fontPos.x + wstrHighlightMsg.size() * 19.8f, fontPos.y + 44.8f);
		}
	}
	else if (m_tMessageDesc.wstrFontTag == L"Font_Dialog_Boss_KR22spac10")
	{
		pos2D = _float2(fontPos.x + wstrHighlightMsg.size() * 14.2f, fontPos.y + 45.f);
		if (wstrHighlightMsg.size() == 7)
			pos2D = _float2(fontPos.x + wstrHighlightMsg.size() * 17.92f, fontPos.y + 45.f);
		if (wstrHighlightMsg.size() == 15)
			pos2D = _float2(fontPos.x + wstrHighlightMsg.size() * 19.2f, fontPos.y + 45.f);
	}

	return pos2D;
}

void CUI_MessageWindow::OnEvent()
{
	switch(*m_pCurrentLevelID)
	{
	case LEVEL_TOWN:
	{
		if (m_tMessageDesc.wstrNPC == L"DeeDeeDee")
		{
			CGameObject* pGameObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_TransingStar"));
			CTransingStar* pTransingStar = static_cast<CTransingStar*>(pGameObj);
			pTransingStar->Set_NextLevel(LEVEL_END);
			pTransingStar->Activate(CTransingStar::CLOSE);
			pTransingStar->Set_LargeColor(_float3(95.f / 255.f,  28.f / 255.f, 128.f / 255.f));
			pTransingStar->Set_SmallColor(_float3(167.f / 255.f, 42.f / 255.f, 168.f / 255.f));
		}
		else
		{
			// QZR : 페이드아웃 처리
			m_pGameInstance->Reserve_Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_PARTTIME));
		}
	}
	break;
	case LEVEL_PARTTIME:
	{
		m_pGameInstance->Reserve_Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOWN));
	}
	break;
	case LEVEL_SIMBA:
	{
		CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_APPEAR_END);
	}
	break;
	}
}

//for simba
void CUI_MessageWindow::Start_Message(CGameObject* pObj)
{
	Reset_MessageIndex(nullptr);
	m_bNextDialog1Notified = false;
	m_bNextDialog2Notified = false;
	m_bLastDialogNotified = false;
	m_eCurState = WINDOW_SHOW;
	Show_DialogMessage();
}

// for Fade-Out
void CUI_MessageWindow::Event_Tick(_float fTimeDelta)
{
	static _float fTimeAcc = 0.f;
	fTimeAcc += fTimeDelta;
	if (fTimeAcc > 2.f) // 2초뒤 페이드인
		Ready_FadeOut();
}

void CUI_MessageWindow::Ready_FadeOut()
{
	static _bool bOnceFade = false;
	static _bool bOnceChanger = false;
	CGameObject* pUIObj = m_pGameInstance->Get_GameObject_ByTag(LEVEL_STATIC, TEXT("Layer_ChangerUI"), TEXT("Prototype_GameObject_UI_Fading"));
	CUI_Fading* pFadingUI = static_cast<CUI_Fading*>(pUIObj);

	if (bOnceFade == false)
	{
		pFadingUI->Set_InOutState(CUI_Fading::FADEOUT);
		pFadingUI->Set_IsRender(true);
		bOnceFade = true;
	}
	else if (pFadingUI->Get_FadeRatio() <= 0.f)
	{
		if (bOnceChanger == false)
		{
			m_pGameInstance->Reserve_Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOWN));
			pFadingUI->Set_IsRender(false);
			bOnceChanger = true;
		}
	}
}


CUI_MessageWindow* CUI_MessageWindow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_MessageWindow* pInstance = new CUI_MessageWindow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_MessageWindow"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_MessageWindow::Clone(void* pArg)
{
	CUI_MessageWindow* pInstance = new CUI_MessageWindow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_MessageWindow"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_MessageWindow::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();

	m_tMessageDesc.vecMsg.clear();

	for (auto& iTrans : m_pTransCom)
		Safe_Release(iTrans);

	for (auto& iTex : m_pTexCom)
		Safe_Release(iTex);

	Safe_Release(m_pTexClaw);
	Safe_Release(m_pUIBtn);
}


