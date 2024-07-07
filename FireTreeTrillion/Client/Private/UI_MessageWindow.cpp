#include "stdafx.h"
#include "UI_MessageWindow.h"
#include "UI_BtnIcon.h"
#include "Kirby.h"

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
	m_UIObjDesc.vSize = { 1300.f * 0.8f, 288.f * 0.8f, 1.f };

	_float4 vBaseTrans = { m_UIObjDesc.vPos };
	vBaseTrans.w = 1.f;
	m_pTransCom[TEXMW_BASE]->Set_State(CTransform::STATE_POSITION, vBaseTrans);

	m_pTransCom[TEXMW_BASE]->Set_Scaled(m_UIObjDesc.vSize);

#pragma endregion

#pragma region MESSAGEWINDOW BTN BASE

	_float3 vScale = { 76.f, 76.f, 1.f };
	m_pTransCom[TEXMW_BTNBASE]->Set_Scaled(vScale);
	m_vBtnScale = m_pTransCom[TEXMW_BTNBASE]->Get_Scaled();

	_float4 vBtnTrans = { 479.f, -390.f, 1.f, 1.f };
	m_pTransCom[TEXMW_BTNBASE]->Set_State(CTransform::STATE_POSITION, vBtnTrans);

#pragma endregion

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	m_vBaseScale = m_pTransformCom->Get_Scaled();

	m_UIObjDesc.fAlpha = 0.f;
	m_eCurState = WINDOW_HIDE;

#pragma region UI_BUTTON

	m_pUIBtn = static_cast<CUI_BtnIcon*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_UI_BtnIcon")));
	if (nullptr == m_pUIBtn)
		return E_FAIL;

#pragma endregion

	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();

	return S_OK;
}

_int CUI_MessageWindow::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	m_pUIBtn->Tick(fTimeDelta);

	//특정 트리거가 발동할 경우, 해당 Window UI를 출력
	if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN) && WINDOW_HIDE == m_eCurState) //테스트용
	{
		m_eCurState = WINDOW_SHOW;
		m_pUIBtn->Set_BtnState(CUI_BtnIcon::BTN_STATE::BTN_BLINK);
	}
	//Window UI 출력은 스크립트가 종료될때까지 유지
	//스크립트 인덱스가 종료될 경우, State를 변경
	else if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN) && WINDOW_SHOW == m_eCurState) //테스트용
	{
		m_eCurState = WINDOW_HIDE;
		m_pUIBtn->Set_BtnState(CUI_BtnIcon::BTN_STATE::BTN_HIDE);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_DOWN)) // RESET
	{
		m_iCurMessageIndex = 0.f;
		m_iCurCharIndex = 0.f;
	}

	_float3 vOffset = { 0.9f, 0.9f, 1.f };
	_float3 vShowScale{};
	switch (m_eCurState)
	{
	case WINDOW_IDLE: 
		break;

	case WINDOW_HIDE: //알파 값 및 스케일 감소
		m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;	
		//vOffset.y -= EASE_OUT(fTimeDelta * 2.5f);
		//m_pTransCom[TEXMW_BASE]->Set_Scaled(m_vBaseScale * vOffset);
		//m_pTransCom[TEXMW_BTNBASE]->Set_Scaled(m_vBtnScale * vOffset);
		break;

	case WINDOW_SHOW: //알파 값 및 스케일 증가
		m_UIObjDesc.fAlpha += fTimeDelta * 5.f;	
		//vOffset.y += EASE_OUT(fTimeDelta * 5.f);
		//vShowScale = m_vBaseScale * vOffset;
		//m_pTransCom[TEXMW_BASE]->Set_Scaled(vShowScale);

		//if (vShowScale.y > m_vBaseScale.y)
			//m_pTransCom[TEXMW_BASE]->Set_Scaled(m_vBaseScale);
			//m_pTransCom[TEXMW_BTNBASE]->Set_Scaled(m_vBtnScale);
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

	if(m_eCurState != HIDE_WINDOW)
		Display_Message(fTimeDelta);

	return OBJ_NOEVENT;
}

void CUI_MessageWindow::Late_Tick(_float fTimeDelta)
{
	m_pUIBtn->Late_Tick(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_MessageWindow::Render()
{
	HRESULT hr;
#pragma region RENDER_BINDSET
	
	for (_uint iTEXIx = 0; iTEXIx < TEXMW_NONE; ++iTEXIx)
	{
		if (FAILED(m_pTransCom[iTEXIx]->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		//셰이더 파일의 매트릭스 정보를 가져와 바인딩
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
			return E_FAIL;

		PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };
		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTextureCom, iTEXIx);
		CHECK_FAILED(hr);
	}
#pragma endregion
	
	m_pUIBtn->Render();

	// 폰트 출력
	if (m_eCurState != WINDOW_HIDE)
	{
		for (auto& Message : m_tMessageDesc.vecMsg)
			Render_Message();
	}

	return S_OK;
}

#ifdef DEBUG
void CUI_MessageWindow::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case WINDOW_IDLE:	ImGui::Text(u8"WINDOW_IDLE");	break;
	case WINDOW_HIDE:	ImGui::Text(u8"WINDOW_HIDE"); break;
	case WINDOW_SHOW:	ImGui::Text(u8"WINDOW_SHOW"); break;
	case WINDOW_NONE:	default: ImGui::Text(u8"WINDOW_NONE"); break;
	}
}
#endif // DEBUG

HRESULT CUI_MessageWindow::Add_Transform(void* _pArg)
{
	for (_uint iTrans = 0; iTrans < TEXMW_NONE; ++iTrans)
	{
		m_pTransCom[iTrans] = CTransform::Create(m_pDevice, m_pContext);

		if (nullptr == m_pTransCom[iTrans])
			return E_FAIL;

		if (FAILED(m_pTransCom[iTrans]->Initialize(_pArg)))
			return E_FAIL;

		//m_Components.emplace(g_strTransformTag, m_pTransCom[iTrans]);
		//Safe_AddRef(m_pTransCom[iTrans]);
	}

	return S_OK;
}

HRESULT CUI_MessageWindow::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	//대화하는 대상에 따라 텍스처를 변경하여 출력 (현재는 한 장)
	if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Texture_UI_MessageWindow"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
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

// 다이얼로그 호출 : vec에 담아둔 메세지를 출력합니다.
HRESULT CUI_MessageWindow::Display_Message(_float _fTimeDelta)
{
	if (m_tMessageDesc.wstrFontTag.empty())
		return S_OK;

	m_fElapsedTime += _fTimeDelta;
	if (m_fElapsedTime >= m_fDisplayTime) //경과시간 대비 출력시간 체크
	{
		m_fElapsedTime = 0.f;

		if (m_iCurMessageIndex < m_tMessageDesc.vecMsg.size()) //벡터에 담긴 메시지들의 크기를 체크
		{
			wstring wstrMsg = m_tMessageDesc.vecMsg[m_iCurMessageIndex];
			if (m_iCurCharIndex < wstrMsg.length()) //메시지 길이 체크
				m_iCurCharIndex++;
			else //메시지 길이를 넘겼을 경우, 초기화
			{
				//m_iCurMessageIndex = 0.f;
				//m_iCurCharIndex = 0.f;
			}
		}
	}

	//추후, A버튼 입력 전까지 대기하는 로직 필요
	//iCurCharIndex 비교 체크하여 동일/초과할 경우 대기.
	//A버튼 입력 상태를 확인할 경우, 다음 iCurMeesageIndex로 변경하여 스크립트 문단을 넘긴다.

	return S_OK;
}

// 다이얼로그 메시지 출력
HRESULT CUI_MessageWindow::Render_Message()
{
	//Actor(NPC) 대상 별 SpriteFont 폰트 수정 필요
	wstring wstrFontTag = m_tMessageDesc.wstrFontTag;
	_float2 vFontPos = m_tMessageDesc.fFontPos;
	_float4 vFontRGBA = m_tMessageDesc.fFontRGBA;

	_float2 vFontSize = m_tMessageDesc.fFontSize;
	_float2 vFontScale = m_tMessageDesc.fFontScale;
	_float fRadian = XMConvertToRadians(m_tMessageDesc.fRadian);

	if (m_iCurMessageIndex < m_tMessageDesc.vecMsg.size())
	{
		wstring wstrMsg = m_tMessageDesc.vecMsg[m_iCurMessageIndex];
		wstring& wstrSubstrMessage = wstrMsg.substr(0, m_iCurCharIndex);

		m_pGameInstance->Render_Font(wstrFontTag, wstrSubstrMessage, vFontPos, vFontRGBA, fRadian, vFontSize, vFontScale);
	}

	return S_OK;
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
	__super::Free();

	m_tMessageDesc.vecMsg.clear();

	for (auto& iTrans : m_pTransCom)
		Safe_Release(iTrans);

	Safe_Release(m_pDialog);
	Safe_Release(m_pUIBtn);
}


