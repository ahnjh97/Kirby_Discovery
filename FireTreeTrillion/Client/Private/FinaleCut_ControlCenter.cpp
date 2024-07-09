#include "stdafx.h"
#include "FinaleCut_ControlCenter.h"

CFinaleCut_ControlCenter::CFinaleCut_ControlCenter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CFinaleCut_ControlCenter::CFinaleCut_ControlCenter(const CFinaleCut_ControlCenter& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CFinaleCut_ControlCenter::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFinaleCut_ControlCenter::Initialize(void* pArg)
{
    GAMEOBJECT_DESC* Desc = nullptr;

    if (pArg != nullptr)
        Desc = (GAMEOBJECT_DESC*)pArg;

    if (FAILED(__super::Initialize(Desc)))
        return E_FAIL;

    return S_OK;

}

_int CFinaleCut_ControlCenter::Tick(_float fTimeDelta)
{
    if (m_bDead == true)
        return OBJ_DEAD;
    return OBJ_NOEVENT;
}

void CFinaleCut_ControlCenter::Late_Tick(_float fTimeDelta)
{
}

HRESULT CFinaleCut_ControlCenter::Render()
{
    return S_OK;
}

CFinaleCut_ControlCenter* CFinaleCut_ControlCenter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CFinaleCut_ControlCenter* pInstance = new CFinaleCut_ControlCenter(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CFinaleCut_ControlCenter"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CFinaleCut_ControlCenter::Clone(void* pArg)
{
    CFinaleCut_ControlCenter* pInstance = new CFinaleCut_ControlCenter(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CFinaleCut_ControlCenter"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CFinaleCut_ControlCenter::Free()
{
    __super::Free();
}
