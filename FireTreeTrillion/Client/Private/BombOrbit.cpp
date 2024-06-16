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

void CBombOrbit::Update_OrbitPosition(_float4 vPos, _float4 vNormal)
{
	_float4 vNewLook = -1.f * vNormal;

	_vector vWorldUpVec =
		(vNewLook == _float4(0.f, 1.f, 0.f, 0.f)) || (vNewLook == _float4(0.f, -1.f, 0.f, 0.f)) ?
		_float4(0.0001f, 1.f, 0.f, 0.f) : _float4(0.f, 1.f, 0.f, 0.f);

	vWorldUpVec = XMVector3Normalize(vWorldUpVec);

	_vector vNewRight = XMVector3Normalize(XMVector3Cross(vNewLook, vWorldUpVec));
	_vector vNewUp = XMVector3Normalize(XMVector3Cross(vNewLook, vNewRight));

	m_pTransformCom->Set_State(CTransform::STATE_LOOK, vNewLook);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, vNewRight);
	m_pTransformCom->Set_State(CTransform::STATE_UP, vNewUp);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos - (vNewLook * 0.3f) );

	m_pTransformCom->Set_Scaled(2.f, 2.f, 1.f);
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
	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_BombOrbit"),
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

	_float3 vColor = { 1.f, 0.f, 0.f };
	_float fAlpha = { 1.f };

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &vColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &fAlpha, sizeof(_float))))
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
