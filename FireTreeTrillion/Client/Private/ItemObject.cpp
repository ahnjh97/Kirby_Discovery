#include "stdafx.h"
#include "ItemObject.h"
#include "Kirby.h"
#include "Gm_DynamicField.h"

CItemObject::CItemObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXObject{ pDevice , pContext }
{
}

CItemObject::CItemObject(const CItemObject& rhs)
    : CPhysXObject{ rhs }
{
}

void CItemObject::RegisterToDynamicField()
{
    CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
    if (nullptr == pKirby)
        return;

    _float fDis = m_pTransformCom->RayCast(CTransform::DYNAMIC, _float3(0, -1, 0));
    if (6.f > fDis)
    {
        CGameObject* pObj = pKirby->FindDynamicField(m_pTransformCom->Get_MostRecentActor());
        if (nullptr == pObj)
            return;

        CGm_DynamicField* pDynamicField = dynamic_cast<CGm_DynamicField*>(pObj);
        if (nullptr == pDynamicField)
            return;

        pDynamicField->EmplaceBackItem(this);
    }
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

    m_bUpdate_FXSocketMatrix = false;

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
