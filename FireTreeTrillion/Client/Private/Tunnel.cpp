#include "stdafx.h"
#include "TunnelRock.h"
#include "HitBox.h"
#include "Tunnel.h"
#include "Kirby.h"
#include "Camera_Main.h"

CTunnel::CTunnel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPhysXObject{ pDevice, pContext }
{
}

CTunnel::CTunnel(const CTunnel& rhs)
    : CPhysXObject{ rhs }
{
}

HRESULT CTunnel::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTunnel::Initialize(void* pArg)
{
    GAMEOBJECT_DESC		GameObjectDesc{};
    if (nullptr != pArg)
        GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

    GameObjectDesc.fSpeedPerSec = 10.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    if (FAILED(Add_Components(GameObjectDesc.wstrModelName)))
        return E_FAIL;

    //unordered_set<_uint> setNonParticleMeshes;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        string strMeshName = m_pModelCom->Get_MeshName(i);
        if ("GsDefault_3__GsDefaultSideC" == strMeshName) {
            m_vecPassIndices.push_back(MAP_MASKED_NORMAL_O);
            //setNonParticleMeshes.insert(i);
        }
            
        else if ("GsDefault_3__GsDefaultCeilingC" == strMeshName)
        {
            m_vecPassIndices.push_back(MAP_BLEND_NONDISCARD);
            //setNonParticleMeshes.insert(i);
        }
        else
        {
            m_vecPassIndices.push_back(-1);

            //// 메쉬 복제 후 새로운 메쉬하여 오브젝트 생성하는 로직-> 아 블렌더가 정점 자동병합 해놨네 이놈이 아

            //unordered_set<string> setCommonString;
            //unordered_set<string> setExcludingMesh = { "GsDefault_3__GsDefaultSideC", "GsDefault_3__GsDefaultCeilingC" };

            //CTunnelRock::TUNNELROCK_DESC tDesc{};
            //_float3 vOffset{};
            //tDesc.pModel = m_pModelCom->CreateModelFromMesh(i, vOffset, setCommonString, setExcludingMesh);
            //if (nullptr == tDesc.pModel)
            //    continue;
            //_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
            //matWorld._41 = matWorld._41 + vOffset.x;
            //matWorld._42 = matWorld._42 + vOffset.y;
            //matWorld._43 = matWorld._43 + vOffset.z;

            //tDesc.matWorld = matWorld;
            //tDesc.iShaderVars = 6;
            //tDesc.fRimWidth = 0.2f;
            //CGameObject* pTunnelRock = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_TunnelRock"), &tDesc);
            //m_vecTunnelRocks.push_back(pTunnelRock);
        }
    }

    for (_uint j = 0; j < 4; j++)
    {
        for (_uint i = 0; i <= 16; i++)
        {
            GAMEOBJECT_DESC tDesc{};
            tDesc.matWorld = GameObjectDesc.matWorld;
            tDesc.wstrModelName = TEXT("TunnelRock") + to_wstring(i);
            CGameObject* pTunnelRock = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_TunnelRock"), &tDesc);
            m_vecTunnelRocks.push_back(pTunnelRock);
        }
    }

    m_fSamplingFactor = 0.1f;

    return S_OK;
}

_int CTunnel::Tick(_float fTimeDelta)
{
    if (true == m_bDead)
        return OBJ_DEAD;

    //if (m_pGameInstance->Get_KeyState(DIK_SPACE, KEY_DOWN))
    //    m_bCollsion = true;

    if (true == m_bCollsion)
    {
        m_fTime += fTimeDelta;
        if(m_fTime > 3.f)
            return OBJ_DEAD;

        for (auto& tunnelRock : m_vecTunnelRocks)
        {
            if (nullptr == tunnelRock)
                continue;
            tunnelRock->Tick(fTimeDelta);
        }
    }

    return OBJ_NOEVENT;
}

void CTunnel::Late_Tick(_float fTimeDelta)
{
    for (auto& tunnelRock : m_vecTunnelRocks)
    {
        if (nullptr == tunnelRock)
            continue;
        tunnelRock->Late_Tick(fTimeDelta);
    }

    if(false == m_bHide)
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTunnel::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        /*if (-1 == m_vecPassIndices[i])
            continue;*/

        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(m_vecPassIndices[i])))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CTunnel::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
    if (true == m_bCollsion)
        return;

    CKirby* pKirby = static_cast<CKirby*>(pObject);
    if (pKirby->Get_KirbyInfo()->m_bBooster == false)
        return;

    pKirby->Set_HitStop();
    CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
    pCamera->Make_Shake(2.f);

    m_bCollsion = true;
    m_bHide = true;
}

HRESULT CTunnel::Add_Components(const wstring& _wstrModelTag)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    /* For. Com_Mask_BaseColor */
    HRESULT hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TunnelMask_BaseColor"),
        TEXT("Com_Mask_BaseColor"), (CComponent**)&m_pTextureCom[TEX_BASECOLOR]);
    CHECK_FAILED(hr);

    /* For. Com_Mask_Normal */
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TunnelMask_Normal"),
        TEXT("Com_Mask_Normal"), (CComponent**)&m_pTextureCom[TEX_NORMAL]);
    CHECK_FAILED(hr);

    /* For. Com_Mask_MRA */
    hr = __super::Add_Component(TEXT("Prototype_Component_Texture_TunnelMask_MRA"),
        TEXT("Com_Mask_MRA"), (CComponent**)&m_pTextureCom[TEX_MRA]);
    CHECK_FAILED(hr);

    CHitBox::HITBOX_DESC HitBox{};
    HitBox.pOwner = this;
    HitBox.pDesc = &m_tColliderDesc[BODY];
    HitBox.pCollisionType = OBJECT;
    if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
        return E_FAIL;
    Set_BodyCollider(COLLIDER_SPHERE, 0, 0.f, 5.5f);

    return S_OK;
}

HRESULT CTunnel::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_fSamplingFactor, sizeof(m_fSamplingFactor))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fZero, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pTextureCom[TEX_BASECOLOR]->Bind_ShaderResource(m_pShaderCom, "g_MaskBaseTex")))
        return E_FAIL;
    if (FAILED(m_pTextureCom[TEX_NORMAL]->Bind_ShaderResource(m_pShaderCom, "g_MaskNormalTex")))
        return E_FAIL;
    if (FAILED(m_pTextureCom[TEX_MRA]->Bind_ShaderResource(m_pShaderCom, "g_MaskMRATex")))
        return E_FAIL;

    return S_OK;
}

CTunnel* CTunnel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTunnel* pInstance = new CTunnel(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CTunnel"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTunnel::Clone(void* pArg)
{
    CTunnel* pInstance = new CTunnel(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CTunnel"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTunnel::Free()
{
    __super::Free();

    for (auto& tunnelRock : m_vecTunnelRocks)
        Safe_Release(tunnelRock);

    for (_uint i = 0; i < TEX_END; i++)
        Safe_Release(m_pTextureCom[i]);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}