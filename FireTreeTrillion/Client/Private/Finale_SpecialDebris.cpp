#include "Finale_SpecialDebris.h"

CFinale_SpecialDebris::CFinale_SpecialDebris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
}

CFinale_SpecialDebris::CFinale_SpecialDebris(const CFinale_SpecialDebris& rhs)
{
}

HRESULT CFinale_SpecialDebris::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CFinale_SpecialDebris::Initialize(void* pArg)
{
	return E_NOTIMPL;
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
	return E_NOTIMPL;
}

HRESULT CFinale_SpecialDebris::Render_LightDepth()
{
	return E_NOTIMPL;
}

void CFinale_SpecialDebris::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

HRESULT CFinale_SpecialDebris::Add_Components(wstring wstrModelProtoTag)
{
	return E_NOTIMPL;
}

HRESULT CFinale_SpecialDebris::Bind_ShaderResources()
{
	return E_NOTIMPL;
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
}
