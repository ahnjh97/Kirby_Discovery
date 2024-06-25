#include "stdafx.h"
#include "UI_PartTime.h"

#include "PartTimeHelper.h"

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

	, m_SizeBar2D(rhs.m_SizeBar2D)
	, m_SizeTimeBarBlank2D(rhs.m_SizeTimeBarBlank2D)
	, m_SizeScoreBar2D(rhs.m_SizeScoreBar2D)
	, m_SizeCategory2D(rhs.m_SizeCategory2D)
	, m_SizeDeeFace2D(rhs.m_SizeDeeFace2D)
{
}

HRESULT CUI_PartTime::Initialize_Prototype()
{
	fill(m_arrTexures.begin(), m_arrTexures.end(), nullptr);
	fill(m_arrSizeRatio.begin(), m_arrSizeRatio.end(), 1);

	m_arrSize[0] = m_arrSize[4] = m_SizeBar2D;
	m_arrSize[1] = m_arrSize[2] = m_arrSize[3] = m_SizeTimeBarBlank2D;
	m_arrSize[5] = m_SizeCategory2D;
	m_arrSize[6] = m_SizeScoreBar2D;
	m_arrSize[7] = m_SizeDeeFace2D;
	m_arrOriginalSize = m_arrSize;

	fill(m_arrPosition.begin(), m_arrPosition.end(), _float2());
	fill(m_arrColor.begin(), m_arrColor.end(), _float3(1.f, 1.f, 1.f));

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

	m_bIsRender = true;

	return S_OK;
}

_int CUI_PartTime::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	// 점수를 받는 여부상관없이 시간을 관리합니다.
	Compute_Timer(fTimeDelta);

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
	if (m_bIsRender == false) return S_OK;

	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	for (_int i = 0; i < m_arrTexures.size(); ++i)
	{
		// 디 표정 맞추기
		if(i>=7 && i<=9)
			if (false == Setup_DeeFace(i)) continue;

		// UI별 포지션, 사이즈, 컬러 조정
		Setup_PosSizeColor(i);

		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		CHECK_FAILED(hr);

		m_pShaderCom->Bind_RawValue("g_vRColor",	&m_arrColor[i], sizeof(_float3));
		if (i == 2)
		{
			_float fRatio = { .7f };
			m_pShaderCom->Bind_RawValue("g_fMaskRatio", &fRatio, sizeof(_float));
		}
		if (i == 3)
		{
			_float fRatio = { .5f };
			m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fRatioTimeBar, sizeof(_float));
		}
		if (i == 4) 
		{
			_float fRatio = { 1.f }; // 리셋
			m_pShaderCom->Bind_RawValue("g_fMaskRatio", &fRatio, sizeof(_float));
		}
		
		hr = m_pShaderCom->Begin(14);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}

	return S_OK;
}

#ifdef _DEBUG
void CUI_PartTime::Render_IMGUI()
{
	char ratio[16];
	ImGui::DragFloat(ratio, (_float*)&m_fRatioTimeBar, 0.01f, 0.01f, 1.f);
	ImGui::Separator(); ImGui::NewLine();

	for (_int i = 0; i < m_arrPosition.size(); ++i)
	{
		/*char name[16], size[16], color[16];
		sprintf_s(name, "pos%d", i);
		sprintf_s(size, "size%d", i);
		sprintf_s(color, "color%d", i);

		ImGui::DragFloat(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 0.1f, 2.f);
		ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
		ImGui::DragFloat3(color, (_float*)&m_arrColor[i], 0.01f, 0.f, 1.f);

		m_arrSize[i].x = m_arrOriginalSize[i].x * m_arrSizeRatio[i];
		m_arrSize[i].y = m_arrOriginalSize[i].y * m_arrSizeRatio[i];

		m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_arrPosition[i].x - g_iWinSizeX * 0.5f,
						- m_arrPosition[i].y + g_iWinSizeY * 0.5f,
						0.f,
						1.f));

		ImGui::NewLine();*/
	}

	//char test[16], test2[16];
	//ImGui::DragFloat3(test, (_float*)&m_vTESTCOLOR, 0.01f, 0.f, 1.f);
	//ImGui::DragFloat3(test2, (_float*)&m_vTESTCOLOR2, 0.01f, 0.f, 1.f);
}
#endif

HRESULT CUI_PartTime::Add_Components()
{
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

	//if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby_Mask"),
	//	TEXT("Com_Texture_Mask"), (CComponent**)&m_pTexMask)))
	//	return E_FAIL;
#pragma endregion

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_PartTime::Bind_ShaderResources()
{
	HRESULT hr;
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

		m_arrColor[iTextureNum] = _float3(0.73f, 1.f, 0.62f);		// 연초록
		if (m_fRatioTimeBar < 0.2f)
			m_arrColor[iTextureNum] = _float3(0.45f, 0.45f, 0.45f); // 빨간색일때는 따라오는 친구가 보이지 않습니다.
		else if (m_fRatioTimeBar < 0.8f)
			m_arrColor[iTextureNum] = _float3(1.f, 0.85f, 0.6f);	// 연주황
	}
	break;
	case 3: // 점수bar 실질적인 친구
	{
		m_arrSize[iTextureNum] = m_SizeTimeBarBlank2D;
		m_arrPosition[iTextureNum] = _float2(840.f, 82.f);

		// 현재 남아있는 Ratio에 따른 색 변경처리
		m_arrColor[iTextureNum] = _float3(0.45f, 1.f, 0.5f); // 초록
		if (m_fRatioTimeBar < 0.2f)
			m_arrColor[iTextureNum] = _float3(1.f, 0.3f, 0.45f); // 뻘겅
		else if(m_fRatioTimeBar < 0.8f)
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
		iCurFaceNum = 9;
	else if (m_fRatioTimeBar < 0.8f)
		iCurFaceNum = 7;

	return (iTextureNum == iCurFaceNum) ? true : false;
}

void CUI_PartTime::Compute_Timer(_float fTimeDelta)
{
	m_fStandardTime += fTimeDelta;
	if (m_fStandardTime - m_fBeforeTime >= 1.f)
	{
		m_fCurTime = 50.f - m_fStandardTime;
		Change_TimeTexures(m_fCurTime);
		
		if (m_fCurTime <= 0.9f) //게임이 종료되었다고 helper에게 알리기
		{
			CPartTimeHelper::Get_Instance()->HandleGame(0);
		}
		else if (m_fCurTime <= 20.9f) // 타임이 20일 때, 점심시간 시작.
		{
			CPartTimeHelper::Get_Instance()->NotifyObserver();
		}
		m_fBeforeTime = m_fStandardTime;
	}
}

void CUI_PartTime::Compute_TimeScore(_float fTimeDelta)
{
	m_fRatioTimeBar -= fTimeDelta * 0.1f;
}

// 타임 숫자 텍스쳐 변경
void CUI_PartTime::Change_TimeTexures(_float _fTime)
{
	_int iTime = static_cast<_int>(_fTime);

	// 몫 == 십의 자리수
	_int iShare = (iTime / 10);
	// 나머지 == 일의 자리수
	_int iRest = (iTime % 10);

	// 첫번째 텍스쳐 iShare과 대응되는 숫자 텍스쳐로 변경
	// 두번째 텍스쳐 iRest와 대응되는 숫자 텍스쳐로 변경
}

//void CUI_PartTime::Compute_Timer(_float fTimeDelta)
//{
//#pragma region 분홍색 게이지 공식
//
//	// 현재 커비의 HP 맥스치
//	_float fTimeMax = 50.f;
//	_float fCurrentTime = fTimeMax - fTimeDelta;
//
//	// 이 비율은 0 ~ 1 사이에 있어야 한다.
//	m_fCurHpRatio = (fCurrentTime / fTimeMax);
//
//	// 처음에 동기화 작업을 1회 한다. 만약, 레벨이 넘어간다면 이 불값은 다시 true로 만들어야 한다.
//	if (m_bInitializeHp == true)
//	{
//		m_fPreHpRatio = m_fSlowHpRatio = m_fHpRatio = m_fCurHpRatio;
//		m_bInitializeHp = false;
//	}
//
//#pragma endregion
//
//#pragma region 노란색 게이지 공식
//
//	// 피가 닳았다는 뜻이다.
//	if (m_fCurHpRatio < m_fPreHpRatio)
//	{
//		if (m_isHealing == true)
//		{
//			m_isHealing = false;
//			m_fHealHoleTime = 0.f;
//			m_fDeltaRatio = 0.f;
//			m_bDeltaRatio = true;
//		}
//
//		m_bShaking = TRUE;
//		m_bAlarm = TRUE;
//		// 피가 찼으니까 분홍이에게 대입을 한다.
//		m_fHpRatio = m_fCurHpRatio;
//		m_isDamage = true;
//
//		m_fIdleTime = 0.f;
//	}
//	// 피가 찼다는 뜻이다.
//	else if (m_fCurHpRatio > m_fPreHpRatio)
//	{
//		if (m_isDamage == true)
//		{
//			m_isDamage = false;
//			m_fDamageHoleTime = 0.f;
//			m_fDeltaRatio = 0.f;
//			m_bDeltaRatio = true;
//			m_bAlarm = false;
//		}
//
//		// 피가 찼으니까 노랑이한테 대입을 한다.
//		m_fSlowHpRatio = m_fCurHpRatio;
//		m_isHealing = true;
//
//		m_fIdleTime = 0.f;
//	}
//	else
//	{
//		m_fIdleTime += fTimeDelta;
//	}
//
//
//	// 만약, 커비가 데미지를 입었다면?
//	if (m_isDamage == true)
//	{
//		m_fDamageHoleTime += fTimeDelta;
//
//		// 만약, 피가 닳고 0.8초가 넘어갔다면?
//		if (m_fDamageHoleTime > 0.8f)
//		{
//			if (m_bDeltaRatio == true)
//			{
//				// 차이값을 구했다.
//				m_fDeltaRatio = m_fSlowHpRatio - m_fHpRatio;
//				m_bDeltaRatio = false;
//			}
//			// 차이값을 한번 구했다면, 그만큼 피를 틱당 깎아준다.
//			m_fSlowHpRatio -= m_fDeltaRatio * fTimeDelta * 2.f;
//
//			if (m_fSlowHpRatio < m_fHpRatio)
//			{
//				m_bDeltaRatio = true;
//				m_isDamage = false;
//				m_fDamageHoleTime = 0.f;
//				m_fDeltaRatio = 0.f;
//				m_fSlowHpRatio = m_fHpRatio;
//				m_bAlarm = false;
//			}
//		}
//	}
//	// 만약, 커비가 힐을 했다면?
//	else if (m_isHealing == true)
//	{
//		m_fHealHoleTime += fTimeDelta;
//
//		// 만약, 피가 회복되고 0.8초가 넘어갔다면?
//		if (m_fHealHoleTime > 0.8f)
//		{
//			if (m_bDeltaRatio == true)
//			{
//				// 차이값을 구했다.
//				m_fDeltaRatio = m_fSlowHpRatio - m_fHpRatio;
//				m_bDeltaRatio = false;
//			}
//			// 차이값을 한번 구했다면, 그만큼 피를 틱당 올려준다.
//			m_fHpRatio += m_fDeltaRatio * fTimeDelta * 2.f;
//
//			if (m_fSlowHpRatio < m_fHpRatio)
//			{
//				m_bDeltaRatio = true;
//				m_isHealing = false;
//				m_fHealHoleTime = 0.f;
//				m_fDeltaRatio = 0.f;
//				m_fHpRatio = m_fSlowHpRatio;
//			}
//		}
//	}
//	m_fPreHpRatio = m_fCurHpRatio;
//#pragma endregion
//
//#pragma region 피통 UI 반짝반짝 코드
//
//	// 노란 게이지가 반짝이가 되는 중
//	if (m_bAlarm == true)
//	{
//		m_fAlarmTime += fTimeDelta * 40.f;
//		//-1 ~ 1 사이의 범위 -> -0.5 ~ 0.5 사이의 범위
//		m_fAlarmColor = (sin(m_fAlarmTime)) * 0.5f;
//	}
//	// 노란 게이지가 반짝이지 않는 중
//	else
//	{
//		m_fAlarmColor = 0.f;
//		m_fAlarmTime = 0.f;
//	}
//
//#pragma endregion
//
//}

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

