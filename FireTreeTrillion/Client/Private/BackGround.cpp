#include "stdafx.h"
#include "..\Public\BackGround.h"
CBackGround::CBackGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}
CBackGround::CBackGround(const CBackGround& rhs)
	: CUIObject{ rhs }
{
}
HRESULT CBackGround::Initialize_Prototype()
{
	return S_OK;
}
HRESULT CBackGround::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;
	if (FAILED(Add_Components()))
		return E_FAIL;
	UIOBJ_DESC BG_DESC = {};
	BG_DESC.wstrUITag = { TEXT("BG_Logo") };
	BG_DESC.vCenter = { g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f };
	BG_DESC.vSize = { (_float)g_iWinSizeX, (_float)g_iWinSizeY };
	BG_DESC.vPos = { BG_DESC.vCenter.x, BG_DESC.vCenter.y };
	BG_DESC.fDegree = { 0.f };
	BG_DESC.iTexIndex = { 0 };

	m_pTransformCom->Set_Scaled(BG_DESC.vSize.x, BG_DESC.vSize.y, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		BG_DESC.vPos.x - BG_DESC.vCenter.x,
		-BG_DESC.vPos.y + BG_DESC.vCenter.y, 0.f, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	return S_OK;
}

_int CBackGround::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CBackGround::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CBackGround::Render()
{
	if (FAILED(Bind_ShaderResources(m_pShaderCom, PS_DEFAULT, m_pTransformCom, m_pTextureCom, 0)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBackGround::Render_IMGUI()
{}
#endif // DEBUG

HRESULT CBackGround::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Texture_Logo"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;
	return S_OK;
}
HRESULT CBackGround::Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex)
{
	CHECK_NULLPTR(_pShaderCom);

	if (FAILED(_pTransCom->Bind_ShaderResource(_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	_pTextureCom->Bind_ShaderResource(_pShaderCom, "g_DiffuseTexture", _iTexIndex);

	if (FAILED(_pShaderCom->Begin(1)))
		return E_FAIL;
	if (FAILED(Bind_VIBuffer(m_pVIBufferCom)))
		return E_FAIL;
	return S_OK;
}

HRESULT CBackGround::Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom)
{
	if (FAILED(_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}
CBackGround* CBackGround::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBackGround* pInstance = new CBackGround(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBackGround"));
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CBackGround::Clone(void* pArg)
{
	CBackGround* pInstance = new CBackGround(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CBackGround"));
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CBackGround::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}