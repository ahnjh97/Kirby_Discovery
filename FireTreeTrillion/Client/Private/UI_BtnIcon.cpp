#include "stdafx.h"
#include "UI_BtnIcon.h"
#include "Kirby.h"

CUI_BtnIcon::CUI_BtnIcon(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject { _pDevice, _pContext }
{
}

CUI_BtnIcon::CUI_BtnIcon(const CUI_BtnIcon& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CUI_BtnIcon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_BtnIcon::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* BtnIconDesc{};

	if (_pArg != nullptr)
		BtnIconDesc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;

	_float3 vScale = { 128.f, 128.f, 1.f };
	_float3 vOffset = { 0.65f, 0.65f, 1.f };
	m_vOrigScale = vScale * vOffset;
	m_pTransformCom->Set_Scaled(m_vOrigScale);

	//_float4 vTrans = { 478.f, -388.f, 1.f, 1.f };
	_float4 vTrans = { 508.f, -393.f, 1.f, 1.f };
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vTrans);

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_eCurState = BTN_IDLE;
	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();

	return S_OK;
}

_int CUI_BtnIcon::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	_float3 vOffset = { 0.6f, 0.6f, 1.f };
	switch (m_eCurState)
	{
	case BTN_HIDE:
		m_fBlinkAlpha -= fTimeDelta * 5.f;
		m_fBtnAlpha -= fTimeDelta * 5.f;
		break;

	case BTN_BLINK: //알파 값 증감
		m_fBtnAlpha = 1.f;
		m_fBlinkTime += fTimeDelta;

		if (m_fBlinkTime > 0.5f && m_fBlinkTime < 1.f)
			m_fBlinkAlpha = 0.5f;

		if (m_fBlinkTime > 1.f) //초기화
		{
			m_fBlinkAlpha = 0.f;
			m_fBlinkTime = 0.f;
		}
		break;
		
	case BTN_SELECT: //스케일 증감
		m_fBlinkAlpha = 0.f;
		m_fSelectTime += fTimeDelta * 2.f;
		m_pTransformCom->Set_Scaled(m_vOrigScale * vOffset);
		
		if (m_fSelectTime > 0.1f && m_fSelectTime < 0.2f)
		{
			//그래프 MAX값은 1이어야하며, 범위는 0 ~ 1로 설정되어야함
			vOffset.x += EASE_OUT(fTimeDelta * 5.f);
			vOffset.y += EASE_OUT(fTimeDelta * 5.f);
			m_pTransformCom->Set_Scaled(m_vOrigScale * vOffset);
		}

		if (m_fSelectTime > 0.2f) //초기화
		{
			m_fSelectTime = 0.f;
			m_pTransformCom->Set_Scaled(m_vOrigScale);
			m_eCurState = BTN_BLINK;
		}
		break;

	case BTN_NONE:	default:	break;
	}

	if (m_fBlinkAlpha <= 0.f && m_fBtnAlpha <= 0.f) //알파 값 보정 및 업데이트 중지
	{
		m_fBlinkAlpha = 0.f;
		m_fBtnAlpha = 0.f;
		return OBJ_NOEVENT;
	}

	return OBJ_NOEVENT;
}

void CUI_BtnIcon::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_BtnIcon::Render()
{
	HRESULT hr;

#pragma region RENDER_BINDSET

	//렌더 OFF
	if (BTN_HIDE == m_eCurState && 0.f == m_fBlinkAlpha && 0.f == m_fBtnAlpha)
		return S_OK;

	for (_uint iTEXIx = 0; iTEXIx < TEXBTN_NONE; ++iTEXIx)
	{
		PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };
		TEX_BTNTYPE eTexType = { TYPE_DEFAULT };

		if (TEXBTN_BASE == iTEXIx)
		{
			ePassType = POSTEX_UIWHITEALPHA;
			m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fBtnAlpha, sizeof(_float)); //알파를 별개로 조정
			m_fAlpha = 1.f;
			
			switch (*m_pCurrentLevelID)
			{
			case LEVEL_TOWN:
				eTexType = TYPE_DEFAULT;
				break;

			case LEVEL_DEEDEEDEE: case LEVEL_SIMBA: case LEVEL_FINALBOSS: case LEVEL_FINALE:
				eTexType = TYPE_BOSS;
				break;

			default: break;
			}
		}

		if (TEXBTN_BRIGHT == iTEXIx)
		{
			m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fBlinkAlpha, sizeof(_float));
			eTexType = TYPE_DEFAULT;
		}

		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		//셰이더 파일의 매트릭스 정보를 가져와 바인딩
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
			return E_FAIL;

		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTexCom[iTEXIx], eTexType);
		CHECK_FAILED(hr);
			
#pragma endregion

	}

	return S_OK;
}

#ifdef _DEBUG
void CUI_BtnIcon::Render_IMGUI()
{
	switch (m_eCurState)
	{
	case BTN_IDLE:		ImGui::Text(u8"BTN_IDLE");	break;
	case BTN_HIDE:		ImGui::Text(u8"BTN_HIDE");	break;
	case BTN_BLINK:		ImGui::Text(u8"BTN_BLINK"); break;
	case BTN_SELECT:	ImGui::Text(u8"BTN_SELECT"); break;
	case BTN_NONE:		default: ImGui::Text(u8"BTN_NONE"); break;
	}
}
#endif

HRESULT CUI_BtnIcon::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

  	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_BtnIconBase"),
		TEXT("Com_TexBase"), (CComponent**)&m_pTexCom[TEXBTN_BASE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_BtnIconBright"),
		TEXT("Com_TexBright"), (CComponent**)&m_pTexCom[TEXBTN_BRIGHT])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_BtnIcon::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
{
	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//셰이더의 원시데이터 가져와 저장
	_pShaderCom->Bind_RawValue("g_vRColor", &m_UIObjDesc.vColorRGB, sizeof(_float3));

	//Begin() > Apply() 함수 호출 전 셰이더 전역 데이터를 저장해야함
	if (FAILED(_pShaderCom->Begin(_iPassIndex)))
		return E_FAIL;

	if (FAILED(Bind_VIBuffer(m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_BtnIcon::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CUI_BtnIcon* CUI_BtnIcon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_BtnIcon* pInstance = new CUI_BtnIcon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_BtnIcon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_BtnIcon::Clone(void* pArg)
{
	CUI_BtnIcon* pInstance = new CUI_BtnIcon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CUI_BtnIcon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_BtnIcon::Free()
{
	__super::Free();
	
	for (auto& iTex : m_pTexCom)
		Safe_Release(iTex);
}


