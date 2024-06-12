#pragma once
#include "stdafx.h"
#include "Particle.h"


CParticle::CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEffect{ pDevice, pContext }
{
}

CParticle::CParticle(const CParticle& rhs)
	:CEffect{ rhs }
{
}


HRESULT CParticle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CParticle::Initialize_Prototype(PARTICLE_DESC FXDesc)
{

	m_strFXName = FXDesc.strFXName;
	m_FXDesc = FXDesc;

	return S_OK;
}

HRESULT CParticle::Initialize(void* pArg)
{
	PARTICLE_DESC FXDesc{};

	//prototype 만드는 단계에서 이미 정보를 저장한 상황.
	if (m_FXDesc.strFXName != "NONE")
	{
		FXDesc = m_FXDesc;
	}

	else if (pArg != nullptr)
	{
		FXDesc = *(PARTICLE_DESC*)pArg;
	}

	HRESULT hr;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);

	hr = Add_Components(FXDesc);
	CHECK_FAILED(hr);


	//fx 툴 레벨에서는 clone할 때 정보를 새로 기입하는 상황.
	if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
		m_FXDesc = FXDesc;

	//m_fDuration.second = 10.f;

	//기본 상태 세팅
	INSTANCE_DESC instanceDesc{};

	instanceDesc.vecMoveCommands.resize(INSTANCE_END);
	instanceDesc.vecMoveCommands[INSTANCE_DROP] = true;
	instanceDesc.vecMoveCommands[INSTANCE_SPREAD] = true;
	instanceDesc.vecMoveCommands[INSTANCE_DECELERATE] = true;

	instanceDesc.vPivot = { 0.f, -1.f, 0.f };
	instanceDesc.vRange = { 2.f, 2.f, 2.f };
	instanceDesc.fSpeed = { 3.f };
	instanceDesc.fSpeedRandomOffset = { 1.f };

	instanceDesc.fStartDelay = { .1f };
	instanceDesc.fStarDelayRandomOffset = { .2f };

	instanceDesc.fLifetime = { 1.4f };
	instanceDesc.fLifetimeRandomOffset = { .3f };
	instanceDesc.bIsLoop = true;
	Update_InstanceInfo(&instanceDesc);

	return S_OK;
}

void CParticle::Update_InstanceInfo(INSTANCE_DESC* _instanceDesc)
{
	//이건 파티클에 저장하는 거. 값 있을 때만.
	if (nullptr != _instanceDesc)
		m_InstanceDesc = *_instanceDesc;

	m_pVIBufferCom->Update_InstanceDesc(m_InstanceDesc);
}

void CParticle::Fill_SaveData(PARTICLE_DATA* pFXData)
{
	pFXData->iNameStrLen = (_uint)m_strFXName.size();
	pFXData->strName = m_strFXName;

	pFXData->iBufferStrLen = (_uint)m_strBufferTag.size();
	pFXData->strBufferName = CUtils::WstrToStr(m_strBufferTag);

	pFXData->iTexStrLen = (_uint)m_strTexTag.size();
	pFXData->strTexName = CUtils::WstrToStr(m_strTexTag);

	pFXData->iMaskTexStrLen = (_uint)m_strMaskTexTag.size();
	pFXData->strMaskTexName = CUtils::WstrToStr(m_strMaskTexTag);

	pFXData->iNumInstance = m_FXDesc.iNumInstance;

	pFXData->fDuration = m_fDuration.second;
	pFXData->fLifetime = m_InstanceDesc.fLifetime;
	pFXData->fLifetimeRandomOffset = m_InstanceDesc.fLifetimeRandomOffset;
	pFXData->fStartDelay = m_InstanceDesc.fStartDelay;
	pFXData->fStarDelayRandomOffset = m_InstanceDesc.fStarDelayRandomOffset;
	pFXData->vCenter = m_InstanceDesc.vCenter;
	pFXData->vRange = m_InstanceDesc.vRange;
	pFXData->vRotation = m_InstanceDesc.vRotation;
	pFXData->vRotationRandomOffset = m_InstanceDesc.vRotationRandomOffset;
	pFXData->vScale = m_InstanceDesc.vScale;
	pFXData->vScaleRandomOffset = m_InstanceDesc.vScaleRandomOffset;
	pFXData->vDir = m_InstanceDesc.vDir;
	pFXData->vDirRandomOffset = m_InstanceDesc.vDirRandomOffset;
	pFXData->fSpeed = m_InstanceDesc.fSpeed;
	pFXData->fSpeedRandomOffset = m_InstanceDesc.fSpeedRandomOffset;
	pFXData->vColor = m_InstanceDesc.vColor;
	pFXData->vColorRandomOffset = m_InstanceDesc.vColorRandomOffset;
	pFXData->fAlpha = m_InstanceDesc.fAlpha;
	pFXData->fAlphaRandomOffset = m_InstanceDesc.fAlphaRandomOffset;

	pFXData->vPivot = m_InstanceDesc.vPivot;
	pFXData->bIsLoop = m_InstanceDesc.bIsLoop;
	pFXData->bIsBillboard = m_InstanceDesc.bIsBillboard;
	pFXData->bIsColorRender = m_InstanceDesc.bIsColorRender;
	pFXData->bIsBloom = m_InstanceDesc.bIsBloom;
	pFXData->iMoveCommandsNum = m_InstanceDesc.vecMoveCommands.size();
	pFXData->vecMoveCommands = m_InstanceDesc.vecMoveCommands;

	pFXData->eRenderGroup = m_eRenderGroup;
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
			m_pVIBufferCom->Revive();
		}
		else
			m_bDead = true;
	}

	m_pVIBufferCom->Compute_AllLifeTime(_fTimeDelta);

	if (m_fDuration.second <= m_fDuration.first)
		return OBJ_NOEVENT;



	if (m_InstanceDesc.vecMoveCommands[INSTANCE_DROP])
		m_pVIBufferCom->Drop(_fTimeDelta);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_SPREAD])
		m_pVIBufferCom->Spread(_fTimeDelta);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_DECELERATE])
		m_pVIBufferCom->Decelerate(_fTimeDelta);


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

	if (FAILED(m_pShaderCom->Begin(m_iPassIdx)))
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

		//현재 VtxInstance Shader Pass 2까지
		m_iMaxPassIdx = 1;
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

	hr = m_pShaderCom->Bind_RawValue("g_vRColor", &m_InstanceDesc.vColor, sizeof(_float3));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_InstanceDesc.fAlpha, sizeof(_float));
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
