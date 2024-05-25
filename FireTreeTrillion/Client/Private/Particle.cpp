#pragma once
#include "stdafx.h"
#include "Particle.h"


CParticle::CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEffect{pDevice, pContext}
{
}

CParticle::CParticle(const CParticle& rhs)
	:CEffect{rhs}
{
}

HRESULT CParticle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParticle::Initialize_Prototype(PARTICLE_DESC FXDesc)
{
	return S_OK;
}

HRESULT CParticle::Initialize(void* pArg)
{
	PARTICLE_DESC FXDesc{};

	if (pArg != nullptr)
	{
		FXDesc = *(PARTICLE_DESC*)pArg;
	}

	HRESULT hr;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);

	hr = Add_Components(FXDesc);
	CHECK_FAILED(hr);

	return S_OK;
}

void CParticle::Update_InstanceInfo(INSTANCE_DESC& _instanceDesc)
{
	m_InstanceDesc = _instanceDesc;
	m_pVIBufferCom->Update_InstanceInfo(_instanceDesc);
}

_int CParticle::Tick(_float _fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	if (Calculate_Duration(_fTimeDelta))
	{
		//툴에서는 다시 시작하기
		if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
		{
			m_fDuration.first = 0.f;
		}
		else
			m_bDead = true;
	}

	if (Calculate_Lifetime(_fTimeDelta))
	{

	}

	return OBJ_NOEVENT;
}

void CParticle::Late_Tick(_float fTimeDelta)
{
	if (m_bIsColorRender)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	if (m_bIsBloom)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

HRESULT CParticle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CParticle::Add_Components(PARTICLE_DESC& _FXDesc)
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(_FXDesc.strTexTag),
		TEXT("Com_DiffuseTexture"), (CComponent**)&m_pTextureCom[TEX_DIFFUSE]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(_FXDesc.strMaskTexTag),
		TEXT("Com_MaskTexture"), (CComponent**)&m_pTextureCom[TEX_MASK]);
	CHECK_FAILED(hr);



	if (_FXDesc.strBufferTag == "Prototype_Component_VIBuffer_Instance_Point")
	{
		CVIBuffer_Instance_Point::INSTANCE_POINT_DESC InstanceDesc{};
		InstanceDesc.iNumInstance = _FXDesc.iNumInstance;
		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(_FXDesc.strBufferTag), 
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom, &InstanceDesc);
		CHECK_FAILED(hr);


		hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

		CHECK_FAILED(hr);
	}
	else
	{
		hr = __super::Add_Component(*m_pCurrentLevelID, CUtils::StrToWstr(_FXDesc.strBufferTag),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);

		CHECK_FAILED(hr);


		hr = __super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Shader_FXModel"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

		CHECK_FAILED(hr);
	}


	return S_OK;
}

HRESULT CParticle::Bind_ShaderResources(_int iTexIdx, _int iMaskTexIdx)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;


	HRESULT hr;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;


	hr = m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4));
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iTexIdx);
	CHECK_FAILED(hr);
	
	hr = m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", m_iMaskTexIdx);
	CHECK_FAILED(hr);

	return S_OK;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParticle* pInstance = new CParticle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CParticle"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PARTICLE_DESC FXDesc)
{
	CParticle* pInstance = new CParticle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(FXDesc)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CParticle"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CParticle::Clone(void* pArg)
{
	CParticle* pInstance = new CParticle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CParticle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParticle::Free()
{
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pModelCom);

	for (auto& texture : m_pTextureCom)
		Safe_Release(texture);

	Safe_Release(m_pShaderCom);

	__super::Free();
}
