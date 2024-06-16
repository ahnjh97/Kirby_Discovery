#include "stdafx.h"
#include "MapObject.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXObject{ pDevice , pContext }
{
}

CMapObject::CMapObject(const CMapObject& rhs)
    : CPhysXObject{ rhs }
{
}

HRESULT CMapObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject::Initialize(void* pArg)
{
    HRESULT hr;
    hr = __super::Initialize(pArg);
    CHECK_FAILED(hr);

    return S_OK;
}

_int CMapObject::Tick(_float fTimeDelta)
{
    __super::Tick(fTimeDelta);


    return OBJ_NOEVENT;
}

void CMapObject::Late_Tick(_float fTimeDelta)
{
    __super::Late_Tick(fTimeDelta);
}

HRESULT CMapObject::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CMapObject::Render_LightDepth()
{
    __super::Render_LightDepth();

    return S_OK;
}

void CMapObject::Render_IMGUI()
{
    __super::Render_IMGUI();

}

CGameObject* CMapObject::Clone(void* pArg)
{
    return nullptr;
}

void CMapObject::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pControllerCom);
}
