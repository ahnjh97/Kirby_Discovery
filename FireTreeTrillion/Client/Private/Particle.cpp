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
	return S_OK;
}

_int CParticle::Tick(_float fTimeDelta)
{
	return _int();
}

void CParticle::Late_Tick(_float fTimeDelta)
{
}

HRESULT CParticle::Render()
{
	return S_OK;
}

HRESULT CParticle::Add_Components(FX_DESC& FXDesc)
{
	return S_OK;
}

HRESULT CParticle::Bind_ShaderResources(_int iTexIdx, _int iMaskTexIdx)
{
	return S_OK;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CParticle* CParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, PARTICLE_DESC FXDesc)
{
	return nullptr;
}

CGameObject* CParticle::Clone(void* pArg)
{
	return nullptr;
}

void CParticle::Free()
{
}
