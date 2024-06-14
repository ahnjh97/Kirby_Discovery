#include "stdafx.h"
#include "PhysXObject.h"

#include "FSM.h"
#include "Effect.h"

CPhysXObject::CPhysXObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice , pContext }
{
}

CPhysXObject::CPhysXObject(const CPhysXObject& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CPhysXObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPhysXObject::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CPhysXObject::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	return OBJ_NOEVENT;
}

void CPhysXObject::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CPhysXObject::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CPhysXObject::Render_LightDepth()
{
	__super::Render_LightDepth();

	return S_OK;
}

#ifdef _DEBUG
void CPhysXObject::Render_IMGUI()
{
	__super::Render_IMGUI();

}
#endif

void CPhysXObject::Add_Effect(CEffect* pEffect)
{
	m_FXList.emplace_back(pEffect);
	Safe_AddRef(pEffect);
}

void CPhysXObject::Delete_AllEffect()
{
	
	if (m_FXList.empty())
		return;

	for (auto& FX : m_FXList)
	{
		FX->Set_Dead();
		Safe_Release(FX);
	}

	m_FXList.clear();
	
}

void CPhysXObject::Delete_Effect(string strTag)
{
}

void CPhysXObject::Free()
{
	__super::Free();

	for (auto& fx : m_FXList)
		Safe_Release(fx);

}
