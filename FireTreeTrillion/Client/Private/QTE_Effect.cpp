#include "stdafx.h"
#include "QTE_Effect.h"
#include "FinaleKirby.h"

CQTE_Effect::CQTE_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CQTE_Effect::CQTE_Effect(const CQTE_Effect& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CQTE_Effect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CQTE_Effect::Initialize(void* pArg)
{
    QTEEFFECTDESC		Desc{};

    if (pArg != nullptr)
        Desc = *(QTEEFFECTDESC*)pArg;

    Desc.fSpeedPerSec = 10.f;
    Desc.fRotationPerSec = XMConvertToRadians(90.0f);

    m_vColor = Desc.vColor;
    m_vOffSet = Desc.vOffSet;


    if (FAILED(__super::Initialize(&Desc)))
        return E_FAIL;
    if (FAILED(Add_Components()))
        return E_FAIL;

    m_fSizeX = g_iWinSizeX;
    m_fSizeY = g_iWinSizeY;
    m_fX = g_iWinSizeX * 0.5f;
    m_fY = g_iWinSizeY * 0.5f;
    XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fSizeX, m_fSizeY, 0.f, 1.f));

    CGameObject* pFinaleKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
    _float4 vKirbyPos = pFinaleKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
    _float4 vUIPos = vKirbyPos + m_vOffSet;
    _matrix VPMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _vector vViewportPos = XMVector3TransformCoord(vUIPos, VPMatrix);
    _float fViewX = XMVectorGetX(vViewportPos);
    _float fViewY = XMVectorGetY(vViewportPos);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(fViewX * m_fX, fViewY * m_fY, 1.f, 1.f));

    _float fScale = m_fSizeY * 0.18f * m_fScale;
    m_pTransformCom->Set_Scaled(fScale, fScale, 1.f);

    return S_OK;
}

_int CQTE_Effect::Tick(_float fTimeDelta)
{
    if (m_bDead == true)
        return OBJ_DEAD;

    QTE_Animation(fTimeDelta);
    Update_Pos(fTimeDelta);

    return OBJ_NOEVENT;
}

void CQTE_Effect::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CQTE_Effect::Render()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
        return E_FAIL;
    if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &m_vColor, sizeof(_float3))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Begin(POSTEX_QTEEFFECT)))
        return E_FAIL;
    if (FAILED(m_pVIBufferCom->Bind_Buffers()))
        return E_FAIL;
    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;
    return S_OK;
}

HRESULT CQTE_Effect::Add_Components()
{
    // UI 셰이더 전용
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;
    // Rect를 써야 한다.
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
        TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
        return E_FAIL;
    if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_QTE_Effect"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
        return E_FAIL;

    return S_OK;
}

void CQTE_Effect::QTE_Animation(_float fTimeDelta)
{
    m_fScale += fTimeDelta * 3.6f;
    m_fAlpha -= fTimeDelta * 4.f;

    if (m_fAlpha < 0.f)
    {
        m_fAlpha = 0.f;
        m_bDead = true;
    }

    _float fScale = m_fSizeY * 0.18f * m_fScale;
    m_pTransformCom->Set_Scaled(fScale, fScale, fScale);

}

void CQTE_Effect::Update_Pos(_float fTimeDelta)
{
    CGameObject* pFinaleKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
    if (pFinaleKirby == nullptr)
        return;

    _float4 vKirbyPos = static_cast<CFinaleKirby*>(pFinaleKirby)->m_vBonePos;
    _float4 vUIPos = vKirbyPos + m_vOffSet;
    _matrix VPMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
    _vector vViewportPos = XMVector3TransformCoord(vUIPos, VPMatrix);
    _float fViewX = XMVectorGetX(vViewportPos);
    _float fViewY = XMVectorGetY(vViewportPos);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(fViewX * m_fX, fViewY * m_fY, 1.f, 1.f));
}

CQTE_Effect* CQTE_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CQTE_Effect* pInstance = new CQTE_Effect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Created : CQTE_Effect"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CQTE_Effect::Clone(void* pArg)
{
    CQTE_Effect* pInstance = new CQTE_Effect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CQTE_Effect"));
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CQTE_Effect::Free()
{
    __super::Free();
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pTextureCom);

}
