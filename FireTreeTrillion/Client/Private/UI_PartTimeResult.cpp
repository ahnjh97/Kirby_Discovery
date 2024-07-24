#include "stdafx.h"
#include "GameInstance.h"
#include "Camera.h"

#include "UI_PartTimeResult.h"
#include "Utils.h"

#include "Level_Loading.h"
#include "PartTimeHelper.h"
#include "PartTimerKirby.h"
#include "HungryDee.h"
#include <UI_MessageWindow.h>

CUI_PartTimeResult::CUI_PartTimeResult(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_PartTimeResult::CUI_PartTimeResult(const CUI_PartTimeResult& rhs)
	: CUIObject{ rhs }
	, m_arrTexures(rhs.m_arrTexures)
	//, m_arrayStarMatrix(rhs.m_arrayStarMatrix)
	, m_arrScoreDigits(rhs.m_arrScoreDigits)

	, m_arrPosition(rhs.m_arrPosition)
	, m_arrSize(rhs.m_arrSize)
	, m_arrOriginalSize(rhs.m_arrOriginalSize)
	, m_arrSizeRatio(rhs.m_arrSizeRatio)
{
}

HRESULT CUI_PartTimeResult::Initialize_Prototype()
{
	fill(m_arrTexures.begin(), m_arrTexures.end(), nullptr);
	//fill(m_arrayStarMatrix.begin(), m_arrayStarMatrix.end(), _float4x4());

	_float2 temp2D = _float2();
	fill(m_arrPosition.begin(), m_arrPosition.end(), temp2D);
	fill(m_arrSize.begin(), m_arrSize.end(), temp2D);
	_int iZero(0);
	fill(m_arrScoreDigits.begin(), m_arrScoreDigits.end(), iZero);

	m_arrSize[0] = m_SizeScoreBar2D * 0.9f;
	m_arrSize[1] = m_arrSize[2] = m_arrSize[3] = m_SizeDigits2D;
	m_arrSize[4] = m_SizeScoreResult2D * 0.9f;
	m_arrSize[5] = m_arrSize[6] = m_arrSize[7] = _float2(m_SizeDigits2D.x * 0.8f, m_SizeDigits2D.y * 0.8f);

	m_arrOriginalSize = m_arrSize;
	fill(m_arrSizeRatio.begin(), m_arrSizeRatio.end(), _float2(1.f, 1.f));

	return S_OK;
}

HRESULT CUI_PartTimeResult::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	CPartTimeHelper::Get_Instance()->Register_PartTimeResult(this);

	Initialize_TexturePos();
	m_bIsRender = false;

	return S_OK;
}

_int CUI_PartTimeResult::Tick(_float fTimeDelta)
{
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	__super::Tick(m_fTimeDelta);

	return OBJ_NOEVENT;
}

void CUI_PartTimeResult::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_PartTimeResult::Render()
{
	if (m_bIsRender == false) return S_OK;

	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	for (_int i = 0; i < m_arrTexures.size(); ++i)
	{
		if (i != 0 && i != 4) continue;

		m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_arrPosition[i].x - g_iWinSizeX * 0.5f,
				-m_arrPosition[i].y + g_iWinSizeY * 0.5f,
				0.f,
				1.f));

		hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
		CHECK_FAILED(hr);

		hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		CHECK_FAILED(hr);

		_float fOffset = 1.f;
		m_pShaderCom->Bind_RawValue("g_fAlpha", &fOffset, sizeof(_float));

		hr = m_pShaderCom->Begin(POSTEX_ALPHABLEND_NOTEST);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}

	Render_Digits();
	Render_Font();

	return S_OK;
}

#ifdef _DEBUG
void CUI_PartTimeResult::Render_IMGUI()
{
	//ImGui::Separator(); ImGui::NewLine();

	//for (_int i = 0; i < m_arrPosition.size(); ++i)
	//{
	//	char name[16], size[16], color[16];
	//	sprintf_s(name,  "pos%d",   i);
	//	sprintf_s(size,  "size%d",  i);

	//	ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
	//	ImGui::DragFloat2(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 1.f, 3.f);

	//	m_arrSize[i].x = m_arrOriginalSize[i].x * m_arrSizeRatio[i].x;
	//	m_arrSize[i].y = m_arrOriginalSize[i].y * m_arrSizeRatio[i].y;
	//	m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
	//		XMVectorSet(m_arrPosition[i].x   - g_iWinSizeX * 0.5f,
	//					- m_arrPosition[i].y + g_iWinSizeY * 0.5f,
	//					0.f,
	//					1.f));

	//	ImGui::NewLine();
	//}
}
#endif

void CUI_PartTimeResult::Render_Digits()
{
	HRESULT hr(S_OK);
	static _float fTimeAcc = 0.f;
	static _bool bOnce = false;
	static _bool bTwice = false;

	m_fMoveRatio += m_fTimeDelta * 3.f;
	if (m_fMoveRatio >= 1.f)
	{
		m_fMoveRatio = 1.f;
		fTimeAcc += m_fTimeDelta;
		if (fTimeAcc >= 0.1f)
		{
			_int iAddNum = 1;

			_float fRealTotalScore = Change_ScoreTextures(iAddNum);
			if (m_fScore < fRealTotalScore) // 30만큼 점수판 += 점수
			{
				m_bRenderTotalScore = true;
				if (fTimeAcc >= 2.f)
				{
					// 최종 점수만큼 ScoreTextures가 채워져 있다.
					if (m_bRenderTotalScore)
					{
						Render_TotalScore();
						if (bOnce == false)
						{
							CPartTimerKirby* pKirby = dynamic_cast<CPartTimerKirby*>(m_pGameInstance->Get_GameObject_ByTag(*m_pCurrentLevelID, L"Layer_Player", L"Prototype_GameObject_PartTimerKirby"));
							pKirby->Change_State(CPartTimerKirby::FOODSHOP_RESULTWINSTART, 50.f, false, true);
							bOnce = true;
						}
					}
				}

				// 다이얼로그 생성
				if (fTimeAcc >= 4.f)
				{
					CUI_MessageWindow* pMWindow = dynamic_cast<CUI_MessageWindow*>
						(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_UI_Msg_Parttimer_Dee")));
					CHECK_NULLPTR(pMWindow);
					pMWindow->Show_DialogMessage();
				}
			}
			else
			{
				//헝그리 디 결과 창에 만듭니다~
				CHungryDee::HUNGRYDEE_DESC HungryDeeDesc{};
				HungryDeeDesc.fSpeedPerSec = 5.f;
				HungryDeeDesc.fRotationPerSec = ToRadian(90.f);
				HungryDeeDesc.eAnim = DEESHOPANIM_WAIT;
				HungryDeeDesc.iIdx = ((_int)fRealTotalScore / 30) - 1;

				m_pGameInstance->Add_Clone(LEVEL_PARTTIME, TEXT("Layer_Dee"), TEXT("Prototype_GameObject_HungryDee"), &HungryDeeDesc);
			}
			if (false == m_bRenderTotalScore)
			{
				fTimeAcc = 0.f;
				m_fMoveRatio = 0.f;
			}



		}
	}

	Repose_ScoreTextures();
	for (_int i = 5; i <= 7; ++i)
	{
		_float fPosRatio = sin(m_fMoveRatio * 3.14159f);
		m_fMovePosition2D = _float2(m_arrPosition[i].x, m_arrPosition[i].y - fPosRatio * 30.f);
		m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_fMovePosition2D.x - g_iWinSizeX * 0.5f,
				-m_fMovePosition2D.y + g_iWinSizeY * 0.5f,
				0.f, 1.f));

		hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
		CHECK_FAILED(hr);

		hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_arrScoreDigits[i - 5]);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR_HORIZONTALCUT);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}
}

void CUI_PartTimeResult::Render_TotalScore()
{
	HRESULT hr(S_OK);

	static _float fTimeAcc = 0.f;
	m_fSizeRatio += m_fTimeDelta * 3.f;
	if (m_fSizeRatio >= 1.f)
	{
		m_fSizeRatio = 1.f;
		fTimeAcc += m_fTimeDelta;
		if (fTimeAcc >= 1.f)
		{
			static _bool bOnce{ false };
			if (!bOnce)
			{
				// 효선아 여기야 >> 게임 점수 다 뜨고 이펙트 나오는 구간
				// 감사합니다

				bOnce = true;

				_int iLevel = *CGameInstance::Get_Instance()->Get_CurrentLevelID();
				auto layerList = CGameInstance::Get_Instance()->Get_List(iLevel, TEXT("Layer_Dee"));
				if (!layerList->empty())
				{
					for (auto pGameObj : *layerList)
					{
						CHungryDee* pDee = static_cast<CHungryDee*>(pGameObj);
						pDee->Win();
					}
				}

				//결과 UI
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_FoodGame success UI"))))
					return;


				//콘페티
				//_float4x4 CamWorld = m_pGameInstance->Get_Transform_Inv(CPipeLine::D3DTS_VIEW);
				//_float3 vCamPos = CamWorld.Translation();

				//for (_int i = 0; i < 80; ++i)
				//{
				//	CEffect::FX_DESC FXDesc{};
				//	FXDesc.vInitPos = vCamPos + _float3{ CUtils::Make_RandomFloat(-5.f, 5.f), 0.f, 8.f } + (_float3)CUtils::Make_Random_Vector(CUtils::Make_RandomFloat(2.f, 5.f));

				//	//FXDesc.vInitRot =CUtils::Make_Degree_FromDir(CUtils::Make_Random_Vector(1.f));
				//	FXDesc.fStartDelay = CUtils::Make_RandomFloat(0.f, 3.f);

				//	wstring strPrototypeTag = TEXT("Prototype_GameObject_foodgame clear confetti ");
				//	switch (CUtils::Make_RandomInt(1, 4))
				//	{
				//	case 1: strPrototypeTag += L"A"; break;
				//	case 2: strPrototypeTag += L"B"; break;
				//	case 3: strPrototypeTag += L"C"; break;
				//	case 4: strPrototypeTag += L"D"; break;
				//	default:
				//		break;
				//	}
				//	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), strPrototypeTag, &FXDesc)))
				//		return;
				//}

			}
		}
	}

	Repose_TotalScoreTextures();
	for (_int i = 1; i <= 3; ++i)
	{
		_float fSizeRatio = sin(m_fSizeRatio * 3.14159f);
		m_fSize2D = _float2(m_arrSize[i].x + fSizeRatio * 10.f, m_arrSize[i].y + fSizeRatio * 10.f);
		m_pTransformCom->Set_Scaled(m_fSize2D.x, m_fSize2D.y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_arrPosition[i].x - g_iWinSizeX * 0.5f,
				-m_arrPosition[i].y + g_iWinSizeY * 0.5f,
				0.f, 1.f));

		hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_arrScoreDigits[i - 1]);
		CHECK_FAILED(hr);

		hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
		CHECK_FAILED(hr);

		hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_arrScoreDigits[i - 1]);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR_HORIZONTALCUT);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}
}

void CUI_PartTimeResult::Render_Font()
{
	// 폰트
	wstring wstrFontTag = L"Font_KoreanGDB_KR22spac10";
	_int iNumDee = (_int)(m_fScore / 30.f);
	wstring wstrMsg = L"합계 " + CUtils::StrToWstr(to_string(iNumDee)) + L"인";
	_float4 vRGBA = { 75.f / 255.f, 58.f / 255.f, 22.f / 255.f, 1.f };
	m_pGameInstance->Render_Font(wstrFontTag, wstrMsg, _float2(520.f, 490.f), vRGBA, 0.f, _float2(15.f, 15.f), _float2(1.f, 1.f));
}

void CUI_PartTimeResult::Initialize_TexturePos()
{
	m_arrPosition[0] = _float2(800.f, 222.f);

	m_arrPosition[1] = _float2(760.f, 220.f);
	m_arrPosition[2] = _float2(800.f, 220.f);
	m_arrPosition[3] = _float2(840.f, 220.f);

	m_arrPosition[4] = _float2(800.f, 490.f);

	m_arrPosition[5] = _float2(1000.f, 490.f);
	m_arrPosition[6] = _float2(1035.f, 490.f);
	m_arrPosition[7] = _float2(1070.f, 490.f);
}

// 받은 스코어 점수를 출력합니다.
_int CUI_PartTimeResult::Change_ScoreTextures(_int iNum)
{
	static _int iScoreAccum = 0;
	iScoreAccum += 30 * iNum;
	if (iScoreAccum > m_fScore)
		return iScoreAccum;

	_int iShareHund = (iScoreAccum / 100);
	if (iShareHund < 10)
		m_arrScoreDigits[0] = iShareHund;

	_int iShareTen = ((iScoreAccum % 100) / 10);
	if (iShareTen < 10)
		m_arrScoreDigits[1] = iShareTen;

	_int iRest = (iScoreAccum % 10);
	m_arrScoreDigits[2] = iRest;

	return iScoreAccum;
}

HRESULT CUI_PartTimeResult::Add_Components()
{
	HRESULT hr(S_OK);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

#pragma region 텍스쳐 컴포넌트
	// 점수판 배경
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_GameFoodUI_ScoreBar"),
		TEXT("Com_Texture_ScoreBar"), (CComponent**)&m_arrTexures[0]);
	CHECK_FAILED(hr);

	// 총 토탈 점수판
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_ScoreDigits"),
		TEXT("Com_Texture_ScoreDigits_000"), (CComponent**)&m_arrTexures[1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_ScoreDigits"),
		TEXT("Com_Texture_ScoreDigits_00"), (CComponent**)&m_arrTexures[2]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_ScoreDigits"),
		TEXT("Com_Texture_ScoreDigits_0"), (CComponent**)&m_arrTexures[3]);
	CHECK_FAILED(hr);

	// 총 점수판 배경
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_GameFoodUI_ResultBar"),
		TEXT("Com_Texture_ResultBar"), (CComponent**)&m_arrTexures[4]);
	CHECK_FAILED(hr);

	// 올라간 숫자판
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_ScoreDigits"),
		TEXT("Com_Texture_TotalScoreDigits_000"), (CComponent**)&m_arrTexures[5]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_ScoreDigits"),
		TEXT("Com_Texture_TotalScoreDigits_00"), (CComponent**)&m_arrTexures[6]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Texture_ScoreDigits"),
		TEXT("Com_Texture_TotalScoreDigits_0"), (CComponent**)&m_arrTexures[7]);
	CHECK_FAILED(hr);

	// CLEAR 텍스쳐
	//hr = __super::Add_Component(TEXT("Prototype_Component_Texture_GameFoodUI_FoodGameTextMask"),
	//	TEXT("Com_Texture_FoodGameTextMask"), (CComponent**)&m_arrTexures[8]);
	//CHECK_FAILED(hr);

	// 마스킹 텍스쳐
	hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTexMask);
	CHECK_FAILED(hr);
#pragma endregion

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CUI_PartTimeResult::Bind_ShaderResources()
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

// 123 : 토탈 숫자
// 567 : 올라가는 점수 숫자
// 0일 때, 십의자리일때, 백의자리일때 숫자텍스쳐 위치 보정
void CUI_PartTimeResult::Repose_TotalScoreTextures()
{
	// 리셋
	m_arrPosition[1] = _float2(-100.f, -100.f);
	m_arrPosition[2] = _float2(-100.f, -100.f);
	m_arrPosition[3] = _float2(-100.f, -100.f);

	if (m_fScore <= 0.f)			// 0일 때
		m_arrPosition[3] = _float2(800.f, 220.f);
	else if (m_fScore < 100.f)		// 십의 자리 일때
	{
		m_arrPosition[2] = _float2(777.f, 220.f); //40 > 46                                
		m_arrPosition[3] = _float2(823.f, 220.f);
	}
	else							// 백의 자리 일때
	{
		m_arrPosition[1] = _float2(754.f, 220.f);
		m_arrPosition[2] = _float2(800.f, 220.f);
		m_arrPosition[3] = _float2(846.f, 220.f);
	}
}

void CUI_PartTimeResult::Repose_ScoreTextures()
{
	// 리셋
	m_arrPosition[5] = _float2(-100.f, -100.f);
	m_arrPosition[6] = _float2(-100.f, -100.f);
	m_arrPosition[7] = _float2(-100.f, -100.f);

	_float fScore = m_arrScoreDigits[0] * 100 + m_arrScoreDigits[1] * 10 + m_arrScoreDigits[2];
	if (fScore <= 0.f)			// 0일 때
		m_arrPosition[7] = _float2(1035.f, 490.f);
	else if (fScore < 100.f)		// 십의 자리 일때
	{
		m_arrPosition[6] = _float2(1024.f, 490.f);
		m_arrPosition[7] = _float2(1058.f, 490.f);
	}
	else							// 백의 자리 일때
	{
		m_arrPosition[5] = _float2(1001.f, 490.f); //30 > 32
		m_arrPosition[6] = _float2(1033.f, 490.f);
		m_arrPosition[7] = _float2(1065.f, 490.f);
	}
}

CUI_PartTimeResult* CUI_PartTimeResult::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_PartTimeResult* pInstance = new CUI_PartTimeResult(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_PartTimeResult"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_PartTimeResult::Clone(void* pArg)
{
	CUI_PartTimeResult* pInstance = new CUI_PartTimeResult(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_PartTimeResult"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_PartTimeResult::Free()
{
	for (auto& texure : m_arrTexures)
		Safe_Release(texure);

	Safe_Release(m_pTexMask);
	__super::Free();
}

