#include "stdafx.h"
#include "ItemObject.h"

CItemObject::CItemObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXObject{ pDevice , pContext }
{
}

CItemObject::CItemObject(const CItemObject& rhs)
    : CPhysXObject{ rhs }
{
}

HRESULT CItemObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CItemObject::Initialize(void* pArg)
{
    HRESULT hr;
    hr = __super::Initialize(pArg);
    CHECK_FAILED(hr);

    return S_OK;
}

_int CItemObject::Tick(_float fTimeDelta)
{
    __super::Tick(fTimeDelta);


    return OBJ_NOEVENT;
}

void CItemObject::Late_Tick(_float fTimeDelta)
{
    __super::Late_Tick(fTimeDelta);
}

HRESULT CItemObject::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CItemObject::Render_LightDepth()
{
    __super::Render_LightDepth();

    return S_OK;
}

#ifdef _DEBUG

void CItemObject::Render_IMGUI()
{
    __super::Render_IMGUI();

}

#endif

CGameObject* CItemObject::Clone(void* pArg)
{
    return nullptr;
}

void CItemObject::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pControllerCom);


}
