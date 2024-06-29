#include "stdafx.h"
#include "UI_PartTimeDee.h"

#include "PartTimeHelper.h"

const _float fOffsetY = 10.f;

CUI_PartTimeDee::CUI_PartTimeDee(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CUI_PartTimeDee::CUI_PartTimeDee(const CUI_PartTimeDee& rhs)
	: CUIObject{ rhs }
	, m_arrTexures(rhs.m_arrTexures)
	, m_arrSize(rhs.m_arrSize)
	, m_arrPosition(rhs.m_arrPosition)
	, m_arrOriginalSize(rhs.m_arrOriginalSize)
	, m_arrSizeRatio(rhs.m_arrSizeRatio)
	, m_arrColor(rhs.m_arrColor)
	, m_SizeDialog2D(rhs.m_SizeDialog2D)
	, m_SizeFood2D(rhs.m_SizeFood2D)
{
}

HRESULT CUI_PartTimeDee::Initialize_Prototype()
{
	fill(m_arrTexures.begin(), m_arrTexures.end(), nullptr);

	m_arrSize[0] = m_arrSize[1] = m_SizeDialog2D;
	m_arrSize[2] = m_arrSize[3] = m_arrSize[4] = m_arrSize[5] = m_SizeFood2D;
	_float2 InitialValue2D = {0.f, 0.f};
	fill(m_arrPosition.begin(), m_arrPosition.end(), InitialValue2D);

	m_arrOriginalSize = m_arrSize;
	_float fInitialValue = 1.f;
	fill(m_arrSizeRatio.begin(), m_arrSizeRatio.end(), fInitialValue);
	_float3 vInitialValue = {0.f, 0.f, 0.f};
	fill(m_arrColor.begin(), m_arrColor.end(), vInitialValue);

	return S_OK;
}

HRESULT CUI_PartTimeDee::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_bIsRender = true;

	fill(m_arrFoodRender.begin(), m_arrFoodRender.end(), false);
	PARTTIME_ITEM eItem = CPartTimeHelper::Get_Instance()->Get_PartTimeItem();
	_int iTexture = static_cast<_int>(eItem);
	m_arrFoodRender[iTexture] = true;

	return S_OK; 
}

_int CUI_PartTimeDee::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	// for masking
	m_fMask += fTimeDelta * 0.1f;
	if (1.f < m_fMask) m_fMask = 1.f;

	return OBJ_NOEVENT;
}

void CUI_PartTimeDee::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_PartTimeDee::Render()
{
	if (m_bIsRender == false) return S_OK;

	HRESULT hr;
	hr = Bind_ShaderResources();
	CHECK_FAILED(hr);

	for (_int i = 0; i < m_arrTexures.size(); ++i)
	{
		if (m_eDialogTheme == THINKING) // dirty
			if (i != 1) continue;
		else
			if (i == 1) continue;

		if (i >= 2) // 4개의 음식중 true처리가 되어있는 음식만 렌더합니다.
		{
			if (m_arrFoodRender[i - 2] == false)
				continue;
		}

		// UI별 포지션, 사이즈, 컬러 조정
		Setup_PosSizeColor(i);

		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		if (i >= 2)
		{
			_float fAlpha = 1.f;
			m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(_float));
			if(m_bRandomColor)
				m_pShaderCom->Bind_RawValue("g_vRColor", &m_vFoodColor, sizeof(_float3));
			
			// Binding DIFFUSE
			hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
			
			// Binding MASK
			if (m_bRandomMask)
			{
				_int iMask = 2;
				m_pShaderCom->Bind_RawValue("g_iMasking",   &iMask,   sizeof(_int));
				m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fMask, sizeof(_float));
				m_pTexMask->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0);
			}
			
			hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR_VERTICALCUT);
			CHECK_FAILED(hr);
		}
		else
		{
			hr = m_arrTexures[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0);
			hr = m_pShaderCom->Begin(POSTEX_ALPHATEST_COLOR_HORIZONTALCUT);
			CHECK_FAILED(hr);
		}

		hr = m_pVIBufferCom->Bind_Buffers();
		CHECK_FAILED(hr);

		hr = m_pVIBufferCom->Render();
		CHECK_FAILED(hr);
	}

	return S_OK;
}

#ifdef _DEBUG
void CUI_PartTimeDee::Render_IMGUI()
{
	//char ratio[16] = "";
	////ImGui::DragFloat(ratio, (_float*)&m_fRatioTimeBar, 0.01f, 0.01f, 1.f);
	//ImGui::Separator(); ImGui::NewLine();

	//for (_int i = 0; i < m_arrPosition.size(); ++i)
	//{
	//	char name[16], size[16], color[16] = "";
	//	sprintf_s(name, "pos%d", i);
	//	sprintf_s(size, "size%d", i);
	//	//sprintf_s(color, "color%d", i);

	//	ImGui::DragFloat(size,   (_float*)&m_arrSizeRatio[i], 0.05f, 0.1f, 2.f);
	//	ImGui::DragFloat2(name,  (_float*)&m_arrPosition[i]);
	//	//ImGui::DragFloat3(color, (_float*)&m_arrColor[i], 0.01f, 0.f, 1.f);

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
	//ImGui::DragFloat(test, (_float*)&m_test, 1.f, 1.f, 500.f);
	//ImGui::DragFloat3(test2, (_float*)&m_vTESTCOLOR2, 0.01f, 0.f, 1.f);
}

#endif

HRESULT CUI_PartTimeDee::Add_Components()
{
	HRESULT hr(S_OK);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	#pragma region 텍스쳐 컴포넌트
	// 말풍선 배경 : 질문지
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_OrderCloud"),
		TEXT("Com_Texture_OrderCloud"), (CComponent**)&m_arrTexures[0])))
		return E_FAIL;
	// 말풍선 배경 : 대기하고 있는 와들디에게 띄우는 것
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_ThinkingCloud"),
		TEXT("Com_Texture_ThinkingCloud"), (CComponent**)&m_arrTexures[1])))
		return E_FAIL;
	// 음식
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_cake"),
		TEXT("Com_Texture_cake"), (CComponent**)&m_arrTexures[2])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_tomato"),
		TEXT("Com_Texture_tomato"), (CComponent**)&m_arrTexures[3])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_energydrink"),
		TEXT("Com_Texture_energydrink"), (CComponent**)&m_arrTexures[4])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameFoodUI_burger"),
		TEXT("Com_Texture_burger"), (CComponent**)&m_arrTexures[5])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_FoodShape"),
		TEXT("Com_Texture_FoodShape"), (CComponent**)&m_pTexMask)))
		return E_FAIL;
#pragma endregion

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_PartTimeDee::Bind_ShaderResources()
{
	HRESULT hr(S_OK);
	CHECK_NULLPTR(m_pShaderCom);

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	return S_OK;
}

void CUI_PartTimeDee::Setup_PosSizeColor(_int iTextureNum)
{
	switch (iTextureNum)
	{
	case 0: // 주문하려는 말풍선
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet((m_vFinPos.x * g_iWinSizeX) * 0.5f,
				(m_vFinPos.y * g_iWinSizeY) * 0.5f,
				0.f,
				1.f));
	}
	break;
	case 1: // 고민하는 말풍선
		m_arrSize[iTextureNum] = m_arrSize[0] * 0.8f;
	break;
	case 2: // 음식 : 케이크
	case 3: // 음식 : 토마토
	case 4: // 음식 : 요구루투
	case 5: // 음식 : 함바그
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet((m_vFinPos.x * g_iWinSizeX) * 0.5f,
				(m_vFinPos.y * g_iWinSizeY) * 0.5f + fOffsetY,
				0.f,
				1.f));
	}
	break;
	}

	m_pTransformCom->Set_Scaled(m_arrSize[iTextureNum].x, m_arrSize[iTextureNum].y, 1.f);
}

void CUI_PartTimeDee::Change_Dialog(PARTTIME_ITEM eItem)
{
	fill(m_arrFoodRender.begin(), m_arrFoodRender.end(), false);
	_int iTexture = static_cast<_int>(eItem);
	m_arrFoodRender[iTexture] = true;

	Make_RandomImg();
}

void CUI_PartTimeDee::Make_RandomImg()
{
	// Random Color
	_int iRandomColor = CUtils::Make_RandomInt(0, 1);
	m_bRandomColor = iRandomColor;

	// Random Masking
	_int iRandomMask = CUtils::Make_RandomInt(0, 1);
	m_bRandomMask = iRandomMask;
}

void CUI_PartTimeDee::Update_Pos(_float3 _vPosition)
{
	_float4 vNewPosition = _float4{ _vPosition.x, _vPosition.y, _vPosition.z, 1.f };

	// 뷰-투영까지 온 상황
	_matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	_matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_matrix ComMatrix = ViewMatrix * ProjMatrix;
	// 뿅
	m_vFinPos = XMVector3TransformCoord(XMLoadFloat4(&vNewPosition), ComMatrix);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet( (m_vFinPos.x * g_iWinSizeX) * 0.5f,
			(m_vFinPos.y * g_iWinSizeY) * 0.5f,
					0.f,
					1.f));
}

CUI_PartTimeDee* CUI_PartTimeDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_PartTimeDee* pInstance = new CUI_PartTimeDee(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_PartTimeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_PartTimeDee::Clone(void* pArg)
{
	CUI_PartTimeDee* pInstance = new CUI_PartTimeDee(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_PartTimeDee"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_PartTimeDee::Free()
{
	for (auto& texure : m_arrTexures)
		Safe_Release(texure);

	Safe_Release(m_pTexMask);
	__super::Free();
}

