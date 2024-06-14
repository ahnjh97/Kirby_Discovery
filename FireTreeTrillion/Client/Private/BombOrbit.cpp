#include "stdafx.h"
#include "BombOrbit.h"

CBombOrbit::CBombOrbit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CBombOrbit::CBombOrbit(const CBombOrbit& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CBombOrbit::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBombOrbit::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	GameObjectDesc.fSpeedPerSec = 1.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	return S_OK;
}

_int CBombOrbit::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;


	return OBJ_NOEVENT;
}

void CBombOrbit::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CBombOrbit::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(POSTEX_ALPHABLEND)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CBombOrbit::Update_OrbitPosition(_fvector vPos, _fvector vNormal)
{


}

HRESULT CBombOrbit::Add_Components()
{
	// UI ¼ÎÀÌ´õ Àü¿ë
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	// Rect¸¦ ½á¾ß ÇÑ´Ù.
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	// ÃÑ ÀÌÆåÆ® (ºÒ²É)
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Moon"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBombOrbit::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
		return E_FAIL;

	return S_OK;
}

CBombOrbit* CBombOrbit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBombOrbit* pInstance = new CBombOrbit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBombOrbitGlow"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBombOrbit::Clone(void* pArg)
{
	CBombOrbit* pInstance = new CBombOrbit(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CBombOrbit"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBombOrbit::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
