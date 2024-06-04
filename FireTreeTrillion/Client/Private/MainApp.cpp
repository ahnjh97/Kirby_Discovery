#include "stdafx.h"
#include "MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"

#include "tinyxml2.h"
#include "Utils.h"
#include "SingleEffect.h"
#include "Particle.h"
#include "MultiEffect.h"


CMainApp::CMainApp()
	: m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);		

	
}


HRESULT CMainApp::Initialize()
{
	ENGINE_DESC		EngineDesc = {};

	EngineDesc.hWnd = g_hWnd;
	EngineDesc.isWindowed = true;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;

	/* 내 게임의 기초 초기화 과정을 거치자. */
	if (FAILED(m_pGameInstance->Initialize_Engine(g_hInst, LEVEL_END, EngineDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;

	if (FAILED(Ready_Fonts()))
		return E_FAIL;

	//loader의 Loading_StaticComponentAll() 로 옮김
	if (FAILED(Ready_Prototype_Component_For_Static()))
		return E_FAIL;

	if (FAILED(Open_Level(LEVEL_LOGO)))
		return E_FAIL;
	
	Create_JSON();

	return S_OK;
}

void CMainApp::Tick(_float fTimeDelta)
{
	m_pGameInstance->Tick_Engine(fTimeDelta);

	if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_LALT, KEY_PRESS))
	{
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_1, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_GAMEPLAY))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_2, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOOL_FX))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_3, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOOL_UI))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_4, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOOL_ANIM))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_5, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOOL_MAP))))
				return;
		}
	}
}

HRESULT CMainApp::Render(_float fTimeDelta)
{
	if (nullptr == m_pGameInstance)
		return E_FAIL;
	
	m_pGameInstance->Begin_Draw(_float4(0.5f, 0.f, 1.f, 1.f));

	m_pGameInstance->Draw(fTimeDelta);

#ifdef _DEBUG
	
	//렌더 타겟 뷰 ON/OFF
	if (m_pGameInstance->Get_DIKeyState(DIK_F1, KEY_DOWN))
		m_IsRenderRTV = !m_IsRenderRTV;


	// RTV_FONT 추가
#pragma region GAME_OBJ

	if (m_IsRenderRTV)
		Render_RTVFonts();

#endif // _DEBUG
	

	m_pGameInstance->End_Draw();

	return	S_OK;
}

HRESULT CMainApp::Ready_Fonts()
{
	// MakeSpriteFont "넥슨lv1고딕 Bold" /FontSize:30 /FastPack /CharacterRegion:0x0020-0x00FF /CharacterRegion:0x3131-0x3163 /CharacterRegion:0xAC00-0xD800 /DefaultCharacter:0xAC00 142.spritefont

	// 05.25) a자막체 영문 폰트 추가
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_HUDSub_EN10"), TEXT("../Bin/Resources/Fonts/HUD_Sub_EN10.spritefont"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_HUDSub_KR15"), TEXT("../Bin/Resources/Fonts/HUD_Sub_KR15.SpriteFont"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CMainApp::Open_Level(LEVEL eLevelID)
{
	if (LEVEL_LOADING == eLevelID)
		return E_FAIL;

	m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, eLevelID));

	return	S_OK;
}

//HRESULT CMainApp::Ready_Prototype_Component_For_Static()
//{
//
//	//HRESULT hr;
//
//
//	///* For.Prototype_Component_VIBuffer_Rect */
//	//if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
//	//	CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
//	//	return E_FAIL;
//
//	///* For.Prototype_Component_VIBuffer_Instance_Point */
//	//if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Instance_Point"),
//	//	CVIBuffer_Instance_Point::Create(m_pDevice, m_pContext))))
//	//	return E_FAIL;
//
//	///* For.Prototype_Component_Shader_VtxPosTex */
//	//if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
//	//	CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
//	//	return E_FAIL;
//
//	////point instance 쉐이더
//	///* For.Prototype_Component_Shader_VtxInstance_Point */
//	//if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
//	//	CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Point.hlsl"), VTXINSTANCE_POINT::Elements, VTXINSTANCE_POINT::iNumElements))))
//	//	return E_FAIL;
//
//	//wstring wstrPrototypeTag = L"Prototype_Component_FXModel_";
//
//	//hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + L"Logo",
//	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Logo/Logo.png")));
//	//CHECK_FAILED(hr);
//
//
//	//이펙트 디버깅용 이펙트 텍스쳐(FX Texture)
//	//wstrPrototypeTag = L"Prototype_Component_FXTexture_";
//
//	//hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + L"Logo",
//	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Logo/Logo.png")));
//	//CHECK_FAILED(hr);
//
//	//hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + L"Test",
//	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effects/test.png")));
//	//CHECK_FAILED(hr);
//
//	//hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + L"SimpleStar",
//	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Effects/simpleStar.png")));
//	//CHECK_FAILED(hr);
//
//	//hr = m_pGameInstance->Add_Prototype(LEVEL_STATIC, wstrPrototypeTag + L"SimpleSolid",
//	//	CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/Simple/simpleSolid_%d.png"), 2));
//	//CHECK_FAILED(hr);
//
//
//
//	//if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CTexture::Create(m_pDevice, m_pContext, wstrFullPath, iNumTextures))))
//	//	return E_FAIL;
//
//	return	S_OK;
//}

HRESULT CMainApp::Ready_Prototype_Component_For_Static()
{

	path FXPath("../Bin/Resources/Effects/Single/");
	if (!exists(FXPath) || !is_directory(FXPath))
	{
		ALARM_FAIL(TEXT("망했어 경로 없다"));
		return E_FAIL;
	}

	//단일 이펙트
	for (auto& entry : directory_iterator(FXPath))
	{
		auto& filePath = entry.path();
		string strname = filePath.stem().string();

		if (filePath.extension() != ".bin")
			continue;

		SINGLE_FX_DATA FXData = {};

		CUtils::Load_Effect(filePath, &FXData);

		CSingleEffect::FX_DESC FXDesc{};

		FXDesc.strFXName = FXData.strName;
		FXDesc.strBufferTag = FXData.strBufferName;
		FXDesc.strTexTag = FXData.strTexName;
		FXDesc.strMaskTexTag = FXData.strMaskTexName;

		FXDesc.fDuration = FXData.fDuration;
		FXDesc.fLifetime = FXData.fLifetime;

		FXDesc.iPassIdx = FXData.iPassIdx;
		FXDesc.iTexIdx = FXData.iTexIdx;
		FXDesc.iMaskTexIdx = FXData.iMaskTexIdx;

		FXDesc.bIsLoop = FXData.bIsLoop;
		FXDesc.bIsBillboard = FXData.bIsBillboard;
		FXDesc.bIsOrthographic = FXData.bIsOrthographic;
		FXDesc.bIsColorRender = FXData.bIsColorRender;
		FXDesc.bIsBloom = FXData.bIsBloom;

		FXDesc.fRimLightThreshold = FXData.fRimLightThreshold;
		FXDesc.eRenderGroup = FXData.eRenderGroup;

		for (_uint i = 0; i < FXData.iPropertyMapNum; ++i)
		{
			FXDesc.Keyframes.emplace(FXData.vecKeyframeInfo[i].first, FXData.vecKeyframes[i]);
		}


		wstring wstrProtoName = { TEXT("Prototype_GameObject_") + CUtils::StrToWstr(strname) };

		if (FAILED(m_pGameInstance->Add_Prototype(wstrProtoName, CSingleEffect::Create(m_pDevice, m_pContext, FXDesc))))
			return E_FAIL;

		
		//Make_Effect(FXData);
	}



	FXPath = "../Bin/Resources/Effects/Multi/";
	if (!exists(FXPath) || !is_directory(FXPath))
	{
		ALARM_FAIL(TEXT("망했어 경로 없다"));
		return E_FAIL;
	}

	//복합 이펙트
	for (auto& entry : directory_iterator(FXPath))
	{
		auto& filePath = entry.path();
		string strname = filePath.stem().string();

		if (filePath.extension() != ".bin")
			continue;

		MULTI_FX_DATA FXData = {};
		CUtils::Load_Effect(filePath, &FXData);
		
		CMultiEffect::MULTI_FX_DESC FXDesc = {};

		FXDesc.strFXName = FXData.strName;
		for (auto& FXPair : FXData.FXs)
			FXDesc.FXs.push_back(FXPair.second);


		wstring wstrProtoName = { TEXT("Prototype_GameObject_") + CUtils::StrToWstr(strname) };

		if (FAILED(m_pGameInstance->Add_Prototype(wstrProtoName, CMultiEffect::Create(m_pDevice, m_pContext, FXDesc))))
			return E_FAIL;

	}



	return	S_OK;
}

#ifdef _DEBUG
_bool CMainApp::Render_RTVFonts()
{
	// RTV_FONT 추가
#pragma region GAME_OBJ

	_float fRTVFont = { 100.f };
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Diffuse"),
		_float2(5.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Normal"),
		_float2(fRTVFont + 5.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Depth"),
		_float2(fRTVFont + 105.f, g_iWinSizeY - 90.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Field Depth"),
		_float2(fRTVFont + 205.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Stencil"),
		_float2(fRTVFont + 305.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("RimLight"),
		_float2(fRTVFont + 405.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

#pragma endregion

#pragma region LIGHT_ACC

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Shade"),
		_float2(fRTVFont + 555.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Specular"),
		_float2(fRTVFont + 655.f, g_iWinSizeY - 90.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

#pragma endregion

	//SHADOW_OBJ
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("LightDepth"),
		_float2(5.f, g_iWinSizeY - 190.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), 0.f);

#pragma region BLOOM_BLUR

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Blur_X"),
		_float2(fRTVFont + 55.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Blur_Y"),
		_float2(fRTVFont + 155.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Effect"),
		_float2(fRTVFont + 255.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

#pragma endregion

	//SKY
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Sky"),
		_float2(fRTVFont + 405.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	//Radial Blur
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("RadialBlur"),
		_float2(fRTVFont + 555.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("Blend"),
		_float2(fRTVFont + 655.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("NonLight"),
		_float2(fRTVFont + 755.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("DOFBlur"),
		_float2(fRTVFont + 855.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("D-MotionBlur"),
		_float2(fRTVFont + 955.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("MotionBlur"),
		_float2(fRTVFont + 1055.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("M.R.A"),
		_float2(fRTVFont + 1155.f, g_iWinSizeY - 190.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	return TRUE;
}
#endif // _DEBUG
/// <summary>
/// 1. 내가 저장하고자 하는 원소들의 개수(iCnt)를 저장한다.
/// 2. 저장하고자하는 Element들(현 예시 4개)을 원하는 경로(strPath)에 저장하는 함수
/// 3. 추가해야하는 헤더파일 #include "tinyxml2.h", #include "Utils.h"
/// </summary>
void CMainApp::CreateXML()
{
	string strPath = "../Bin/Resources/Data/ItemInfo.xml";

	tinyxml2::XMLDocument	m_xmlDocument;
	tinyxml2::XMLNode* m_pNode;

	m_pNode = m_xmlDocument.NewElement("Root");
	m_xmlDocument.InsertFirstChild(m_pNode);

	tinyxml2::XMLElement* m_pElement = m_xmlDocument.NewElement("Version");
	m_pElement->SetText(240405);
	m_pNode->InsertEndChild(m_pElement);

	m_pElement = m_xmlDocument.NewElement("Count");
	_uint iCnt = 1;								 // 저장하고자 하는 data들의 개수 : m_vecItemInfo.size();
	m_pElement->SetText(iCnt);
	m_pNode->InsertEndChild(m_pElement);

	for (_uint i = 0; i < iCnt; ++i)
	{
		string strData = "Data" + to_string(i);
		m_pElement = m_xmlDocument.NewElement(strData.c_str());

		string strTemp = "test";				// 저장하고자 하는 data 1 : m_vecItemInfo[i].strName;
		m_pElement->SetAttribute("Name", strTemp.c_str());

		wstring wstrTemp = L"test2";			// 저장하고자 하는 data 2 : m_vecItemInfo[i].wstrItemObj_ProtoTAG;
		strTemp = CUtils::WstrToStr(wstrTemp);
		m_pElement->SetAttribute("ItemObj_ProtoTAG", strTemp.c_str());

		wstrTemp = L"test3";					// 저장하고자 하는 data 3 : m_vecItemInfo[i].wstrUITexture_ProtoTAG;
		strTemp = CUtils::WstrToStr(wstrTemp);
		m_pElement->SetAttribute("UITexture_ProtoTAG", strTemp.c_str());

		_int itemCnt = 1;						// 저장하고자 하는 data 4 : m_vecItemInfo[i].iEA;
		m_pElement->SetText(itemCnt);

		m_pNode->InsertEndChild(m_pElement);
	}

	tinyxml2::XMLError error = m_xmlDocument.SaveFile(strPath.c_str());
}

/// <summary>
/// 1. 내가 불러오고자 하는 원소들의 개수(iCnt)를 불러온다.
/// 2. 예시코드엔 불러오는 데이터들을 구조체담아서 vector 컨테이너에 저장한 형태입니다.
/// 3. 추가해야하는 헤더파일 #include "tinyxml2.h", #include "Utils.h"
/// </summary>
void CMainApp::Read_XML()
{
	tinyxml2::XMLDocument	m_xmlDocument;
	tinyxml2::XMLNode* m_pNode;
	tinyxml2::XMLElement* m_pElement;

	tinyxml2::XMLError error = m_xmlDocument.LoadFile("../Bin/Resources/Data/ItemInfo.xml");
	m_pNode = m_xmlDocument.FirstChild();

	_int	iRead, iCnt/*, iEA*/;

	m_pElement = m_pNode->FirstChildElement("Version");
	m_pElement->QueryIntText(&iRead);
	m_pElement = m_pNode->FirstChildElement("Count");
	m_pElement->QueryIntText(&iCnt);

	for (_int i = 0; i < iCnt; ++i)
	{
		string strData = "Data" + to_string(i);
		m_pElement = m_pNode->FirstChildElement(strData.c_str());

		// 읽어들이는 data들을 구조체에 담아서 사용
		//ITEM_INFO itemInfo{};
		//itemInfo.strName = m_pElement->Attribute("Name");
		//itemInfo.wstrItemObj_ProtoTAG = CUtils::StrToWstr(m_pElement->Attribute("ItemObj_ProtoTAG"));
		//itemInfo.wstrUITexture_ProtoTAG = CUtils::StrToWstr(m_pElement->Attribute("UITexture_ProtoTAG"));
		//m_pElement->QueryIntText(&iEA);
		//itemInfo.iEA = iEA;

		// 구조체들을 관리하는 컨테이너에 담기
		//m_vecItemInfo.push_back(itemInfo);
	}
}

/// <summary>
/// 1. 
/// 2. 추가할 #include 파일은 없습니다.
/// </summary>
void CMainApp::Create_N_ReadJSON()
{
	try
	{
		// JSON 파일을 읽기 모드로 열기 : 기본 위치는 '..\Client\Default\'임!
		ifstream ifs("../Bin/Resources/Data/example.json");
		if (!ifs.is_open())
		{
			throw runtime_error("failed to open the file");
		}

		// 파일 내용을 문자열로 읽기
		string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

		// JSON 문자열 파싱
		Document doc;
		doc.Parse(json.c_str());

		// JSON 객체 편집
		Value& title = doc["title"];
		title.SetString("New Title");

		Value& email = doc["authors"][0]["email"];
		email.SetString("newemail@example.com");

		// JSON 문자열 생성
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		doc.Accept(writer);

		// 파일에 JSON 문자열 쓰기
		ofstream ofs("new_example.json");
		ofs << buffer.GetString();

		ofs << endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	}
}

void CMainApp::Create_JSON()
{
	try
	{
		// JSON 파일을 읽기 모드로 열기 : 기본 위치는 '..\Client\Default\'임!
		ifstream ifs("../Bin/Resources/Data/example.json");
		if (!ifs.is_open())
		{
			throw runtime_error("failed to open the file");
		}

		// 파일 내용을 문자열로 읽기
		string json((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

		// JSON 문자열 파싱
		Document doc;
		doc.Parse(json.c_str());

		// JSON 객체 편집
		Value& title = doc["title"];
		title.SetString("New Title");

		Value& email = doc["authors"][0]["email"];
		email.SetString("newemail@example.com");

		// JSON 문자열 생성
		StringBuffer buffer;
		PrettyWriter<StringBuffer> writer(buffer);
		doc.Accept(writer);

		// 파일에 JSON 문자열 쓰기
		ofstream ofs("../Bin/Resources/Data/new_example.json");
		ofs << buffer.GetString();

		ofs << endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	}
}

bool ParseJson(Document& doc, const string& jsonData)
{
	if (doc.Parse(jsonData.c_str()).HasParseError())
	{
		return false;
	}

	return doc.IsObject();

	//rapidjson::ParseResult result = doc.Parse(jsonData.c_str());
	//if (result.IsError())
	//printf( "RapidJson parse error: %s (%lu)\n", rapidjson::GetParseError_En(result.Code()), result.Offset());
	//return !result.IsError();
}

string JsonDocToString(Document& doc, bool isPretty = false)
{
	StringBuffer buffer;
	if (isPretty)
	{
		PrettyWriter<StringBuffer> writer(buffer);
		doc.Accept(writer);
	}
	else
	{
		Writer<StringBuffer> writer(buffer);
		doc.Accept(writer);
	}
	return buffer.GetString();
}

void TestJson_Parse()
{
	// 1. Parse a JSON string into DOM.
	const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
	Document doc;
	ParseJson(doc, json);

	// 2. Modify it by DOM.
	Value& s = doc["stars"];
	s.SetInt(s.GetInt() + 1);

	string jsonString = JsonDocToString(doc, true);
	printf(jsonString.c_str());
}

void TestJson_AddMember()
{
	// 1. Parse a JSON string into DOM.
	//Document doc;
	//doc.SetObject();
	Document doc(kObjectType);

	Document::AllocatorType& allocator = doc.GetAllocator();
	doc.AddMember("project", "rapidjson", allocator);
	doc.AddMember("stars", 10, allocator);

	string jsonString = JsonDocToString(doc, true);
	printf(jsonString.c_str());
}

CMainApp * CMainApp::Create()
{
	CMainApp*		pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CMainApp"));
		
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Release(m_pGameInstance);

	CGameInstance::Release_Engine();
}
