#include "stdafx.h"
#include "RigidObject.h"

CRigidObject::CRigidObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice , pContext }
{
}

CRigidObject::CRigidObject(const CRigidObject& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CRigidObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRigidObject::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CRigidObject::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CRigidObject::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CRigidObject::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CRigidObject::Render_LightDepth()
{
	__super::Render();

	return S_OK;
}

#ifdef _DEBUG
void CRigidObject::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif

void CRigidObject::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
}
