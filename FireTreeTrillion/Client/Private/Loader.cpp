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

#include "TestUI.h"
#include "UI_Editor.h"

#include "RigidBody.h"

//#include "Body_Player.h"
//#include "Weapon.h"
//#include "Player.h"

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext}
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
	CLoader*		pLoader = (CLoader*)pArg;

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
		hr = Loading_For_UITool();
		break;
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
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BackGround"),  CBackGround);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("UI_Test"), CTestUI);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestMap"), CTestTerrain);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TestModel"), CTestModel);

	// 05.20) 원본 추가
	//    /*      GameObj_IMGUI_UI_Editor    */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("IMGUI_UI_Editor"), CUI_Editor);

	///* For.Prototype_GameObject_Player */
	//if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Player"),
	//	CPlayer::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_Part_Body_Player */
	//if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Part_Body_Player"),
	//	CBody_Player::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_Part_Weapon */
	//if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Part_Weapon"),
	//	CWeapon::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

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
	HRESULT hr;
	LEVEL eLevel = LEVEL_GAMEPLAY;
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	if (FAILED(Add_Texture(eLevel, "Logo", "Logo/Logo.png")))
		return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	// 모아놓은 Model 한번에 생성.
	hr = Add_Models(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("물리 컴포넌트(을) 로딩 중 입니다.");
	// 리지드바디
	hr = m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_RigidBody"), CRigidBody::Create(m_pDevice, m_pContext));
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	// 모아놓은 Shaders 한번에 생성
	hr = Add_Shaders(eLevel);
	CHECK_FAILED(hr);

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_IsFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_UITool()
{
	LEVEL eLevel = LEVEL_TOOL_UI;
	HRESULT hr;

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
		_matrix      TransformMatrix = XMMatrixIdentity();
		TransformMatrix = XMMatrixScaling(ModelInfo.fScale, ModelInfo.fScale, ModelInfo.fScale) * XMMatrixRotationY(XMConvertToRadians(ModelInfo.fDegree));

		wstring wstrModelName = CUtils::StrToWstr(ModelInfo.strModelName);
		wstring wstrPrototypeTag = L"Prototype_Component_Model_" + wstrModelName;

		if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag,
			CModel::Create(m_pDevice, m_pContext, TransformMatrix, ModelInfo))))
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
		m_vecModelInfo.emplace_back(MODEL{ "Fiona", TYPE_ANIM });
		m_vecModelInfo.emplace_back(MODEL{ "Dee", TYPE_ANIM, 0.01f });
		m_vecModelInfo.emplace_back(MODEL{ "Kirby", TYPE_ANIM });

		m_vecModelInfo.emplace_back(MODEL{ "TestMap", TYPE_NONANIM });
		m_vecModelInfo.emplace_back(MODEL{ "TestMap2", TYPE_NONANIM, 0.01f });
	}

}

HRESULT CLoader::Add_Shaders(LEVEL eLevel)
{
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

	return S_OK;
}

HRESULT CLoader::Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring wstrPrototypeTag = L"Prototype_Component_Texture_" + converter.from_bytes(strPrototypeName);
	wstring wstrFullPath = L"../Bin/Resources/Textures/" + converter.from_bytes(strFolderAndFileName);

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CTexture::Create(m_pDevice, m_pContext, wstrFullPath, iNumTextures))))
		return E_FAIL;

	return S_OK;
}

CLoader * CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLoader*		pInstance = new CLoader(pDevice, pContext);

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

