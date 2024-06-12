#include "stdafx.h"
#include "HUD_KirbyStatus.h"

CHUD_KirbyStatus::CHUD_KirbyStatus(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CHUD_KirbyStatus::CHUD_KirbyStatus(const CHUD_KirbyStatus& _rhs)
	: CUIObject{ _rhs }
{
}

HRESULT CHUD_KirbyStatus::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_KirbyStatus::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* HUDKirby_Desc{};
	if (nullptr != _pArg)
		HUDKirby_Desc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;
	
	m_UIObjDesc = (*HUDKirby_Desc);
	m_UIObjDesc.eUIType = (*HUDKirby_Desc).eUIType;
	m_UIObjDesc.vColorRGB = (*HUDKirby_Desc).vColorRGB;
	m_UIObjDesc.fAlpha = (*HUDKirby_Desc).fAlpha;
	m_UIObjDesc.vDegree = (*HUDKirby_Desc).vDegree;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*HUDKirby_Desc).iTexIndex;

	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*HUDKirby_Desc).wstrText;


	m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
			m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y, 0.f, 1.f));

#pragma region SET_PROJ

	if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
	{
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
		XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	}

	if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
	{
		m_pTransformCom->Rotation(XMVectorSet(AXIS_X), XMConvertToRadians(m_UIObjDesc.vDegree.x));
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Y), XMConvertToRadians(m_UIObjDesc.vDegree.y));
		m_pTransformCom->Rotation(XMVectorSet(AXIS_Z), XMConvertToRadians(m_UIObjDesc.vDegree.z));
	}

#pragma endregion

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	return S_OK;
}

_int CHUD_KirbyStatus::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CHUD_KirbyStatus::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_KirbyStatus::Render()
{
	if (UI_TEXTURE == m_UIObjDesc.eUIType)
	{
		if (PROJ_ORTHO == m_UIObjDesc.eUIProj)
			Render_OrthoProj(m_pShaderCom, m_pTransformCom);

		if (PROJ_PERSPEC == m_UIObjDesc.eUIProj)
			Render_PerspecProj(m_pShaderCom, m_pTransformCom);
	}

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		_float2 vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x,
							-m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y };

		_float4 vFontRGBA = { m_UIObjDesc.vColorRGB.x, m_UIObjDesc.vColorRGB.y, m_UIObjDesc.vColorRGB.z, m_UIObjDesc.fAlpha };
		if (FAILED(m_pGameInstance->
			Render_Font(TEXT("Font_HUDSub_KR15"), m_UIObjDesc.wstrText, vFontPos, vFontRGBA,
				XMConvertToRadians(m_UIObjDesc.vDegree.z))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_StatusBar_Kirby"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Render_OrthoProj(CShader* _pShaderCom, CTransform* _pTransCom)
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

HRESULT CHUD_KirbyStatus::Render_PerspecProj(CShader* _pShaderCom, CTransform* _pTransCom)
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

HRESULT CHUD_KirbyStatus::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex)
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

HRESULT CHUD_KirbyStatus::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CHUD_KirbyStatus* CHUD_KirbyStatus::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_KirbyStatus* pInstance = new CHUD_KirbyStatus(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_KirbyStatus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_KirbyStatus::Clone(void* pArg)
{
	CHUD_KirbyStatus* pInstance = new CHUD_KirbyStatus(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_KirbyStatus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_KirbyStatus::Free()
{
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}


