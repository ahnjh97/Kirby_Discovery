#include "stdafx.h"
#include "FinaleBuildingPartical.h"

CFinaleBuildingPartical::CFinaleBuildingPartical(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFinaleBuildingPartical::CFinaleBuildingPartical(const CFinaleBuildingPartical& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinaleBuildingPartical::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleBuildingPartical::Initialize(void* pArg)
{
    GAMEOBJECT_DESC* Desc = nullptr;

    if (pArg != nullptr)
        Desc = (GAMEOBJECT_DESC*)pArg;

    if (FAILED(__super::Initialize(Desc)))
        return E_FAIL;

    if (FAILED(Add_Components()))
        return E_FAIL;

    m_bMotionBlur = false;
    m_bRimLight = true;
    m_bStencil = true;

    return S_OK;
}

_int CFinaleBuildingPartical::Tick(_float fTimeDelta)
{
    if (m_bDead == true)
        return OBJ_DEAD;

    if (m_bActive == false)
        return OBJ_NOEVENT;

    m_fTimeDelta = m_pGameInstance->Get_SecondTimer();


    _float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
    _float4 vDelta = (m_vDir * m_fTimeDelta * m_fSpeed);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDelta);
    m_pTransformCom->Turn(m_fTurnAxis, m_fTimeDelta, m_fTurn);

    m_fActiveTime += m_fTimeDelta;

    if (m_fActiveTime > 60.f)
        m_bActive = false;

    return OBJ_NOEVENT;
}

void CFinaleBuildingPartical::Late_Tick(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 50.0f))
    {
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
    }

}

HRESULT CFinaleBuildingPartical::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom[m_eType]->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom[m_eType]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom[m_eType]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pModelCom[m_eType]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
            return E_FAIL;
        _float fWhiteColorDiffuse = 0.f;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
            return E_FAIL;

        /* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
        if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
            return E_FAIL;

        m_pModelCom[m_eType]->Render(i);
    }

    return S_OK;
}

void CFinaleBuildingPartical::Set_BuildingPartical(_float4 vPos)
{
    m_eType = (BUILDINGTYPE)CUtils::Make_RandomInt(0, 3);
    m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

    m_vDir = CUtils::Make_RandomAngle_Vector(20.f, _float4(0.f, 1.f, 0.f, 0.f));
    m_vDir.w = 0.f;
    m_fSpeed = CUtils::Make_RandomFloat(10.f, 20.f);
    m_bActive = true;

    m_fTurn = CUtils::Make_RandomFloat(5.f, 30.f);
    m_fTurnAxis = CUtils::Make_Random_Vector(1.f);
    m_fTurnAxis.w = 0.f;
    m_fActiveTime = 0.f;
}

HRESULT CFinaleBuildingPartical::Add_Components()
{
    HRESULT hr;
    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbBrokenBuildingAC"),
        TEXT("Com_ModelA"), (CComponent**)&m_pModelCom[A]);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbBrokenBuildingBL"),
        TEXT("Com_ModelB"), (CComponent**)&m_pModelCom[B]);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbBrokenBuildingCL"),
        TEXT("Com_ModelC"), (CComponent**)&m_pModelCom[C]);
    CHECK_FAILED(hr);

    hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbBrokenBuildingDL"),
        TEXT("Com_ModelD"), (CComponent**)&m_pModelCom[D]);
    CHECK_FAILED(hr);

    // 텍스쳐 준비할 것.
    return S_OK;
}

HRESULT CFinaleBuildingPartical::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    return S_OK;
}

CFinaleBuildingPartical* CFinaleBuildingPartical::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CFinaleBuildingPartical* pInstance = new CFinaleBuildingPartical(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CFinaleBuildingPartical"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CFinaleBuildingPartical::Clone(void* pArg)
{
    CFinaleBuildingPartical* pInstance = new CFinaleBuildingPartical(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CFinaleBuildingPartical"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CFinaleBuildingPartical::Free()
{
    __super::Free();

    for (auto& pModel : m_pModelCom)
        Safe_Release(pModel);
    Safe_Release(m_pShaderCom);
}
