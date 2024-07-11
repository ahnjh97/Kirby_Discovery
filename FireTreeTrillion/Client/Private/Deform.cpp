#include "stdafx.h"
#include "Deform.h"

CDeform::CDeform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXObject{ pDevice, pContext }
{
}

CDeform::CDeform(const CDeform& rhs)
    : CPhysXObject{ rhs }
{
}

HRESULT CDeform::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CDeform::Initialize(void* pArg)
{
    HRESULT hr;
    hr = __super::Initialize(pArg);

    CHECK_FAILED(hr);

    return S_OK;
}

_int CDeform::Tick(_float fTimeDelta)
{
    __super::Tick(fTimeDelta);

    return OBJ_NOEVENT;
}

void CDeform::Late_Tick(_float fTimeDelta)
{
    __super::Late_Tick(fTimeDelta);
}

HRESULT CDeform::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CDeform::Render_LightDepth()
{
    return S_OK;
}

#ifdef _DEBUG
void CDeform::Render_IMGUI()
{
    __super::Render_IMGUI();

}
#endif


void CDeform::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CDeform::Compute_MotionBlur()
{
    _vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
    _matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
    _float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
    _float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

    _float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

    m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
    m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
    m_vMotionVelocity.z = m_ePhyXState != PO_NORMAL ? 1.f : 0.f;

    m_vPreScreenPos = vCurScreenPos;
}

void CDeform::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pControllerCom);
}
