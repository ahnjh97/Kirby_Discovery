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

	UIOBJ_DESC* LayerUI_Desc{};
	if (nullptr != _pArg)
		LayerUI_Desc = (UIOBJ_DESC*)_pArg;

	//if (UI_TEXTURE == (*LayerUI_Desc).eUIType)
	//{
	if (FAILED(Add_Components()))
		return E_FAIL;
	//}

	m_UIObjDesc = (*LayerUI_Desc);
	m_UIObjDesc.eUIType = (*LayerUI_Desc).eUIType;
	m_UIObjDesc.vColorRGB = (*LayerUI_Desc).vColorRGB;
	m_UIObjDesc.fAlpha = (*LayerUI_Desc).fAlpha;

	if (UI_TEXTURE == m_UIObjDesc.eUIType)
		m_iTexIndex = (*LayerUI_Desc).iTexIndex;

	if (UI_FONT == m_UIObjDesc.eUIType)
		m_UIObjDesc.wstrText = (*LayerUI_Desc).wstrText;


		m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, 
			XMVectorSet(m_UIObjDesc.vPos.x - m_UIObjDesc.vCenter.x + m_UIObjDesc.vCenter.x,
						m_UIObjDesc.vPos.y - m_UIObjDesc.vCenter.y + m_UIObjDesc.vCenter.y, 0.f, 1.f));
		m_pTransformCom->Rotation(XMVectorSet(0.f, 0.f, 1.f, 1.f), XMConvertToRadians(m_UIObjDesc.fDegree));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

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
		// PS_ALPHABLEND > PS_DEFAULT로 변경
		if (FAILED(Bind_ShaderResources(m_pShaderCom, PS_DEFAULT, m_pTransformCom, m_pTextureCom, m_iTexIndex)))
			return E_FAIL;
	}

	if (UI_FONT == m_UIObjDesc.eUIType)
	{
		//if (FAILED(Bind_ShaderResources(m_pShaderCom, PS_DEFAULT, m_pTransformCom)))
		//	return E_FAIL;
		_float2 vFontPos = { m_UIObjDesc.vPos.x + m_UIObjDesc.vCenter.x, 
							- m_UIObjDesc.vPos.y + m_UIObjDesc.vCenter.y};

		//스프라이트 폰트 렌더 (폰트 테스트용)
		if (FAILED(m_pGameInstance->
			Render_Font(TEXT("Font_HUDSub_KR15"), m_UIObjDesc.wstrText, vFontPos, m_UIObjDesc.vColorRGB, 
				XMConvertToRadians(m_UIObjDesc.fDegree))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	//if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_KirbyBarHard"),
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameComplete"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex)
{
	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//// 05.24) 카메라 줌인/아웃용 Matrix 처리
	//_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix();
	//_float4x4 ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	//_float4x4 ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//Begin() > Apply() 함수 호출 전 셰이더 전역 데이터를 저장해야함
	if (FAILED(_pShaderCom->Begin(_iPassIndex)))
		return E_FAIL;

	if (FAILED(Bind_VIBuffer(m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_KirbyStatus::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom)
{
	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

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


