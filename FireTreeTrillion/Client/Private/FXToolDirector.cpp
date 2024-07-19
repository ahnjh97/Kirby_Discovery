#include "stdafx.h"
#ifdef _DEBUG
#include "FXToolDirector.h"
#endif
#include "GameInstance.h"

#include "SingleEffect.h"
#include "Particle.h"
#include "MultiEffect.h"
#include "Camera_Free.h"

#define COLOR_BLUE				ImVec4(0.26f, 0.59f, 0.98f, 0.40f)
#define COLOR_ORANGE			ImVec4(1.0f, 0.5f, 0.0f, 1.0f)
#define COLOR_LIGHTYELLOW		ImVec4(1.0f, .8f, 0.0f, 1.0f)

#define COLOR_DARKPINK			ImVec4(0.6f, 0.18f, 0.37f, 1.0f)
#define COLOR_PINK				ImVec4(0.8f, 0.18f, 0.37f, 1.0f)
#define COLOR_LIGHTPINK			ImVec4(1.0f, 0.18f, 0.37f, 1.0f)

#define MULTIFX_PATH			"../Bin/Resources/Effects/Multi/"
#define SINGLEFX_PATH			"../Bin/Resources/Effects/Single/"
#define PARTICLE_PATH			"../Bin/Resources/Effects/Particle/"


static const vector<char*> s_ModelPasses = { "0 | NORMAL_0", "1 | NORMAL_X", "2 | SHADOW", "3 | SKY", "4 | BLOOM", "5 | NONBLUR"
	,"6 | TRIGGER", "7 | ALPHABLEND", "8 | DEFERREDINFO", "9 | NEARCLIP", "10 | KIRBYPART WHITEFX", "11 | MONSTERPART",
	"12 | DEFAULTFX","13 | BLENDFX_LINEARDIFFUSE", "14 | BLENDFX_CLAMPDIFFUSE",	"15 | WHITEFX_LINEARDIFFUSE", "16 | WHITEFX_CLAMPDIFFUSE"
	,"17 | MODEL_EMISSIVE_NORMAL_O", "18 | MODEL_EMISSIVE_NORMAL_X", "19 | MODEL_STAR",	"20 | MODEL_COIN"
};

static const vector<char*> s_PosTexPasses = { "0 | DEFAULT", "1 | SOLIDBLEND", "2 | BLENDFX", "3 | BLOOM", "4 | DEFAULTFX",
	"5 | BLEND_NOZTEST" ,"6 | WHITEFX", "7 | UI_MASK", "8 | UI_MASK2", "9 | SOFTFX", "10 | SOFTALPHAFX"
	, "11 | UIWHITEALPHA" , "12 | ALPHABLEND_NOTEST" , "13 | ALPHATEST_COLOR", "14 | ALPHATEST_COLOR_HORIZONTALCUT", "15 | BOSS_BARPASS_DEFAULT"
, "16 | ALPHATEST_COLOR_VERTICALCUT" , "17 | FOCUSINGPOSITION" , "18 | FADEINOUT", "19 | UIMWBASE_CLAW", "20 | QTEBASE"
, "21 | QTEPLATE" , "22 | QTEEFFECT" , "23 | SPAWNEFFECT", "24 | BLENDFX_SOFTEFFECT_X"
};

CFXToolDirector::CFXToolDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CFXToolDirector::CFXToolDirector(const CFXToolDirector& rhs)
	:CGameObject{ rhs }
{
}


//세 이펙트를 만든다.
void CFXToolDirector::Make_Effect(SINGLE_FX_DATA& _FXData)
{
	CSingleEffect::FX_DESC FXDesc{};

	FXDesc.strFXName = _FXData.strName;
	FXDesc.strBufferTag = _FXData.strBufferName;
	FXDesc.strTexTag = _FXData.strTexName;
	FXDesc.strMaskTexTag = _FXData.strMaskTexName;

	FXDesc.fDuration = _FXData.fDuration;
	FXDesc.fLifetime = _FXData.fLifetime;

	FXDesc.iPassIdx = _FXData.iPassIdx;
	FXDesc.iTexIdx = _FXData.iTexIdx;
	FXDesc.iMaskTexIdx = _FXData.iMaskTexIdx;

	FXDesc.bIsLoop = _FXData.bIsLoop;
	FXDesc.bIsBillboard = _FXData.bIsBillboard;
	FXDesc.bIsOrthographic = _FXData.bIsOrthographic;
	FXDesc.bIsColorRender = _FXData.bIsColorRender;
	FXDesc.bIsBloom = _FXData.bIsBloom;

	FXDesc.fRimLightThreshold = _FXData.fRimLightThreshold;
	FXDesc.vContinuousRotation = _FXData.vContinuousRotation;
	FXDesc.eRenderGroup = _FXData.eRenderGroup;
	FXDesc.eTimer = _FXData.eTimer;
	if (FXDesc.eTimer == TIMER_SECOND)
		int a = 0;

	for (_uint i = 0; i < _FXData.iPropertyMapNum; ++i)
	{
		FXDesc.Keyframes.emplace(_FXData.vecKeyframeInfo[i].first, _FXData.vecKeyframes[i]);
	}

	if (_FXData.iPropertyMapNum < KF_MASKUVANGLE)
	{
		//기본 세팅된 키프레임들.
		vector<FX_KEYFRAME> newProperty;
		FX_KEYFRAME newStartKeyframe{ 0.f, Vector3::Zero, EASE_LINEAR };
		FX_KEYFRAME newEndKeyframe{ 1.f, Vector3::Zero, EASE_LINEAR };

		//위치
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);

		FXDesc.Keyframes.emplace(KF_MASKUVOFFSET, newProperty);
		FXDesc.Keyframes.emplace(KF_MASKUVANGLE, newProperty);

	}

	CSingleEffect* pSingleFX = static_cast<CSingleEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_SingleEffect"), &FXDesc));
	m_FXs.emplace_back(pSingleFX);

}

void CFXToolDirector::Make_Effect(PARTICLE_DATA& _FXData)
{
	CParticle::PARTICLE_DESC ParticleDesc{};
	INSTANCE_DESC InstanceDesc{};

	ParticleDesc.strFXName = _FXData.strName;
	ParticleDesc.strBufferTag = _FXData.strBufferName;
	ParticleDesc.strTexTag = _FXData.strTexName;
	ParticleDesc.strMaskTexTag = _FXData.strMaskTexName;

	ParticleDesc.iPassIdx = _FXData.iPassIdx;
	ParticleDesc.iTexIdx = _FXData.iTexIdx;
	ParticleDesc.iMaskTexIdx = _FXData.iMaskTexIdx;

	ParticleDesc.iNumInstance = _FXData.iNumInstance;
	ParticleDesc.eRenderGroup = _FXData.eRenderGroup;
	ParticleDesc.eTimer = _FXData.eTimer;

	ParticleDesc.bIsLoop = _FXData.bIsLoop;
	ParticleDesc.bIsBillboard = _FXData.bIsBillboard;
	ParticleDesc.bIsBloom = _FXData.bIsBloom;

	ParticleDesc.fDuration = _FXData.fDuration;
	ParticleDesc.fLifetime.second = _FXData.fLifetime;

	InstanceDesc.vecMoveCommands = _FXData.vecMoveCommands;
	if (InstanceDesc.vecMoveCommands.size() < INSTANCE_END)
		InstanceDesc.vecMoveCommands.resize(INSTANCE_END);

	InstanceDesc.iNumInstance = _FXData.iNumInstance;
	InstanceDesc.fLifetime = _FXData.fLifetime;
	InstanceDesc.fLifetimeRandomOffset = _FXData.fLifetimeRandomOffset;
	InstanceDesc.fStartDelay = _FXData.fStartDelay;
	InstanceDesc.fStarDelayRandomOffset = _FXData.fStarDelayRandomOffset;
	InstanceDesc.vCenter = _FXData.vCenter;

	InstanceDesc.vRange = _FXData.vRange;
	InstanceDesc.fMinRange = _FXData.fMinRange;
	InstanceDesc.fMaxRange = _FXData.fMaxRange;


	InstanceDesc.vRotation = _FXData.vRotation;
	InstanceDesc.vRotationRandomOffset = _FXData.vRotationRandomOffset;

	InstanceDesc.vScale = _FXData.vScale;
	InstanceDesc.vScaleRandomOffset = _FXData.vScaleRandomOffset;
	InstanceDesc.vDir = _FXData.vDir;
	InstanceDesc.vDirRandomOffset = _FXData.vDirRandomOffset;
	InstanceDesc.fSpeed = _FXData.fSpeed;
	InstanceDesc.fSpeedRandomOffset = _FXData.fSpeedRandomOffset;

	InstanceDesc.fOrbitSpeed = _FXData.fOrbitSpeed;
	InstanceDesc.fOrbitSpeedRandomOffset = _FXData.fOrbitSpeedRandomOffset;

	InstanceDesc.fAccSupplyAmount = _FXData.fAccSupplyAmount;
	InstanceDesc.fTurnSupplyAmount = _FXData.fTurnSupplyAmount;

	InstanceDesc.vColor = _FXData.vColor;
	InstanceDesc.vColorRandomOffset = _FXData.vColorRandomOffset;

	InstanceDesc.vTargetColor = _FXData.vTargetColor;
	InstanceDesc.vTargetColorRandomOffset = _FXData.vTargetColorRandomOffset;

	InstanceDesc.fAlpha = _FXData.fAlpha;
	InstanceDesc.fAlphaRandomOffset = _FXData.fAlpha;
	InstanceDesc.vPivot = _FXData.vPivot;


	CParticle* pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle"), &ParticleDesc));
	m_Particles.emplace_back(pParticle);
	pParticle->Update_InstanceInfo(&InstanceDesc);

}

void CFXToolDirector::Make_Effect(MULTI_FX_DATA& _FXData)
{
	CMultiEffect::MULTI_FX_DESC FXDesc = {};

	FXDesc.strFXName = _FXData.strName;
	for (auto& FXPair : _FXData.FXs)
		FXDesc.FXs.push_back(FXPair.second);

	CMultiEffect* pMultiFX = static_cast<CMultiEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_MultiEffect"), &FXDesc));
	m_MultiFXs.emplace_back(pMultiFX);
}

//이펙트 찾기
CEffect* CFXToolDirector::Find_Effect(string strName)
{
	for (CEffect* FX : m_FXs)
	{
		if (FX != nullptr && FX->Get_Name() == strName)
			return FX;
	}

	for (CEffect* FX : m_Particles)
	{
		if (FX != nullptr && FX->Get_Name() == strName)
			return FX;
	}

	for (CEffect* FX : m_MultiFXs)
	{
		if (FX != nullptr && FX->Get_Name() == strName)
			return FX;
	}

	return nullptr;
}


//이펙트 싹 저장
HRESULT CFXToolDirector::Save_AllEffect()
{
	for (auto& FX : m_FXs)
	{
		wstring wstrName = CUtils::StrToWstr(FX->Get_Name());

		Save_Effect(FX, wstrName);
	}

	for (auto& FX : m_Particles)
	{
		wstring wstrName = CUtils::StrToWstr(FX->Get_Name());

		//파티클
		Save_Particle(FX, wstrName);
	}

	for (auto& FX : m_MultiFXs)
	{
		wstring wstrName = CUtils::StrToWstr(FX->Get_Name());

		Save_MultiEffect(FX, wstrName);
	}

	MSG_BOX(TEXT("저장 끝~"));

	return S_OK;
}

HRESULT CFXToolDirector::Save_Effect(CEffect* pEffect, const wstring& strFileName)
{

	wstring wstrExactPath = TEXT("../Bin/Resources/Effects/Single/") + strFileName + TEXT(".bin");
	ofstream OutputFile(wstrExactPath, ios::binary | ios::out);

	if (!OutputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));

	SINGLE_FX_DATA FXData{};
	pEffect->Fill_SaveData(&FXData);
	if (FXData.strName == "FlowerLeaf A")
		int a = 0;
	OutputFile.write(reinterpret_cast<const char*>(&FXData.iNameStrLen), sizeof(_uint));
	OutputFile.write(FXData.strName.c_str(), FXData.iNameStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iBufferStrLen), sizeof(_uint));
	OutputFile.write(FXData.strBufferName.c_str(), FXData.iBufferStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iTexStrLen), sizeof(_uint));
	OutputFile.write(FXData.strTexName.c_str(), FXData.iTexStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iMaskTexStrLen), sizeof(_uint));
	OutputFile.write(FXData.strMaskTexName.c_str(), FXData.iMaskTexStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fDuration), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fLifetime.first), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fLifetime.second), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iPassIdx), sizeof(_int));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.iTexIdx), sizeof(_int));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.iMaskTexIdx), sizeof(_int));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsLoop), sizeof(_bool));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsBillboard), sizeof(_bool));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsOrthographic), sizeof(_bool));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsColorRender), sizeof(_bool));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsBloom), sizeof(_bool));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fRimLightThreshold), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vContinuousRotation), sizeof(_float3));

	if (FXData.vContinuousRotation.x != 0.f)
		int a = 0;

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iPropertyMapNum), sizeof(_uint));

	for (_uint i = 0; i < FXData.iPropertyMapNum; ++i)
	{
		OutputFile.write(reinterpret_cast<const char*>(&FXData.vecKeyframeInfo[i].first), sizeof(KF_PROPERTY));
		OutputFile.write(reinterpret_cast<const char*>(&FXData.vecKeyframeInfo[i].second), sizeof(_uint));

		for (auto& KF : FXData.vecKeyframes[i])
		{
			OutputFile.write(reinterpret_cast<const char*>(&KF.fTimeRatio), sizeof(_float));
			OutputFile.write(reinterpret_cast<const char*>(&KF.vValue), sizeof(_float3));
			OutputFile.write(reinterpret_cast<const char*>(&KF.eEasing), sizeof(EASING));
		}
	}

	OutputFile.write(reinterpret_cast<const char*>(&FXData.eRenderGroup), sizeof(_uint));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.eTimer), sizeof(TIMER));


	OutputFile.close();

	return S_OK;
}

HRESULT CFXToolDirector::Save_Particle(CEffect* pEffect, const wstring& strFileName)
{
	wstring wstrExactPath = TEXT("../Bin/Resources/Effects/Particle/") + strFileName + TEXT(".bin");
	ofstream OutputFile(wstrExactPath, ios::binary | ios::out);

	if (!OutputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));

	PARTICLE_DATA FXData{};
	pEffect->Fill_SaveData(&FXData);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iNameStrLen), sizeof(_int));
	OutputFile.write(FXData.strName.c_str(), FXData.iNameStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iBufferStrLen), sizeof(_int));
	OutputFile.write(FXData.strBufferName.c_str(), FXData.iBufferStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iTexStrLen), sizeof(_int));
	OutputFile.write(FXData.strTexName.c_str(), FXData.iTexStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iMaskTexStrLen), sizeof(_int));
	OutputFile.write(FXData.strMaskTexName.c_str(), FXData.iMaskTexStrLen);


	OutputFile.write(reinterpret_cast<const char*>(&FXData.iPassIdx), sizeof(_int));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.iTexIdx), sizeof(_int));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.iMaskTexIdx), sizeof(_int));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsLoop), sizeof(_bool));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsBillboard), sizeof(_bool));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.bIsBloom), sizeof(_bool));



	OutputFile.write(reinterpret_cast<const char*>(&FXData.iNumInstance), sizeof(_int));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fDuration), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fLifetime), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fLifetimeRandomOffset), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fStartDelay), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fStarDelayRandomOffset), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vCenter), sizeof(_float3));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vRange), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fMinRange), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fMaxRange), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vRotation), sizeof(_float3));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vRotationRandomOffset), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vScale), sizeof(_float3));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vScaleRandomOffset), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vDir), sizeof(_float3));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vDirRandomOffset), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fSpeed), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fSpeedRandomOffset), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fOrbitSpeed), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fOrbitSpeedRandomOffset), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fAccSupplyAmount), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fTurnSupplyAmount), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vColor), sizeof(_float3));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vColorRandomOffset), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vTargetColor), sizeof(_float3));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.vTargetColorRandomOffset), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.fAlpha), sizeof(_float));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.fAlphaRandomOffset), sizeof(_float));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vPivot), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.vRotationAxis), sizeof(_float3));

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iMoveCommandsNum), sizeof(_int));


	for (_int i = 0; i < FXData.vecMoveCommands.size(); ++i)
	{
		_bool bTemp = FXData.vecMoveCommands[i];
		OutputFile.write(reinterpret_cast<const char*>(&bTemp), sizeof(_bool));
	}

	OutputFile.write(reinterpret_cast<const char*>(&FXData.eRenderGroup), sizeof(_int));
	OutputFile.write(reinterpret_cast<const char*>(&FXData.eTimer), sizeof(TIMER));

	OutputFile.close();

	return S_OK;
}

HRESULT CFXToolDirector::Save_MultiEffect(CEffect* pEffect, const wstring& strFileName)
{
	wstring wstrExactPath = TEXT("../Bin/Resources/Effects/Multi/") + strFileName + TEXT(".bin");
	ofstream OutputFile(wstrExactPath, ios::binary | ios::out);

	if (!OutputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));

	MULTI_FX_DATA FXData{};
	pEffect->Fill_SaveData(&FXData);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iNameStrLen), sizeof(_uint));
	OutputFile.write(FXData.strName.c_str(), FXData.iNameStrLen);

	OutputFile.write(reinterpret_cast<const char*>(&FXData.iFXsNum), sizeof(_uint));

	for (auto& FX : FXData.FXs)
	{
		OutputFile.write(reinterpret_cast<const char*>(&FX.first), sizeof(_uint));
		OutputFile.write(FX.second.c_str(), FX.second.size());
	}

	OutputFile.close();

	return S_OK;
}

//이펙트 싹 로드
HRESULT CFXToolDirector::Load_AllEffect()
{
	for (auto& fxs : m_MultiFXs)
		Safe_Release(fxs);
	m_MultiFXs.clear();

	for (auto& fxs : m_FXs)
		Safe_Release(fxs);
	m_FXs.clear();



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
		Load_Effect(filePath, &FXData);
		Make_Effect(FXData);
	}

	FXPath = "../Bin/Resources/Effects/Particle/";
	if (!exists(FXPath) || !is_directory(FXPath))
	{
		ALARM_FAIL(TEXT("망했어 경로 없다"));
		return E_FAIL;
	}

	//파티클
	for (auto& entry : directory_iterator(FXPath))
	{
		auto& filePath = entry.path();
		string strname = filePath.stem().string();

		if (filePath.extension() != ".bin")
			continue;

		PARTICLE_DATA FXData = {};
		Load_Effect(filePath, &FXData);
		Make_Effect(FXData);
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
		Load_Effect(filePath, &FXData);
		Make_Effect(FXData);
	}


	MSG_BOX(TEXT("로드 완"));


	return S_OK;
}

HRESULT CFXToolDirector::Load_Effect(path _FilePath, SINGLE_FX_DATA* _pData)
{
	ifstream InputFile(_FilePath, ios::binary | ios::in);

	if (!InputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));


	//이펙트 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iNameStrLen), sizeof(_uint));
	_pData->strName.resize(_pData->iNameStrLen);
	InputFile.read(&_pData->strName[0], _pData->iNameStrLen);


	//버퍼 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iBufferStrLen), sizeof(_uint));
	_pData->strBufferName.resize(_pData->iBufferStrLen);
	InputFile.read(&_pData->strBufferName[0], _pData->iBufferStrLen);


	//텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexStrLen), sizeof(_uint));
	_pData->strTexName.resize(_pData->iTexStrLen);
	InputFile.read(&_pData->strTexName[0], _pData->iTexStrLen);


	//마스크 텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexStrLen), sizeof(_uint));
	_pData->strMaskTexName.resize(_pData->iMaskTexStrLen);
	InputFile.read(&_pData->strMaskTexName[0], _pData->iMaskTexStrLen);

	InputFile.read(reinterpret_cast<char*>(&_pData->fDuration), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetime.first), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetime.second), sizeof(_float));


	InputFile.read(reinterpret_cast<char*>(&_pData->iPassIdx), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexIdx), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexIdx), sizeof(_int));


	InputFile.read(reinterpret_cast<char*>(&_pData->bIsLoop), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBillboard), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsOrthographic), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsColorRender), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBloom), sizeof(_bool));

	InputFile.read(reinterpret_cast<char*>(&_pData->fRimLightThreshold), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->vContinuousRotation), sizeof(_float3));
	if (_pData->vContinuousRotation.x != 0.f)
	{
		int a = 0;
	}

	InputFile.read(reinterpret_cast<char*>(&_pData->iPropertyMapNum), sizeof(_uint));

	_pData->vecKeyframeInfo.resize(_pData->iPropertyMapNum);
	_pData->vecKeyframes.resize(_pData->iPropertyMapNum);

	for (_uint i = 0; i < _pData->iPropertyMapNum; ++i)
	{
		InputFile.read(reinterpret_cast<char*>(&_pData->vecKeyframeInfo[i].first), sizeof(KF_PROPERTY));
		InputFile.read(reinterpret_cast<char*>(&_pData->vecKeyframeInfo[i].second), sizeof(_uint));

		_pData->vecKeyframes[i].resize(_pData->vecKeyframeInfo[i].second);

		for (auto& KF : _pData->vecKeyframes[i])
		{
			InputFile.read(reinterpret_cast<char*>(&KF.fTimeRatio), sizeof(_float));
			InputFile.read(reinterpret_cast<char*>(&KF.vValue), sizeof(_float3));
			InputFile.read(reinterpret_cast<char*>(&KF.eEasing), sizeof(EASING));
		}
	}

	InputFile.read(reinterpret_cast<char*>(&_pData->eRenderGroup), sizeof(_uint));
	InputFile.read(reinterpret_cast<char*>(&_pData->eTimer), sizeof(TIMER));


	return S_OK;
}

HRESULT CFXToolDirector::Load_Effect(path _FilePath, PARTICLE_DATA* _pData)
{
	ifstream InputFile(_FilePath, ios::binary | ios::in);

	if (!InputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));


	//이펙트 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iNameStrLen), sizeof(_int));
	_pData->strName.resize(_pData->iNameStrLen);
	InputFile.read(&_pData->strName[0], _pData->iNameStrLen);

	if (_pData->strName == "particle parse test")
	{
		_int a = 0;
	}

	//버퍼 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iBufferStrLen), sizeof(_int));
	_pData->strBufferName.resize(_pData->iBufferStrLen);
	InputFile.read(&_pData->strBufferName[0], _pData->iBufferStrLen);


	//텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexStrLen), sizeof(_int));
	_pData->strTexName.resize(_pData->iTexStrLen);
	InputFile.read(&_pData->strTexName[0], _pData->iTexStrLen);


	//마스크 텍스쳐 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexStrLen), sizeof(_int));
	_pData->strMaskTexName.resize(_pData->iMaskTexStrLen);
	InputFile.read(&_pData->strMaskTexName[0], _pData->iMaskTexStrLen);


	InputFile.read(reinterpret_cast<char*>(&_pData->iPassIdx), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->iTexIdx), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->iMaskTexIdx), sizeof(_int));

	InputFile.read(reinterpret_cast<char*>(&_pData->bIsLoop), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBillboard), sizeof(_bool));
	InputFile.read(reinterpret_cast<char*>(&_pData->bIsBloom), sizeof(_bool));



	InputFile.read(reinterpret_cast<char*>(&_pData->iNumInstance), sizeof(_int));

	InputFile.read(reinterpret_cast<char*>(&_pData->fDuration), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetime), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fLifetimeRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fStartDelay), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fStarDelayRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->vCenter), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vRange), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->fMinRange), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fMaxRange), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->vRotation), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vRotationRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vScale), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vScaleRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vDir), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vDirRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->fSpeed), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fSpeedRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fOrbitSpeed), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fOrbitSpeedRandomOffset), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->fAccSupplyAmount), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fTurnSupplyAmount), sizeof(_float));

	InputFile.read(reinterpret_cast<char*>(&_pData->vColor), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vColorRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vTargetColor), sizeof(_float3));
	InputFile.read(reinterpret_cast<char*>(&_pData->vTargetColorRandomOffset), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->fAlpha), sizeof(_float));
	InputFile.read(reinterpret_cast<char*>(&_pData->fAlphaRandomOffset), sizeof(_float));


	InputFile.read(reinterpret_cast<char*>(&_pData->vPivot), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->vRotationAxis), sizeof(_float3));

	InputFile.read(reinterpret_cast<char*>(&_pData->iMoveCommandsNum), sizeof(_int));
	_pData->vecMoveCommands.clear();
	_pData->vecMoveCommands.reserve(_pData->iMoveCommandsNum);


	for (_int i = 0; i < _pData->iMoveCommandsNum; ++i)
	{
		_bool bTemp = { false };
		InputFile.read(reinterpret_cast<char*>(&bTemp), sizeof(_bool));
		_pData->vecMoveCommands.emplace_back(bTemp);
	}

	InputFile.read(reinterpret_cast<char*>(&_pData->eRenderGroup), sizeof(_int));
	InputFile.read(reinterpret_cast<char*>(&_pData->eTimer), sizeof(TIMER));


	InputFile.close();
	return S_OK;
}

HRESULT CFXToolDirector::Load_Effect(path _FilePath, MULTI_FX_DATA* _pData)
{

	ifstream InputFile(_FilePath, ios::binary | ios::in);

	if (!InputFile.is_open())
		ALARM_FAIL(TEXT("망했어"));


	//이펙트 이름
	InputFile.read(reinterpret_cast<char*>(&_pData->iNameStrLen), sizeof(_uint));
	_pData->strName.resize(_pData->iNameStrLen);
	InputFile.read(&_pData->strName[0], _pData->iNameStrLen);


	InputFile.read(reinterpret_cast<char*>(&_pData->iFXsNum), sizeof(_uint));
	_pData->FXs.resize(_pData->iFXsNum);

	for (auto& FX : _pData->FXs)
	{
		InputFile.read(reinterpret_cast<char*>(&FX.first), sizeof(_uint));
		FX.second.resize(FX.first);
		InputFile.read(&FX.second[0], FX.first);
	}

	return S_OK;
}

HRESULT CFXToolDirector::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFXToolDirector::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	if (nullptr == pArg)
	{
		GameObjectDesc.fSpeedPerSec = 5.f;
		GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	}
	else
	{
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;
	}


	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED_MSG(hr, "Failed To Initialize : CFXToolDirector");

	hr = Add_Components();
	CHECK_FAILED_MSG(hr, "Failed To Add Components : CFXToolDirector");

	//static 프로토타입 중 이펙트 툴에서 사용할 컴포넌트의 이름을 가져온다.
	hr = Ready_FXPrototypeVector();
	CHECK_FAILED_MSG(hr, "Failed To Add Components : CFXToolDirector");

	m_pGameInstance->Set_IMGUIStyle(CImGUI_Manager::HYO);

	return S_OK;
}

_int CFXToolDirector::Tick(_float _fTimeDelta)
{
	//이펙트 저장 / 불러오기
	if (m_pGameInstance->Get_KeyState(DIK_LCONTROL, KEY_PRESS))
	{
		if (m_pGameInstance->Get_KeyState(DIK_S, KEY_DOWN))
		{
			Save_AllEffect();
		}
		else if (m_pGameInstance->Get_KeyState(DIK_L, KEY_DOWN))
		{
			Load_AllEffect();
		}
	}

	return OBJ_NOEVENT;
}

void CFXToolDirector::Late_Tick(_float _fTimeDelta)
{

	//0, 0 중심선을 긋는다.
	Render_AxisLines();

	//그리드를 그린다.
	m_pGameInstance->RenderGrid();

	//단일 이펙트 생성, 목록 확인
	Render_FXHierarchy();

	//복합 이펙트 생성, 목록 확인
	Render_MultiFXHierarchy();

	//이펙트들의 변수 설정
	Render_FXProperty();

	//이펙트의 재생 바 설정
	Render_FXPlayBar(_fTimeDelta);


	//플레이 바 재생 중일 때 이펙트 틱 돌리기
	if (m_bPlayingBar)
	{
		//복합 이펙트 아니면 FX에 있는 이펙트 업데이트
		if (m_eSelected == SELECTED_SINGLE_FX)
			m_FXs[m_iSelectedFXIdx]->Late_Tick(_fTimeDelta);
		else if (m_eSelected == SELECTED_PARTICLE_FX)
			m_Particles[m_iSelectedParticleIdx]->Late_Tick(_fTimeDelta);
		//아니면 복합 이펙트 업데이트
		else if (m_eSelected == SELECTED_MULTI_FX)
			m_MultiFXs[m_iSelectedMultiFXIdx]->Late_Tick(_fTimeDelta);
	}
	else
	{
		//복합 이펙트 아니면 FX에 있는 이펙트 업데이트
		if (m_eSelected == SELECTED_SINGLE_FX )
			m_FXs[m_iSelectedFXIdx]->Add_RenderGroup();
		else if (m_eSelected == SELECTED_PARTICLE_FX)
			m_Particles[m_iSelectedParticleIdx]->Add_RenderGroup();
		//아니면 복합 이펙트 업데이트
		else if (m_eSelected == SELECTED_MULTI_FX)
			m_MultiFXs[m_iSelectedMultiFXIdx]->Add_RenderGroup();
	}

}

void CFXToolDirector::Render_AxisLines()
{
	ImDrawList* drawList = GetForegroundDrawList();


	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW);
	ProjMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ);
	_float4x4 VPMatrix = ViewMatrix * ProjMatrix;

	auto TransformToScreen = [&](XMVECTOR worldPos)
		{
			XMVECTOR screenPos = XMVector3TransformCoord(worldPos, VPMatrix);
			screenPos = XMVectorMultiplyAdd(screenPos, XMVectorSet(0.5f, -0.5f, 1.0f, 0.0f), XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f));
			screenPos = XMVectorMultiply(screenPos, XMVectorSet(g_iWinSizeX, g_iWinSizeY, 1.f, 0.f));
			return ImVec2(XMVectorGetX(screenPos), XMVectorGetY(screenPos));
		};



	_float3 vCenter = { 0.f, 0.f, 0.f };
	ImVec2 center = TransformToScreen(XMLoadFloat3(&vCenter));

	_float fRadius = 3.0f;

	_float fBottomRadius = 5.0f;
	_float fTopRadius = 5.0f;
	_float fHeight = 10.0f;
	_int iSliceCnt = 8;

	ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	vector<ImVec2> bottomCircle, topCircle;


	// Define points in world space
	XMVECTOR origin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR xAxisStart = XMVectorSet(-2.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR xAxisEnd = XMVectorSet(2.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR zAxisStart = XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f);
	XMVECTOR zAxisEnd = XMVectorSet(0.0f, 0.0f, 2.0f, 1.0f);

	// Transform to screen space
	ImVec2 screenOrigin = TransformToScreen(origin);
	ImVec2 screenXAxisStart = TransformToScreen(xAxisStart);
	ImVec2 screenZAxisStart = TransformToScreen(zAxisStart);

	ImVec2 screenXAxisEnd = TransformToScreen(xAxisEnd);
	ImVec2 screenZAxisEnd = TransformToScreen(zAxisEnd);


	// 화살표를 그리는 람다 함수
	auto AddArrow = [drawList](ImVec2 p1, ImVec2 p2, ImU32 col, float thickness, float arrowSize)
		{
			ImVec2 direction = ImVec2(p2.x - p1.x, p2.y - p1.y);
			float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
			direction.x /= length;
			direction.y /= length;

			drawList->AddLine(p1, p2, col, thickness);

			ImVec2 arrowLeft = ImVec2(
				p2.x - direction.x * arrowSize - direction.y * arrowSize * 0.5f,
				p2.y - direction.y * arrowSize + direction.x * arrowSize * 0.5f
			);
			ImVec2 arrowRight = ImVec2(
				p2.x - direction.x * arrowSize + direction.y * arrowSize * 0.5f,
				p2.y - direction.y * arrowSize - direction.x * arrowSize * 0.5f
			);

			drawList->AddTriangleFilled(p2, arrowLeft, arrowRight, col);
		};

	// 화살표 그리기
	AddArrow(screenOrigin, screenXAxisEnd, IM_COL32(255, 50, 255, 255), 1.0f, 12.0f);
	AddArrow(screenOrigin, screenZAxisEnd, IM_COL32(50, 50, 255, 255), 1.0f, 12.0f);
}

HRESULT CFXToolDirector::Render()
{
	return S_OK;
}

void CFXToolDirector::Render_IMGUI()
{



	__super::Render_IMGUI();
}

void CFXToolDirector::Draw_Cylinder(const ImVec2& center, float bottomRadius, float topRadius, float height, int sliceCount, const ImVec4& color)
{
	ImDrawList* drawList = GetForegroundDrawList();
	ImVec2 screenPosBottom, screenPosTop;

	for (int j = 0; j <= sliceCount; ++j)
	{
		float theta = j * 2.0f * DirectX::XM_PI / sliceCount;
		ImVec2 posBottom = center + ImVec2(bottomRadius * cosf(theta), height / 2);
		ImVec2 posTop = center + ImVec2(topRadius * cosf(theta), -height / 2);
		if (j > 0)
		{
			drawList->AddLine(screenPosBottom, posBottom, ImColor(color.x, color.y, color.z, color.w));
			drawList->AddLine(screenPosTop, posTop, ImColor(color.x, color.y, color.z, color.w));
			drawList->AddLine(screenPosBottom, screenPosTop, ImColor(color.x, color.y, color.z, color.w));
		}
		screenPosBottom = posBottom;
		screenPosTop = posTop;
	}
}


//단일 이펙트들의 생성 세팅, 계층을 보여준다.
void CFXToolDirector::Render_FXHierarchy()
{
	Begin(u8"만들기");

	static ImGuiTextFilter DiffuseFilter;
	DiffuseFilter.Draw(u8"디퓨즈 검색");

	//색 텍스쳐
	if (BeginCombo(u8"디퓨즈 텍스쳐", m_FXTexList[m_iAddingFXTexIdx], ImGuiComboFlags_PopupAlignLeft))
	{
		for (size_t i = 0; i < (_int)m_FXTexList.size(); i++)
		{
			const bool bSelected = (m_iAddingFXTexIdx == i);
			if (DiffuseFilter.PassFilter(m_FXTexList[i]) &&
				Selectable(m_FXTexList[i], bSelected))
				m_iAddingFXTexIdx = i;


			if (bSelected)
				SetItemDefaultFocus();
		}
		EndCombo();
	}

	Dummy({ 0.f, 10.f });



	static ImGuiTextFilter MaskFilter;
	MaskFilter.Draw(u8"마스크 검색");

	//마스크 텍스쳐
	if (BeginCombo(u8"마스크 텍스쳐", m_FXMaskTexList[m_iAddingFXMaskTexIdx], ImGuiComboFlags_PopupAlignLeft))
	{
		for (size_t i = 0; i < (_int)m_FXMaskTexList.size(); i++)
		{
			const bool bSelected = (m_iAddingFXMaskTexIdx == i);
			if (MaskFilter.PassFilter(m_FXMaskTexList[i]) &&
				Selectable(m_FXMaskTexList[i], bSelected))
				m_iAddingFXMaskTexIdx = i;


			if (bSelected)
				SetItemDefaultFocus();
		}
		EndCombo();
	}


	Separator();
	Columns(2);

	static ImGuiTextFilter BufferFilter;
	BufferFilter.Draw(u8"버퍼 검색");

	//버퍼
	if (BeginCombo(u8"버퍼", m_FXBufferList[m_iAddingFXBufferIdx], ImGuiComboFlags_PopupAlignLeft))
	{
		for (size_t i = 0; i < (_int)m_FXBufferList.size(); i++)
		{
			const bool bSelected = (m_iAddingFXBufferIdx == i);
			if (BufferFilter.PassFilter(m_FXBufferList[i]) &&
				Selectable(m_FXBufferList[i], bSelected))
				m_iAddingFXBufferIdx = i;


			if (bSelected)
				SetItemDefaultFocus();
		}
		EndCombo();
	}


	//single effect를 생성한다.
	if (Button(u8"이펙트 생성"))
	{
		//이름 정해주기
		CSingleEffect::FX_DESC singleFXDesc{};
		singleFXDesc.fDuration = 1.f;
		string strComponentTag = "Prototype_Component_";

		string strBaseName{ "Default FX " };
		switch (m_iAddingFXBufferIdx)
		{
		case 0:
			strBaseName = "Rect FX ";
			break;
		case 1:
			break;
		default:
			strBaseName = "New FX ";
			break;
		}

		//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
		char szSuffix = 'A';
		while (true)
		{
			_bool bDoesExistSameName{ false };
			singleFXDesc.strFXName = strBaseName + szSuffix;

			//중복 이름 있으면 안됨
			for (const auto& fx : m_FXs)
			{
				if (fx->m_strFXName == singleFXDesc.strFXName)
				{
					bDoesExistSameName = true;
					break;
				}
			}

			//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
			if (!bDoesExistSameName || 'Z' <= szSuffix)
				break;

			++szSuffix;
		}

		//버퍼, 텍스쳐, 마스크 텍스쳐 컴포넌트 이름 떤져준다.
		singleFXDesc.strBufferTag = strComponentTag + m_FXBufferList[m_iAddingFXBufferIdx];
		singleFXDesc.strTexTag = strComponentTag + m_FXTexList[m_iAddingFXTexIdx];
		singleFXDesc.strMaskTexTag = strComponentTag + m_FXMaskTexList[m_iAddingFXMaskTexIdx];

		//기본 세팅된 키프레임들.
		vector<FX_KEYFRAME> newProperty;
		FX_KEYFRAME newStartKeyframe{ 0.f, Vector3::Zero, EASE_OUT };
		FX_KEYFRAME newEndKeyframe{ 1.f, Vector3::Zero, EASE_OUT };

		//위치
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);

		singleFXDesc.Keyframes.emplace(KF_POS, newProperty);

		//회전
		newProperty.clear();
		newStartKeyframe.vValue = Vector3::Zero;
		newEndKeyframe.vValue = Vector3::Zero;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_ROT, newProperty);

		//크기
		newProperty.clear();
		newStartKeyframe.vValue = Vector3::One;
		newStartKeyframe.eEasing = EASE_IN;
		newEndKeyframe.vValue = Vector3::One;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_SCALE, newProperty);


		//R(색상)
		newProperty.clear();
		newStartKeyframe.vValue = { 1.f, 1.f, 1.f };
		newEndKeyframe.vValue = { 1.f, 1.f, 1.f };
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_RCOLOR, newProperty);

		//G
		newProperty.clear();
		newStartKeyframe.vValue = { 0.f, 1.f, 0.f };
		newEndKeyframe.vValue = { 0.f, 1.f, 0.f };
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_GCOLOR, newProperty);

		//B
		newProperty.clear();
		newStartKeyframe.vValue = { 0.f, 0.f, 1.f };
		newEndKeyframe.vValue = { 0.f, 0.f, 1.f };
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_BCOLOR, newProperty);

		//B
		newProperty.clear();
		newStartKeyframe.vValue = Vector3::One;
		newEndKeyframe.vValue = Vector3::One;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_ALPHA, newProperty);

		newProperty.clear();
		newStartKeyframe.vValue = Vector3::Zero;
		newEndKeyframe.vValue = Vector3::Zero;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_MASK, newProperty);

		newProperty.clear();
		newStartKeyframe.vValue = Vector3::Zero;
		newStartKeyframe.eEasing = EASE_LINEAR;
		newEndKeyframe.vValue = Vector3::Zero;
		newProperty.push_back(newStartKeyframe);
		newProperty.push_back(newEndKeyframe);
		singleFXDesc.Keyframes.emplace(KF_UVOFFSET, newProperty);

		singleFXDesc.Keyframes.emplace(KF_MASKUVOFFSET, newProperty);

		singleFXDesc.Keyframes.emplace(KF_MASKUVANGLE, newProperty);

		CSingleEffect* pSingleFX = static_cast<CSingleEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_SingleEffect"), &singleFXDesc));
		m_FXs.emplace_back(pSingleFX);
	}


	NextColumn();

	InputInt(u8"갯수", &m_iAddingInstanceNum, 1, 500);

	if (Button(u8"파티클 생성"))
	{

		//이름 정해주기
		CParticle::PARTICLE_DESC ParticleDesc{};
		string strComponentTag = "Prototype_Component_";

		string strBaseName{ "Default FX" };
		switch (m_iAddingFXBufferIdx)
		{
		case 1:
			break;
		default:
			strBaseName = "Particle ";
			break;
		}

		//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다. 
		char szSuffix = 'A';
		while (true)
		{
			_bool bDoesExistSameName{ false };
			ParticleDesc.strFXName = strBaseName + szSuffix;

			//중복 이름 있으면 안됨
			for (const auto& fx : m_FXs)
			{
				if (fx->m_strFXName == ParticleDesc.strFXName)
				{
					bDoesExistSameName = true;
					break;
				}
			}

			//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
			if (!bDoesExistSameName || 'Z' <= szSuffix)
				break;

			++szSuffix;
		}


		//버퍼, 텍스쳐, 마스크 텍스쳐 컴포넌트 이름 떤져준다.
		ParticleDesc.strBufferTag = strComponentTag + "VIBuffer_Instance_Point";
		ParticleDesc.strTexTag = strComponentTag + m_FXTexList[m_iAddingFXTexIdx];
		ParticleDesc.strMaskTexTag = strComponentTag + m_FXMaskTexList[m_iAddingFXMaskTexIdx];
		ParticleDesc.iNumInstance = m_iAddingInstanceNum;
		ParticleDesc.fDuration = 5.f;
		CParticle* pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle"), &ParticleDesc));
		m_Particles.emplace_back(pParticle);
	}

	End();



	//이펙트 초기 값과 키프레임 값을 편집한다.
	Begin(u8"편집하기");

	SeparatorText(u8"단일 이펙트 목록");

	static ImGuiTextFilter filter;
	filter.Draw(u8"단일 이펙트 검색");

	BeginChild(u8"목록", ImVec2(0, 150), true);


	for (_int i = 0; i < m_FXs.size(); ++i)
	{

		// 목록 중 하나 선택하면 해당 객체의 값을 ui에 매칭
		if (filter.PassFilter(m_FXs[i]->m_strFXName.c_str())
			&& Selectable(m_FXs[i]->m_strFXName.c_str(), m_iSelectedFXIdx == i))
		{
			m_iSelectedFXIdx = i;

			m_eSelected = SELECTED_SINGLE_FX;
			m_bPlayingBar = false;
			m_bLooping = m_FXs[i]->m_bIsLoop;
			m_iCurFXPassIdx = m_FXs[i]->m_iPassIdx;
			m_iCurFXTexIdx = m_FXs[i]->m_iTexIdx;
			m_iCurFXMaskTexIdx = m_FXs[i]->m_iMaskTexIdx;
			m_iCurRenderGroup = m_FXs[i]->m_eRenderGroup;
			m_iCurTimer = m_FXs[i]->m_eTimer;
			m_fTotalPlayDuration = m_FXs[i]->m_fDuration.second;

			memcpy(m_fLifetime, &m_FXs[i]->m_fLifetime, sizeof(_float2));
			memcpy(m_vRotation, &m_FXs[i]->m_vContinuousRotation, sizeof(_float3));

		}


		if (m_iSelectedFXIdx == i && IsItemHovered() && IsMouseReleased(1))
			OpenPopup("FXMenu");

	}

	//우측 키를 누르면 나오는 메뉴들
	if (BeginPopup("FXMenu"))
	{
		if (MenuItem(u8"선택된 이펙트 그룹에 추가") && m_iSelectedFXIdx != -1 && m_iSelectedMultiFXIdx != -1)
		{
			m_MultiFXs[m_iSelectedMultiFXIdx]->Add_Effect(m_FXs[m_iSelectedFXIdx]);
		}

		if (MenuItem(u8"새 이펙트 그룹 생성"))
		{
			CMultiEffect::MULTI_FX_DESC FxDesc = {};

			string strBaseName{ "Multi FX " };

			//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
			char szSuffix = 'A';
			while (true)
			{
				_bool bDoesExistSameName{ false };
				FxDesc.strFXName = strBaseName + szSuffix;

				//중복 이름 있으면 안됨
				for (const auto& fx : m_MultiFXs)
				{
					if (fx->m_strFXName == FxDesc.strFXName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
				if (!bDoesExistSameName || 'Z' <= szSuffix)
					break;

				++szSuffix;
			}

			CMultiEffect* pMultiFX = static_cast<CMultiEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_MultiEffect"), &FxDesc));
			m_MultiFXs.emplace_back(pMultiFX);

			m_MultiFXs.back()->Add_Effect(m_FXs[m_iSelectedFXIdx]);
		}

		if (MenuItem(u8"삭제"))
		{
			string strName = m_FXs[m_iSelectedFXIdx]->m_strFXName;
			string strPath = SINGLEFX_PATH;
			strPath += strName + ".bin";
			MoveTo_TrashBin(strPath);


			Safe_Release(m_FXs[m_iSelectedFXIdx]);
			m_FXs.erase(m_FXs.begin() + m_iSelectedFXIdx);
			m_eSelected = SELECTED_END;

			if (m_FXs.size() <= m_iSelectedFXIdx)
				--m_iSelectedFXIdx;
			else if (m_FXs.empty())
			{
				m_iSelectedFXIdx = -1;
			}
		}

		if (MenuItem(u8"모두 복사 생성"))
		{
			//이름 정해줘
			string strBaseName{ "New FX " };
			string strName{};

			//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
			char szSuffix = 'A';
			while (true)
			{
				_bool bDoesExistSameName{ false };
				strName = strBaseName + szSuffix;

				//중복 이름 있으면 안됨
				for (const auto& fx : m_FXs)
				{
					if (fx->m_strFXName == strName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				for (const auto& fx : m_Particles)
				{
					if (fx->m_strFXName == strName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
				if (!bDoesExistSameName || 'Z' <= szSuffix)
					break;
				++szSuffix;
			}

			SINGLE_FX_DATA FXData{};
			m_FXs[m_iSelectedFXIdx]->Fill_SaveData(&FXData);
			FXData.strName = strName;
			Make_Effect(FXData);

		}

		if (MenuItem(u8"변수만 복사 생성"))
		{
			//이름 정해줘
			string strBaseName{ "New FX " };
			string strName{};


			char szSuffix = 'A';
			while (true)
			{
				_bool bDoesExistSameName{ false };
				strName = strBaseName + szSuffix;

				//중복 이름 있으면 안됨
				for (const auto& fx : m_FXs)
				{
					if (fx->m_strFXName == strName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
				if (!bDoesExistSameName || 'Z' <= szSuffix)
					break;

				++szSuffix;
			}

			if (m_eSelected == SELECTED_SINGLE_FX)
			{
				//일단 단일 이펙트만
				SINGLE_FX_DATA FXData{};
				m_FXs[m_iSelectedFXIdx]->Fill_SaveData(&FXData);

				FXData.strName = strName;

				//버퍼, 텍스쳐, 마스크 텍스쳐 컴포넌트 이름 떤져준다.
				string strComponentTag = "Prototype_Component_";
				FXData.strBufferName = strComponentTag + m_FXBufferList[m_iAddingFXBufferIdx];
				FXData.strTexName = strComponentTag + m_FXTexList[m_iAddingFXTexIdx];
				FXData.strMaskTexName = strComponentTag + m_FXMaskTexList[m_iAddingFXMaskTexIdx];
				FXData.iPassIdx = 0;
				FXData.iMaskTexIdx = 0;
				FXData.iTexIdx = 0;

				Make_Effect(FXData);
			}
		}

		EndPopup();
	}


	EndChild();


	//파티클 목록

	SeparatorText(u8"파티클 목록");

	static ImGuiTextFilter ParticleFilter;
	ParticleFilter.Draw(u8"파티클 검색");

	BeginChild(u8"파티클 목록", ImVec2(0, 150), true);


	for (_int i = 0; i < m_Particles.size(); ++i)
	{

		// 목록 중 하나 선택하면 해당 객체의 값을 ui에 매칭
		if (ParticleFilter.PassFilter(m_Particles[i]->m_strFXName.c_str())
			&& Selectable(m_Particles[i]->m_strFXName.c_str(), m_iSelectedParticleIdx == i))
		{
			m_iSelectedParticleIdx = i;

			m_eSelected = SELECTED_PARTICLE_FX;
			m_bPlayingBar = false;
			m_bLooping = m_Particles[i]->m_bIsLoop;
			m_iCurFXPassIdx = m_Particles[i]->m_iPassIdx;
			m_iCurFXTexIdx = m_Particles[i]->m_iTexIdx;
			m_iCurFXMaskTexIdx = m_Particles[i]->m_iMaskTexIdx;
			m_iCurRenderGroup = m_Particles[i]->m_eRenderGroup;
			m_iCurTimer = m_Particles[i]->m_eTimer;
			m_fTotalPlayDuration = m_Particles[i]->m_fDuration.second;

			memcpy(m_fLifetime, &m_Particles[i]->m_fLifetime, sizeof(_float2));
			memcpy(m_vRotation, &m_Particles[i]->m_vContinuousRotation, sizeof(_float3));


			//파티클이면 추가 변수 매칭

			CParticle* pCurParticle = static_cast<CParticle*>(m_Particles[i]);
			memcpy(m_vCenter, &pCurParticle->m_InstanceDesc.vCenter, sizeof(_float3));
			memcpy(m_vRange, &pCurParticle->m_InstanceDesc.vRange, sizeof(_float3));
			memcpy(m_vRotation, &pCurParticle->m_InstanceDesc.vRotation, sizeof(_float3));
			memcpy(m_vRotationRandomOffset, &pCurParticle->m_InstanceDesc.vRotationRandomOffset, sizeof(_float3));
			memcpy(m_vScale, &pCurParticle->m_InstanceDesc.vScale, sizeof(_float3));
			memcpy(m_vScaleRandomOffset, &pCurParticle->m_InstanceDesc.vScaleRandomOffset, sizeof(_float3));
			memcpy(m_vDir, &pCurParticle->m_InstanceDesc.vDir, sizeof(_float3));
			memcpy(m_vDirRandomOffset, &pCurParticle->m_InstanceDesc.vDirRandomOffset, sizeof(_float3));
			memcpy(m_vColor, &pCurParticle->m_InstanceDesc.vColor, sizeof(_float3));
			memcpy(m_vColorRandomOffset, &pCurParticle->m_InstanceDesc.vColorRandomOffset, sizeof(_float3));
			memcpy(m_vPivot, &pCurParticle->m_InstanceDesc.vPivot, sizeof(_float3));

		}


		if (m_iSelectedParticleIdx == i && IsItemHovered() && IsMouseReleased(1))
			OpenPopup("ParticleMenu");

	}

	//우측 키를 누르면 나오는 메뉴들
	if (BeginPopup("ParticleMenu"))
	{
		if (MenuItem(u8"선택된 이펙트 그룹에 추가") && m_iSelectedParticleIdx != -1 && m_iSelectedMultiFXIdx != -1)
		{
			m_MultiFXs[m_iSelectedMultiFXIdx]->Add_Effect(m_Particles[m_iSelectedParticleIdx]);
		}

		if (MenuItem(u8"새 이펙트 그룹 생성"))
		{
			CMultiEffect::MULTI_FX_DESC FxDesc = {};

			string strBaseName{ "Multi FX " };

			//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
			char szSuffix = 'A';
			while (true)
			{
				_bool bDoesExistSameName{ false };
				FxDesc.strFXName = strBaseName + szSuffix;

				//중복 이름 있으면 안됨
				for (const auto& fx : m_MultiFXs)
				{
					if (fx->m_strFXName == FxDesc.strFXName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
				if (!bDoesExistSameName || 'Z' <= szSuffix)
					break;

				++szSuffix;
			}

			CMultiEffect* pMultiFX = static_cast<CMultiEffect*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_MultiEffect"), &FxDesc));
			m_MultiFXs.emplace_back(pMultiFX);

			m_MultiFXs.back()->Add_Effect(m_Particles[m_iSelectedParticleIdx]);
		}

		if (MenuItem(u8"삭제"))
		{
			string strName = m_Particles[m_iSelectedParticleIdx]->m_strFXName;
			string strPath = m_eSelected == SELECTED_SINGLE_FX ? SINGLEFX_PATH : PARTICLE_PATH;
			strPath += strName + ".bin";
			MoveTo_TrashBin(strPath);


			Safe_Release(m_Particles[m_iSelectedParticleIdx]);
			m_Particles.erase(m_Particles.begin() + m_iSelectedParticleIdx);
			m_eSelected = SELECTED_END;

			if (m_Particles.size() <= m_iSelectedParticleIdx)
				--m_iSelectedFXIdx;
			else if (m_Particles.empty())
			{
				m_iSelectedParticleIdx = -1;
			}
		}

		if (MenuItem(u8"모두 복사 생성"))
		{
			//이름 정해줘
			string strBaseName{ "New FX " };
			string strName{};

			//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
			char szSuffix = 'A';
			while (true)
			{
				_bool bDoesExistSameName{ false };
				strName = strBaseName + szSuffix;


				for (const auto& fx : m_FXs)
				{
					if (fx->m_strFXName == strName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				for (const auto& fx : m_Particles)
				{
					if (fx->m_strFXName == strName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
				if (!bDoesExistSameName || 'Z' <= szSuffix)
					break;

				++szSuffix;
			}


			PARTICLE_DATA ParticleData{};
			m_Particles[m_iSelectedParticleIdx]->Fill_SaveData(&ParticleData);
			ParticleData.strName = strName;
			Make_Effect(ParticleData);


		}

		if (MenuItem(u8"변수만 복사 생성"))
		{
			//이름 정해줘
			string strBaseName{ "New FX " };
			string strName{};

			//default 이펙트 이름 뒤에 중복 존재 시 알파벳을 붙인다.
			char szSuffix = 'A';
			while (true)
			{
				_bool bDoesExistSameName{ false };
				strName = strBaseName + szSuffix;

				for (const auto& fx : m_Particles)
				{
					if (fx->m_strFXName == strName)
					{
						bDoesExistSameName = true;
						break;
					}
				}

				//중복 이름이 없거나, 알파벳이 초과하면 반복 끝
				if (!bDoesExistSameName || 'Z' <= szSuffix)
					break;

				++szSuffix;
			}


			PARTICLE_DATA ParticleData{};
			m_Particles[m_iSelectedParticleIdx]->Fill_SaveData(&ParticleData);

			ParticleData.strName = strName;

			ParticleData.iPassIdx = 0;
			ParticleData.iMaskTexIdx = 0;
			ParticleData.iTexIdx = 0;
			ParticleData.iNumInstance = m_iAddingInstanceNum;
			string strComponentTag = "Prototype_Component_";
			ParticleData.strTexName = strComponentTag + m_FXTexList[m_iAddingFXTexIdx];
			ParticleData.strMaskTexName = strComponentTag + m_FXMaskTexList[m_iAddingFXMaskTexIdx];

			Make_Effect(ParticleData);

		}
		EndPopup();
	}


	EndChild();


	End();

}


//싱글 이펙트의 상시 변수를 조정한다.
void CFXToolDirector::Render_FXProperty()
{
	//아무것도 안 누른 상태면 보이지 않음
	if (m_eSelected == SELECTED_END)
		return;

	//단일 or 파티클 이펙트를 특정하지 않으면 보이지 않음
	if ((m_eSelected == SELECTED_SINGLE_FX || m_eSelected == SELECTED_PARTICLE_FX) && m_iSelectedFXIdx == -1)
		return;
	//복합 이펙트를 특정하지 않으면 보이지 않음
	if (m_eSelected == SELECTED_MULTI_FX && m_iSelectedMultiFXIdx == -1)
		return;


	//이펙트 기본 변수 세팅
	Begin(u8"속성 편집");

	//현재 선택한 이펙트를 다르게 한다.
	CEffect* pCurFX{ nullptr };
	
	if (m_eSelected == SELECTED_SINGLE_FX)
		pCurFX = m_FXs[m_iSelectedFXIdx];
	else if (m_eSelected == SELECTED_PARTICLE_FX)
		pCurFX = m_Particles[m_iSelectedParticleIdx];

	else
		pCurFX = m_MultiFXs[m_iSelectedMultiFXIdx];

	//파티클인가?
	_bool bIsParticle = _bool{ dynamic_cast<CSingleEffect*>(pCurFX) == nullptr };


	char tempBuf[256];
	strncpy_s(tempBuf, pCurFX->m_strFXName.c_str(), sizeof(tempBuf));
	tempBuf[sizeof(tempBuf) - 1] = 0;

	//이름
	if (InputText("Name", tempBuf, sizeof(tempBuf)))
	{
		if (tempBuf[0] == '\0')
			strcpy_s(tempBuf, "Default");

		m_curFXName = string(tempBuf);
		pCurFX->m_strFXName = m_curFXName;
	}

	Text(u8"버퍼");
	SameLine();
	Text(CUtils::WstrToStr(pCurFX->m_strBufferTag).c_str());

	Text(u8"텍스쳐");
	SameLine();
	Text(CUtils::WstrToStr(pCurFX->m_strTexTag).c_str());

	Text(u8"마스크 텍스쳐");
	SameLine();
	Text(CUtils::WstrToStr(pCurFX->m_strMaskTexTag).c_str());

	//단일 이펙트가 가지고 있는 변수
	if (m_eSelected != SELECTED_MULTI_FX)
	{
		//루프, 빌보드
		if (Checkbox(u8"루프", &pCurFX->m_bIsLoop) && bIsParticle)
		{
			m_bPlayingBar = false;
			pCurFX->Reset_Duration();
			static_cast<CParticle*>(pCurFX)->Update_InstanceInfo();
		}


		//ui처럼 보이는 직교 이펙트
		if (!bIsParticle)
		{
			SameLine();
			if (Checkbox(u8"빌보딩", &pCurFX->m_bIsBillboard))
			{
				//m_bPlayingBar = false;
				//pCurFX->Reset_Duration();
				//static_cast<CParticle*>(pCurFX)->Update_InstanceInfo();
			}

			SameLine();
			Checkbox(u8"직교", &pCurFX->m_bIsOrthographic);
		}

		SameLine();
		//블룸 효과
		if (Checkbox(u8"블룸", &pCurFX->m_bIsBloom) && bIsParticle)
		{
			m_bPlayingBar = false;
			pCurFX->Reset_Duration();
			static_cast<CParticle*>(pCurFX)->Update_InstanceInfo();
		}

		//렌더 그룹 설정
		if (RadioButton(u8"No Render", m_iCurRenderGroup == CRenderer::RENDER_END))
		{
			m_iCurRenderGroup = CRenderer::RENDER_END;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_END;
		}
		SameLine();

		if (RadioButton(u8"Priority", m_iCurRenderGroup == CRenderer::RENDER_PRIORITY))
		{
			m_iCurRenderGroup = CRenderer::RENDER_PRIORITY;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_PRIORITY;
		}
		SameLine();

		if (RadioButton(u8"NonBlend", m_iCurRenderGroup == CRenderer::RENDER_NONBLEND))
		{
			m_iCurRenderGroup = CRenderer::RENDER_NONBLEND;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_NONBLEND;
		}
		SameLine();

		if (RadioButton(u8"Nonlight", m_iCurRenderGroup == CRenderer::RENDER_NONLIGHT))
		{
			m_iCurRenderGroup = CRenderer::RENDER_NONLIGHT;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_NONLIGHT;
		}


		if (RadioButton(u8"Blend", m_iCurRenderGroup == CRenderer::RENDER_BLEND))
		{
			m_iCurRenderGroup = CRenderer::RENDER_BLEND;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_BLEND;
		}

		SameLine();

		if (RadioButton(u8"UI", m_iCurRenderGroup == CRenderer::RENDER_UI))
		{
			m_iCurRenderGroup = CRenderer::RENDER_UI;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_UI;
		}

		SameLine();

		if (RadioButton(u8"Super UI", m_iCurRenderGroup == CRenderer::RENDER_SUPERUI))
		{
			m_iCurRenderGroup = CRenderer::RENDER_SUPERUI;
			pCurFX->m_eRenderGroup = CRenderer::RENDER_SUPERUI;
		}

		Spacing();

		//타이머 설정
		if (RadioButton(u8"Timer None", m_iCurTimer == TIMER_NONE))
		{
			m_iCurTimer = TIMER_NONE;
			pCurFX->m_eTimer = TIMER_NONE;
		}
		SameLine();

		if (RadioButton(u8"First Timer", m_iCurTimer == TIMER_FIRST))
		{
			m_iCurTimer = TIMER_FIRST;
			pCurFX->m_eTimer = TIMER_FIRST;
		}
		SameLine();

		if (RadioButton(u8"Second Timer", m_iCurTimer == TIMER_SECOND))
		{
			m_iCurTimer = TIMER_SECOND;
			pCurFX->m_eTimer = TIMER_SECOND;
		}

		Spacing();
		if (!bIsParticle && DragFloat3(u8"지속 회전", m_vRotation, .05f, -180.f, 180.f, "%.2f", ImGuiSliderFlags_None))
		{
			pCurFX->m_vContinuousRotation.x = m_vRotation[0];
			pCurFX->m_vContinuousRotation.y = m_vRotation[1];
			pCurFX->m_vContinuousRotation.z = m_vRotation[2];
		}
	}

	Separator();

	//전체 시간
	if (DragFloat(u8"재생 시간", &pCurFX->m_fDuration.second, .1f, 0.f, 300.f, "%.2f"))
	{
		m_fTotalPlayDuration = pCurFX->m_fDuration.second;
	}



	//복합 이펙트는 재생 시간까지만 보인다.
	if (m_eSelected == SELECTED_MULTI_FX)
	{
		End();
		return;
	}


	//이펙트의 재생 수명
	if (DragFloat2(u8"수명", m_fLifetime, .1f, 0.f, pCurFX->m_fDuration.second, "%.2f"))
	{
		memcpy(&pCurFX->m_fLifetime, m_fLifetime, sizeof(_float2));

		if (bIsParticle)
		{
			static_cast<CParticle*>(pCurFX)->m_InstanceDesc.fLifetime = m_fLifetime[1];
			static_cast<CParticle*>(pCurFX)->Update_InstanceInfo();
		}
	}

	if (bIsParticle)
	{
		if (DragFloat(u8"수명 랜덤", &(static_cast<CParticle*>(pCurFX)->m_InstanceDesc.fLifetimeRandomOffset), .1f, 0.f, 1000.f, "%.2f"))
		{
			static_cast<CParticle*>(pCurFX)->Update_InstanceInfo();
		}
	}

	if (m_eSelected == SELECTED_SINGLE_FX)
	{
		if (static_cast<CSingleEffect*>(pCurFX)->IsModelBuffer())
		{
			if (Combo(u8"렌더 패스", &m_iCurFXPassIdx, s_ModelPasses.data(), (_int)s_ModelPasses.size()))
			{
				if (m_iCurFXPassIdx < 0)
					m_iCurFXPassIdx = 0;

				if (pCurFX->m_iMaxPassIdx < m_iCurFXPassIdx)
					m_iCurFXPassIdx = pCurFX->m_iMaxPassIdx;

				pCurFX->m_iPassIdx = m_iCurFXPassIdx;
			}
		}
		else
		{
			if (Combo(u8"렌더 패스", &m_iCurFXPassIdx, s_PosTexPasses.data(), (_int)s_PosTexPasses.size()))
			{
				if (m_iCurFXPassIdx < 0)
					m_iCurFXPassIdx = 0;

				if (pCurFX->m_iMaxPassIdx < m_iCurFXPassIdx)
					m_iCurFXPassIdx = pCurFX->m_iMaxPassIdx;

				pCurFX->m_iPassIdx = m_iCurFXPassIdx;
			}
		}
	}
	else
	{
		if (InputInt(u8"렌더 패스", &m_iCurFXPassIdx, 1, pCurFX->m_iMaxPassIdx))
		{
			if (m_iCurFXPassIdx < 0)
				m_iCurFXPassIdx = 0;

			if (pCurFX->m_iMaxPassIdx < m_iCurFXPassIdx)
				m_iCurFXPassIdx = pCurFX->m_iMaxPassIdx;

			pCurFX->m_iPassIdx = m_iCurFXPassIdx;
		}
	}




	if (InputInt(u8"디퓨즈 인덱스", &m_iCurFXTexIdx, 1, pCurFX->m_iMaxTexIdx))
	{
		//가능한 인덱스로 한 번 보정해 주기
		if (m_iCurFXTexIdx < 0)
			m_iCurFXTexIdx = 0;

		if (pCurFX->m_iMaxTexIdx < m_iCurFXTexIdx)
			m_iCurFXTexIdx = pCurFX->m_iMaskTexIdx;

		pCurFX->m_iTexIdx = m_iCurFXTexIdx;
	}

	if (InputInt(u8"마스크 인덱스", &m_iCurFXMaskTexIdx, 1, pCurFX->m_iMaxMaskTexIdx))
	{
		//가능한 인덱스로 한 번 보정해 주기
		if (m_iCurFXMaskTexIdx < 0)
			m_iCurFXMaskTexIdx = 0;

		if (pCurFX->m_iMaxMaskTexIdx < m_iCurFXMaskTexIdx)
			m_iCurFXMaskTexIdx = pCurFX->m_iMaskTexIdx;

		pCurFX->m_iMaskTexIdx = m_iCurFXMaskTexIdx;
	}

	//단일 이펙트는 여기까지만 보인다.
	if (!bIsParticle)
	{
		End();
		return;
	}

#pragma region  파티클 편집. 캐스팅한다.
	CParticle* pCurParticle = static_cast<CParticle*>(pCurFX);

	_bool bEdited{ false };
	auto moveIter = pCurParticle->m_InstanceDesc.vecMoveCommands.begin();

	_bool bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_SIMPLEMOVE];


	SeparatorText(u8"이동");

	if (Checkbox(u8"SimpleMove", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_SIMPLEMOVE] = bCommand;
		bEdited = true;
	}

	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_DROP];

	if (Checkbox(u8"Drop", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_DROP] = bCommand;
		bEdited = true;
	}
	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_SPREAD];
	if (Checkbox(u8"Spread", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_SPREAD] = bCommand;
		bEdited = true;
	}
	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ASSEMBLE];
	if (Checkbox(u8"Assemble", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ASSEMBLE] = bCommand;
		bEdited = true;
	}

	SeparatorText(u8"가감속");

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ACCELERATION];
	if (Checkbox(u8"Accelerate", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ACCELERATION] = bCommand;
		bEdited = true;
	}
	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_DECELERATE];
	if (Checkbox(u8"Decelerate", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_DECELERATE] = bCommand;
		bEdited = true;
	}
	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ORBITACCELERATION];
	if (Checkbox(u8"Orbit Accelerate", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ORBITACCELERATION] = bCommand;
		bEdited = true;
	}
	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ORBITDECELERATE];
	if (Checkbox(u8"Orbit Decelerate", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ORBITDECELERATE] = bCommand;
		bEdited = true;
	}

	Spacing();

	SeparatorText(u8"회전");

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_WIGGLE];
	if (Checkbox(u8"Wiggle", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_WIGGLE] = bCommand;
		bEdited = true;
	}

	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ORBIT];
	if (Checkbox(u8"공전", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_ORBIT] = bCommand;
		bEdited = true;
	}

	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_CUSTOMORBITAXIS];
	if (Checkbox(u8"회전축 고정", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_CUSTOMORBITAXIS] = bCommand;
		bEdited = true;
	}

	SeparatorText(u8"크기");

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_APPEAR];
	if (Checkbox(u8"Appear", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_APPEAR] = bCommand;
		bEdited = true;
	}
	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_DISAPPEAR];
	if (Checkbox(u8"Disappear", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_DISAPPEAR] = bCommand;
		bEdited = true;
	}



	SeparatorText(u8"기타");

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_TAIL];
	if (Checkbox(u8"Tail", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_TAIL] = bCommand;
		bEdited = true;
	}

	if (IsItemHovered())
	{
		BeginTooltip();
		Text(u8"이펙트가 꼬리에 꼬리를 뭅니다.");
		EndTooltip();
	}

	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_GRAVITY];
	if (Checkbox(u8"Gravity", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_GRAVITY] = bCommand;
		bEdited = true;
	}

	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_SPHERERANDOM];
	if (Checkbox(u8"구 범위 생성", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_SPHERERANDOM] = bCommand;
		bEdited = true;
	}

	if (IsItemHovered())
	{
		BeginTooltip();
		Text(u8"이펙트 생성 범위를 구로 설정합니다. 구 최소 범위와 최대 범위에 따라 범위가 결정됩니다.");
		EndTooltip();
	}

	SameLine();

	bCommand = pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_COLORINTERPOLATE];

	if (Checkbox(u8"색상 보간", &bCommand))
	{
		pCurParticle->m_InstanceDesc.vecMoveCommands[INSTANCE_COLORINTERPOLATE] = bCommand;
		bEdited = true;
	}
	if (IsItemHovered())
	{
		BeginTooltip();
		Text(u8"색상을 라이프 타임에 따라 목표 색상으로 보간합니다.");
		EndTooltip();
	}

	Dummy({ 0.f, 10.f });
	Separator();
	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"시작 딜레이", &pCurParticle->m_InstanceDesc.fStartDelay, .1f, 0.f, 100.f, "%.2f"))
		bEdited = true;
	if (DragFloat(u8"시작 딜레이 랜덤", &pCurParticle->m_InstanceDesc.fStarDelayRandomOffset, .1f, 0.f, 100.f, "%.1f"))
		bEdited = true;

	Dummy({ 0.f, 10.f });
	Separator();
	Dummy({ 0.f, 10.f });

	if (DragFloat3(u8"중점", m_vCenter, .01f, -1000.f, 1000.f, "%.2f"))
	{
		pCurParticle->m_InstanceDesc.vCenter = { m_vCenter[0], m_vCenter[1], m_vCenter[2] };
		bEdited = true;
	}

	if (DragFloat3(u8"범위", m_vRange, .01f, 0.f, 1000.f, "%.2f"))
	{
		pCurParticle->m_InstanceDesc.vRange = { m_vRange[0], m_vRange[1], m_vRange[2] };
		bEdited = true;
	}

	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"최소 구 범위", &pCurParticle->m_InstanceDesc.fMinRange, .01f, 0.f, 1000.f, "%.2f"))
		bEdited = true;
	if (DragFloat(u8"최대 구 범위", &pCurParticle->m_InstanceDesc.fMaxRange, .01f, 0.f, 1000.f, "%.2f"))
		bEdited = true;

	Dummy({ 0.f, 10.f });

	if (DragFloat3(u8"피봇", m_vPivot, .01f, -1000.f, 1000.f, "%.2f"))
	{
		pCurParticle->m_InstanceDesc.vPivot = { m_vPivot[0], m_vPivot[1], m_vPivot[2] };
		bEdited = true;
	}
	Dummy({ 0.f, 10.f });

	if (DragFloat3(u8"회전 축", &(pCurParticle->m_InstanceDesc.vRotationAxis.x), .01f, -1.f, 1.f, "%.2f"))
	{
		bEdited = true;
	}

	Dummy({ 0.f, 10.f });
	Separator();
	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"회전", &(pCurParticle->m_InstanceDesc.vRotation.z), .01f, -360.f, 360.f, "%.2f"))
	{
		//pCurParticle->m_InstanceDesc.vRotation = { m_vRotation[0], m_vRotation[1], m_vRotation[2] };
		bEdited = true;
	}

	if (DragFloat(u8"회전 랜덤", &(pCurParticle->m_InstanceDesc.vRotationRandomOffset.z), .01f, 0.f, 360.f, "%.2f"))
	{
		//pCurParticle->m_InstanceDesc.vRotationRandomOffset = { m_vRotationRandomOffset[0], m_vRotationRandomOffset[1], m_vRotationRandomOffset[2] };
		bEdited = true;
	}

	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"크기", &pCurParticle->m_InstanceDesc.vScale.x, .01f, 0.001f, 1000.f, "%.3f"))
	{
		//pCurParticle->m_InstanceDesc.vScale = { m_vScale[0], m_vScale[1], m_vScale[2] };
		bEdited = true;
	}
	if (DragFloat(u8"크기 랜덤", &pCurParticle->m_InstanceDesc.vScaleRandomOffset.x, .01f, 0.f, 999.f, "%.3f"))
	{
		//pCurParticle->m_InstanceDesc.vScaleRandomOffset = { m_vScaleRandomOffset[0], m_vScaleRandomOffset[1], m_vScaleRandomOffset[2] };
		bEdited = true;
	}

	Dummy({ 0.f, 10.f });

	if (DragFloat3(u8"방향", m_vDir, .01f, 0.f, 100.f, "%.2f"))
	{
		pCurParticle->m_InstanceDesc.vDir = { m_vDir[0], m_vDir[1], m_vDir[2] };
		bEdited = true;
	}
	if (DragFloat3(u8"방향 랜덤", m_vDirRandomOffset, .01f, 0.f, 100.f, "%.2f"))
	{
		pCurParticle->m_InstanceDesc.vDirRandomOffset = { m_vDirRandomOffset[0], m_vDirRandomOffset[1], m_vDirRandomOffset[2] };
		bEdited = true;
	}

	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"속도", &(pCurParticle->m_InstanceDesc.fSpeed), .01f, 0.f, 100.f, "%.2f"))
		bEdited = true;
	if (DragFloat(u8"속도 랜덤", &pCurParticle->m_InstanceDesc.fSpeedRandomOffset, .01f, 0.f, 100.f, "%.2f"))
		bEdited = true;

	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"공전 속도", &(pCurParticle->m_InstanceDesc.fOrbitSpeed), .01f, 0.f, 10000.f, "%.2f"))
		bEdited = true;
	if (DragFloat(u8"공전 속도 랜덤", &pCurParticle->m_InstanceDesc.fOrbitSpeedRandomOffset, .01f, 0.f, 10000.f, "%.2f"))
		bEdited = true;

	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"이동 감가속 속력", &(pCurParticle->m_InstanceDesc.fAccSupplyAmount), .01f, 0.f, 10000.f, "%.2f"))
		bEdited = true;

	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"회전 감가속 속력", &(pCurParticle->m_InstanceDesc.fTurnSupplyAmount), .01f, 0.f, 10000.f, "%.2f"))
		bEdited = true;


	Dummy({ 0.f, 10.f });
	Separator();
	Dummy({ 0.f, 10.f });

	if (ColorEdit3(u8"색상", m_vColor))
	{
		pCurParticle->m_InstanceDesc.vColor = { m_vColor[0], m_vColor[1], m_vColor[2] };
		bEdited = true;
	}

	if (DragFloat3(u8"색상 랜덤", m_vColorRandomOffset, .01f, 0.f, 1.f, "%.2f"))
	{
		pCurParticle->m_InstanceDesc.vColorRandomOffset = { m_vColorRandomOffset[0], m_vColorRandomOffset[1], m_vColorRandomOffset[2] };
		bEdited = true;
	}

	if (ColorEdit3(u8"목표 색상", &(pCurParticle->m_InstanceDesc.vTargetColor.x)))
		bEdited = true;


	if (DragFloat3(u8"목표 색상 랜덤", &(pCurParticle->m_InstanceDesc.vTargetColorRandomOffset.x), .01f, 0.f, 1.f, "%.2f"))
		bEdited = true;


	Dummy({ 0.f, 10.f });

	if (DragFloat(u8"알파", &pCurParticle->m_InstanceDesc.fAlpha, .01f, 0.f, 1.f, "%.2f"))
		bEdited = true;

	//if (DragFloat(u8"알파 랜덤", &pCurParticle->m_InstanceDesc.fAlphaRandomOffset, .01f, 0.f, 1.f, "%.2f"))
	//	bEdited = true;



#pragma endregion

	End();

	if (bEdited)
	{
		pCurParticle->Update_InstanceInfo();
	}


}

//이펙트의 플레이 바를 띄운다.
void CFXToolDirector::Render_FXPlayBar(_float _fTimeDelta)
{

	//아무것도 선택되지 않았다면 return
	if (m_eSelected == SELECTED_END)
		return;


	Begin(u8"Bar", nullptr, ImGuiWindowFlags_NoTitleBar);

	CEffect* pCurFX{ nullptr };

	if (m_eSelected == SELECTED_SINGLE_FX)
		pCurFX = m_FXs[m_iSelectedFXIdx];
	else if (m_eSelected == SELECTED_PARTICLE_FX)
		pCurFX = m_Particles[m_iSelectedParticleIdx];
	else if (m_eSelected == SELECTED_MULTI_FX)
		pCurFX = m_MultiFXs[m_iSelectedMultiFXIdx];

	CHECK_NULLPTR(pCurFX);

	Columns(3);
	SetColumnWidth(0, 220.f);
	SetColumnWidth(1, 110.f);

	m_eSelected == SELECTED_MULTI_FX ? Text("Cur Duration %.2f", pCurFX->m_fDuration.first) :
		Text("Cur Lifetime %.2f", pCurFX->m_fLifeRatio);

	if (m_eSelected == SELECTED_SINGLE_FX)
	{
		Dummy(ImVec2(0, 78));

		//플레이 바의 첫번째 열에 이펙트의 현재 속성 값을 띄운다.

		Text(u8"위치");
		SameLine();
		Text("%.2f %.2f %.2f\t", pCurFX->m_vCurPos.x, pCurFX->m_vCurPos.y, pCurFX->m_vCurPos.z);
		//SameLine();



		Text(u8"회전");
		SameLine();
		Text("%.2f %.2f %.2f\t", pCurFX->m_vCurRot.x, pCurFX->m_vCurRot.y, pCurFX->m_vCurRot.z);
		//SameLine();



		Text(u8"크기");
		SameLine();
		Text("%.2f %.2f %.2f\t", pCurFX->m_vCurScale.x, pCurFX->m_vCurScale.y, pCurFX->m_vCurScale.z);


		Text(u8"Color");
		SameLine();
		Text("\t%.2f %.2f %.2f\t", pCurFX->m_vCurRColor.x, pCurFX->m_vCurRColor.y, pCurFX->m_vCurRColor.z);

		/*
				Text(u8"G");
				SameLine();
				Text("\t%.2f %.2f %.2f\t", pCurFX->m_vCurGColor.x, pCurFX->m_vCurGColor.y, pCurFX->m_vCurGColor.z);
				SameLine();
				if (SmallButton(u8"G 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
				{
					////m_bOpenKeyframeEditor = false;
					FX_KEYFRAME newKeyframe{};
					newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
					newKeyframe.eEasing = EASE_OUT;
					newKeyframe.vValue = pCurFX->m_vCurGColor;

					pCurFX->Add_Keyframe(newKeyframe, KF_GCOLOR);
				}

				Text(u8"B");
				SameLine();
				Text("\t%.2f %.2f %.2f\t", pCurFX->m_vCurBColor.x, pCurFX->m_vCurBColor.y, pCurFX->m_vCurBColor.z);
				SameLine();
				if (SmallButton(u8"B 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
				{
					////m_bOpenKeyframeEditor = false;
					FX_KEYFRAME newKeyframe{};
					newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
					newKeyframe.eEasing = EASE_OUT;
					newKeyframe.vValue = pCurFX->m_vCurBColor;

					pCurFX->Add_Keyframe(newKeyframe, KF_BCOLOR);
				}
				*/
		Text(u8"알파");
		SameLine();
		Text("\t%.2f\t\t\t", pCurFX->m_fCurAlpha);


		Text(u8"마스크 임계");
		SameLine();
		Text("\t%.2f\t", pCurFX->m_fCurMaskThreshold);


		Text(u8"UV 이동 오프셋");
		SameLine();
		Text("\t%.2f %.2f\t", pCurFX->m_vCurUVOffset.x, pCurFX->m_vCurUVOffset.y);


		Text(u8"마스크 UV 이동 오프셋");
		SameLine();
		Text("\t%.2f %.2f\t", pCurFX->m_vCurMaskUVOffset.x, pCurFX->m_vCurMaskUVOffset.y);


		Text(u8"마스크 UV 회전 오프셋");
		SameLine();
		Text("\t%.2f\t", pCurFX->m_vCurMaskUVAngle);

	}


	NextColumn();

	Dummy(ImVec2(0, 98));

	if (m_eSelected == SELECTED_SINGLE_FX)
	{
		if (SmallButton(u8"위치 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{

			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = pCurFX->m_vCurPos;

			pCurFX->Add_Keyframe(newKeyframe, KF_POS);
		}

		if (SmallButton(u8"회전 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{

			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = pCurFX->m_vCurRot;

			pCurFX->Add_Keyframe(newKeyframe, KF_ROT);
		}


		if (SmallButton(u8"크기 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{

			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = pCurFX->m_vCurScale;

			pCurFX->Add_Keyframe(newKeyframe, KF_SCALE);
		}

		if (SmallButton(u8"색 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{
			////m_bOpenKeyframeEditor = false;
			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = pCurFX->m_vCurRColor;

			pCurFX->Add_Keyframe(newKeyframe, KF_RCOLOR);
		}


		if (SmallButton(u8"알파 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{
			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = { pCurFX->m_fCurAlpha, 0.f, 0.f };

			pCurFX->Add_Keyframe(newKeyframe, KF_ALPHA);
		}


		if (SmallButton(u8"임계 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{
			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = { pCurFX->m_fCurMaskThreshold, 0.f, 0.f };

			pCurFX->Add_Keyframe(newKeyframe, KF_MASK);
		}

		if (SmallButton(u8"uv 이동 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{

			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = { pCurFX->m_vCurUVOffset.x, pCurFX->m_vCurUVOffset.y, 0.f };

			pCurFX->Add_Keyframe(newKeyframe, KF_UVOFFSET);
		}

		if (SmallButton(u8"Mask 이동 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{

			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = { pCurFX->m_vCurMaskUVOffset.x, pCurFX->m_vCurMaskUVOffset.y, 0.f };

			pCurFX->Add_Keyframe(newKeyframe, KF_MASKUVOFFSET);
		}


		if (SmallButton(u8"Mask 회전 추가") && (0.f < pCurFX->m_fLifeRatio && pCurFX->m_fLifeRatio < 1.f))
		{
			FX_KEYFRAME newKeyframe{};
			newKeyframe.fTimeRatio = pCurFX->m_fLifeRatio;
			newKeyframe.eEasing = EASE_OUT;
			newKeyframe.vValue = { pCurFX->m_vCurMaskUVAngle, 0.f, 0.f };

			pCurFX->Add_Keyframe(newKeyframe, KF_MASKUVANGLE);
		}
	}


	NextColumn();

	if (m_bPlayingBar)
		PushStyleColor(ImGuiCol_Button, COLOR_ORANGE);
	else
		PushStyleColor(ImGuiCol_Button, COLOR_LIGHTPINK);

	//재생
	if (ArrowButton(u8"Play", ImGuiDir_Right) || m_pGameInstance->Get_KeyState(DIK_SPACE, KEY_DOWN))
	{
		m_bPlayingBar = !m_bPlayingBar;

		//재생 다시 시작하는 상태에서 
		if (m_bPlayingBar)
		{
			//파티클은 무조건 reset duration 하도록 하세요
			if (m_eSelected == SELECTED_PARTICLE_FX)
			{
				m_Particles[m_iSelectedParticleIdx]->Reset_Duration();
				m_fCurPlayDuration = 0.f;
				static_cast<CParticle*>(m_Particles[m_iSelectedParticleIdx])->Update_InstanceInfo();
			}
			//나머지 fx 편집 중 재생 시 duration이 넘어가 있었다면 ? duration reset
			else if (m_fCurPlayDuration >= m_fTotalPlayDuration)
			{
				m_fCurPlayDuration = 0.f;

				if (m_eSelected == SELECTED_SINGLE_FX)
				{
					m_FXs[m_iSelectedFXIdx]->Reset_Duration();
				}
				else if (m_eSelected == SELECTED_MULTI_FX)
				{
					m_MultiFXs[m_iSelectedMultiFXIdx]->Reset_Duration();
				}
			}
		}

		SameLine();
	}

	PopStyleColor();
	SameLine();


	NewLine();

	Text(u8"Duration");

	//재생 바 폭 맞추기
	_float fWidth = GetContentRegionAvail().x;
	PushItemWidth(fWidth);


	if (m_eSelected == SELECTED_SINGLE_FX)
	{
		MakeBar_SingleFXProperty(_fTimeDelta, fWidth);
	}
	else if (m_eSelected == SELECTED_PARTICLE_FX)
	{
		MakeBar_ParticleFXProperty(_fTimeDelta, fWidth);
	}
	else if (m_eSelected == SELECTED_MULTI_FX)
	{
		MakeBar_MultiFXProperty(_fTimeDelta, fWidth);
	}

	if (m_bPlayingBar)
	{
		m_fCurPlayDuration += _fTimeDelta;

		if (m_fTotalPlayDuration <= m_fCurPlayDuration)
		{
			m_fCurPlayDuration = m_fTotalPlayDuration;
			m_bPlayingBar = m_bLoopingBar ? true : false;

		}
	}

	PopItemWidth();

	End();
}

//키프레임 일련을 렌더한다.
void CFXToolDirector::MakeBar_SingleFXProperty(_float _fTimeDelta, _float _fWidth)
{
	if (m_iSelectedFXIdx == -1)
		return;

	CEffect* pCurFX = m_FXs[m_iSelectedFXIdx];
	CHECK_NULLPTR(pCurFX);


	if (SliderFloat("##", &m_fCurPlayDuration, 0.f, m_fTotalPlayDuration, "%.2f"))
	{
		m_FXs[m_iSelectedFXIdx]->m_fDuration.first = m_fCurPlayDuration;

		if (!m_bPlayingBar)
			m_FXs[m_iSelectedFXIdx]->Late_Tick(_fTimeDelta);
	}

	//키프레임 팝업 사이즈
	ImVec2 vPopupSize = { 160.f, 140.f };

	//서로 서로 띄워준다.
	Dummy(ImVec2(0, 35));

	ImVec2 vPos = GetCursorScreenPos();
	_float fInitialYPos = vPos.y - 250.f;

	//각 키프레임 플레이 바의 위치의 상대 위치로 매칭하기.

	//위치
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_POS);

	//회전
	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_ROT);

	//크기
	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_SCALE);


	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_RCOLOR);

	//Dummy(ImVec2(0, 15));
	//Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_GCOLOR);

	//Dummy(ImVec2(0, 15));
	//Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_BCOLOR);

	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_ALPHA);

	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_MASK);

	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_UVOFFSET);

	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_MASKUVOFFSET);

	Dummy(ImVec2(0, 15));
	Make_KeyframeList(_fWidth, fInitialYPos, pCurFX, KF_MASKUVANGLE);


	if (BeginPopup(u8"키프레임"))
	{

#pragma region 팝업 위 제목

		string strTitle{ "" };
		switch (m_eSelectedProperty)
		{
		case KF_POS:
			strTitle = "Pos";
			break;
		case KF_ROT:
			strTitle = "Rot";
			break;
		case KF_SCALE:
			strTitle = "Scale";
			break;
		case KF_RCOLOR:
			strTitle = "Color";
			break;
		case KF_GCOLOR:
			strTitle = "G";
			break;
		case KF_BCOLOR:
			strTitle = "B";
			break;
		case KF_ALPHA:
			strTitle = "Alpha";
			break;
		case KF_MASK:
			strTitle = "Mask Threshold";
			break;
		case KF_UVOFFSET:
			strTitle = "UV Offset";
			break;
		case KF_MASKUVOFFSET:
			strTitle = "Mask UV Offset";
			break;
		case KF_MASKUVANGLE:
			strTitle = "Mask UV Angle";
			break;
		default:
			break;
		}

		Text(strTitle.c_str());

#pragma endregion

		if (DragFloat(u8"시간", &m_FXs[m_iSelectedFXIdx]->m_Keyframes[m_eSelectedProperty][m_iSelectedKFIdx].fTimeRatio, .01f, .01f, .99f, "%.2f"))
		{

		}

		//어떤 속성을 편집하느냐에 따라 최소, 최대 범위 정한다.
		_float2 vValueRange{ 0.f, 1.f };
		if (m_eSelectedProperty == KF_POS)
			vValueRange = { -5000.f, 5000.f };
		else if (m_eSelectedProperty == KF_ROT)
			vValueRange = { -360.f, 360.f };
		else if (m_eSelectedProperty == KF_SCALE)
			vValueRange = { .001f, 10000.f };
		else if (m_eSelectedProperty == KF_UVOFFSET)
			vValueRange = { -100.f, 100.f };
		else if (m_eSelectedProperty == KF_MASKUVOFFSET)
			vValueRange = { -100.f, 100.f };
		else if (m_eSelectedProperty == KF_MASKUVANGLE)
			vValueRange = { -360.f, 360.f };

		if (DragFloat3("Value", m_vKFPopupValue, .01f, vValueRange.x, vValueRange.y, "%.3f"))
		{
			if (m_eSelectedProperty == KF_SCALE)
			{
				m_vKFPopupValue[0] = m_vKFPopupValue[0] <= 0.f ? .001f : m_vKFPopupValue[0];
				m_vKFPopupValue[1] = m_vKFPopupValue[1] <= 0.f ? .001f : m_vKFPopupValue[1];
				m_vKFPopupValue[2] = m_vKFPopupValue[2] <= 0.f ? .001f : m_vKFPopupValue[2];
			}

			m_FXs[m_iSelectedFXIdx]->m_Keyframes[m_eSelectedProperty][m_iSelectedKFIdx].vValue = _float3{ m_vKFPopupValue[0], m_vKFPopupValue[1], m_vKFPopupValue[2] };
			m_FXs[m_iSelectedFXIdx]->Late_Tick(_fTimeDelta);
			m_FXs[m_iSelectedFXIdx]->m_fDuration.first = m_fCurPlayDuration;

		}
		if (Combo(u8"Easing", &m_eKFPopupEasing, m_Easing.data(), (_int)m_Easing.size()))
		{
			m_FXs[m_iSelectedFXIdx]->m_Keyframes[m_eSelectedProperty][m_iSelectedKFIdx].eEasing = (EASING)m_eKFPopupEasing;
			m_FXs[m_iSelectedFXIdx]->Late_Tick(_fTimeDelta);
			m_FXs[m_iSelectedFXIdx]->m_fDuration.first = m_fCurPlayDuration;
		}

		if (Button(u8"키프레임 삭제"))
		{
			//			m_bOpenKeyframeEditor = false;
			CloseCurrentPopup();
			m_FXs[m_iSelectedFXIdx]->Delete_Keyframe(m_eSelectedProperty, m_iSelectedKFIdx);
		}

		SameLine();
		if (Button(u8"닫기"))
		{
			//			m_bOpenKeyframeEditor = false;
			CloseCurrentPopup();
		}

		EndPopup();
	}
}

//한 변수의 키프레임을 렌더한다.
void CFXToolDirector::Make_KeyframeList(_float _fWidth, _float _fInitialYPos, CEffect* _pCurFX, KF_PROPERTY _eRenderProperty)
{
	ImVec2 vPos = GetCursorScreenPos();
	ImVec2 vPopupSize = { 160.f, 140.f };
	_int iTempKFIdx = 0;

	ImU32 vColor = IM_COL32(255, 0, 100, 255);
	if (IsMouseHoveringRect(ImVec2(vPos.x, vPos.y - 5), ImVec2(vPos.x + _fWidth, vPos.y + 5)))
		vColor = IM_COL32(255, 255, 100, 255);

	GetWindowDrawList()->AddLine(vPos, ImVec2(vPos.x + _fWidth, vPos.y), vColor, 1.f);

	for (auto& keyframe : _pCurFX->m_Keyframes[_eRenderProperty])
	{
		//_float fRatio = (_pCurFX->m_fLifetime.first + (keyframe.fTimeRatio * (_pCurFX->m_fLifetime.second - _pCurFX->m_fLifetime.first)));
		_float fRatio = keyframe.fTimeRatio;
		_float fPosX = fRatio * _fWidth /*/ _pCurFX->m_fDuration.second*/;
		ImVec2 vCurPos = vPos + ImVec2{ fPosX, 2.f };

		GetWindowDrawList()->AddCircleFilled(vCurPos, 6.0f, IM_COL32(255, 255, 100, 255));

		ImVec2 mousePos = GetIO().MousePos;
		float fDistance = (_float)sqrt(pow(mousePos.x - vCurPos.x, 2) + pow(mousePos.y - vCurPos.y, 2));

		//키프레임 범위 안에서 마우스 클릭 발생 시 키프레임 위치에 ui 띄움
		if (fDistance <= 8.0f && IsMouseClicked(0))
		{
			m_eSelectedProperty = _eRenderProperty;
			m_iSelectedKFIdx = iTempKFIdx;

			//위치, 스케일 키프레임 옆에 맞춘다
			SetNextWindowSize(vPopupSize);
			SetNextWindowPos(ImVec2(vCurPos.x, _fInitialYPos));

			_float3 vValue = m_FXs[m_iSelectedFXIdx]->m_Keyframes[m_eSelectedProperty][m_iSelectedKFIdx].vValue;
			memcpy(m_vKFPopupValue, &vValue, sizeof(_float3));
			m_eKFPopupEasing = m_FXs[m_iSelectedFXIdx]->m_Keyframes[m_eSelectedProperty][m_iSelectedKFIdx].eEasing;

			OpenPopup(u8"키프레임");

		}

		++iTempKFIdx;
	}

}

void CFXToolDirector::MakeBar_ParticleFXProperty(_float _fTimeDelta, _float _fWidth)
{
	if (m_iSelectedParticleIdx == -1)
		return;


	CEffect* pCurFX = m_Particles[m_iSelectedParticleIdx];
	CHECK_NULLPTR(pCurFX);


	if (SliderFloat("##", &m_fCurPlayDuration, 0.f, m_fTotalPlayDuration, "%.2f"))
	{
		m_Particles[m_iSelectedParticleIdx]->Late_Tick(_fTimeDelta);
		m_Particles[m_iSelectedParticleIdx]->m_fDuration.first = m_fCurPlayDuration;
	}
}

void CFXToolDirector::MakeBar_MultiFXProperty(_float _fTimeDelta, _float _fWidth)
{
	if (m_iSelectedFXIdx == -1)
		return;

	CEffect* pCurFX = m_MultiFXs[m_iSelectedMultiFXIdx];
	CHECK_NULLPTR(pCurFX);


	if (SliderFloat("##", &m_fCurPlayDuration, 0.f, m_fTotalPlayDuration, "%.2f"))
	{
		m_MultiFXs[m_iSelectedMultiFXIdx]->Late_Tick(_fTimeDelta);
		m_MultiFXs[m_iSelectedMultiFXIdx]->m_fDuration.first = m_fCurPlayDuration;
	}
}

void CFXToolDirector::Render_MultiFXHierarchy()
{
	Begin(u8"복합 이펙트", nullptr, ImGuiWindowFlags_NoCollapse);


	SeparatorText(u8"복합 이펙트 목록");

	static ImGuiTextFilter filter;
	string szName;
	filter.Draw(u8"복합 이펙트 검색");

	BeginChild(u8"목록", ImVec2(0, 200), true);

	for (_int i = 0; i < m_MultiFXs.size(); ++i)
	{

		if (filter.PassFilter(m_MultiFXs[i]->m_strFXName.c_str())
			&& Selectable(m_MultiFXs[i]->m_strFXName.c_str(), m_iSelectedMultiFXIdx == i))
		{
			////m_bOpenKeyframeEditor = false;
			m_iSelectedMultiFXIdx = i;

			m_eSelected = SELECTED_MULTI_FX;

			m_bPlayingBar = false;
			//m_bLooping = m_MultiFXs[i]->m_bIsLoop;
			/*m_iCurFXPassIdx = m_FXs[i]->m_iPassIdx;
			m_iCurFXTexIdx = m_FXs[i]->m_iTexIdx;
			m_iCurFXMaskTexIdx = m_FXs[i]->m_iMaskTexIdx;*/

			m_fTotalPlayDuration = m_MultiFXs[i]->m_fDuration.second;

			//m_fLifetime[0] = m_MultiFXs[i]->m_fLifetime.first;
			//m_fLifetime[1] = m_MultiFXs[i]->m_fLifetime.second;

		}

		if (m_iSelectedMultiFXIdx == i && IsItemHovered() && IsMouseReleased(1))
			OpenPopup("MultiFXMenu");

	}

	//우측 키를 누르면 나오는 메뉴들
	if (BeginPopup("MultiFXMenu"))
	{

		if (MenuItem(u8"삭제"))
		{
			string strName = m_MultiFXs[m_iSelectedMultiFXIdx]->m_strFXName;
			string strPath = MULTIFX_PATH + strName + ".bin";
			MoveTo_TrashBin(strPath);


			Safe_Release(m_MultiFXs[m_iSelectedMultiFXIdx]);
			m_MultiFXs.erase(m_MultiFXs.begin() + m_iSelectedMultiFXIdx);
			m_eSelected = SELECTED_END;

			if (m_MultiFXs.size() <= m_iSelectedMultiFXIdx)
				--m_iSelectedMultiFXIdx;
			else if (m_MultiFXs.empty())
			{
				m_iSelectedMultiFXIdx = -1;
			}
		}


		EndPopup();
	}


	EndChild();


	if (m_iSelectedMultiFXIdx != -1)
	{
		SeparatorText(u8"추가된 이펙트");

		for (int j = 0; j < m_MultiFXs[m_iSelectedMultiFXIdx]->Get_Size(); ++j)
		{
			Selectable(static_cast<CMultiEffect*>(m_MultiFXs[m_iSelectedMultiFXIdx])->m_FXs[j]->m_strFXName.c_str());

			////TODO: 
			//if (Selectable(m_CompositeEffects[i]->m_strName.c_str(), m_iEffectIdxInComposition))
			//{
			//	m_iEffectIdxInComposition = i;
				/*m_curEffectName = m_CompositeEffects[i]->m_strName;
				m_fTotalPlayDuration = m_CompositeEffects[i]->m_fDestDuration;*/

				//}

		}
	}


	End();
}

HRESULT CFXToolDirector::Ready_FXPrototypeVector()
{
	CComponent_Manager::PROTOTYPES* pStaticProtoMap{ nullptr };

	pStaticProtoMap = m_pGameInstance->Get_ComMap(LEVEL_STATIC);
	if (nullptr == pStaticProtoMap)
		return E_FAIL;

	Ready_Ingredient(TEXT("Texture_FX"), &m_FXTexList, pStaticProtoMap);
	Ready_Ingredient(TEXT("Texture_FX"), &m_FXMaskTexList, pStaticProtoMap);



	Ready_Ingredient(TEXT("VIBuffer_Rect"), &m_FXBufferList, pStaticProtoMap);
	Ready_Ingredient(TEXT("VIBuffer_UpperRect"), &m_FXBufferList, pStaticProtoMap);

	wstring strModelTag = TEXT("Model_");

	Ready_Ingredient(strModelTag + L"BushCutM", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"Cylinder", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"Dash", &m_FXBufferList, pStaticProtoMap);
	Ready_Ingredient(strModelTag + L"Torus", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"Smoke", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"Sword", &m_FXBufferList, pStaticProtoMap);
	Ready_Ingredient(strModelTag + L"UpwardSlash", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"Tornado", &m_FXBufferList, pStaticProtoMap);
	Ready_Ingredient(strModelTag + L"Vacuum", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"FX", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"PopStar", &m_FXBufferList, pStaticProtoMap);

	Ready_Ingredient(strModelTag + L"SkySphere", &m_FXBufferList, pStaticProtoMap);
	Ready_Ingredient(strModelTag + L"RayArrow", &m_FXBufferList, pStaticProtoMap);

	return S_OK;
}

void CFXToolDirector::MoveTo_TrashBin(string& filePath)
{
	path trashFolderPath("../Bin/Resources/Effects/TrashBin/");

	try
	{
		if (exists(filePath))
		{
			path srcPath(filePath);
			path dstPath = path(trashFolderPath) / srcPath.filename();

			// 휴지통 폴더로 파일 이동
			rename(srcPath, dstPath);

			cout << "File moved to trash folder: " << dstPath << endl;
		}
		else
		{
			cout << "File does not exist: " << filePath << endl;
		}
	}
	catch (const filesystem_error& e)
	{
		cerr << "Filesystem error: " << e.what() << endl;
	}

}

void CFXToolDirector::Ready_Ingredient(wstring wstrSearchTag, vector<char*>* vecCombo, CComponent_Manager::PROTOTYPES* comMap)
{
	for (auto& comPair : *comMap)
	{
		if (comPair.first.find(wstrSearchTag) != wstring::npos)
		{
			string strComName = CUtils::WstrToStr(comPair.first);
			strComName = string(strComName.begin() + 20, strComName.end());
			char* tempStr = new char[strComName.size() + 1];
			strcpy_s(tempStr, strComName.size() + 1, strComName.c_str());
			vecCombo->push_back(tempStr);
			//Safe_Delete_Array(tempStr);
		}
	}
}
//
//void CFXToolDirector::FilteredCombo(const char* label, int* current_item, const vector<char*>& items)
//{
//	static unordered_map<string, ImGuiTextFilter> filter_map;
//
//	// 유일한 필터 키를 생성
//	string key = label;
//	if (filter_map.find(key) == filter_map.end())
//	{
//		filter_map[key] = ImGuiTextFilter();
//	}
//
//	ImGuiTextFilter& filter = filter_map[key];
//	vector<char*> filtered_items;
//
//	// 검색 입력 필드
//	filter.Draw((string("Search##") + label).c_str());
//
//	// 검색어에 따라 아이템 필터링
//	for (const auto& item : items)
//	{
//		if (filter.PassFilter(item))
//		{
//			filtered_items.push_back(item);
//		}
//	}
//
//	// 현재 선택된 아이템의 표시
//	const char* current_item_text = (filtered_items.empty() || *current_item >= static_cast<int>(filtered_items.size()))
//		? "" : filtered_items[*current_item];
//
//	// 콤보박스 아이템 표시
//	if (ImGui::BeginCombo(label, current_item_text))
//	{
//		for (size_t i = 0; i < filtered_items.size(); i++)
//		{
//			bool is_selected = (find(items.begin(), items.end(), filtered_items[i]) - items.begin()) == *current_item;
//			if (ImGui::Selectable(filtered_items[i], is_selected))
//			{
//				*current_item = static_cast<int>(distance(items.begin(), find(items.begin(), items.end(), filtered_items[i])));
//			}
//			if (is_selected)
//			{
//				ImGui::SetItemDefaultFocus();
//			}
//		}
//		ImGui::EndCombo();
//	}
//}

HRESULT CFXToolDirector::Add_Components()
{
	return S_OK;
}

CFXToolDirector* CFXToolDirector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFXToolDirector* pInstance = new CFXToolDirector(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CFXToolDirector"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFXToolDirector::Clone(void* pArg)
{
	CFXToolDirector* pInstance = new CFXToolDirector(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CFXToolDirector"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFXToolDirector::Free()
{
	for (auto& pFX : m_FXs)
		Safe_Release(pFX);

	m_FXs.clear();

	for (auto& pFX : m_MultiFXs)
		Safe_Release(pFX);

	m_MultiFXs.clear();

	for (auto& sz : m_FXTexList)
		Safe_Delete(sz);
	m_FXTexList.clear();

	for (auto& sz : m_FXBufferList)
		Safe_Delete(sz);
	m_FXBufferList.clear();

	for (auto& sz : m_FXMaskTexList)
		Safe_Delete(sz);
	m_FXMaskTexList.clear();

	__super::Free();
}


