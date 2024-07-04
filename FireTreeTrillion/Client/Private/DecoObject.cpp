#include "stdafx.h"
#include "DecoObject.h"

CDecoObject::CDecoObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{pDevice, pContext}
{
}

CDecoObject::CDecoObject(const CDecoObject& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CDecoObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDecoObject::Initialize(void* pArg)
{
	return S_OK;
}

_int CDecoObject::Tick(_float fTimeDelta)
{
	return _int();
}

void CDecoObject::Late_Tick(_float fTimeDelta)
{
}

HRESULT CDecoObject::Render()
{
	return S_OK;
}

HRESULT CDecoObject::Render_LightDepth()
{
	return S_OK;
}

void CDecoObject::Render_IMGUI()
{
}

void CDecoObject::Set_Animation(_int iAnimIndex)
{
}

HRESULT CDecoObject::Add_Components()
{
	return S_OK;
}

HRESULT CDecoObject::Bind_ShaderResources()
{
	return S_OK;
}

CDecoObject* CDecoObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CDecoObject::Clone(void* pArg)
{
	return nullptr;
}

void CDecoObject::Free()
{
}
