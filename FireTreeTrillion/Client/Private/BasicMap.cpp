#include "stdafx.h"
#include "BlendMapObject.h"
#include "MapToolObject.h"
#include "ShadowDeco.h"
#include "BasicMap.h"
#include "AnimDeco.h"
#include "HitBox.h"
#include "OcTree.h"
#include "Model.h"

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
    MAP_DESC		GameObjectDesc{};
    if (nullptr != pArg)
        GameObjectDesc = *(MAP_DESC*)pArg;

    GameObjectDesc.fSpeedPerSec = 10.f;
    GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(&GameObjectDesc)))
        return E_FAIL;

    m_setShadowDecos = { "CmStreetLampA", "CmStreetLampD", "GsTreeA", "GsTreeB", "GsTreeC", "GsTelephonePoleB"
    , "GsTrafficSignalAL" };

    // 옥트리를 생성하는 맵
    m_setOctreeMaps = { L"Level0Stage1Step01", L"Level0Stage1Step02", L"Level1Stage1Step01" };

    wstring wstrModelTag = GameObjectDesc.wstrModelName;
    // 이 객체가 BlendMap인지 아닌지 검사
    if (wstrModelTag.length() > 5 && wstrModelTag.substr(wstrModelTag.length() - 5) == TEXT("Blend"))
    {
        m_bBlendMap = true;
        m_eRenderGroup = CRenderer::RENDER_BLEND;
    }

    if (FAILED(Add_Components(wstrModelTag)))
        return E_FAIL;

    m_fTime = m_fNonMatchTime = 100.f;
        
    SetUpShaderInfo(wstrModelTag);

    m_vecConstantNames = { "g_DiffuseTexture", "g_NormalTexture", "g_MRATexture", "g_fSamplingFactor"
    , "g_bStencil", "g_bRimLight", "m_fRimWidth", "g_bMotionBlur", "g_BoneMatrices" };

    m_vecStencilRimLightMotionBlurNames = { "g_bStencil", "g_bRimLight", "m_fRimWidth", "g_bMotionBlur" };

    if (true == CheckIfBlendMapExists(wstrModelTag))
    {
        if (FAILED(Add_BlendMap(wstrModelTag)))
            return E_FAIL;
    }

    if (LEVEL_TOOL_MAP != *m_pCurrentLevelID)
        m_pStaticActor = m_pModelCom->ReturnStaticActor(GameObjectDesc.matWorld);

    if (true == m_bBlendMap)
        return S_OK;

    // 옥트리를 생성하는 레벨들의 맵
    if(IsOctreeMapModel(wstrModelTag))
    {
        TraverseBlendDecoInfoTxts(m_mapBlendMeshesIndices, m_mapBlendObjStaticActor);

        _vector vMin = XMLoadFloat3(&GameObjectDesc.vMin);
        _vector vMax = XMLoadFloat3(&GameObjectDesc.vMax);
        _matrix matWorld = m_pTransformCom->Get_WorldMatrix();
        vMin = XMVector3TransformCoord(vMin, matWorld);
        vMax = XMVector3TransformCoord(vMax, matWorld);

        _float3 vMinFloat3{}, vMaxFloat3{};
        XMStoreFloat3(&vMinFloat3, vMin);
        XMStoreFloat3(&vMaxFloat3, vMax);

        m_pOcTree = m_pModelCom->Create_OcTree(vMinFloat3, vMaxFloat3, m_vecPassIndices, m_vecSamplingFactors, m_vecConstantNames);
        
        // --- ModelName -- TriggerRadius -- IdleIndex & Speed -- ActionIndex & Speed ----------
        SetUpAnimDecoInfo("BushL", 1.5f, 2, 60.f, 0, 50.f);
        SetUpAnimDecoInfo("BushM", 1.1f, 2, 60.f, 0, 50.f);
        SetUpAnimDecoInfo("BushS", 0.8f, 2, 60.f, 0, 50.f);
        SetUpAnimDecoInfo("PopFlower", 0.8f, 2, 120.f, 1, 130.f);

        InsertMapDecos();
    }
    else
    {
        if (LEVEL_TOOL_MAP != *m_pCurrentLevelID)
        {
            TraverseBlendDecoInfoTxts(m_mapBlendMeshesIndices, m_mapBlendObjStaticActor);

            ReadMapDecoTxts();
            ReadDecos_ForSmallLevels();
        }
    }

    return S_OK;
}

_int CBasicMap::Tick(_float fTimeDelta)
{
    if (true == m_bDead)
        return OBJ_DEAD;

    m_fTime += fTimeDelta;
  
    return OBJ_NOEVENT;
}

void CBasicMap::Late_Tick(_float fTimeDelta)
{
    if(true == m_bBlendMap)
        Compute_ViewZ();

    if (nullptr != m_pBlendMap)
        m_pBlendMap->Late_Tick(fTimeDelta);

    for (auto& blendDeco : m_vecBlendObjects)
        blendDeco->Late_Tick(fTimeDelta);

    for (auto& shadowDeco : m_vecShadowObjects)
        shadowDeco->Late_Tick(fTimeDelta);

    m_pGameInstance->Add_RenderGroup(m_eRenderGroup, this);
}

HRESULT CBasicMap::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;
    if ((LEVEL_GAMEPLAY == *m_pCurrentLevelID || LEVEL_INTRO == *m_pCurrentLevelID
        || LEVEL_RACING == *m_pCurrentLevelID) && m_pOcTree != nullptr)
    {
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fNonMatchTime, sizeof(_float))))
            return E_FAIL;
        if (FAILED(m_pNonAnimShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
            return E_FAIL;
        if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
            return E_FAIL;

        m_iRenderAll = m_iRenderMyMesh = 0;
        m_pOcTree->Culling(m_pGameInstance, m_pShaderCom, m_pNonAnimShaderCom, m_pAnimShaderCom
            , m_iRenderAll, m_iRenderMyMesh);

       /* for (auto& blendDeco : m_vecBlendObjects)
            blendDeco->Late_Tick(m_pGameInstance->Get_FirstTimer());*/
    }
    else if (LEVEL_PARK == *m_pCurrentLevelID)
    {
        _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

        for (size_t i = 0; i < iNumMeshes; i++)
        {
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_EmissiveTexture", i, TextureType_EMISSIVE)))
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

        if (false == m_bBlendMap && LEVEL_TOOL_MAP != *m_pCurrentLevelID)
            Render_NonOctreeMapDecos();
    }
    else
    {
        _uint iNumMeshes = m_pModelCom->Get_NumMeshes();

        for (size_t i = 0; i < iNumMeshes; i++)
        {
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
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

        if (false == m_bBlendMap && LEVEL_TOOL_MAP != *m_pCurrentLevelID)
            Render_NonOctreeMapDecos();
    }

    return S_OK;
}

#ifdef _DEBUG
void CBasicMap::Render_IMGUI()
{
    ImGui::Text("Octrees: %d", m_pGameInstance->Get_NumOctree());
    ImGui::Text("RenderAll: %d", m_iRenderAll);
    ImGui::Text("RenderMyMesh: %d", m_iRenderMyMesh);
}
#endif

HRESULT CBasicMap::Add_Components(const wstring& _wstrModelTag)
{
    HRESULT hr(S_OK);

    /* For.Com_Shader */
    hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
                                TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
    CHECK_FAILED(hr);

    /* For.Com_Model */
    hr = __super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
                                TEXT("Com_Model"), (CComponent**)&m_pModelCom);
    CHECK_FAILED(hr);

    if (*m_pCurrentLevelID != LEVEL_TOOL_MAP)
    {
        /* For.Com_Shader */
        hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
                                    TEXT("Com_Shader_NonAnim"), (CComponent**)&m_pNonAnimShaderCom);
        CHECK_FAILED(hr);

        /* For.Com_Shader */
        hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
                                    TEXT("Com_Shader_Anim"), (CComponent**)&m_pAnimShaderCom);
        CHECK_FAILED(hr);


        /* For.Com_Texture */
        hr = __super::Add_Component(TEXT("Prototype_Component_Texture_FX_Mask_Bubble2"),
                                    TEXT("Com_Texture"), (CComponent**)&m_pTextureCom);
        CHECK_FAILED(hr);
    }

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

    if (LEVEL_TOOL_MAP == *m_pCurrentLevelID)
        return S_OK;

    if (FAILED(m_pAnimShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pAnimShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;

    if (FAILED(m_pNonAnimShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pNonAnimShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pNonAnimShaderCom, "g_ObjNearClipTexture")))
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
    _uint iNumMeshes = m_pModelCom->Get_NumMeshes();
    m_vecPassIndices.resize(iNumMeshes);
    m_vecSamplingFactors.resize(iNumMeshes);
    fill(m_vecSamplingFactors.begin(), m_vecSamplingFactors.end(), 1.f);

    if (true == m_bBlendMap) {
        fill(m_vecPassIndices.begin(), m_vecPassIndices.end(), 4);
        return;
    }

    string strFilePath = "../../../objects_txt/" + CUtils::WstrToStr(_wstrModelTag) + "_ShaderInfo.txt";

    fstream fileStream(strFilePath, ios::in | ios::binary);
    if (fileStream.is_open() == false)
    {
        wstring wstrError = TEXT("Failed to Open: ") + _wstrModelTag + L"_ShaderInfo.txt";
        return;
    }

    _uint iPassIndex{};
    _float fSamplingFactor{};
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        fileStream.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));
        fileStream.read(reinterpret_cast<char*>(&fSamplingFactor), sizeof(fSamplingFactor));
        if (fileStream.eof()) {
            fileStream.close();
            return;
        }
            
        m_vecPassIndices[i] = iPassIndex;
        m_vecSamplingFactors[i] = fSamplingFactor;
    }

    fileStream.close();
}

_bool CBasicMap::CheckIfBlendMapExists(const wstring& _wstrModelTag)
{
    string strPath = "../../../model_txt/MapObjs/NonAnim/";
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

void CBasicMap::InsertMapDecos()
{
    if (nullptr == m_pOcTree)
        return;

    string strLevel;
    if (LEVEL_INTRO == *m_pCurrentLevelID)
        strLevel = "Intro";
    else if (LEVEL_GAMEPLAY == *m_pCurrentLevelID)
        strLevel = "Stage1";
    else if (LEVEL_RACING == *m_pCurrentLevelID)
        strLevel = "Racing";
    else
        return;
        
    string strPath = "../../../objects_txt/" + strLevel + "_DecoObjs.txt";

    ifstream fileInput(strPath, ios::binary);
    if (!fileInput.is_open())
    {
        wstring wstrError = L"Failed to Open:" + CUtils::StrToWstr(strLevel) + L"_DecoObjs.txt";
        MSG_BOX(wstrError.c_str());
        return;
    }
    
    vector<CModel*> vecNonCols;
    vector<CModel*> vecAnims;
    vector<CModel*> vecActors;

    _uint iMapObjType{};
    _uint iStrLength{};
    string strModelName;
    _float4x4 matWorld{};
    _uint iShaderVars{};
    _float fRimWidth{};
    _uint iPassIndex{};

    _uint iNumObjects{};
    fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

    for (_uint i = 0; i < iNumObjects; i++)
    {
        fileInput.read(reinterpret_cast<char*>(&iMapObjType), sizeof(iMapObjType));
        fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
        strModelName.resize(iStrLength);
        fileInput.read(&strModelName[0], iStrLength);
        fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
        fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
        fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));
        fileInput.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));

        TYPE eType = TYPE_NONANIM;
        string strFolder = string("MapDeco/");
        if (CMapToolObject::MAPOBJ_ANIM == iMapObjType)
        {
            eType = TYPE_ANIM;
            strFolder = string("MapDeco/");
        }

        CModel* pModel = CModel::Create(m_pDevice, m_pContext, MODEL{ strModelName, eType, 1.f, 0.f, 0, strFolder, false});

        if (nullptr == pModel) {
            fileInput.close();
            return;
        }

        PxRigidStatic* pRigidStatic = { nullptr };
        pModel->Set_WorldMatrixForOctree(matWorld);
 
        auto mapIter = m_mapBlendMeshesIndices.find(strModelName);
        if (mapIter != m_mapBlendMeshesIndices.end())
        {
            CBlendMapObject::BLENDMAPOBJ_DESC tBlendObjDesc{};
            tBlendObjDesc.matWorld = matWorld;
            tBlendObjDesc.tModel = MODEL{ strModelName, eType, 1.f, 0.f, 0, strFolder, false };
            tBlendObjDesc.iShaderVars = iShaderVars;
            tBlendObjDesc.fRimWidth = fRimWidth;
            tBlendObjDesc.setBlendMeshIndices = mapIter->second;
            CBlendMapObject* pBlendMapObj = dynamic_cast<CBlendMapObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BlendMapObject"), &tBlendObjDesc));

            if (nullptr != pBlendMapObj) {
                m_vecBlendObjects.push_back(pBlendMapObj);
                pModel->Set_BlendObject(pBlendMapObj);
            }

            pModel->RemoveBlendMeshes(mapIter->second);

            if (pModel->Get_NumMeshes() == 0)
                continue;
        }

        if (true == IsShadowDeco(strModelName))
        {
            CShadowDeco::SHADOWDECO_DESC tShadowDecoDesc{};
            tShadowDecoDesc.pDecoModel = pModel;
            CShadowDeco* pShadowDeco = dynamic_cast<CShadowDeco*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_ShadowDeco"), &tShadowDecoDesc));
            m_vecShadowObjects.push_back(pShadowDeco);
            iShaderVars |= 4;
        }

        pModel->SetUpStencilRimLightMotionBlurPassIndex(iShaderVars, fRimWidth, iPassIndex);


        if (CMapToolObject::MAPOBJ_NONCOL == iMapObjType)
        {
            vecNonCols.push_back(pModel);
        }
        else if (CMapToolObject::MAPOBJ_ANIM == iMapObjType)
        {
            auto idleIter = m_ModelIdleAnimMap.find(strModelName);
            if (idleIter == m_ModelIdleAnimMap.end())
                continue;
            _uint iIdleIndex = idleIter->second.first;
            _float iIdleSpeed = idleIter->second.second;
            pModel->Set_Animation(iIdleIndex, iIdleSpeed, true, true);
            pModel->SetUp_ModelIdleAnimForOctree(iIdleIndex, iIdleSpeed);

            pRigidStatic = AddTriggerActorForAnimDeco(strModelName, matWorld);
            if (nullptr == pRigidStatic)
                continue;
            auto animIter = m_ModelActionAnimMap.find(strModelName);
            if (animIter == m_ModelActionAnimMap.end())
                continue;

            m_pGameInstance->Emplace_MapDecoTrigger(pRigidStatic, pModel, animIter->second.first, animIter->second.second);
            CAnimDeco::ANIMDECO_DESC tAnimDeco{};
            tAnimDeco.pAnimDecoModel = pModel;
            CAnimDeco* pAnimDecoObj = dynamic_cast<CAnimDeco*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_AnimDeco"), &tAnimDeco));
            m_vecAnimDecoGameObjs.push_back(pAnimDecoObj);
            
            vecAnims.push_back(pModel);
        }
        else if (CMapToolObject::MAPOBJ_ACTOR == iMapObjType)
        {
            pModel->CreateStaticActor(matWorld);
            vecActors.push_back(pModel);
        }
    }

    fileInput.close();

    m_pOcTree->InsertNonCols(vecNonCols);
    m_pOcTree->InsertColAnims(vecAnims);
    m_pOcTree->InsertColNonAnims(vecActors);
}

PxRigidStatic* CBasicMap::AddTriggerActorForAnimDeco(const string& _strModelName, _float4x4& _matWorld)
{
    auto pPhysics = m_pGameInstance->Get_Physics();
    PxMaterial* pMtrl = m_pGameInstance->Get_Material();

    auto iter = m_ModelShapeRadiiMap.find(_strModelName);
    if (iter == m_ModelShapeRadiiMap.end()) {
        MSG_BOX(TEXT("Failed to Create: CBasicMap::TriggerActorForAnimDeco"));
        return nullptr;
    }

    _float fRadius = iter->second;

    PxShape* pShape = pPhysics->createShape(PxSphereGeometry(fRadius), *pMtrl);

    PxMat44 pxMat = CUtils::To_Float4x4(_matWorld);
    PxTransform transform = CUtils::mat44ToTransform(pxMat);

    PxRigidStatic* pStaticActor = pPhysics->createRigidStatic(transform);
    pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
    pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
    pStaticActor->attachShape(*pShape);

    if (nullptr == pShape || nullptr == pStaticActor)
    {
        MSG_BOX(TEXT("Failed to Create: CBasicMap::TriggerActorForAnimDeco"));
        return nullptr;
    }
    m_pGameInstance->AddActor(*pStaticActor);
    m_vecAnimDecoTriggersActors.emplace_back(pStaticActor);
    pShape->release();
    //m_vecShapes.emplace_back(pShape);
    return pStaticActor;
}

void CBasicMap::SetUpAnimDecoInfo(const string& _strModelName, _float _fTriggerRadius, _uint iIdleIndex, _float fIdleAnimSpeed, _uint iActionIndex, _float fActionAnimSpeed)
{
    m_ModelShapeRadiiMap.emplace(_strModelName, _fTriggerRadius);
    m_ModelIdleAnimMap.emplace(_strModelName, pair<_uint, _float>(iIdleIndex, fIdleAnimSpeed));
    m_ModelActionAnimMap.emplace(_strModelName, pair<_uint, _float>(iActionIndex, fActionAnimSpeed));
}

void CBasicMap::ReadDecos_ForSmallLevels()
{
    Release_MapDecos();

    string strLevel;
    if (LEVEL_TOWN == *m_pCurrentLevelID)
        strLevel = "Town";
    else if (LEVEL_DEEDEEDEE == *m_pCurrentLevelID)
        strLevel = "DeeDeeDee";
    else if (LEVEL_PARTTIME == *m_pCurrentLevelID)
        strLevel = "PartTime";
    else if (LEVEL_PARK == *m_pCurrentLevelID)
        strLevel = "Park";
    else if (LEVEL_SIMBA == *m_pCurrentLevelID)
        strLevel = "Simba";
    else if (LEVEL_FINALBOSS == *m_pCurrentLevelID)
        strLevel = "FinalBoss";
    else
        return;

    string strPath = "../../../objects_txt/" + strLevel + "_DecoObjs.txt";

    ifstream fileInput(strPath, ios::binary);
    if (!fileInput.is_open())
    {
        wstring wstrError = L"Failed to Open:" + CUtils::StrToWstr(strLevel) + L"_DecoObjs.txt";
        MSG_BOX(wstrError.c_str());
        return;
    }

    _uint iMapObjType{};
    _uint iStrLength{};
    string strModelName;
    _float4x4 matWorld{};
    _uint iShaderVars{};
    _float fRimWidth{};
    _uint iPassIndex{};

    _uint iNumObjects{};
    fileInput.read(reinterpret_cast<char*>(&iNumObjects), sizeof(iNumObjects));

    for (_uint i = 0; i < iNumObjects; i++)
    {
        fileInput.read(reinterpret_cast<char*>(&iMapObjType), sizeof(iMapObjType));
        fileInput.read(reinterpret_cast<char*>(&iStrLength), sizeof(iStrLength));
        strModelName.resize(iStrLength);
        fileInput.read(&strModelName[0], iStrLength);
        fileInput.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
        fileInput.read(reinterpret_cast<char*>(&iShaderVars), sizeof(iShaderVars));
        fileInput.read(reinterpret_cast<char*>(&fRimWidth), sizeof(fRimWidth));
        fileInput.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));

        TYPE eType = TYPE_NONANIM;
        string strFolder;
        if (LEVEL_TOWN == *m_pCurrentLevelID || LEVEL_PARTTIME == *m_pCurrentLevelID || LEVEL_DEEDEEDEE == *m_pCurrentLevelID)
            strFolder = string("TownDeco/");
        else if(LEVEL_PARK == *m_pCurrentLevelID)
            strFolder = string("ParkDeco/");
        else if (LEVEL_FINALBOSS == *m_pCurrentLevelID || LEVEL_SIMBA == *m_pCurrentLevelID)
            strFolder = string("LabDiscovera_Deco/");

        if (true == IsMapDeco(strModelName))
            strFolder = string("MapDeco/");

        if (CMapToolObject::MAPOBJ_ANIM == iMapObjType)
            eType = TYPE_ANIM;

        CModel* pModel = CModel::Create(m_pDevice, m_pContext, MODEL{ strModelName, eType, 1.f, 0.f, 0, strFolder, false });

        if (nullptr == pModel) {
            fileInput.close();
            return;
        }

        PxRigidStatic* pRigidStatic = { nullptr };
        pModel->Set_WorldMatrixForOctree(matWorld);

        auto mapIter = m_mapBlendMeshesIndices.find(strModelName);
        if (mapIter != m_mapBlendMeshesIndices.end())
        {
            CBlendMapObject::BLENDMAPOBJ_DESC tBlendObjDesc{};
            tBlendObjDesc.matWorld = matWorld;
            tBlendObjDesc.tModel = MODEL{ strModelName, eType, 1.f, 0.f, 0, strFolder, false };
            tBlendObjDesc.iShaderVars = iShaderVars;
            tBlendObjDesc.fRimWidth = fRimWidth;
            tBlendObjDesc.setBlendMeshIndices = mapIter->second;
            CBlendMapObject* pBlendMapObj =  dynamic_cast<CBlendMapObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_BlendMapObject"), &tBlendObjDesc));

            if (nullptr != pBlendMapObj) {
                m_vecBlendObjects.push_back(pBlendMapObj);
                pModel->Set_BlendObject(pBlendMapObj);
            }

            pModel->RemoveBlendMeshes(mapIter->second);

            if (pModel->Get_NumMeshes() == 0)
                continue;
        }

        if (true == IsShadowDeco(strModelName))
        {
            CShadowDeco::SHADOWDECO_DESC tShadowDecoDesc{};
            tShadowDecoDesc.pDecoModel = pModel;
            CShadowDeco* pShadowDeco = dynamic_cast<CShadowDeco*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_ShadowDeco"), &tShadowDecoDesc));
            m_vecShadowObjects.push_back(pShadowDeco);
            iShaderVars |= 4;
        }

        pModel->SetUpStencilRimLightMotionBlurPassIndex(iShaderVars, fRimWidth, iPassIndex);

        if (CMapToolObject::MAPOBJ_NONCOL == iMapObjType)
            m_vecNonAnimDecos.push_back(pModel);

        else if (CMapToolObject::MAPOBJ_ANIM == iMapObjType)
        {
            auto idleIter = m_ModelIdleAnimMap.find(strModelName);
            if (idleIter == m_ModelIdleAnimMap.end())
                continue;
            _uint iIdleIndex = idleIter->second.first;
            _float iIdleSpeed = idleIter->second.second;
            pModel->Set_Animation(iIdleIndex, iIdleSpeed, true, true);
            pModel->SetUp_ModelIdleAnimForOctree(iIdleIndex, iIdleSpeed);

            pRigidStatic = AddTriggerActorForAnimDeco(strModelName, matWorld);
            if (nullptr == pRigidStatic)
                continue;
            auto animIter = m_ModelActionAnimMap.find(strModelName);
            if (animIter == m_ModelActionAnimMap.end())
                continue;

            m_pGameInstance->Emplace_MapDecoTrigger(pRigidStatic, pModel, animIter->second.first, animIter->second.second);
            m_vecAnimDecos.push_back(pModel);
        }
        else if (CMapToolObject::MAPOBJ_ACTOR == iMapObjType)
        {
            m_vecDecoStaticActors.push_back(pModel->ReturnStaticActor(matWorld));
            m_vecNonAnimDecos.push_back(pModel);
        }
    }

    fileInput.close();
}

void CBasicMap::Release_MapDecos()
{
    for (auto& shadowObj : m_vecShadowObjects)
        Safe_Release(shadowObj);
    m_vecShadowObjects.clear();

    for (auto& blendObj : m_vecBlendObjects)
        Safe_Release(blendObj);
    m_vecBlendObjects.clear();

    for (auto& animDecoObj : m_vecAnimDecoGameObjs)
        Safe_Release(animDecoObj);
    m_vecAnimDecoGameObjs.clear();

    for (auto& nonAnimDeco : m_vecNonAnimDecos)
        Safe_Release(nonAnimDeco);
    m_vecNonAnimDecos.clear();

    for (auto& animDeco : m_vecAnimDecos)
        Safe_Release(animDeco);
    m_vecAnimDecos.clear();
}

HRESULT CBasicMap::Render_NonOctreeMapDecos()
{
    for (auto& nonAnim : m_vecNonAnimDecos) {
        if (nullptr == nonAnim)
            continue;

        _uint iNumMeshes = nonAnim->Get_NumMeshes();
        nonAnim->Bind_StencilRimLightMotionBlur(m_pNonAnimShaderCom, m_vecStencilRimLightMotionBlurNames);
        if (FAILED(nonAnim->Bind_WorldMatrixForOctree(m_pNonAnimShaderCom)))
            return E_FAIL;

        _uint iModelPassIndex = nonAnim->Get_ModelPassIndex();
        for (_uint i = 0; i < iNumMeshes; i++) {
            if (FAILED(nonAnim->Bind_ShaderResource(m_pNonAnimShaderCom, m_vecConstantNames[0].c_str(), i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(nonAnim->Bind_ShaderResource(m_pNonAnimShaderCom, m_vecConstantNames[1].c_str(), i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(nonAnim->Bind_ShaderResource(m_pNonAnimShaderCom, m_vecConstantNames[2].c_str(), i, TextureType_METALNESS)))
                return E_FAIL;
            if (FAILED(m_pNonAnimShaderCom->Begin(iModelPassIndex)))
                return E_FAIL;
            if (FAILED(nonAnim->Render(i)))
                return E_FAIL;
        }
    }

    for (auto& animDeco : m_vecAnimDecos)
    {
        if (nullptr == animDeco)
            continue;

        _uint iNumMeshes = animDeco->Get_NumMeshes();

        if (FAILED(animDeco->Play_Animation(m_pGameInstance->Get_SecondTimer())))
            return E_FAIL;
        if (true == animDeco->IsFinished())
            animDeco->ReturnToIdle();
        if (FAILED(animDeco->Bind_StencilRimLightMotionBlur(m_pAnimShaderCom, m_vecStencilRimLightMotionBlurNames)))
            return E_FAIL;
        if (FAILED(animDeco->Bind_WorldMatrixForOctree(m_pAnimShaderCom)))
            return E_FAIL;

        _uint iModelPassIndex = animDeco->Get_ModelPassIndex();
        for (_uint i = 0; i < iNumMeshes; i++) {
            if (FAILED(animDeco->Bind_ShaderResource(m_pAnimShaderCom, m_vecConstantNames[0].c_str(), i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(animDeco->Bind_ShaderResource(m_pAnimShaderCom, m_vecConstantNames[1].c_str(), i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(animDeco->Bind_ShaderResource(m_pAnimShaderCom, m_vecConstantNames[2].c_str(), i, TextureType_METALNESS)))
                return E_FAIL;
            if (FAILED(animDeco->Bind_BoneMatrices(m_pAnimShaderCom, m_vecConstantNames[8].c_str(), i)))
                return E_FAIL;
            if (FAILED(m_pAnimShaderCom->Begin(iModelPassIndex)))
                return E_FAIL;
            if (FAILED(animDeco->Render(i)))
                return E_FAIL;
        }
    }

    /*for (auto& blendDeco : m_vecBlendObjects)
        blendDeco->Late_Tick(m_pGameInstance->Get_FirstTimer());*/

    return S_OK;
}

void CBasicMap::ReadMapDecoTxts()
{
    string strPath = "../../../model_txt/MapDeco/NonAnim/";

    directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
    directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

    while (dir_iter != end_iter) {
        if (is_regular_file(*dir_iter)) {
            string strFilePath = dir_iter->path().filename().string();
            string strModelName = strFilePath.substr(0, strFilePath.length() - 4);
            m_setMapDecoNames.insert(strModelName);
        }
        ++dir_iter;
    }
}

_bool CBasicMap::IsMapDeco(const string& _strModelName)
{
    if (m_setMapDecoNames.end() != m_setMapDecoNames.find(_strModelName))
        return true;

    return _bool();
}

_bool CBasicMap::IsBlendDeco(const string& _strModelName)
{
    if (m_mapBlendMeshesIndices.end() != m_mapBlendMeshesIndices.find(_strModelName))
        return true;

    return _bool();
}

void CBasicMap::TraverseBlendDecoInfoTxts(unordered_map<string, unordered_set<_uint>>& _mapBlendMeshIndices
        , unordered_map<string, _bool>& _mapBlendObjStaticActor)
{
    string strPath = "../../../objects_txt/BlendDecoInfo/";

    directory_iterator end_iter;  // 디렉토리 순회의 끝을 나타내는 iterator
    directory_iterator dir_iter(strPath);  // 지정된 경로의 시작 iterator

    while (dir_iter != end_iter) {
        if (is_regular_file(*dir_iter)) {
            string strFilePath = dir_iter->path().filename().string();
            string strFileName = strFilePath.substr(0, strFilePath.length() - 4);

            string strModelName;
            string::size_type pos = strFileName.find('_');
            if (pos != string::npos)
                strModelName = strFileName.substr(0, pos);
            else
                continue;

            unordered_set<_uint> setBlendMeshesIndices;
            _bool bStaticActor = false;
            _bool bRead = ReadBlendMeshesIndices(dir_iter->path().generic_string(), strModelName, setBlendMeshesIndices, bStaticActor);

            if (true == bRead) {
                _mapBlendMeshIndices.emplace(strModelName, setBlendMeshesIndices);
                _mapBlendObjStaticActor.emplace(strModelName, bStaticActor);
            }
        }
        ++dir_iter;
    }
}

_bool CBasicMap::ReadBlendMeshesIndices(const string& _strFullPath, const string& _strModelName
        , unordered_set<_uint>& _setMeshIndices, _bool& _bStaticActor)
{
    ifstream fileInput(_strFullPath, ios::binary);
    if (fileInput.is_open() == false)
    {
        wstring wstrError = TEXT("Failed to open : ") + CUtils::StrToWstr(_strModelName) + TEXT("_BlendMeshes.txt");
        MSG_BOX(wstrError.c_str());
        return false;
    }

    _bool bStaticActor{};
    fileInput.read(reinterpret_cast<char*>(&bStaticActor), sizeof(bStaticActor));
    _bStaticActor = bStaticActor;

    _uint iNumBlendMeshes{};
    fileInput.read(reinterpret_cast<char*>(&iNumBlendMeshes), sizeof(iNumBlendMeshes));

    _uint iMeshIndex{};
    for (_uint i = 0; i < iNumBlendMeshes; i++)
    {
        fileInput.read(reinterpret_cast<char*>(&iMeshIndex), sizeof(iMeshIndex));
        _setMeshIndices.insert(iMeshIndex);
    }

    fileInput.close();

    return true;
}

_bool CBasicMap::IsShadowDeco(const string& _strModelName)
{
    if (m_setShadowDecos.end() != m_setShadowDecos.find(_strModelName))
        return true;

    return _bool();
}

_bool CBasicMap::IsOctreeMapModel(const wstring& _wstrModelName)
{
    if (m_setOctreeMaps.end() != m_setOctreeMaps.find(_wstrModelName))
        return true;

    return _bool();
}

void CBasicMap::Save_OctreeData(const string& strLevel)
{
    string tempFileName = "temp_" + strLevel + "_Octree.txt";
    ofstream outputFile(tempFileName, ios::out | ios::binary);
    if (!outputFile.is_open()) // 임시파일 열렸는지 확인
    {
        wstring wstrErrorMsg = TEXT("Failed to Open: ") + CUtils::StrToWstr(tempFileName);
        MSG_BOX(wstrErrorMsg.c_str());
        return;
    }

    m_pOcTree->Save_OctreeData(outputFile);

    outputFile.close();

    if (!outputFile)
    {
        wstring wstrError = TEXT("Failed to write data to ") + CUtils::StrToWstr(tempFileName);
        MSG_BOX(wstrError.c_str());
        remove(tempFileName.c_str()); // 임시파일 삭제
        return;
    }

    // 현재시간 받아오기
    auto now = chrono::system_clock::now();
    time_t currentTime = chrono::system_clock::to_time_t(now);

    struct tm timeinfo;
    localtime_s(&timeinfo, &currentTime);

    // 현재 시간을 문자열로 변환
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);

    string fileName_Time = "../../../objects_txt/" + string(buffer) + "_" + strLevel + "_Octree.txt";
    string fileName = "../../../objects_txt/" + strLevel + "_Octree.txt";
    if (rename(fileName.c_str(), fileName_Time.c_str()) != 0)
    {
        MSG_BOX(TEXT("Failed to rename original file."));
        return;
    }

    if (rename(tempFileName.c_str(), fileName.c_str()) != 0) // 임시파일 이름을 level 이름으로 변경
    {
        wstring wstrError2 = TEXT("Failed to rename ") + CUtils::StrToWstr(tempFileName);
        MSG_BOX(wstrError2.c_str());
        remove(tempFileName.c_str()); // 임시파일 삭제
        return;
    }

    MSG_BOX(TEXT("Octree Saved."));
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

    Release_MapDecos();
    
    for (auto& trigger : m_vecAnimDecoTriggersActors)
        m_pGameInstance->ReleaseActor(trigger);
    m_vecAnimDecoTriggersActors.clear();

    for (auto& staticActor : m_vecDecoStaticActors)
        m_pGameInstance->ReleaseActor(staticActor);
    m_vecDecoStaticActors.clear();

    m_pGameInstance->ReleaseActor(m_pStaticActor);

    Safe_Release(m_pOcTree);
    Safe_Release(m_pBlendMap);

    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pNonAnimShaderCom);
    Safe_Release(m_pAnimShaderCom);
    Safe_Release(m_pModelCom);
}