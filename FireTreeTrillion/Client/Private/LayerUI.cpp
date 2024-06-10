#include "stdafx.h"
#include "LayerUI.h"

CLayerUI::CLayerUI(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CLayerUI::CLayerUI(const CLayerUI& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CLayerUI::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLayerUI::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* LayerUI_Desc{};
	if (nullptr != _pArg)
		LayerUI_Desc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_UIObjDesc = (*LayerUI_Desc);
	m_UIObjDesc.eUIType = (*LayerUI_Desc).eUIType;
	m_UIObjDesc.vColorRGB = (*LayerUI_Desc).vColorRGB;
	m_UIObjDesc.fAlpha = (*LayerUI_Desc).fAlpha;
	m_UIObjDesc.vDegree = (*LayerUI_Desc).vDegree;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*LayerUI_Desc).iTexIndex;
		
	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*LayerUI_Desc).wstrText;
	

	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, 
		XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
					m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y, 
					m_UIObjDesc.vPos.z, 1.f));


#pragma region SET_PROJ

	if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
	{
		m_UIObjDesc.vDegree.z = (*LayerUI_Desc).vDegree.z;
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	}

	if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
	{
		m_UIObjDesc.vDegree = (*LayerUI_Desc).vDegree;
		m_pTransformCom->Rotation(XMVectorSet(AXIS_X), XMConvertToRadians(m_UIObjDesc.vDegree.x));
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Y), XMConvertToRadians(m_UIObjDesc.vDegree.y));
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	}

#pragma endregion
		
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	return S_OK;
}

_int CLayerUI::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CLayerUI::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CLayerUI::Render()
{
	if (UI_TEXTURE == m_UIObjDesc.eUIType)
	{
		if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
			Render_OrthoProj(m_pShaderCom, m_pTransformCom);

		if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
			Render_PerspecProj(m_pShaderCom, m_pTransformCom); //원근 회전의 데이터는 잘 로드되나 렌더 시에 실적용이 안되는 현상
	}

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		_float2 vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x, 
							- m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y};

		_float4 vFontRGBA = { m_UIObjDesc.vColorRGB.x, m_UIObjDesc.vColorRGB.y, m_UIObjDesc.vColorRGB.z, m_UIObjDesc.fAlpha };
		if (FAILED(m_pGameInstance->
			Render_Font(TEXT("Font_HUDSub_KR15"), m_UIObjDesc.wstrText, vFontPos, vFontRGBA,
				XMConvertToRadians(m_UIObjDesc.vDegree.z))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLayerUI::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_KirbyStatus"),
	//if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameComplete"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLayerUI::Render_OrthoProj(CShader* _pShaderCom, CTransform* _pTransCom)
{
	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(Bind_ShaderResources(_pShaderCom, PS_ALPHABLEND, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLayerUI::Render_PerspecProj(CShader* _pShaderCom, CTransform* _pTransCom)
{
	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	_float4x4 ViewMatrix{}; //= m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
	_float4x4 ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjMatrix)))
		return E_FAIL;

	if (FAILED(Bind_ShaderResources(_pShaderCom, PS_ALPHABLEND, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLayerUI::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
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

HRESULT CLayerUI::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CLayerUI* CLayerUI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLayerUI* pInstance = new CLayerUI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CLayerUI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLayerUI::Clone(void* pArg)
{
	CLayerUI* pInstance = new CLayerUI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLayerUI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLayerUI::Free()
{
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}


