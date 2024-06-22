#include "stdafx.h"
#include "MapToolObject.h"
#include "BasicMap.h"
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

    wstring wstrModelTag = GameObjectDesc.wstrModelName;


    if (wstrModelTag != TEXT("Town") && wstrModelTag != TEXT("LbLastBossStage") && wstrModelTag != TEXT("TownShop")
        && wstrModelTag.substr(wstrModelTag.length() - 5) == TEXT("Blend"))
    {
        m_bBlendMap = true;
        m_eRenderGroup = CRenderer::RENDER_BLEND;
    }

    if (FAILED(Add_Components(wstrModelTag)))
        return E_FAIL;

    SetUpShaderInfo(wstrModelTag);

    m_vecConstantNames = { "g_DiffuseTexture", "g_NormalTexture", "g_MRATexture", "g_fSamplingFactor"
    , "g_bStencil", "g_bRimLight", "m_fRimWidth", "g_bMotionBlur", "g_BoneMatrices" };
    m_vecStencilRimLightMotionBlurNames = { "g_bStencil", "g_bRimLight", "m_fRimWidth", "g_bMotionBlur" };

    if(wstrModelTag != TEXT("Town") && wstrModelTag != TEXT("LbLastBossStage") && wstrModelTag != TEXT("TownShop")
        && false == m_bBlendMap)
    {
        if (true == CheckIfBlendMapExists(GameObjectDesc.wstrModelName)) {
            if (FAILED(Add_BlendMap(wstrModelTag)))
                return E_FAIL;
        }

        m_pOcTree = m_pModelCom->Create_OcTree(GameObjectDesc.vMin, GameObjectDesc.vMax, m_vecPassIndices, m_vecSamplingFactors, m_vecConstantNames);
        
        // --- ModelName -- TriggerRadius -- IdleIndex & Speed -- ActionIndex & Speed ----------
        SetUpAnimDecoInfo("BushL", 1.5f, 2, 60.f, 0, 50.f);
        SetUpAnimDecoInfo("BushM", 1.1f, 2, 60.f, 0, 50.f);
        SetUpAnimDecoInfo("BushS", 0.8f, 2, 60.f, 0, 50.f);
        SetUpAnimDecoInfo("PopFlower", 0.8f, 2, 120.f, 1, 130.f);

        InsertMapDecos();
    }
    if (wstrModelTag == TEXT("Town") || wstrModelTag == TEXT("TownShop")|| wstrModelTag == TEXT("LbLastBossStage")) {
        if(LEVEL_TOOL_MAP != *m_pCurrentLevelID)
            ReadDecos_ForSmallLevels();
    }
        

    if (FAILED(m_pModelCom->CreateStaticActor(GameObjectDesc.matWorld)))
        return E_FAIL;

    m_fTime = m_fNonMatchTime = 100.f;

    return S_OK;
}

_int CBasicMap::Tick(_float fTimeDelta)
{
    if (true == m_bDead)
        return OBJ_DEAD;

    m_fTime += fTimeDelta;

    if (m_pGameInstance->Get_KeyState(DIK_Q, KEY_DOWN))
        m_bCull = !m_bCull;
       
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
    if ((LEVEL_GAMEPLAY == *m_pCurrentLevelID || LEVEL_INTRO == *m_pCurrentLevelID
        || LEVEL_RACING == *m_pCurrentLevelID) && m_pOcTree != nullptr)
    {
        if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fNonMatchTime, sizeof(_float))))
            return E_FAIL;
        _float fWhiteColorDiffuse = 0;
        if (FAILED(m_pNonAnimShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
            return E_FAIL;
        if (FAILED(m_pAnimShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
            return E_FAIL;

        m_iRenderAll = m_iRenderMyMesh = 0;
        m_pOcTree->Culling(m_pGameInstance, m_pShaderCom, m_pNonAnimShaderCom, m_pAnimShaderCom
            , m_iRenderAll, m_iRenderMyMesh);
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
    /*HRESULT hr;
    static _bool bRabbit = false;
    static _bool bCow = false;
    if (ImGui::Checkbox("rabbit", &bRabbit))
    {
        hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, L"Layer_Monster", TEXT("Prototype_GameObject_Rabbit"));
        CHECK_FAILED(hr);
    }
    if (ImGui::Checkbox("cow", &bCow))
    {
        hr = m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, L"Layer_Monster", TEXT("Prototype_GameObject_Buffahorn"));

        CHECK_FAILED(hr);
    }*/

    ImGui::Text("Octrees: %d", m_pGameInstance->Get_NumOctree());
    ImGui::Text("RenderAll: %d", m_iRenderAll);
    ImGui::Text("RenderMyMesh: %d", m_iRenderMyMesh);
}
#endif

HRESULT CBasicMap::Add_Components(const wstring& _wstrModelTag)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
        TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    if (*m_pCurrentLevelID != LEVEL_TOOL_MAP)
    {
        /* For.Com_Shader */
        if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
            TEXT("Com_Shader_NonAnim"), (CComponent**)&m_pNonAnimShaderCom)))
            return E_FAIL;

        /* For.Com_Shader */
        if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
            TEXT("Com_Shader_Anim"), (CComponent**)&m_pAnimShaderCom)))
            return E_FAIL;

        /* For.Com_Texture */
        HRESULT hr = __super::Add_Component(TEXT("Prototype_Component_Texture_FX_Mask_Bubble2"),
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
        //MSG_BOX(wstrError.c_str());
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

        pModel->SetUpStencilRimLightMotionBlurPassIndex(iShaderVars, fRimWidth, iPassIndex);

        PxRigidStatic* pRigidStatic = { nullptr };
        pModel->Set_WorldMatrixForOctree(matWorld);
 
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
    PxMaterial* pMtrl = m_pGameInstance->Get_Physics()->createMaterial(0.5f, 0.5f, 0.6f);

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
    m_vecShapes.emplace_back(pShape);
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
    else if (LEVEL_PARTTIME == *m_pCurrentLevelID)
        strLevel = "PartTime";
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
        if (LEVEL_TOWN == *m_pCurrentLevelID || LEVEL_PARTTIME == *m_pCurrentLevelID)
            strFolder = string("TownDeco/");
        else if (LEVEL_FINALBOSS == *m_pCurrentLevelID)
            strFolder = string("LabDiscovera_Deco/");



        if (CMapToolObject::MAPOBJ_ANIM == iMapObjType)
        {
            eType = TYPE_ANIM;
        }

        CModel* pModel = CModel::Create(m_pDevice, m_pContext, MODEL{ strModelName, eType, 1.f, 0.f, 0, strFolder, false });

        if (nullptr == pModel) {
            fileInput.close();
            return;
        }

        pModel->SetUpStencilRimLightMotionBlurPassIndex(iShaderVars, fRimWidth, iPassIndex);

        PxRigidStatic* pRigidStatic = { nullptr };
        pModel->Set_WorldMatrixForOctree(matWorld);

        if (CMapToolObject::MAPOBJ_NONCOL == iMapObjType)
        {
            m_vecNonAnimDecos.push_back(pModel);
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
            m_vecAnimDecos.push_back(pModel);
        }
        else if (CMapToolObject::MAPOBJ_ACTOR == iMapObjType)
        {
            pModel->CreateStaticActor(matWorld);
            m_vecNonAnimDecos.push_back(pModel);
        }

    }

    fileInput.close();
}

void CBasicMap::Release_MapDecos()
{
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

        for (_uint i = 0; i < iNumMeshes; i++) {
            if (FAILED(nonAnim->Bind_ShaderResource(m_pNonAnimShaderCom, m_vecConstantNames[0].c_str(), i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(nonAnim->Bind_ShaderResource(m_pNonAnimShaderCom, m_vecConstantNames[1].c_str(), i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(nonAnim->Bind_ShaderResource(m_pNonAnimShaderCom, m_vecConstantNames[2].c_str(), i, TextureType_METALNESS)))
                return E_FAIL;
            if (FAILED(m_pNonAnimShaderCom->Begin(nonAnim->Get_ModelPassIndex())))
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

        for (_uint i = 0; i < iNumMeshes; i++) {
            if (FAILED(animDeco->Bind_ShaderResource(m_pAnimShaderCom, m_vecConstantNames[0].c_str(), i, TextureType_DIFFUSE)))
                return E_FAIL;
            if (FAILED(animDeco->Bind_ShaderResource(m_pAnimShaderCom, m_vecConstantNames[1].c_str(), i, TextureType_NORMALS)))
                return E_FAIL;
            if (FAILED(animDeco->Bind_ShaderResource(m_pAnimShaderCom, m_vecConstantNames[2].c_str(), i, TextureType_METALNESS)))
                return E_FAIL;
            if (FAILED(animDeco->Bind_BoneMatrices(m_pAnimShaderCom, m_vecConstantNames[8].c_str(), i)))
                return E_FAIL;
            if (FAILED(m_pAnimShaderCom->Begin(animDeco->Get_ModelPassIndex())))
                return E_FAIL;
            if (FAILED(animDeco->Render(i)))
                return E_FAIL;
        }
    }

    return S_OK;
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
    for (_uint iActorIdx = 0; iActorIdx < m_vecAnimDecoTriggersActors.size(); iActorIdx++)
    {
        PxRigidStatic* pActor = m_vecAnimDecoTriggersActors[iActorIdx];
        if (nullptr != pActor)
        {
            auto pScene = pActor->getScene();
            pScene->removeActor(*pActor);

            PxShape* pShape = m_vecShapes[iActorIdx];
            pActor->detachShape(*pShape);
            pShape->release();
            pShape = nullptr;

            pActor->release();
            pActor = nullptr;
        }
    }
    m_vecAnimDecoTriggersActors.clear();
    m_vecShapes.clear();
        
    Safe_Release(m_pOcTree);
    Safe_Release(m_pBlendMap);

    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pNonAnimShaderCom);
    Safe_Release(m_pAnimShaderCom);
}