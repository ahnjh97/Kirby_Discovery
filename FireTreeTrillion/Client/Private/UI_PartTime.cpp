#include "stdafx.h"
#include "UI_PartTime.h"

#include "PartTimeHelper.h"

const _float g_fTimeSpeed = 0.06f;

CUI_PartTime::CUI_PartTime(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_PartTime::CUI_PartTime(const CUI_PartTime& rhs)
	: CUIObject{ rhs }
	, m_arrTexures(rhs.m_arrTexures)
	, m_arrSize(rhs.m_arrSize)
	, m_arrPosition(rhs.m_arrPosition)
	, m_arrOriginalSize(rhs.m_arrOriginalSize)
	, m_arrSizeRatio(rhs.m_arrSizeRatio)
	, m_arrColor(rhs.m_arrColor)
	, m_arrScoreDigits(rhs.m_arrScoreDigits)
	, m_arrTimerDigits(rhs.m_arrTimerDigits)

	, m_SizeBar2D(rhs.m_SizeBar2D)
	, m_SizeTimeBarBlank2D(rhs.m_SizeTimeBarBlank2D)
	, m_SizeScoreBar2D(rhs.m_SizeScoreBar2D)
	, m_SizeCategory2D(rhs.m_SizeCategory2D)
	, m_SizeDeeFace2D(rhs.m_SizeDeeFace2D)

	, m_fSizeRatio(rhs.m_fSizeRatio)
	, m_fRealTimeSize2D(rhs.m_fRealTimeSize2D)
	, m_fStandardSize2D(rhs.m_fStandardSize2D)
	, m_arrRenderState(rhs.m_arrRenderState)
{
}

HRESULT CUI_PartTime::Initialize_Prototype()
{
	fill(m_arrTexures.begin(), m_arrTexures.end(), nullptr);
	_int iRatio(1);
	fill(m_arrSizeRatio.begin(), m_arrSizeRatio.end(), iRatio);

	m_arrSize[0] = m_arrSize[4] = m_SizeBar2D;
	m_arrSize[1] = m_arrSize[2] = m_arrSize[3] = m_SizeTimeBarBlank2D;
	m_arrSize[5] = m_SizeCategory2D;
	m_arrSize[6] = m_arrSize[15] = m_SizeScoreBar2D;
	m_arrSize[7] = m_SizeDeeFace2D;
	m_arrSize[10] = m_arrSize[11] = m_arrSize[12] = m_arrSize[13] = m_arrSize[14] = m_SizeDigits2D;
	m_arrSize[16] = _float2(g_iWinSizeX, g_iWinSizeY * 2.f);
	m_arrOriginalSize = m_arrSize;

	m_fStandardSize2D = m_fRealTimeSize2D = _float2(m_SizeScoreBar2D.x * 2.f, m_SizeScoreBar2D.y * 2.f);
	
	_float2 temp2D = _float2();
	fill(m_arrPosition.begin(), m_arrPosition.end(), temp2D);
	_float3 temp3D = _float3(1.f, 1.f, 1.f);
	fill(m_arrColor.begin(), m_arrColor.end(), temp3D);
	_int iZero(0);
	fill(m_arrScoreDigits.begin(), m_arrScoreDigits.end(), iZero);
	
	m_arrRenderState[START] = true;
	m_arrRenderState[BASIC] = false;
	m_arrRenderState[FADE] = false;

	m_arrTimerDigits[0] = 5;
	m_arrTimerDigits[1] = 0;
	return S_OK;
}

HRESULT CUI_PartTime::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	CPartTimeHelper::Get_Instance()->Initialize_GameStart();
	CPartTimeHelper::Get_Instance()->Register_UI(this);

	m_bIsRender = true;

	return S_OK;
}

_int CUI_PartTime::Tick(_float fTimeDelta)
{
	if (m_arrRenderState[BASIC] == false && m_arrRenderState[FADE] == false)
		return S_OK;
	if (m_arrRenderState[START] == true)
	{
		if (CPartTimeHelper::Get_Instance()->Handle_GameStart())
			m_arrRenderState[START] = false;
	}

	m_fTimeDelta = fTimeDelta;
	__super::Tick(fTimeDelta);

	// 점수를 받는 여부상관없이 시간을 관리합니다.
	Compute_Timer(fTimeDelta);

	if (m_bGoing)
		Compute_TimerBar(fTimeDelta); // 타임바 이동되는 시간을 조정합니다.

	// 점수를 받음으로써 변화되는 time-bar와 관련된 것을 관리합니다.
	Compute_TimeScore(fTimeDelta);
	
	return OBJ_NOEVENT;
}

void CUI_PartTime::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_PartTime::Render()
{
	if (m_arrRenderState[BASIC] == true)
	{
		HRESULT hr;
		hr = Bind_ShaderResources();
		CHECK_FAILED(hr);

		for (_int i = 0; i < m_arrTexures.size() - 2; ++i)
		{
			// Time-Bar에 따른 디 표정 맞추기
			if (i >= 7 && i <= 9)
				if (false == Setup_DeeFace(i)) continue;

			// UI별 포지션, 사이즈, 컬러 조정
			Setup_PosSizeColor(i);
			if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
				return E_FAIL;

			// 디퓨즈 바인딩
			if (i == 10 || i == 11)
				hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_arrTimerDigits[i - 10]); // 0,1
			else if ((i >= 12) && (i <= 14))
				hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_arrScoreDigits[i - 12]); // 0,1,2
			else
				hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
			CHECK_FAILED(hr);

			// 셰이더 수치 조정
			m_pShaderCom->Bind_RawValue("g_vRColor", &m_arrColor[i], sizeof(_float3));
			if (i == 2) // 먼저 움직이는 이동 목표 Time-Bar
			{
				_int iMask = 1;
				m_pShaderCom->Bind_RawValue("g_iMasking", &iMask, sizeof(_int));
				m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fGoalTimeBar, sizeof(_float));
			}
			if (i == 3) // 실질적인 Time-Bar
			{
				if (false == m_bGoing)
					m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fRatioTimeBar, sizeof(_float));
				else
					m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fRatioBarSub, sizeof(_float));

				// masking
				m_pTexMask->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0);
			}
			if (i == 4)
			{
				// 셰이더 변수들을 리셋시킵니다.
				_float fRatio = { 1.f };
				m_pShaderCom->Bind_RawValue("g_fMaskRatio", &fRatio, sizeof(_float));
				_int iMask = 0;
				m_pShaderCom->Bind_RawValue("g_iMasking", &iMask, sizeof(_int));
			}

			hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR_HORIZONTALCUT);
			CHECK_FAILED(hr);

			hr = m_pVIBufferCom->Bind_Buffers();
			CHECK_FAILED(hr);

			hr = m_pVIBufferCom->Render();
			CHECK_FAILED(hr);
		}
	}

	if (m_arrRenderState[FADE] == true)
	{
		if (m_bRenderGameOver)
			Render_GameOver();
	}
	return S_OK;
}

#ifdef _DEBUG
void CUI_PartTime::Render_IMGUI()
{
	char ratio[16];
	ImGui::DragFloat(ratio, (_float*)&m_fRatioTimeBar, 0.01f, 0.01f, 1.f);
	ImGui::Separator(); ImGui::NewLine();

	//for (_int i = 10; i < m_arrPosition.size(); ++i)
	//{
	//	_int i = 15;
	//	char name[16], size[16], color[16];
	//	sprintf_s(name, "pos%d", i);
	//	sprintf_s(size, "size%d", i);
	//	sprintf_s(color, "color%d", i);

	//	ImGui::DragFloat(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 0.1f, 2.f);
	//	ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
	//	ImGui::DragFloat3(color, (_float*)&m_arrColor[i], 0.01f, 0.f, 1.f);

	//	m_arrSize[i].x = m_arrOriginalSize[i].x * m_arrSizeRatio[i];
	//	m_arrSize[i].y = m_arrOriginalSize[i].y * m_arrSizeRatio[i];

	//	m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
	//		XMVectorSet(m_arrPosition[i].x - g_iWinSizeX * 0.5f,
	//					- m_arrPosition[i].y + g_iWinSizeY * 0.5f,
	//					0.f,
	//					1.f));

	//	ImGui::NewLine();
	//}

	//char test[16], test2[16];
	//ImGui::DragFloat3(test, (_float*)&m_vTESTCOLOR, 0.01f, 0.f, 1.f);
	//ImGui::DragFloat3(test2, (_float*)&m_vTESTCOLOR2, 0.01f, 0.f, 1.f);
}
#endif

void CUI_PartTime::Set_RenderState(STATE _eState, _bool _bState)
{
	 m_arrRenderState[_eState] = _bState;
}

HRESULT CUI_PartTime::Add_Components()
{
	HRESULT hr(S_OK);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	#pragma region 텍스쳐 컴포넌트
	// 배경
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_BaseBar"),
		TEXT("Com_Texture_BaseBar"), (CComponent**)&m_arrTexures[0])))
		return E_FAIL;

	// bar 3개
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_TimeBarBW"),
		TEXT("Com_Texture_TimeBarBW"), (CComponent**)&m_arrTexures[1])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_TimeBarBW"),
		TEXT("Com_Texture_TimeBarPreview"), (CComponent**)&m_arrTexures[2])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_TimeBarBW"),
		TEXT("Com_Texture_TimeBarReal"), (CComponent**)&m_arrTexures[3])))
		return E_FAIL;

	// bar 왼쪽편 와들디 배경
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_DeeBGBW"),
		TEXT("Com_Texture_DeeBGBW"), (CComponent**)&m_arrTexures[4])))
		return E_FAIL;

	// bar 오른편 시계
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_Clock_Orig"),
		TEXT("Com_Texture_Clock"), (CComponent**)&m_arrTexures[5])))
		return E_FAIL;

	// 왼쪽 아래 스코어 판
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_ScoreBar"),
		TEXT("Com_Texture_ScoreBar"), (CComponent**)&m_arrTexures[6])))
		return E_FAIL;	
	
	// 와들디 얼굴 for test
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_AngryDee"),
		TEXT("Com_Texture_DeeFaceAngry"), (CComponent**)&m_arrTexures[7])))
		return E_FAIL;

	// 와들디 얼굴 for test
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_IdleDee"),
		TEXT("Com_Texture_DeeFaceIdle"), (CComponent**)&m_arrTexures[8])))
		return E_FAIL;

	// 와들디 얼굴 for test
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_SadDee"),
		TEXT("Com_Texture_DeeFaceSad"), (CComponent**)&m_arrTexures[9])))
		return E_FAIL;

	// 시간 카운트 다운하는 숫자 텍스쳐
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TempWhiteDigits"),
		TEXT("Com_Texture_TimeDigits_00"), (CComponent**)&m_arrTexures[10]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TempWhiteDigits"),
		TEXT("Com_Texture_TimeDigits_0"), (CComponent**)&m_arrTexures[11]);
	CHECK_FAILED(hr);

	// 스코어 관리하는 숫자 텍스쳐
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TempRedDigits"),
		TEXT("Com_Texture_ScoreDigits_000"), (CComponent**)&m_arrTexures[12]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TempRedDigits"),
		TEXT("Com_Texture_ScoreDigits_00"), (CComponent**)&m_arrTexures[13]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TempRedDigits"),
		TEXT("Com_Texture_ScoreDigits_0"), (CComponent**)&m_arrTexures[14]);
	CHECK_FAILED(hr);

	// GAME-OVER 텍스쳐
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_GameFoodUI_FoodGameTextMask"),
		TEXT("Com_Texture_Gameover"), (CComponent**)&m_arrTexures[15]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_Fade"),
		TEXT("Com_Texture_Fade"), (CComponent**)&m_arrTexures[16]);
	CHECK_FAILED(hr);

	
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTexMask)))
		return E_FAIL;
	#pragma endregion

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_PartTime::Bind_ShaderResources()
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

void CUI_PartTime::Setup_PosSizeColor(_int iTextureNum)
{
	switch (iTextureNum)
	{
	case 0: // 점수bar 배경
	{
		m_arrSize[iTextureNum] = m_SizeBar2D;
		m_arrPosition[iTextureNum] = _float2(785.f, 120.f);
	}
	break;
	case 1: // 점수bar (회색배경)
	{
		m_arrSize[iTextureNum] = m_SizeTimeBarBlank2D;
		m_arrPosition[iTextureNum] = _float2(840.f, 82.f);
		m_arrColor[iTextureNum] = _float3(0.45f, 0.45f, 0.45f);
	}
	break;
	case 2: // 점수bar 실질적으로 움직이는 친구를 뒤늦게 따라다니는 time Bar
	{
		m_arrSize[iTextureNum] = m_SizeTimeBarBlank2D;
		m_arrPosition[iTextureNum] = _float2(840.f, 82.f);

		// 현재 남아있는 Ratio에 따른 색 변경처리
		_float fRatioBar = _float();
		if (false == m_bGoing)
			fRatioBar = 0.f;
		else
			fRatioBar = m_fRatioBarSub;

		m_arrColor[iTextureNum] = _float3(0.73f, 1.f, 0.62f);		// 연초록
		if (fRatioBar < 0.2f)
			m_arrColor[iTextureNum] = _float3(0.45f, 0.45f, 0.45f); // 빨간색일때는 따라오는 친구가 보이지 않습니다.
		else if (fRatioBar < 0.8f)
			m_arrColor[iTextureNum] = _float3(1.f, 0.85f, 0.6f);	// 연주황
	}
	break;
	case 3: // 점수bar 실질적인 친구
	{
		m_arrSize[iTextureNum] = m_SizeTimeBarBlank2D;
		m_arrPosition[iTextureNum] = _float2(840.f, 82.f);

		// 현재 남아있는 Ratio에 따른 색 변경처리
		_float fRatioBar = _float();
		if (false == m_bGoing)
			fRatioBar = m_fRatioTimeBar;
		else
			fRatioBar = m_fRatioBarSub;

		m_arrColor[iTextureNum] = _float3(0.45f, 1.f, 0.5f); // 초록
		if (fRatioBar < 0.2f)
			m_arrColor[iTextureNum] = _float3(1.f, 0.3f, 0.45f); // 뻘겅
		else if (fRatioBar < 0.8f)
			m_arrColor[iTextureNum] = _float3(1.f, 0.7f, 0.2f);  // 주황
	}
	break;
	case 4: // 점수bar 와들디 배경
	{
		m_arrSize[iTextureNum] = m_SizeBar2D;
		m_arrPosition[iTextureNum] = _float2(788.f, 120.f);
		m_arrColor[iTextureNum] = m_arrColor[3];
	}
	break;
	case 5: // 점수bar 오른편 시계
	{
		m_arrSize[iTextureNum] = m_SizeCategory2D * 0.25f;
		m_arrPosition[iTextureNum] = _float2(1412.f, 83.f);
	}
	break;
	case 6: // 왼쪽 아래 스코어 판
	{
		m_arrSize[iTextureNum] = m_SizeScoreBar2D * 0.6f;
		m_arrPosition[iTextureNum] = _float2(180.f, 817.f);
	}
	break;
	case 7: // 테스트용 와들디 얼굴
	case 8: // 테스트용 와들디 얼굴
	case 9: // 테스트용 와들디 얼굴
	{
		m_arrSize[iTextureNum] = m_SizeDeeFace2D * 0.9f;
		m_arrPosition[iTextureNum] = _float2(213.f, 61.f);
	}
	break;
	////////////////////// ↓ ↓ Digits ↓ ↓ /////////////////////////
	case 10: // 타임판 digits 00
		m_arrPosition[iTextureNum] = _float2(1467.f, 74.f);
	break;
	case 11: // 타임판 digits 0
		m_arrPosition[iTextureNum] = _float2(1510.f, 74.f);
	break;
	case 12: // 점수판 digits 000
		m_arrPosition[iTextureNum] = _float2(140.f, 810.f);
	break;
	case 13: // 점수판 digits 00
		m_arrPosition[iTextureNum] = _float2(185.f, 810.f);
	break;
	case 14: // 점수판 digits 0
		m_arrPosition[iTextureNum] = _float2(230.f, 810.f);
	break;
	case 15: // GAME OVER
		//m_arrPosition[iTextureNum] = _float2(830.f, 200.f);
		break;
	}

	m_pTransformCom->Set_Scaled(m_arrSize[iTextureNum].x, m_arrSize[iTextureNum].y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
							XMVectorSet(m_arrPosition[iTextureNum].x - g_iWinSizeX * 0.5f,
										- m_arrPosition[iTextureNum].y + g_iWinSizeY * 0.5f,
										0.f,
										1.f));
}

// 현재 남아있는 Ratio에 따른 와들디 표정 변화
// 7 : angry, 8 : idle, 9 : sad
_bool CUI_PartTime::Setup_DeeFace(_int iTextureNum)
{
	_int iCurFaceNum(8);
	if (m_fRatioTimeBar < 0.2f)
		iCurFaceNum = 7;
	else if (m_fRatioTimeBar < 0.8f)
		iCurFaceNum = 9;

	return (iTextureNum == iCurFaceNum) ? true : false;
}

// 시간이 줄어듦에 따라 처리하는 것을 담고있습니다.
// 타임바와 관계없습니다.
void CUI_PartTime::Compute_Timer(_float fTimeDelta)
{
	if (m_pGameInstance->Get_SecondTimer() == 0.f)
	{
		if(CPartTimeHelper::Get_Instance()->Handle_LunchTime())
			m_pGameInstance->Set_SecondTimerRatio(1.f);
	}
	_float fLunchTime(20.9f), fGameoverTime(0.5f);
	m_fStandardTime += fTimeDelta;
	if (m_fStandardTime - m_fBeforeTime >= 1.f)
	{
		//m_fCurTime = 3.f - m_fStandardTime;
		m_fCurTime = 50.f - m_fStandardTime;
		if (m_fCurTime <= 0.f) m_fCurTime = 0.f;
		Change_TimeTexures(m_fCurTime);
		
		if (m_fCurTime <= fGameoverTime) // GAME OVER 텍스쳐 띄우기
		{
			CPartTimeHelper::Get_Instance()->Handle_UI(CPartTimeHelper::GAMEOVER);
			
		}
		else if (m_fCurTime <= fLunchTime) // 타임이 20일 때, 점심시간 시작.
		{
			CPartTimeHelper::Get_Instance()->NotifyObserver();
			m_pGameInstance->Set_SecondTimerRatio(0.f);
		}
		m_fBeforeTime = m_fStandardTime;
	}
}

// 리얼 타임바가 먼저간 옅은 타임바를 쫓아갑니다.
// 쫓아가는 것을 핸들하는 변수는 m_fRatioBarSub입니다.
void CUI_PartTime::Compute_TimerBar(_float fTimeDelta)
{
	_float fSpeed = g_fTimeSpeed * 5.f;
	m_fRatioBarSub += fTimeDelta * fSpeed;
	if(m_fRatioBarSub >= m_fGoalTimeBar)
	{
		if (m_fGoalTimeBar >= 1.f) m_fRatioBarSub = 1.f;
		m_fRatioTimeBar = m_fRatioBarSub;
		m_bGoing = false; // false면 리얼 타임바는 그만 이동합니다.
		m_fGoalTimeBar = 0.f;
	}
}

// 리얼 타임바는 시간에 따라서 계속 줄어듭니다.
void CUI_PartTime::Compute_TimeScore(_float fTimeDelta)
{
	if(m_fCurTime <= 20.f)
		m_fRatioTimeBar -= fTimeDelta * g_fTimeSpeed * 1.4f;
	else
		m_fRatioTimeBar -= fTimeDelta * g_fTimeSpeed;
}

// 타임 숫자 텍스쳐 변경
void CUI_PartTime::Change_TimeTexures(_float _fTime)
{
	// _fTime값을 올림하여 _int값으로 변환
	_int iTime = static_cast<_int>(ceil(_fTime));

	// 몫 == 십의 자리수
	_int iShare = (iTime / 10);
	if (iShare <= 0 || iShare > 9) iShare = 0;
	// 나머지 == 일의 자리수
	_int iRest = (iTime % 10);
	if (iRest <= 0 || iRest > 9) iRest = 0;

	// 첫번째 텍스쳐 iShare과 대응되는 숫자 텍스쳐로 변경
	m_arrTimerDigits[0] = iShare;
	// 두번째 텍스쳐 iRest와 대응되는 숫자 텍스쳐로 변경
	m_arrTimerDigits[1] = iRest;
}

// 맞추면 먼저가는 옅은 타임바를 올려줍니다. 파라미터 수치만큼 올립니다.
void CUI_PartTime::Add_TimeBar(_float _fTimeBar)
{
	m_fGoalTimeBar = m_fRatioTimeBar + _fTimeBar;
	if (m_fGoalTimeBar >= 1.f) m_fGoalTimeBar = 1.f;
	m_bGoing = true;
}

// 스코어 점수를 조절합니다.
void CUI_PartTime::Add_Score(_int _fPlusScore)
{
	_int iScore = m_arrScoreDigits[0] * 100 + m_arrScoreDigits[1] * 10 + m_arrScoreDigits[2];
	iScore += _fPlusScore;

	_int iShareHund = (iScore / 100);
	if (iShareHund < 10)
		m_arrScoreDigits[0] = iShareHund;
	_int iShareTen = ((iScore % 100) / 10);
	if (iShareTen < 10)
		m_arrScoreDigits[1] = iShareTen;
	_int iRest = (iScore % 10);
	m_arrScoreDigits[2] = iRest;
}

void CUI_PartTime::Render_GameOver()
{
	static _float fTimeAcc = 0.f;
	_int iNum = 15; // GAMEOVER TEXTURE NUM == 15

	m_fSizeRatio += m_fTimeDelta;
	if (m_fSizeRatio >= 1.f)
	{
		m_fSizeRatio = 1.f;
		fTimeAcc += m_fTimeDelta;
		if (fTimeAcc >= 1.f)
		{
			Render_Fade();
			return;
		}
	}

	_float fSizeRatio = 1.f - EaseOutBounce(m_fSizeRatio); // m_fRealTimeSize2D
	m_fRealTimeSize2D = _float2(m_fStandardSize2D.x * fSizeRatio + m_SizeScoreBar2D.x, m_fStandardSize2D.y * fSizeRatio + m_SizeScoreBar2D.y);
	m_arrSize[iNum] = m_fRealTimeSize2D;
	m_pTransformCom->Set_Scaled(m_arrSize[iNum].x, m_arrSize[iNum].y, 1.f);
	m_arrPosition[iNum] = _float2(830.f, 200.f);
	
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
							   XMVectorSet(m_arrPosition[iNum].x - g_iWinSizeX * 0.5f,
							   	-m_arrPosition[iNum].y + g_iWinSizeY * 0.5f,
							   	0.f,
							   	1.f));

	// UI별 포지션, 사이즈, 컬러 조정
	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	// 디퓨즈 바인딩
	hr = m_arrTexures[iNum]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR_HORIZONTALCUT);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);
}

void CUI_PartTime::Render_Fade()
{
	Reset_ShaderValue();

	_int iNum = 16;
	static _float fFadeOutRatio = 1.f;

	_int iFade = 1;
	m_pShaderCom->Bind_RawValue("g_iFade", &iFade, sizeof(_int));
	
	fFadeOutRatio -= m_fTimeDelta * 0.9f;
	_float fRatio = EASE_IN_SINE(fFadeOutRatio);
	m_pShaderCom->Bind_RawValue("g_fFadeRatio", &fRatio, sizeof(_float));

	if (fFadeOutRatio < -0.99f)
		CPartTimeHelper::Get_Instance()->Handle_UI(CPartTimeHelper::OVER);
	if (fFadeOutRatio < -0.f)
	{
		if (!m_bOnce)
		{
			CPartTimeHelper::Get_Instance()->Handle_GameOver();
			m_bOnce = true;
		}
	}
	m_pTransformCom->Set_Scaled(m_arrSize[iNum].x, m_arrSize[iNum].y, 1.f);
	m_arrPosition[iNum] = _float2(800.f, 400.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_arrPosition[iNum].x - g_iWinSizeX * 0.5f,
					-m_arrPosition[iNum].y + g_iWinSizeY * 0.5f,
					0.f,
					1.f));

	// UI별 포지션, 사이즈, 컬러 조정
	HRESULT hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	// 디퓨즈 바인딩
	hr = m_arrTexures[iNum]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(POSTEX_FADEINOUT);
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Bind_Buffers();
	CHECK_FAILED(hr);

	hr = m_pVIBufferCom->Render();
	CHECK_FAILED(hr);
}

void CUI_PartTime::Reset_ShaderValue()
{
	m_fRatioBarSub = 1.f;
	m_fRatioTimeBar = 1.f;
	m_fGoalTimeBar = 1.f;
}

// 탕탕타라라라...
_float CUI_PartTime::EaseOutBounce(_float _value)
{
	const _float n1 = 7.5625;
	const _float d1 = 2.75;

	if (_value < 1 / d1) {
		return n1 * _value * _value;
	}
	else if (_value < 2 / d1) {
		return n1 * (_value -= 1.5 / d1) * _value + 0.75;
	}
	else if (_value < 2.5 / d1) {
		return n1 * (_value -= 2.25 / d1) * _value + 0.9375;
	}
	else {
		return n1 * (_value -= 2.625 / d1) * _value + 0.984375;
	}
}

CUI_PartTime* CUI_PartTime::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_PartTime* pInstance = new CUI_PartTime(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_PartTime"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_PartTime::Clone(void* pArg)
{
	CUI_PartTime* pInstance = new CUI_PartTime(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_PartTime"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_PartTime::Free()
{
	for (auto& texure : m_arrTexures)
		Safe_Release(texure);

	Safe_Release(m_pTexMask);
	__super::Free();
}

