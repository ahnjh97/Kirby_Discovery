#include "stdafx.h"
#include "HUD_Kirby.h"

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

HRESULT CHUD_Kirby::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* HUD_KirbyDESC = nullptr;
	if (nullptr != _pArg)
		HUD_KirbyDESC = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(HUD_KirbyDESC->vSize.x, HUD_KirbyDESC->vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
						HUD_KirbyDESC->vCenter.x - HUD_KirbyDESC->vPos.x,
						-HUD_KirbyDESC->vCenter.y + HUD_KirbyDESC->vPos.y, 0.f, 1.f));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	
	m_UIObjDesc = *HUD_KirbyDESC;

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
	if (FAILED(Bind_ShaderResources(m_pShaderCom, PS_ALPHABLEND, m_pTransformCom, m_pTextureCom, 7)))
		return E_FAIL;

	return S_OK;
}

void CHUD_Kirby::Render_IMGUI()
{
}

HRESULT CHUD_Kirby::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_GameComplete"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHUD_Kirby::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex)
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


