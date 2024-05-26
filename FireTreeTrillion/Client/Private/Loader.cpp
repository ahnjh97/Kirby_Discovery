#include "stdafx.h"
#include "..\Public\Loader.h"
#include <process.h>
#include <codecvt>
#include <locale>

#include "GameInstance.h"
#include "Camera_Free.h"
#include "BackGround.h"
#include "TestModel.h"
#include "TestTerrain.h"

//맵툴
#include "MapToolHelper.h"
#include "MapToolObject.h"
#include "BasicMap.h"
#include "Grid.h"

#pragma region TOO_UI

#include "TestUI.h"
#include "UI_Editor.h"

#pragma endregion

#include "RigidBody.h"
#include "CharacterController.h"


//이펙트 툴
#include "FXToolDirector.h"
#include "SingleEffect.h"
#include "MultiEffect.h"

//#include "Body_Player.h"
//#include "Weapon.h"
//#include "Player.h"
#include "Kirby.h"

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

_uint APIENTRY LoadingMain(void* pArg)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	/* 로더에게 지정된 레벨을 준비해라*/
	CLoader* pLoader = (CLoader*)pArg;

	if (FAILED(pLoader->Start()))
		return 1;

	CoUninitialize();

	return 0;
}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_Critical_Section);

	/* 스레드를 생성하낟. */
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;


	return S_OK;
}

HRESULT CLoader::Start()
{
	EnterCriticalSection(&m_Critical_Section);

	HRESULT		hr = { 0 };
	SetUp_ModelScaleRotation(m_eNextLevelID);
	switch (m_eNextLevelID)
	{
	case LEVEL_LOGO:
	{
		hr = Loading_ObjectAll();
		CHECK_FAILED(hr);
		hr = Loading_For_Logo();
		break;
	}
	case LEVEL_GAMEPLAY:
		hr = Loading_For_GamePlay();
		break;

	// 05.20) UI Tool 레벨 추가
	case LEVEL_TOOL_UI:
		hr = Loading_For_Tool_UI();
		break;

	case LEVEL_TOOL_FX:
	{
		hr = Loading_For_Tool_FX();
		break;
	}

	case LEVEL_TOOL_ANIM:
	{
		hr = Loading_For_Tool_Anim();
		break;
	}

	case LEVEL_TOOL_MAP:
	{
		hr = Loading_For_Tool_Map();
		break;
	}

	}
	if (FAILED(hr))
		return E_FAIL;

	LeaveCriticalSection(&m_Critical_Section);

	return S_OK;
}

/// <summary> 게임에서 필요한 오브젝트 프로토타입을 추가한다. </summary>
HRESULT CLoader::Loading_ObjectAll()
{
	m_strLoadingText = TEXT("객체의 원형를(을) 로딩 중 입니다.");
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BackGround"), CBackGround);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_Test"), CTestUI);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestMap"), CTestTerrain);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestModel"), CTestModel);

	//이펙트 툴 용
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("FXToolDirector"), CFXToolDirector);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CSingleEffect"), CSingleEffect);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("CMultiEffect"), CMultiEffect);

	// MapTool GameObject Prototypes
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Grid"), CGrid);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BasicMap"), CBasicMap);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("MapToolHelper"), CMapToolHelper);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("MapToolObject"), CMapToolObject);

	// 05.20) 원본 추가
	 /*      GameObject_IMGUI_UI_Editor    */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("IMGUI_UI_Editor"), CUI_Editor);

	// For Kirby
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kirby"), CKirby);

	return S_OK;
}

HRESULT CLoader::Loading_For_Logo()
{
	LEVEL eLevel = LEVEL_LOGO;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	/* For.Prototype_Component_Texture_Logo */
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_GamePlay()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_GAMEPLAY;
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;

	// 커비 얼굴 텍스쳐 로드
	Add_KirbyFaceTexture(eLevel);


	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	// 모아놓은 Model 한번에 생성.
	hr = Add_Models(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	/* 리지드바디 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);
	/* 캐릭터 컨트롤러 */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_CharacterController"), CCharacterController::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_FX()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOOL_FX;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	//if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
	//	return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	// 모아놓은 Model 한번에 생성.
	//hr = Add_Models(eLevel);
	//CHECK_FAILED(hr);
	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	// 리지드바디
	//hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	//CHECK_FAILED(hr);

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_Anim()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOOL_ANIM;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	//if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
	//	return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	// 모아놓은 Model 한번에 생성.
	//hr = Add_Models(eLevel);
	//CHECK_FAILED(hr);
	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	// 리지드바디
	//hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	//CHECK_FAILED(hr);

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_Map()
{
	HRESULT hr = S_OK;
	LEVEL eLevel = LEVEL_TOOL_MAP;

	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("VI버퍼(을) 로딩 중 입니다.");
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Grid"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 200, 200))))
		return E_FAIL;

	m_strLoadingText = TEXT("쉐이더(을) 로딩 중 입니다.");
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("모델(을) 로딩 중 입니다.");
	if(FAILED(Add_AllModelTxts(eLevel, TYPE_NONANIM)))
		return E_FAIL;
	
	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Tool_UI()
{
	LEVEL eLevel = LEVEL_TOOL_UI;
	HRESULT hr = S_OK;

#pragma region TEXTURE

	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;

	m_strLoadingText = TEXT("Loading For Texture : Complete!");

#pragma endregion

	m_strLoadingText = TEXT("Loading For UITool : Complete!");

	m_IsFinished = TRUE;
	return S_OK;
}

HRESULT CLoader::Add_Models(LEVEL eLevel)
{
	// SetUp_ModelScaleRotation 함수에서 모아놓은 Model들을 타입에 따라서 Component 생성한다.
	for (auto& ModelInfo : m_vecModelInfo)
	{

		if (ModelInfo.strModelName == "KirbyDefault")
		{
			_int i = 0;
		}

		wstring wstrModelName = CUtils::StrToWstr(ModelInfo.strModelName);
		wstring wstrPrototypeTag = L"Prototype_Component_Model_" + wstrModelName;

		if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, ModelInfo))))
			return E_FAIL;
	}

	return S_OK;
}

// 여기다가 모든 Model을 셋업한다.
void CLoader::SetUp_ModelScaleRotation(LEVEL eLevel)
{
	// MODEL 구조체 생성자 순서		: 이름 (파일이름) / ANIMTYPE / Scale / Degree (Y) / Root
	// MODEL 구조체 생성자 기본 값  : ""			  / TYPE_END /  1.f  /    0.f     / 4
	if (eLevel == LEVEL_LOGO)
	{

	}
	else if (eLevel == LEVEL_GAMEPLAY)
	{
		m_vecModelInfo.emplace_back("Fiona", TYPE_ANIM );
		m_vecModelInfo.emplace_back("Dee", TYPE_ANIM, 0.01f);
		m_vecModelInfo.emplace_back("Kirby", TYPE_ANIM);

		m_vecModelInfo.emplace_back("TestMap", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("TestMap2", TYPE_NONANIM, 0.01f);

		// For Kirby Body
		m_vecModelInfo.emplace_back("KirbyBalloon", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyDefault", TYPE_ANIM, 1.f, 180.f);
		m_vecModelInfo.emplace_back("KirbyVacuum", TYPE_ANIM, 1.f, 180.f);

		m_vecModelInfo.emplace_back("GsBenchAL", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Level0Stage1Step01", TYPE_NONANIM);
		m_vecModelInfo.emplace_back("Level1Stage1Step01", TYPE_NONANIM);
	}
	else if (eLevel == LEVEL_TOOL_MAP) 
	{		
		// 맵툴에서는 크기나 회전 상태 바꾸고 싶은 모델만 여기에 등록. 안바꾸고싶으면 NonAnim, 크기1, 회전 0도로 자동 추가됨
		m_vecModelInfo.emplace_back("Book", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("TestMap2", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("Trigger", TYPE_NONANIM, 0.01f);
		m_vecModelInfo.emplace_back("Camera", TYPE_NONANIM, 0.2f , 270.f);
	}
}

HRESULT CLoader::Add_Shaders(LEVEL eLevel)
{
	HRESULT hr = S_OK;
	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxCube */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxCube"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxCube.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxInstance_Rect */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxInstance_Rect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Rect.hlsl"), VTXINSTANCE_RECT::Elements, VTXINSTANCE_RECT::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxInstance_Point */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Point.hlsl"), VTXINSTANCE_POINT::Elements, VTXINSTANCE_POINT::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel_Map */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel_Map"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel_Map.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	//이펙트용 쉐이더

	/* For.Prototype_Component_Shader_FXPosTex */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_FXPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
	CHECK_FAILED(hr);

	/* For.Prototype_Component_Shader_FXModel */
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_FXModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CLoader::Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures)
{
	wstring wstrPrototypeTag = L"Prototype_Component_Texture_" + CUtils::StrToWstr(strPrototypeName); 
	wstring wstrFullPath = L"../Bin/Resources/Textures/" + CUtils::StrToWstr(strFolderAndFileName);

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CTexture::Create(m_pDevice, m_pContext, wstrFullPath, iNumTextures))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Add_KirbyFaceTexture(LEVEL eLevel)
{
	if (FAILED(Add_Texture(eLevel, "anger", "KirbyFace/anger.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "blink", "KirbyFace/blink.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "close", "KirbyFace/close.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "doubt", "KirbyFace/doubt.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "idle", "KirbyFace/idle.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "pupil", "KirbyFace/pupil.png")))
		return E_FAIL;


	if (FAILED(Add_Texture(eLevel, "mouth_anger", "KirbyFace/mouth_anger.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_base", "KirbyFace/mouth_base.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_happy", "KirbyFace/mouth_happy.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_smile", "KirbyFace/mouth_smile.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "mouth_surprise", "KirbyFace/mouth_surprise.png")))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Add_AllModelTxts(LEVEL eLevel, TYPE eType)
{
	HRESULT hr = S_OK;

	wstring wstrRootFolderPath = TEXT("../../../model_txt/");
	if (TYPE_ANIM == eType)
		wstrRootFolderPath += TEXT("Anim/");
	else if (TYPE_NONANIM == eType)
		wstrRootFolderPath += TEXT("NonAnim/");

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((wstrRootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return E_FAIL;
	}

	list<wstring> txtList;
	TraverseModelTxts(wstrRootFolderPath, txtList);

	for (auto listIter : txtList)
	{
		wstring wstrModelName = listIter.substr(0, listIter.length() - 4);
		string strModelName = CUtils::WstrToStr(wstrModelName);
		
		_bool bFound = { false };
		MODEL tModelInfo = MODEL{ strModelName ,  TYPE_NONANIM };
		for (auto& modelInfo : m_vecModelInfo)
		{
			if (modelInfo.strModelName == strModelName)
			{
				tModelInfo = modelInfo;
				break;
			}
		}

		wstring wstrPrototypeTag = TEXT("Prototype_Component_Model_") + CUtils::StrToWstr(tModelInfo.strModelName);
		hr = m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CModel::Create(m_pDevice, m_pContext, tModelInfo));
		CHECK_FAILED(hr);

		if (FAILED(hr))
			return E_FAIL;
	}

	FindClose(hFind);

 	return S_OK;
}

void CLoader::TraverseModelTxts(const wstring& rootFolderPath, list<wstring>& fileList)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((rootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return;
	}

	do {
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
				// 재귀적으로 하위 폴더도 순회
				TraverseModelTxts(rootFolderPath + L"\\" + findFileData.cFileName, fileList);
			}
		}
		else {
			// 파일이면 리스트에 추가
			fileList.push_back(wstring(findFileData.cFileName));
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}

CLoader * CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLoader* pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Created : CLoader"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoader::Free()
{
	WaitForSingleObject(m_hThread, INFINITE);

	DeleteObject(m_hThread);

	CloseHandle(m_hThread);

	DeleteCriticalSection(&m_Critical_Section);

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

