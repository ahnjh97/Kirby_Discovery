#include "stdafx.h"
#include "HUD_Status.h"

CHUD_Kirby::CHUD_Kirby(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CHUD_Kirby::CHUD_Kirby(const CHUD_Kirby& _rhs)
    : CUIObject {_rhs}
{
}

HRESULT CHUD_Kirby::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CHUD_Kirby::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	if (FAILED(Add_Components()))
		return E_FAIL;

#pragma region HUD_StatusHP

	UIOBJ_DESC LOGO_DESC{};
	LOGO_DESC.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	LOGO_DESC.vSize = { 100.f, 100.f };
	LOGO_DESC.vPos = { LOGO_DESC.vCenter.x/* - 200.f*/, LOGO_DESC.vCenter.y/* - 200.f */};

	m_pTransformCom->Set_Scaled(LOGO_DESC.vSize.x, LOGO_DESC.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
					LOGO_DESC.vCenter.x - LOGO_DESC.vPos.x,
					-LOGO_DESC.vCenter.y + LOGO_DESC.vPos.y, 0.f, 1.f));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_UIObjDesc = LOGO_DESC;

#pragma endregion

	return S_OK;
}

_int CHUD_Kirby::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CHUD_Kirby::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_Kirby::Render()
{
	HRESULT hr;
	for (size_t i = 0; i < 21; i++)
	{
		hr = Bind_ShaderResources(m_pShaderCom, PS_ALPHABLEND, m_pTransformCom, m_pTextureCom, i);
		CHECK_FAILED(hr);

		m_pTransformCom->Set_Scaled(m_UIObjDesc.vSize.x, m_UIObjDesc.vSize.y, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
			m_UIObjDesc.vCenter.x - m_UIObjDesc.vPos.x,
			-m_UIObjDesc.vCenter.y + m_UIObjDesc.vPos.y, 0.f, 1.f));

	}

	return S_OK;
}

void CHUD_Kirby::Render_IMGUI()
{
}

HRESULT CHUD_Kirby::Add_Components()
{
	HRESULT hr;
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_KirbyBarHard"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CHUD_Kirby::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex)
{
	CHECK_NULLPTR(_pShaderCom);

	HRESULT hr;
	hr = _pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix");
	CHECK_FAILED(hr);

	//// 05.24) 카메라 줌인/아웃용 Matrix 처리
	//_matrix WorldMatrix = m_pTransformCom->Get_WorldMatrix();

	//_float4x4 ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
	//_float4x4 ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

	//셰이더 파일의 매트릭스 정보를 가져와 바인딩
	hr = _pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix);
	CHECK_FAILED(hr);

	hr = _pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix);
	CHECK_FAILED(hr);

	//셰이더 파일의 텍스처 정보를 가져와 바인딩
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	//Begin() > Apply() 함수 호출 전 셰이더 전역 데이터를 저장해야함
	hr = _pShaderCom->Begin(_iPassIndex);
	CHECK_FAILED(hr);

	hr = Bind_VIBuffer(m_pVIBufferCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CHUD_Kirby::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CHUD_Kirby* CHUD_Kirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_Kirby* pInstance = new CHUD_Kirby(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_Kirby"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_Kirby::Clone(void* pArg)
{
	CHUD_Kirby* pInstance = new CHUD_Kirby(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_Kirby"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHUD_Kirby::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}


