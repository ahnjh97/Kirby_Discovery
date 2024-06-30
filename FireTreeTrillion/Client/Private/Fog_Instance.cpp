#include "stdafx.h"
#include "Fog_Instance.h"
#include "Camera_Main.h"

CFog_Instance::CFog_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFog_Instance::CFog_Instance(const CFog_Instance& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFog_Instance::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFog_Instance::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;
	}

	if (FAILED(__super::Initialize(pGameObjectDesc)))
		return E_FAIL;

	INSTANCE_DESC tInstanceDesc{};
	tInstanceDesc.vCenter = _float3(0, 0, 0);
	tInstanceDesc.vPivot = _float3(0, 0, 0);
	tInstanceDesc.iNumInstance = 50.f;
	tInstanceDesc.eInstanceShape = INSTANCE_SHAPE_SPHERE;
	tInstanceDesc.vRange = _float3(3.f, 3.f, 3.f);
	tInstanceDesc.bIsLoop = true;
	tInstanceDesc.fLifetime = FLT_MAX;
	tInstanceDesc.vScale = _float3(30.f, 30.f, 1.f);
	tInstanceDesc.bRandPos = true;

	if (FAILED(Add_Components(tInstanceDesc)))
		return E_FAIL;

	m_fAlpha = 0.5f;
	m_pTransformCom->Set_Scaled(30.f * pGameObjectDesc->matWorld._11, 30.f * pGameObjectDesc->matWorld._22, 1.f * pGameObjectDesc->matWorld._33);

	m_iRandomFog = 2;

	return S_OK;
}

_int CFog_Instance::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CFog_Instance::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CFog_Instance::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;
	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

	if (FAILED(m_pShaderCom->Begin(INSTANCEPOINT_BLENDFX)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CFog_Instance::Add_Components(INSTANCE_DESC& _tInstanceDesc)
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Instance_Point"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom, &_tInstanceDesc)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_Terrain_Fog"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFog_Instance::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	HRESULT hr = m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4));
	CHECK_FAILED(hr);

	_float4 vLook = m_pGameInstance->Get_CamLook();
	hr = m_pShaderCom->Bind_RawValue("g_vCamLook", &vLook, sizeof(_float4));
	CHECK_FAILED(hr);

	if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_Depth"), "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iRandomFog)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
		return E_FAIL;
	hr = m_pShaderCom->Bind_RawValue("g_vRColor", &m_vColor, sizeof(_float3));
	CHECK_FAILED(hr);

	return S_OK;
}

CFog_Instance* CFog_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFog_Instance* pInstance = new CFog_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFog_Instance"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFog_Instance::Clone(void* pArg)
{
	CFog_Instance* pInstance = new CFog_Instance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CFog_Instance"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFog_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
