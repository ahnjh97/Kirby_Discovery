#include "stdafx.h"
#include "UI_MessageWindow.h"
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

	UIOBJ_DESC* MessageWindowDesc{};
	if (_pArg != nullptr)
		MessageWindowDesc = (UIOBJ_DESC*)_pArg;
	
	m_UIObjDesc = *MessageWindowDesc;

	if (FAILED(Add_Transform(_pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
#pragma region MESSAGEWINDOW BASE

	m_pTransCom[TEXMW_BASE]->Set_Scaled(m_UIObjDesc.vSize);
	_float4 vBaseTrans = { m_UIObjDesc.vPos };
	vBaseTrans.w = 1.f;
	m_pTransCom[TEXMW_BASE]->Set_State(CTransform::STATE_POSITION, vBaseTrans);

#pragma endregion

#pragma region MESSAGEWINDOW BTN BASE

	_float3 vScale = { 76.f, 76.f, 1.f };
	m_pTransCom[TEXMW_BTNBASE]->Set_Scaled(vScale);

	_float4 vBtnTrans = { 480.f, -390.f, 1.f, 1.f };
	m_pTransCom[TEXMW_BTNBASE]->Set_State(CTransform::STATE_POSITION, vBtnTrans);

#pragma endregion

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_UIObjDesc.fAlpha = 0.f;
	m_eCurState = WINDOW_HIDE;

	return S_OK;
}

_int CUI_MessageWindow::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	//특정 트리거가 발동할 경우, 해당 Window UI를 출력
	if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN) && WINDOW_HIDE == m_eCurState) //테스트용
		m_eCurState = WINDOW_SHOW;
	
	//Window UI 출력은 스크립트가 종료될때까지 유지
	//스크립트 인덱스가 종료될 경우, State를 변경
	else if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE, KEY_DOWN) && WINDOW_SHOW == m_eCurState) //테스트용
		m_eCurState = WINDOW_HIDE;

	switch (m_eCurState)
	{
	case WINDOW_HIDE: m_UIObjDesc.fAlpha -= fTimeDelta * 5.f;	break;
	case WINDOW_SHOW: m_UIObjDesc.fAlpha = 1.f;	break;
	case WINDOW_IDLE: default:	break;
	}

	if (m_UIObjDesc.fAlpha <= 0.f) //알파 값 보정 및 업데이트 중지
	{
		m_UIObjDesc.fAlpha = 0.f;
		return OBJ_NOEVENT;
	}

	return OBJ_NOEVENT;
}

void CUI_MessageWindow::Late_Tick(_float fTimeDelta)
{
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

#pragma endregion

	}

	/*
	//SpriteFont 폰트 수정 필요. 임시 주석처리
	wstring wstrFontTag = { TEXT("Font_HUDSub_KR15") };
	wstring wstrText = { TEXT("???") };
	_float2 vFontPos = { 410.f, 660.f };
	//_float4 vFontRGBA = { 176.f / 255.f, 12.f / 255.f, 24.f / 255.f, m_UIObjDesc.fAlpha };
	
	_float4 vFontRGBA = { m_UIObjDesc.vColorRGB};
	vFontRGBA.w = m_UIObjDesc.fAlpha;

	_float2 vFontOrig = { 1.f, 1.f };
	_float2 vFontScale = { 1.2f, 1.2f };
	_float fRadian = { XMConvertToRadians(0.f) };

	m_pGameInstance->Render_Font(wstrFontTag, wstrText, vFontPos, vFontRGBA, fRadian, vFontOrig, vFontScale);
	*/

	return S_OK;
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

		//m_Components.emplace(g_strTransformTag, m_pTransformCom);
		Safe_AddRef(m_pTransCom[iTrans]);
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

	for (auto& iTrans : m_pTransCom)
		Safe_Release(iTrans);

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
}


