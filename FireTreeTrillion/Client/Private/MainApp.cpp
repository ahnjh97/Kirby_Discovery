#include "stdafx.h"
#include "MainApp.h"

#include "GameInstance.h"
#include "Level_Loading.h"
#include "LevelChanger.h"

#include "tinyxml2.h"
#include "Utils.h"
#include "SingleEffect.h"
#include "Particle.h"
#include "MultiEffect.h"

#include "PartTimeHelper.h"
#include "CollisionCenter.h"
#include "EventCenter.h"

CMainApp::CMainApp() : m_pGameInstance(CGameInstance::Get_Instance())
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

	/* �� ������ ���� �ʱ�ȭ ������ ��ġ��. */
	if (FAILED(m_pGameInstance->Initialize_Engine(g_hInst, LEVEL_END, EngineDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;

	if (FAILED(Ready_Fonts()))
		return E_FAIL;

	//loader�� Loading_StaticComponentAll() �� �ű�
	if (FAILED(Ready_Prototype_Component_For_Static()))
		return E_FAIL;

	if (FAILED(Open_Level(LEVEL_LOGO)))
		return E_FAIL;

	CCollisionCenter::Get_Instance()->Initialize();
	CEventCenter::Get_Instance()->Initialize();

	return S_OK;
}

void CMainApp::Tick(_float fTimeDelta)
{
	m_pGameInstance->Tick_Engine(fTimeDelta);

	CCollisionCenter::Get_Instance()->Collision_Tick(fTimeDelta);

	if (m_pGameInstance->Get_IsReadyGameStart())
	{
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_LALT, KEY_PRESS))
	{
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_GRAVE, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_INTRO))))
				return;
		}

		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_1, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_TOWN))))
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
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_6, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_RACING))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_7, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_PARTTIME))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_Q, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_FINALBOSS))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_W, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_DEEDEEDEE))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_A, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_GAMEPLAY))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_8, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_FINALE))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_S, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_SIMBA))))
				return;
		}
		if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_0, KEY_DOWN))
		{
			if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_PARK))))
				return;
		}
	}
	}

	m_pGameInstance->LateTick_Engine(fTimeDelta);
}

HRESULT CMainApp::Render(_float fTimeDelta)
{
	if (nullptr == m_pGameInstance)
		return E_FAIL;

	m_pGameInstance->Begin_Draw(_float4(0.5f, 0.f, 1.f, 1.f));

	m_pGameInstance->Draw(fTimeDelta);

#ifdef _DEBUG

	if (m_pGameInstance->Get_IsRenderRTV())
		Render_RTVFonts();

#endif // _DEBUG


	m_pGameInstance->End_Draw();

	return	S_OK;
}

HRESULT CMainApp::Ready_Fonts()
{
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_HUDSub_EN10"),
		TEXT("../Bin/Resources/Fonts/HUD_Sub_EN10.spritefont"))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_HUDSub_KR15"),
		TEXT("../Bin/Resources/Fonts/HUD_Sub_KR15.SpriteFont"))))
		return E_FAIL;

	// [공통 Flag]
	// /CharacterRegion: 0x0030-0x0039(숫자) 0xAC00-0xD7AF(한글) 0x0020-0x007E(영어 + 특문)

	// FOT-Seurat Pro EB :: StarPoint 폰트
	// Fontsize 30
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_HUD_StarPoint_NUM30"),
		TEXT("../Bin/Resources/Fonts/HUD_StarPoint_NUM30.spritefont"))))
		return E_FAIL;
	
	// a대한늬우스L, a대한늬우스M :: 보스 대사 폰트
	// /Fontsize: 22 /LineSpacing: 10
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_Dialog_Boss_KR22spac10"),
		TEXT("../Bin/Resources/Fonts/Dialog_Boss_KR22spac10.spritefont"))))
		return E_FAIL;

	// a신고딕 :: 일반 다이얼로그 대사 폰트
	// /Fontsize: 20 /LineSpacing: 10
	//if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_Dialog_KR18spac10"),
	//	TEXT("../Bin/Resources/Fonts/Dialog_KR18spac10.spritefont"))))
	//	return E_FAIL;
	
	// a중고딕 KoreanGDB
	// /Fontsize: 22 /LineSpacing: 10
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_KoreanGDB_KR22spac10"),
		TEXT("../Bin/Resources/Fonts/KoreanGDB_KR22spac10.spritefont"))))
		return E_FAIL;

	// a태고딕 KoreanGDE
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_KoreanGDE_KR22spac10"),
		TEXT("../Bin/Resources/Fonts/KoreanGDE_KR22spac10.spritefont"))))
		return E_FAIL;


	// a둥근헤드 :: 일반 다이얼로그 타이틀 폰트
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_Dialog_Title_KR22spac10"),
		TEXT("../Bin/Resources/Fonts/Dialog_Title_KR22spac10.spritefont"))))
		return E_FAIL;

	// a자막체 :: 페르소나5 로얄의 잔재... 
	if (FAILED(m_pGameInstance->Add_Font(m_pDevice, m_pContext, TEXT("Font_Dialog_SubKR18spac10"),
		TEXT("../Bin/Resources/Fonts/Dialog_SubKR18spac10.spritefont"))))
		return E_FAIL;


	return S_OK;
}

HRESULT CMainApp::Ready_Object_For_Static()
{
	HRESULT hr(S_OK);


	return S_OK;
}

HRESULT CMainApp::Open_Level(LEVEL eLevelID)
{
	if (LEVEL_LOADING == eLevelID)
		return E_FAIL;

	m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, eLevelID));

	return	S_OK;
}

HRESULT CMainApp::Ready_Prototype_Component_For_Static()
{

	HRESULT hr;

	
	path FXPath("../Bin/Resources/Effects/Single/");
	if (!exists(FXPath) || !is_directory(FXPath))
	{
		ALARM_FAIL("싱글이펙트 로드 망함");
		return E_FAIL;
	}

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
		FXDesc.eTimer = FXData.eTimer;
		FXDesc.vContinuousRotation = FXData.vContinuousRotation;

		for (_uint i = 0; i < FXData.iPropertyMapNum; ++i)
		{
			FXDesc.Keyframes.emplace(FXData.vecKeyframeInfo[i].first, FXData.vecKeyframes[i]);
		}


		wstring wstrProtoName = { TEXT("Prototype_GameObject_") + CUtils::StrToWstr(strname) };

		hr = m_pGameInstance->Add_Prototype(wstrProtoName, CSingleEffect::Create(m_pDevice, m_pContext, FXDesc));
		CHECK_FAILED(hr);


		//Make_Effect(FXData);
	}


	
	FXPath = "../Bin/Resources/Effects/Particle/";
	if (!exists(FXPath) || !is_directory(FXPath))
	{
		ALARM_FAIL(TEXT("fail"));
		return E_FAIL;
	}


	for (auto& entry : directory_iterator(FXPath))
	{
		auto& filePath = entry.path();
		string strname = filePath.stem().string();

		if (filePath.extension() != ".bin")
			continue;

		PARTICLE_DATA FXData = {};

		CUtils::Load_Effect(filePath, &FXData);

		CParticle::PARTICLE_DESC ParticleDesc{};
		INSTANCE_DESC InstanceDesc{};

		ParticleDesc.strFXName = FXData.strName;
		ParticleDesc.strBufferTag = FXData.strBufferName;
		ParticleDesc.strTexTag = FXData.strTexName;
		ParticleDesc.strMaskTexTag = FXData.strMaskTexName;

		ParticleDesc.iPassIdx = FXData.iPassIdx;
		ParticleDesc.iTexIdx = FXData.iTexIdx;
		ParticleDesc.iMaskTexIdx = FXData.iMaskTexIdx;

		ParticleDesc.iNumInstance = FXData.iNumInstance;
		ParticleDesc.eRenderGroup = FXData.eRenderGroup;
		ParticleDesc.eTimer = FXData.eTimer;

		ParticleDesc.bIsLoop = FXData.bIsLoop;
		ParticleDesc.bIsBillboard = FXData.bIsBillboard;
		ParticleDesc.bIsBloom = FXData.bIsBloom;

		ParticleDesc.fDuration = FXData.fDuration;
		ParticleDesc.fLifetime.second = FXData.fLifetime;

		InstanceDesc.vecMoveCommands = FXData.vecMoveCommands;


		InstanceDesc.fLifetime = FXData.fLifetime;
		InstanceDesc.fLifetimeRandomOffset = FXData.fLifetimeRandomOffset;
		InstanceDesc.fStartDelay = FXData.fStartDelay;
		InstanceDesc.fStarDelayRandomOffset = FXData.fStarDelayRandomOffset;
		InstanceDesc.vCenter = FXData.vCenter;

		InstanceDesc.vRange = FXData.vRange;

		InstanceDesc.fMinRange = FXData.fMinRange;
		InstanceDesc.fMaxRange = FXData.fMaxRange;

		InstanceDesc.vRotation = FXData.vRotation;
		InstanceDesc.vRotationRandomOffset = FXData.vRotationRandomOffset;
		InstanceDesc.vScale = FXData.vScale;
		InstanceDesc.vScaleRandomOffset = FXData.vScaleRandomOffset;
		InstanceDesc.vDir = FXData.vDir;
		InstanceDesc.vDirRandomOffset = FXData.vDirRandomOffset;
		InstanceDesc.fSpeed = FXData.fSpeed;
		InstanceDesc.fSpeedRandomOffset = FXData.fSpeedRandomOffset;

		InstanceDesc.fOrbitSpeed = FXData.fOrbitSpeed;
		InstanceDesc.fOrbitSpeedRandomOffset = FXData.fOrbitSpeedRandomOffset;

		InstanceDesc.fTurnSpeed = FXData.fTurnSpeed;
		InstanceDesc.fTurnSpeedRandomOffset = FXData.fTurnSpeedRandomOffset;

		InstanceDesc.fAccSupplyAmount = FXData.fAccSupplyAmount;
		InstanceDesc.fOrbitSupplyAmount = FXData.fOrbitSupplyAmount;
		InstanceDesc.fTurnSupplyAmount = FXData.fTurnSupplyAmount;

		InstanceDesc.vColor = FXData.vColor;
		InstanceDesc.vColorRandomOffset = FXData.vColorRandomOffset;
		InstanceDesc.vTargetColor = FXData.vTargetColor;
		InstanceDesc.vTargetColorRandomOffset = FXData.vTargetColorRandomOffset;
		InstanceDesc.fAlpha = FXData.fAlpha;
		InstanceDesc.fAlphaRandomOffset = FXData.fAlpha;
		InstanceDesc.vPivot = FXData.vPivot;

		InstanceDesc.vRotationAxis = FXData.vRotationAxis;

		wstring wstrProtoName = { TEXT("Prototype_GameObject_") + CUtils::StrToWstr(strname) };

		hr = m_pGameInstance->Add_Prototype(wstrProtoName, CParticle::Create(m_pDevice, m_pContext, ParticleDesc));
		CHECK_FAILED(hr);
		static_cast<CParticle*>(m_pGameInstance->Find_Prototype(wstrProtoName))->Update_InstanceInfo(&InstanceDesc);
	}
	

	FXPath = "../Bin/Resources/Effects/Multi/";
	if (!exists(FXPath) || !is_directory(FXPath))
	{
		ALARM_FAIL("multi fx load fail");
		return E_FAIL;
	}

	//���� ����Ʈ
	for (auto& entry : directory_iterator(FXPath))
	{
		auto& filePath = entry.path();
		string strname = filePath.stem().string();
		if (strname == "HS_FB fly smoke")
			_int a = 0;
		if (filePath.extension() != ".bin")
			continue;

		MULTI_FX_DATA FXData = {};
		CUtils::Load_Effect(filePath, &FXData);

		CMultiEffect::MULTI_FX_DESC FXDesc = {};

		FXDesc.strFXName = FXData.strName;
		for (auto& FXPair : FXData.FXs)
			FXDesc.FXs.push_back(FXPair.second);


		wstring wstrProtoName = { TEXT("Prototype_GameObject_") + CUtils::StrToWstr(strname) };


		hr = m_pGameInstance->Add_Prototype(wstrProtoName, CMultiEffect::Create(m_pDevice, m_pContext, FXDesc));
		CHECK_FAILED(hr);

	}

	

	return	S_OK;
}

#ifdef _DEBUG
_bool CMainApp::Render_RTVFonts()
{
	// RTV_FONT �߰�
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

	//UI
	m_pGameInstance->Render_Font(TEXT("Font_HUDSub_EN10"), TEXT("UI"),
		_float2(5.f, 5.f), XMVectorSet(1.f, 1.f, 1.f, 1.f), 0.f);

	return TRUE;
}
#endif // _DEBUG

CMainApp* CMainApp::Create()
{
	CMainApp* pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CMainApp"));

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

	CLevelChanger::Destroy_Instance();
	CPartTimeHelper::Destroy_Instance();
	CCollisionCenter::Destroy_Instance();
	CEventCenter::Destroy_Instance();
}

