#include "stdafx.h"
#include "Fire.h"

CFire::CFire(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFire::CFire(const CFire& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFire::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFire::Initialize(void* pArg)
{
	FIREDESC desc = {};
	if (pArg != nullptr)
		desc = *(FIREDESC*)pArg;

	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;


	m_vOriginPos = desc.vFirePos;
	m_vOriginPos.x += CUtils::Make_RandomFloat(-0.1f, 0.1f);
	m_vOriginPos.z += CUtils::Make_RandomFloat(-0.1f, 0.1f);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vOriginPos);
	m_fMaxRange = m_vOriginPos.y + (desc.fUpRange * CUtils::Make_RandomFloat(0.8f, 1.2f));
	m_fMaxScale = desc.fScale * CUtils::Make_RandomFloat(0.6f, 1.f);
	m_fScale = m_fMaxScale; //*0.1f;
	m_vFirstColor = desc.vFirstColor;
	m_vTargetColor = desc.vTargetColor;
	m_vColor = m_vFirstColor;
	m_pTransformCom->Set_Scaled(m_fMaxScale, m_fMaxScale, m_fMaxScale);

	m_pTransformCom->Turn(XMVectorSet(1.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
	m_pTransformCom->Turn(XMVectorSet(0.f, 0.f, 1.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));


	return S_OK;
}

_int CFire::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	m_fSpeed += m_fTimeDelta * 0.5f;

	_float4 vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vPos.y += m_fSpeed * (60.f * fTimeDelta);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	if (vPos.y > m_vOriginPos.y + (m_fMaxRange * 0.01f))
	{
		m_vColor.x -= (m_vFirstColor.x - m_vTargetColor.x) / 20.f * (150.f * fTimeDelta);
		m_vColor.y -= (m_vFirstColor.y - m_vTargetColor.y) / 20.f * (150.f * fTimeDelta);
		m_vColor.z -= (m_vFirstColor.z - m_vTargetColor.z) / 20.f * (150.f * fTimeDelta);
	}


	if (vPos.y > m_vOriginPos.y + (m_fMaxRange * 0.01f))
	{
		m_fDissolve += 0.21f * (60.f * fTimeDelta);
		m_fScale -= (m_fScale / 6.f) * (60.f * fTimeDelta);
		if (m_fScale <= 0.f)
			m_fScale = 0.00001f;

		m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
	}
	//else
	//{
	//	m_fScale += 0.28f * (60.f * fTimeDelta);
	//	if (m_fScale > m_fMaxScale)
	//	{
	//		m_fScale = m_fMaxScale;
	//	}
	//	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
	//}

	m_pTransformCom->Turn(XMVectorSet(1.f, 1.f, 0.f, 0.f), fTimeDelta, 360.f);
	m_pTransformCom->Turn(XMVectorSet(0.f, 0.f, 1.f, 0.f), fTimeDelta, 270.f);

	if (vPos.y > m_fMaxRange)
	{
		m_bDead = true;
	}

	return OBJ_NOEVENT;
}

void CFire::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CFire::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(MODEL_DISSOLVE)))
			return E_FAIL;

		// 정점 그리기
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CFire::Add_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Model_Cube"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_FX_FireDissolve"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFire::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0);

	_float3 vColor = m_vColor;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &vColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolve", &m_fDissolve, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CFire* CFire::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFire* pInstance = new CFire(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CFire"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFire::Clone(void* pArg)
{
	CFire* pInstance = new CFire(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CFire"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFire::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);

}
