#include "stdafx.h"
#include "BasicMap.h"
#include <filesystem>

using namespace filesystem;

CBasicMap::CBasicMap(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
    : CGameObject{ pDevice, pContext }
{
}

CBasicMap::CBasicMap(const CBasicMap& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CBasicMap::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBasicMap::Initialize(void* pArg)
{
    GAMEOBJECT_DESC		GameObjectDesc{};
    if (nullptr != pArg)
        GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

    GameObjectDesc.fSpeedPerSec = 10.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    wstring wstrModelTag = GameObjectDesc.wstrModelName;

    if (FAILED(Add_Components(wstrModelTag)))
        return E_FAIL;

    if (wstrModelTag.substr(wstrModelTag.length() - 5) != TEXT("Blend")) {
        if (true == CheckIfBlendMapExists(GameObjectDesc.wstrModelName)) {
            if (FAILED(Add_BlendMap(wstrModelTag)))
                return E_FAIL;
        }
    }
    else    
        m_eRenderGroup = CRenderer::RENDER_BLEND;

    SetUpShaderInfo(wstrModelTag);

    if (FAILED(m_pModelCom->CreateStaticActor(m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION))))
        return E_FAIL;

    m_fTime = 100.f;
    m_fNonMatchTime = 100.f;

    return S_OK;
}

_int CBasicMap::Tick(_float fTimeDelta)
{
    if (true == m_bDead)
        return OBJ_DEAD;

    m_fTime += fTimeDelta;

    if (nullptr != m_pBlendMap)
        m_pBlendMap->Tick(fTimeDelta);

    return OBJ_NOEVENT;
}

void CBasicMap::Late_Tick(_float fTimeDelta)
{
    if (nullptr != m_pBlendMap)
        m_pBlendMap->Late_Tick(fTimeDelta);

    m_pGameInstance->Add_RenderGroup(m_eRenderGroup, this);
}

HRESULT CBasicMap::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
            return E_FAIL;
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_vecSamplingFactors[i], sizeof(_float))))
            return E_FAIL;
        if (i == m_iMeshIndex) {
            if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
                return E_FAIL;
        }
        else {
            if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fNonMatchTime, sizeof(_float))))
                return E_FAIL;
        }

        if (FAILED(m_pShaderCom->Begin(m_vecPassIndices[i])))
            return E_FAIL;
                
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
        
    }

    return S_OK;
}

HRESULT CBasicMap::Add_Components(const wstring& _wstrModelTag)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel_Map"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    /* For. Com_Texture */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Texture_GsLandTopNoize_Fur"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CBasicMap::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_NoiseTexture")))
        return E_FAIL;

    return S_OK;
}

HRESULT CBasicMap::Add_BlendMap(const wstring& _wstrModelTag)
{
    GAMEOBJECT_DESC tMapDesc{};
    tMapDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
    tMapDesc.wstrModelName = _wstrModelTag + TEXT("_Blend"); 

    m_pBlendMap = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BasicMap"), &tMapDesc);
    if (nullptr == m_pBlendMap)
        return E_FAIL;

    return S_OK;
}

void CBasicMap::SetUpShaderInfo(const wstring& _wstrModelTag)
{
    m_vecPassIndices.resize(m_pModelCom->Get_NumMeshes());
    m_vecSamplingFactors.resize(m_pModelCom->Get_NumMeshes());
    fill(m_vecSamplingFactors.begin(), m_vecSamplingFactors.end(), 1.f);

    if (m_eRenderGroup == CRenderer::RENDER_BLEND) {
        fill(m_vecPassIndices.begin(), m_vecPassIndices.end(), 4);
        return;
    }

    string strFilePath = "../../../objects_txt/" + CUtils::WstrToStr(_wstrModelTag) + "_ShaderInfo.txt";

    fstream fileStream(strFilePath, ios::in | ios::binary);
    if (fileStream.is_open() == false)
    {
        wstring wstrError = TEXT("Failed to Open: ") + _wstrModelTag + L"_ShaderInfo.txt";
        //MSG_BOX(wstrError.c_str());
        return;
    }

    _uint iPassIndex{};
    _float fSamplingFactor{};
    _int iCount{};
    while (!fileStream.eof()) 
    {
        fileStream.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));
        fileStream.read(reinterpret_cast<char*>(&fSamplingFactor), sizeof(fSamplingFactor));

        if (fileStream.eof())
            break;
        
        m_vecPassIndices[iCount] = iPassIndex;
        m_vecSamplingFactors[iCount] = fSamplingFactor;
        iCount++;
    }

    fileStream.close();
}

_bool CBasicMap::CheckIfBlendMapExists(const wstring& _wstrModelTag)
{
    string strPath = "../../../model_txt/NonAnim/";
    string strBlendMapName = CUtils::WstrToStr(_wstrModelTag) + "_Blend";

    directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
    directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

    while (dir_iter != end_iter) {
        if (is_regular_file(*dir_iter)) {
            string strFilePath = dir_iter->path().filename().string();

            if (strBlendMapName == strFilePath.substr(0, strFilePath.length() - 4))
                return true;
        }
        ++dir_iter;
    }

    return _bool();
}

CBasicMap* CBasicMap::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBasicMap* pInstance = new CBasicMap(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CBasicMap"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBasicMap::Clone(void* pArg)
{
    CBasicMap* pInstance = new CBasicMap(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Clone : CBasicMap"));
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBasicMap::Free()
{
    __super::Free();
    Safe_Release(m_pBlendMap);
    Safe_Release(m_pTextureCom);

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
}