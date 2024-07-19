#pragma once
#include "stdafx.h"
#include "Particle.h"


CParticle::CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CEffect{ pDevice, pContext }
{
}

CParticle::CParticle(const CParticle& rhs)
	:CEffect{ rhs }
	, m_FXDesc{ rhs.m_FXDesc }
	, m_InstanceDesc{ rhs.m_InstanceDesc }
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

		//clone 하면서 새로 들어오는 값!
		if (pArg != nullptr)
		{
			FXDesc.vInitPos = (*(FX_DESC*)pArg).vInitPos;
			FXDesc.vInitRot = (*(FX_DESC*)pArg).vInitRot;
			FXDesc.vInitScale = (*(FX_DESC*)pArg).vInitScale;
			FXDesc.pSocketMatrix = (*(FX_DESC*)pArg).pSocketMatrix;
		}
	}

	else if (pArg != nullptr)
	{
		FXDesc = *(PARTICLE_DESC*)pArg;
	}

	HRESULT hr;

	hr = __super::Initialize(&FXDesc);
	CHECK_FAILED(hr);

	m_bIsBillboard = false;



	hr = Add_Components(FXDesc);
	CHECK_FAILED(hr);


	INSTANCE_DESC instanceDesc{};

	//fx 툴 레벨에서는 clone할 때 정보를 새로 기입하는 상황.
	if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
	{
		m_FXDesc = FXDesc;

		//기본 상태 세팅
		instanceDesc.vecMoveCommands.resize(INSTANCE_END);
		for (auto& moveCmd : instanceDesc.vecMoveCommands)
			moveCmd = false;

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
	}
	else
	{
		Update_InstanceInfo(&m_InstanceDesc);
	}

	if (m_InstanceDesc.vecMoveCommands.size() < INSTANCE_END)
		m_InstanceDesc.vecMoveCommands.resize(INSTANCE_END);

	return S_OK;
}

void CParticle::Update_InstanceInfo(INSTANCE_DESC* _instanceDesc)
{
	//이건 파티클에 저장하는 거. 값 있을 때만.
	if (nullptr != _instanceDesc)
		m_InstanceDesc = *_instanceDesc;

	//loop가 두개여
	m_InstanceDesc.fDuration = m_fDuration.second;
	m_InstanceDesc.bIsLoop = m_bIsLoop;

	m_InstanceDesc.vCenter += m_vInitPos;
	m_InstanceDesc.vPivot += m_vInitPos;

	m_InstanceDesc.vInitScale = m_vInitScale;
	m_InstanceDesc.vInitRot = m_vInitRot;

	if (nullptr != m_pVIBufferCom)
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


	pFXData->iPassIdx = m_iPassIdx;
	pFXData->iTexIdx = m_iTexIdx;
	pFXData->iMaskTexIdx = m_iMaskTexIdx;

	pFXData->bIsLoop = m_bIsLoop;
	pFXData->bIsBillboard = m_bIsBillboard;
	pFXData->bIsBloom = m_bIsBloom;


	pFXData->iNumInstance = m_FXDesc.iNumInstance;

	pFXData->fDuration = m_fDuration.second;
	pFXData->fLifetime = m_InstanceDesc.fLifetime;
	pFXData->fLifetimeRandomOffset = m_InstanceDesc.fLifetimeRandomOffset;
	pFXData->fStartDelay = m_InstanceDesc.fStartDelay;
	pFXData->fStarDelayRandomOffset = m_InstanceDesc.fStarDelayRandomOffset;

	pFXData->eRenderGroup = m_eRenderGroup;
	pFXData->eTimer = m_eTimer;

	pFXData->vCenter = m_InstanceDesc.vCenter;

	pFXData->vRange = m_InstanceDesc.vRange;

	pFXData->fMinRange = m_InstanceDesc.fMinRange;
	pFXData->fMaxRange = m_InstanceDesc.fMaxRange;

	pFXData->vRotation = m_InstanceDesc.vRotation;
	pFXData->vRotationRandomOffset = m_InstanceDesc.vRotationRandomOffset;
	pFXData->vScale = m_InstanceDesc.vScale;
	pFXData->vScaleRandomOffset = m_InstanceDesc.vScaleRandomOffset;


	pFXData->vDir = m_InstanceDesc.vDir;
	pFXData->vDirRandomOffset = m_InstanceDesc.vDirRandomOffset;
	pFXData->fSpeed = m_InstanceDesc.fSpeed;
	pFXData->fSpeedRandomOffset = m_InstanceDesc.fSpeedRandomOffset;

	//추가
	pFXData->fOrbitSpeed = m_InstanceDesc.fOrbitSpeed;
	pFXData->fOrbitSpeedRandomOffset = m_InstanceDesc.fOrbitSpeedRandomOffset;

	pFXData->fAccSupplyAmount = m_InstanceDesc.fAccSupplyAmount;
	pFXData->fTurnSupplyAmount = m_InstanceDesc.fTurnSupplyAmount;

	pFXData->vColor = m_InstanceDesc.vColor;
	pFXData->vColorRandomOffset = m_InstanceDesc.vColorRandomOffset;

	pFXData->vTargetColor = m_InstanceDesc.vTargetColor;
	pFXData->vTargetColorRandomOffset = m_InstanceDesc.vTargetColorRandomOffset;

	pFXData->fAlpha = m_InstanceDesc.fAlpha;
	pFXData->fAlphaRandomOffset = m_InstanceDesc.fAlphaRandomOffset;

	pFXData->vPivot = m_InstanceDesc.vPivot;

	pFXData->vRotationAxis = m_InstanceDesc.vRotationAxis;


	pFXData->iMoveCommandsNum = m_InstanceDesc.vecMoveCommands.size();
	pFXData->vecMoveCommands = m_InstanceDesc.vecMoveCommands;

}

_int CParticle::Tick(_float _fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;



	return OBJ_NOEVENT;
}

void CParticle::Late_Tick(_float _fTimeDelta)
{

	//현재 설정 값으로 적용할 타임델타 값을 바꾼다.
	_float fMyTimeDelta = _fTimeDelta;
	switch (m_eTimer)
	{
	case TIMER_FIRST:
		fMyTimeDelta = m_pGameInstance->Get_FirstTimer();
		break;
	case TIMER_SECOND:
		fMyTimeDelta = m_pGameInstance->Get_SecondTimer();
		break;
	default:
		break;
	}
	 
	//duration 다 끝났다면
 	if (Calculate_Duration(fMyTimeDelta))
	{
		//툴에서는 다시 시작하기
		if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
		{
			m_fDuration.first = 0.f;
			m_pVIBufferCom->Revive();
		}
		else
		{
			m_bDead = true;
		}
	}

	m_pVIBufferCom->Compute_AllLifeTime(fMyTimeDelta);

	if ( m_fDuration.second <= m_fDuration.first )
	{
		m_fDuration.first = m_fDuration.second;

		if (m_fDuration.second == FX_MAXDURATION)
		{
			m_fDuration.first = 0.f;
			m_fLifetime.first = 0.f;
		}
		return;
	}


	if (m_pSoketMatrix != nullptr)
		m_pTransformCom->Set_WorldMatrix(*m_pSoketMatrix);

	//m_pTransformCom->Set_Scaled(m_vInitScale);

	Compute_ViewZ();

	VTXMATRIX* pVertices = m_pVIBufferCom->Map();



	if (m_InstanceDesc.vecMoveCommands[INSTANCE_SIMPLEMOVE])
		m_pVIBufferCom->SimpleMove(fMyTimeDelta, pVertices);


	if (m_InstanceDesc.vecMoveCommands[INSTANCE_SIMPLEMOVE])
		m_pVIBufferCom->SimpleMove(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_DROP])
		m_pVIBufferCom->Drop(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_SPREAD])
		m_pVIBufferCom->Spread(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_APPEAR])
		m_pVIBufferCom->Appear(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_DISAPPEAR])
		m_pVIBufferCom->Disappear(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_DECELERATE])
		m_pVIBufferCom->Decelerate(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_WIGGLE])
		m_pVIBufferCom->Wiggle(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_TAIL])
		m_pVIBufferCom->Tail(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_ASSEMBLE])
		m_pVIBufferCom->Assemble(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_ORBIT])
		m_pVIBufferCom->Orbit(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_ACCELERATION])
		m_pVIBufferCom->Acceleration(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_DECELERATE])
		m_pVIBufferCom->Decelerate(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_ORBITACCELERATION])
		m_pVIBufferCom->OrbitAcceleration(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_ORBITDECELERATE])
		m_pVIBufferCom->OrbitDecelerate(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_GRAVITY])
		m_pVIBufferCom->Gravity(fMyTimeDelta, pVertices);

	if (m_InstanceDesc.vecMoveCommands[INSTANCE_COLORINTERPOLATE])
		m_pVIBufferCom->Color_Interpolate(fMyTimeDelta, pVertices);


	m_pVIBufferCom->Save_PrePos(pVertices);


	//m_pVIBufferCom->Apply_Velocity(fMyTimeDelta, pVertices);

	m_pVIBufferCom->Unmap();



	if ((CRenderer::RENDERGROUP)m_eRenderGroup != CRenderer::RENDER_END)
		m_pGameInstance->Add_RenderGroup((CRenderer::RENDERGROUP)m_eRenderGroup, this);

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


	m_iMaxTexIdx = m_pTextureCom[TEX_DIFFUSE]->Get_TextureNum() - 1;

	m_iMaxMaskTexIdx = m_pTextureCom[TEX_MASK]->Get_TextureNum() - 1;

	if (_FXDesc.strBufferTag == "Prototype_Component_VIBuffer_Instance_Point")
	{
		CVIBuffer_Instance_Point::INSTANCE_POINT_DESC InstanceDesc{};
		InstanceDesc.iNumInstance = _FXDesc.iNumInstance;


		hr = __super::Add_Component(LEVEL_STATIC, CUtils::StrToWstr(_FXDesc.strBufferTag),
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom, &InstanceDesc);
		CHECK_FAILED(hr);


		hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
		CHECK_FAILED(hr);

		//현재 VtxInstance Shader Pass 2까지
		m_iMaxPassIdx = INSTANCEPOINT_END - 1;
	}
	else
	{
		hr = __super::Add_Component(*m_pCurrentLevelID, CUtils::StrToWstr(_FXDesc.strBufferTag),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);

		CHECK_FAILED(hr);


		hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_FXModel"),
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

	_float4 vLook = m_pGameInstance->Get_CamLook();
	hr = m_pShaderCom->Bind_RawValue("g_vCamLook", &vLook, sizeof(_float4));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_vRColor", &m_InstanceDesc.vColor, sizeof(_float3));
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Bind_RawValue("g_fAlpha", &m_InstanceDesc.fAlpha, sizeof(_float));
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iTexIdx);
	CHECK_FAILED(hr);

	hr = m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", m_iMaskTexIdx);
	CHECK_FAILED(hr);

	hr = m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_Depth"), "g_DepthTexture");
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
