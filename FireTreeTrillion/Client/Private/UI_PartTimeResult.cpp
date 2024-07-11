#include "stdafx.h"
#include "UI_PartTimeResult.h"

#include "Level_Loading.h"
#include "PartTimeHelper.h"

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
	m_arrSize[5] = m_arrSize[6] = m_arrSize[7] = m_SizeDigits2D;

	m_arrOriginalSize = m_arrSize;
	_float fInitialValue = 1.f;
	fill(m_arrSizeRatio.begin(), m_arrSizeRatio.end(), fInitialValue);

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
	__super::Tick(fTimeDelta);

	m_fTimeDelta = fTimeDelta;

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
						- m_arrPosition[i].y + g_iWinSizeY * 0.5f,
						0.f,
						1.f));

		hr = m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix");
		CHECK_FAILED(hr);

		hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
		CHECK_FAILED(hr);
		
		hr = m_pShaderCom->Begin(POSTEX_ALPHABLEND_NOTEST);
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render(); 
		CHECK_FAILED(hr);
	}

	Render_Digits();


	return S_OK;
}

#ifdef _DEBUG
void CUI_PartTimeResult::Render_IMGUI()
{
	//char ratio[16];
	////ImGui::DragFloat(ratio, (_float*)&m_fRatioTimeBar, 0.01f, 0.01f, 1.f);
	//ImGui::Separator(); ImGui::NewLine();

	//for (_int i = 0; i < m_arrPosition.size(); ++i)
	//{
	//	char name[16], size[16], color[16];
	//	sprintf_s(name,  "pos%d",   i);
	//	//sprintf_s(size,  "size%d",  i);

	//	ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
	//	//ImGui::DragFloat(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 0.1f, 2.f);

	//	m_arrSize[i].x = m_arrOriginalSize[i].x * m_arrSizeRatio[i];
	//	m_arrSize[i].y = m_arrOriginalSize[i].y * m_arrSizeRatio[i];

	//	m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
	//	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
	//		XMVectorSet(m_arrPosition[i].x   - g_iWinSizeX * 0.5f,
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

void CUI_PartTimeResult::Render_Digits()
{
	HRESULT hr(S_OK);
	static _float fTimeAcc = 0.f;
	m_fMoveRatio += m_fTimeDelta * 3.f;
	if (m_fMoveRatio >= 1.f)
	{
		m_fMoveRatio = 1.f;
		fTimeAcc += m_fTimeDelta;
		if (fTimeAcc >= 1.f)
		{
			_int iAddNum = 1;
			// 와들디 iAddNum만큼 등장 // 효선아 여기야

			// 30만큼 점수판 += 점수
			if (m_fScore < Change_ScoreTextures(iAddNum))
			{
				m_bRenderTotalScore = true;
				if (fTimeAcc >= 2.f)
				{
					// 최종 점수만큼 ScoreTextures가 채워져 있다.
					if (m_bRenderTotalScore)
						Render_TotalScore();
				}
				if (fTimeAcc >= 2.5f)
				{
					// 이펙트 넣기 // 여기야 효선아
				}

				// 다이얼로그 생성
				if (fTimeAcc >= 4.f)
				{
					// 다이얼로그 띄우기 // 이거 a버튼 누르면 town으로 돌아가기
					if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_A, KEY_DOWN))
					{
						hr = m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOWN));
						CHECK_FAILED(hr);
					}
				}
			}

			if (false == m_bRenderTotalScore)
			{
				fTimeAcc = 0.f;
				m_fMoveRatio = 0.f;
			}
		}
	}

	for (_int i = 5; i <= 7; ++i)
	{
		_float fPosRatio = sin(m_fMoveRatio * 3.14159f);
		m_fMovePosition2D = _float2(m_arrPosition[i].x, m_arrPosition[i].y - fPosRatio * 30.f);
		m_pTransformCom->Set_Scaled(m_arrSize[i].x, m_arrSize[i].y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
								   XMVectorSet(m_fMovePosition2D.x - g_iWinSizeX * 0.5f,
								   			   - m_fMovePosition2D.y + g_iWinSizeY * 0.5f,
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
			// 효선아 여기야 >> 게임 점수 다 뜨고 이펙트 나오는 구간
			// QZR 다이얼로그 출력 : 게임 수고했습니다 어쩌구
		}
	}

	for (_int i = 1; i <= 3; ++i)
	{
		_float fSizeRatio = sin(m_fSizeRatio * 3.14159f);
		m_fSize2D = _float2(m_arrSize[i].x + fSizeRatio * 10.f, m_arrSize[i].y + fSizeRatio * 10.f);
		m_pTransformCom->Set_Scaled(m_fSize2D.x, m_fSize2D.y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
								   XMVectorSet(m_arrPosition[i].x - g_iWinSizeX * 0.5f,
								   			   - m_arrPosition[i].y + g_iWinSizeY * 0.5f,
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

void CUI_PartTimeResult::Initialize_TexturePos()
{
	m_arrPosition[0] = _float2(800.f,  222.f);
									   
	m_arrPosition[1] = _float2(740.f,  212.f);
	m_arrPosition[2] = _float2(800.f,  212.f);
	m_arrPosition[3] = _float2(860.f,  212.f);
									   
	m_arrPosition[4] = _float2(800.f,  490.f);
									   
	m_arrPosition[5] = _float2(960.f,  482.f);
	m_arrPosition[6] = _float2(1010.f, 482.f);
	m_arrPosition[7] = _float2(1060.f, 482.f);
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

