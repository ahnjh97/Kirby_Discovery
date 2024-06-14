#include "stdafx.h"
#include "BombOrbitGlow.h"

CBombOrbitGlow::CBombOrbitGlow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CBombOrbitGlow::CBombOrbitGlow(const CBombOrbitGlow& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CBombOrbitGlow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBombOrbitGlow::Initialize(void* pArg)
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

_int CBombOrbitGlow::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;


	return OBJ_NOEVENT;
}

void CBombOrbitGlow::Late_Tick(_float fTimeDelta)
{
	Billboard(fTimeDelta);
	Compute_ViewZ();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CBombOrbitGlow::Render()
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

void CBombOrbitGlow::Update_GlowPosition(_fvector vPos)
{


}

HRESULT CBombOrbitGlow::Add_Components()
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

HRESULT CBombOrbitGlow::Bind_ShaderResources()
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

void CBombOrbitGlow::Billboard(_float fTimeDelta)
{
	// ºôº¸µå
	_float3   vScale = m_pTransformCom->Get_Scaled();
	_float4x4      CamMatrix;
	const CTransform* pCamTransform = static_cast<const CTransform*>(m_pGameInstance->Get_Component(*m_pCurrentLevelID, TEXT("Layer_Camera"), g_strTransformTag));
	CamMatrix = pCamTransform->Get_WorldFloat4x4();

	_vector vLook, vRight, vUp;

	vRight = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_RIGHT);
	vLook = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_LOOK);
	vUp = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_UP);

	vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 1.f), vLook);
	vLook = XMVector3Cross(vRight, vUp);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);
	m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScale.y);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
	// ºôº¸µå ³¡

}

CBombOrbitGlow* CBombOrbitGlow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBombOrbitGlow* pInstance = new CBombOrbitGlow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBombOrbitGlow"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBombOrbitGlow::Clone(void* pArg)
{
	CBombOrbitGlow* pInstance = new CBombOrbitGlow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CBombOrbitGlow"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBombOrbitGlow::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
