#include "stdafx.h"
#include "Single_UI.h"

CSingle_UI::CSingle_UI(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext)
	: CUIObject{ _pDevice, _pContext }
{
}

CSingle_UI::CSingle_UI(const CSingle_UI& _rhs)
    : CUIObject {_rhs}
{
}

HRESULT CSingle_UI::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSingle_UI::Initialize(void* _pArg)
{
	HRESULT hr = __super::Initialize(_pArg);
	CHECK_FAILED(hr);

	UIOBJ_DESC* SingleUI_Desc{};
	if (nullptr != _pArg)
		SingleUI_Desc = (UIOBJ_DESC*)_pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(SingleUI_Desc->vSize.x, SingleUI_Desc->vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
								SingleUI_Desc->vPos.x - SingleUI_Desc->vCenter.x,
								-SingleUI_Desc->vCenter.y + SingleUI_Desc->vPos.y, 0.f, 1.f));
	m_pTransformCom->Rotation(XMVectorSet(0.f, 0.f, 1.f, 1.f), XMConvertToRadians(SingleUI_Desc->fDegree));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));
	
	m_iTexIndex = SingleUI_Desc->iTexIndex;
	m_UIObjDesc = *SingleUI_Desc;
	m_eUIType = TYPE_SINGLE;

#pragma endregion

	return S_OK;
}

_int CSingle_UI::Tick(_float fTimeDelta)
{	
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CSingle_UI::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CSingle_UI::Render()
{
	// PS_ALPHABLEND > PS_DEFAULT로 변경
	if (FAILED(Bind_ShaderResources(m_pShaderCom, PS_DEFAULT, m_pTransformCom, m_pTextureCom, m_iTexIndex)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSingle_UI::Add_Components()
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

HRESULT CSingle_UI::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex)
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

HRESULT CSingle_UI::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CSingle_UI* CSingle_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSingle_UI* pInstance = new CSingle_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CSingle_UI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSingle_UI::Clone(void* pArg)
{
	CSingle_UI* pInstance = new CSingle_UI(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSingle_UI"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSingle_UI::Free()
{
	__super::Free();

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}


