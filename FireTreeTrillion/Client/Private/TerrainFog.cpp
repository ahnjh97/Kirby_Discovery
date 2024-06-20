#include "stdafx.h"
#include "TerrainFog.h"

CTerrainFog::CTerrainFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTerrainFog::CTerrainFog(const CTerrainFog& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CTerrainFog::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrainFog::Initialize(void* pArg)
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

_int CTerrainFog::Tick(_float fTimeDelta)
{
	return _int();
}

void CTerrainFog::Late_Tick(_float fTimeDelta)
{
}

HRESULT CTerrainFog::Render()
{
	return E_NOTIMPL;
}

HRESULT CTerrainFog::Add_Components()
{
	return E_NOTIMPL;
}

HRESULT CTerrainFog::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

void CTerrainFog::Effect_Billboard(_float fTimeDelta)
{
}

CTerrainFog* CTerrainFog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CTerrainFog::Clone(void* pArg)
{
	return nullptr;
}

void CTerrainFog::Free()
{
}
