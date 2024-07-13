#include "stdafx.h"
#include "Finale_SpecialDebris.h"

CFinale_SpecialDebris::CFinale_SpecialDebris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }

{
}

CFinale_SpecialDebris::CFinale_SpecialDebris(const CFinale_SpecialDebris& rhs)
	: CPhysXObject{rhs}
{
}

HRESULT CFinale_SpecialDebris::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinale_SpecialDebris::Initialize(void* pArg)
{
	return S_OK;
}

_int CFinale_SpecialDebris::Tick(_float fTimeDelta)
{
	return _int();
}

void CFinale_SpecialDebris::Late_Tick(_float fTimeDelta)
{
}

HRESULT CFinale_SpecialDebris::Render()
{
	return S_OK;
}

HRESULT CFinale_SpecialDebris::Render_LightDepth()
{
	return S_OK;
}

void CFinale_SpecialDebris::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CFinale_SpecialDebris::Add_Components(wstring wstrModelProtoTag)
{
	return S_OK;
}

HRESULT CFinale_SpecialDebris::Bind_ShaderResources()
{
	return S_OK;
}

_int CFinale_SpecialDebris::Make_Partical()
{
	return _int();
}

void CFinale_SpecialDebris::Compute_MotionBlur()
{
}

CFinale_SpecialDebris* CFinale_SpecialDebris::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CFinale_SpecialDebris::Clone(void* pArg)
{
	return nullptr;
}

void CFinale_SpecialDebris::Free()
{
	__super::Free();
}
