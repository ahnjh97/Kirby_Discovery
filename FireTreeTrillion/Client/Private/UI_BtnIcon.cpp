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
	
	//m_UIObjDesc = *BtnIconDesc;

	if (FAILED(Add_Components()))
		return E_FAIL;

	_float3 vScale = { 128.f * 0.9f, 128.f * 0.9f, 1.f };
	m_pTransformCom->Set_Scaled(vScale);

	_float4 vTrans = { 478.f, -388.f, 1.f, 1.f };
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vTrans);

	//m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	return S_OK;
}

_int CUI_BtnIcon::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD7, KEY_PRESS)) //테스트용
		m_UIObjDesc.fAlpha = 1.f;

	else
		m_UIObjDesc.fAlpha -= fTimeDelta * 10.f;

	if (m_UIObjDesc.fAlpha <= 0.f) //다이얼로그 출력이 끝날경우, 알파 값 조절
	{
		m_UIObjDesc.fAlpha = 0.f;
		return OBJ_NOEVENT;
	}
}

void CUI_BtnIcon::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_BtnIcon::Render()
{
	HRESULT hr;

#pragma region RENDER_BINDSET
	
	for (_uint iTEXIx = 0; iTEXIx < TEXBTN_NONE; ++iTEXIx)
	{
		PASS_POSTEX ePassType = { POSTEX_ALPHABLEND_NOTEST };
		if (TEXBTN_BASE == iTEXIx)
		{
			ePassType = POSTEX_UIWHITEALPHA;
			//m_UIObjDesc.fAlpha = 1.f;
		}

		//if (TEXBTN_BRIGHT == iTEXIx)
		//	m_UIObjDesc.fAlpha = 0.5f;

		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;

		//셰이더 파일의 매트릭스 정보를 가져와 바인딩
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
			return E_FAIL;

		hr = Bind_ShaderResources(m_pShaderCom, ePassType, m_pTextureCom[iTEXIx], 0);
		CHECK_FAILED(hr);

#pragma endregion

	}

	return S_OK;
}

HRESULT CUI_BtnIcon::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

  	if (FAILED(__super::Add_Component(LEVEL_DEEDEEDEE, TEXT("Prototype_Component_Texture_UI_BtnIconBase"),
		TEXT("Com_TextBase"), (CComponent**)&m_pTextureCom[TEXBTN_BASE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_DEEDEEDEE, TEXT("Prototype_Component_Texture_UI_BtnIconBright"),
		TEXT("Com_TexBright"), (CComponent**)&m_pTextureCom[TEXBTN_BRIGHT])))
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

	_pShaderCom->Bind_RawValue("g_fAlpha", &m_UIObjDesc.fAlpha, sizeof(_float));

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
	
	for (auto& iTex : m_pTextureCom)
		Safe_Release(iTex);

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
}


