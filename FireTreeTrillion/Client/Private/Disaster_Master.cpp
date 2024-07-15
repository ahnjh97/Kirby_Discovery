#include "stdafx.h"
#include "Disaster_Master.h"
#include "Baum.h"
#include "FinaleKirby.h"
#include "Light.h"
#include "Particle.h"

#include "FinalePartical_Maker.h"
#include "FinaleCut_ControlCenter.h"

CDisaster_Master::CDisaster_Master(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CDisaster_Master::CDisaster_Master(const CDisaster_Master& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CDisaster_Master::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDisaster_Master::Initialize(void* pArg)
{
	m_pKirby = (CFinaleKirby*)pArg;
	Safe_AddRef(m_pKirby);

	GAMEOBJECT_DESC Desc = {};
	Desc.fSpeedPerSec = 0.f;
	Desc.fRotationPerSec = 0.f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	LIGHT_DESC			LightDesc{};
	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	LightDesc.fRange = 70.f;
	LightDesc.vDiffuse = _float4(.8f, .3f, .06f, 1.f);
	LightDesc.vAmbient = _float4(0.3f, .3f, .3f, 1.f);
	LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
	if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
		return E_FAIL;

	m_pLight = CGameInstance::Get_Instance()->Get_LightLastAddress();
	Safe_AddRef(m_pLight);

	m_pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinalePartical_Maker")));
	Safe_AddRef(m_pMaker);


	for (_int i = 0; i < 20; ++i)
	{
		_float4 vKirbyPos = m_pKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
		_float fZOffSet = { 0.f };
		fZOffSet = CUtils::Make_RandomInt(0, 1) == 0 ? CUtils::Make_RandomFloat(-700.f, -200.f) : CUtils::Make_RandomFloat(200.f, 700.f);
		_float fXOffSet = { 0.f };
		fXOffSet = CUtils::Make_RandomFloat(-300.f, 300.f);
		_float fYOffSet = { 0.f };
		fYOffSet = CUtils::Make_RandomFloat(-50.f, 50.f);

		vKirbyPos.x += fXOffSet;
		vKirbyPos.y += fYOffSet;
		vKirbyPos.z += fZOffSet;
		m_pMaker->Make_BuildingPartical(vKirbyPos);
	}


	return S_OK;
}

_int CDisaster_Master::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;
	if (m_pKirby == nullptr)
		return OBJ_DEAD;

	_float4 vKirbyPos = m_pKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

	if (m_bKirbyCutSceneStart == false)
	{
		if (m_pLight != nullptr)
		{
			_float4 vLightPos = vKirbyPos;
			vLightPos.x += 50.f;
			vLightPos.y += 40.f;
			m_pLight->Update_LightPos(vLightPos);
			m_pGameInstance->Update_LightShadow(vLightPos, vKirbyPos);
		}

		if (vKirbyPos.x > 15.f)
			m_fMakeBaumDelay += fTimeDelta;

		m_fAirParticleDelay += fTimeDelta;
		m_fBuildingParticleDelay += fTimeDelta;

		// 헛방 바움을 생성한다.
		Make_MissBaum();

		// 공기 중에 날아댕기는 파티클을 구현하였다.
		Make_AirParticle();

		Moving_FinaleRoad(vKirbyPos.x);
		Moving_TargetBaum(vKirbyPos.x);

		if (vKirbyPos.x > 1550.f)
		{
			m_bKirbyCutSceneStart = true;


			// 1틱 파편을 대량 생산,
			// 1틱 건물을 대량 생산.
		}
	}
	else if (m_bKirbyCutSceneStart == true)
	{

		if (m_pLight != nullptr)
		{
			_float4 vLightPos = m_pKirby->Compute_RootPos();
			_float4 vLightKirbyPos = vLightPos;
			vLightPos.x += 30.f;
			vLightPos.y += 30.f;
			m_pLight->Update_LightPos(vLightPos);
			m_pGameInstance->Update_LightShadow(vLightPos, vLightKirbyPos);
		}


		m_fAirParticleDelay += fTimeDelta;
		//m_fBuildingParticleDelay += fTimeDelta;

		// 공기 중에 날아댕기는 파티클을 구현하였다.
		Make_CutAirParticle();
	}

	return OBJ_NOEVENT;
}

void CDisaster_Master::Late_Tick(_float fTimeDelta)
{

}

void CDisaster_Master::Make_MissBaum()
{
	// 운석은 2.5초 간격으로 생성되며
	// 운석은 2초동안 공중에서 날아든다.
	if (m_fMakeBaumDelay > 1.1f)
	{
		_float4 vKirbyPos = m_pKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
		_float fKirbySpeed = m_pKirby->Get_KirbyInfo()->m_fMoveSpeed;
		_float4 vNewMyPos = _float4(vKirbyPos.x + 170.f, vKirbyPos.y + 40.f, vKirbyPos.z, 1.f);

		_float fZOffSet = { 0.f };
		fZOffSet = CUtils::Make_RandomInt(0, 1) == 0 ? CUtils::Make_RandomFloat(-70.f, -50.f) : CUtils::Make_RandomFloat(50.f, 70.f);
		_float fXOffSet = { 0.f };
		fXOffSet = CUtils::Make_RandomFloat(0.f, 40.f);

		wstring wstrtag = CUtils::Make_RandomInt(0, 1) == 0 ? TEXT("Baum") : TEXT("StarPiece");
		CBaum::BAUMDESC baumdesc = {};
		baumdesc.vBaumMoveDir = vKirbyPos - vNewMyPos;
		baumdesc.fBaumSpeed = 1.f;
		vNewMyPos.x += (fKirbySpeed * 2.f) + fXOffSet;
		vNewMyPos.z += fZOffSet;
		baumdesc.vPos = vNewMyPos;
		baumdesc.wstrModelName = wstrtag;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Baum"), TEXT("Prototype_GameObject_Baum"), &baumdesc)))
			return;
		m_fMakeBaumDelay = 0.f;
	}
}


// 1초 후에 정확히 그 위치에 떨어지는 운석을 만든다.
void CDisaster_Master::Make_OnTerrainBaum(_float4 _vTargetPos, _bool _bBaum)
{
	_float4 vTargetPos = _vTargetPos;
	_float fKirbySpeed = m_pKirby->Get_KirbyInfo()->m_fMoveSpeed;
	_float4 vNewMyPos = _float4(vTargetPos.x + 100.f, vTargetPos.y + 70.f, vTargetPos.z, 1.f);

	CBaum::BAUMDESC baumdesc = {};
	baumdesc.vBaumMoveDir = vTargetPos - vNewMyPos;
	baumdesc.fBaumSpeed = 1.f;
	baumdesc.vPos = vNewMyPos;
	baumdesc.wstrModelName = _bBaum == true ? TEXT("Baum") : TEXT("StarPiece");
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_Baum"), TEXT("Prototype_GameObject_Baum"), &baumdesc)))
		return;
}

void CDisaster_Master::Make_FinaleRoad(ROADTYPE eType, MOVECMD eMoveType, _float3 vTargetPos, _float3 vLookDir, _float3 vDestPos, _float3 vDestDir, _float fDestZAngle)
{
	CFinaleRoadGrouper::ROADGROUPER_DESC roadGrouperDesc = {};

	roadGrouperDesc.eRoadType = eType;
	roadGrouperDesc.eMoveCommand = eMoveType;
	roadGrouperDesc.fDestZAngle = fDestZAngle;


	_float4x4 InitMat = _float4x4::Identity;
	InitMat.Translation(vTargetPos);
	CUtils::Rotation(InitMat, CUtils::Make_Quat_FromDir(vLookDir));
	roadGrouperDesc.matWorld = InitMat;

	roadGrouperDesc.vDestPos = vDestPos;
	roadGrouperDesc.vDestDir = vDestDir;
	roadGrouperDesc.vDestDir.Normalize();

	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoadGrouper"),
		TEXT("Prototype_GameObject_FinaleRoadGrouper"), &roadGrouperDesc)))
		return;
}

void CDisaster_Master::Make_CutAirParticle()
{
	_float4 vKirbyPos = m_pKirby->Compute_RootPos();

	if (m_bCutInitializeParticle == true)
	{
		for (_int i = 0; i < 100; i++)
		{
			_float4 vNewMyPos = _float4(
				vKirbyPos.x + CUtils::Make_RandomFloat(100.f, 500.f),
				vKirbyPos.y + CUtils::Make_RandomFloat(-300.f, 300.f),
				vKirbyPos.z + CUtils::Make_RandomFloat(-300.f, 300.f),
				1.f);
			m_pMaker->Make_Partical(1, vNewMyPos, 0.f, 2.f, 1.f, _float4(0.f, 1.f, 0.f, 0.f), 10.f, CUtils::Make_RandomFloat(15.f, 30.f), true);
		}

		m_bCutInitializeParticle = false;
	}

	if (m_fAirParticleDelay > 0.1f)
	{
		_float fZOffSet = { 0.f };
		fZOffSet = CUtils::Make_RandomInt(0, 1) == 0 ? CUtils::Make_RandomFloat(-200.f, -50.f) : CUtils::Make_RandomFloat(50.f, 200.f);
		_float fXOffSet = { 0.f };
		fXOffSet = CUtils::Make_RandomFloat(20.f, 500.f);
		_float fYOffSet = { 0.f };
		fYOffSet = CUtils::Make_RandomFloat(-200.f, 10.f);

		vKirbyPos.x += fXOffSet;
		vKirbyPos.y += fYOffSet;
		vKirbyPos.z += fZOffSet;
		m_pMaker->Make_Partical(1, vKirbyPos, 0.f, 2.f, 1.f, _float4(0.f, 1.f, 0.f, 0.f), 10.f, CUtils::Make_RandomFloat(15.f, 30.f), true);
		m_fAirParticleDelay = 0.f;
	}

}

void CDisaster_Master::Make_AirParticle()
{
	if (m_fAirParticleDelay > 0.1f)
	{
		_float4 vKirbyPos = m_pKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
		_float fKirbySpeed = m_pKirby->Get_KirbyInfo()->m_fMoveSpeed;

		_float fZOffSet = { 0.f };
		fZOffSet = CUtils::Make_RandomInt(0, 1) == 0 ? CUtils::Make_RandomFloat(-200.f, -50.f) : CUtils::Make_RandomFloat(50.f, 200.f);
		_float fXOffSet = { 0.f };
		fXOffSet = vKirbyPos.x > 15.f ? CUtils::Make_RandomFloat(20.f, 500.f) : CUtils::Make_RandomFloat(-100.f, 100.f);
		_float fYOffSet = { -50.f };

		vKirbyPos.x += fXOffSet;
		vKirbyPos.y += fYOffSet;
		vKirbyPos.z += fZOffSet;
		m_pMaker->Make_Partical(1, vKirbyPos, 0.f, 2.f, 1.f, _float4(0.f, 1.f, 0.f, 0.f), 10.f, CUtils::Make_RandomFloat(15.f, 30.f), true);
		m_fAirParticleDelay = 0.f;
	}


	if (m_fBuildingParticleDelay > 5.f)
	{
		_float4 vKirbyPos = m_pKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

		_float fZOffSet = { 0.f };
		fZOffSet = CUtils::Make_RandomInt(0, 1) == 0 ? CUtils::Make_RandomFloat(-400.f, -150.f) : CUtils::Make_RandomFloat(150.f, 400.f);
		_float fXOffSet = { 0.f };
		fXOffSet = vKirbyPos.x > 15.f ? CUtils::Make_RandomFloat(0.f, 1000.f) : CUtils::Make_RandomFloat(-100.f, 100.f);
		_float fYOffSet = { -50.f };

		vKirbyPos.x += fXOffSet;
		vKirbyPos.y += fYOffSet;
		vKirbyPos.z += fZOffSet;

		m_pMaker->Make_BuildingPartical(vKirbyPos);
		m_fBuildingParticleDelay = 0.f;
	}
}

void CDisaster_Master::Moving_FinaleRoad(_float fKirbyX)
{

	_float fKirbySpeed = m_pKirby->Get_KirbyInfo()->m_fMoveSpeed;


	if (m_bRoadTrigger[0] == true && fKirbyX + (fKirbySpeed * 5.f) > 1098.f)
	{
		_float3 vDestPos = { 1098.9f, -193.6f, -158.3f };
		_float3 vStartPos = vDestPos + _float3(100.f, 300.f, -100.f);

		Make_FinaleRoad(RTYPE_BUILDINGC, MOVECMD_FLY,
			vStartPos,	{ .4f, .1f, -.03f },
			vDestPos ,	{ .98f, .22f, -.03f },
			25.f);

		m_bRoadTrigger[0] = false;
	}
	else if (m_bRoadTrigger[1] == true && fKirbyX + (fKirbySpeed * 5.f) > 1225.f)
	{
		_float3 vDestPos = { 1225.3f, -131.7f, -140.5f };
		_float3 vStartPos = vDestPos + _float3(100.f, 300.f, -200.f);

		Make_FinaleRoad(RTYPE_BUILDINGA, MOVECMD_FLY,
			vStartPos, { .31f, .32f, -.85f },
			vDestPos, { .91f, .26f, .21f }, -190.f);

		m_bRoadTrigger[1] = false;
	}
	else if (m_bRoadTrigger[2] == true && fKirbyX + (fKirbySpeed * 5.f) > 1369.6f)
	{
		_float3 vDestPos = { 1369.6f,-143.9f, -146.2f };
		_float3 vStartPos = vDestPos + _float3(100.f, 300.f, 200.f);

		Make_FinaleRoad(RTYPE_BUILDINGA, MOVECMD_FLY,
			vStartPos, { .92f, .29f, .06f },
			vDestPos, { .93f, .27f, .06f }, 20.f);

		m_bRoadTrigger[2] = false;
	}
	else if (m_bRoadTrigger[3] == true && fKirbyX + (fKirbySpeed * 5.f) > 1493.f)
	{
		_float3 vDestPos = { 1493.8f, -125.9f, -123.f };
		_float3 vStartPos = vDestPos + _float3(100.f, 300.f, -200.f);

		Make_FinaleRoad(RTYPE_BUILDINGC, MOVECMD_FLY,
			vStartPos, { .77f, .15f, -.59f },
			vDestPos, { .95f, .15f, -.18f }, -5.f);
		
		m_bRoadTrigger[3] = false;
	}
	else if (m_bRoadTrigger[4] == true && fKirbyX + (fKirbySpeed * 5.f) > 1619.6f)
	{
		_float3 vDestPos = { 1619.6f, -102.f, -146.71f };
		_float3 vStartPos = vDestPos + _float3(100.f, 300.f, -200.f);
	
	Make_FinaleRoad(RTYPE_BUILDINGC, MOVECMD_FLY,
		vStartPos, { -.87f, 0.f, 1.f },
		vDestPos, { .94f, .34f, 0.0f }, 3.f);

		m_bRoadTrigger[4] = false;

	}
	//else if (m_bRoadTrigger[4] == true && fKirbyX + (fKirbySpeed * 5.f) > 1619.6f)
	//{
	//	_float3 vDestPos = { 1619.6f, -80.37f, -152.71f };
	//	_float3 vStartPos = vDestPos + _float3(100.f, 200.f, 100.f);

	//	Make_FinaleRoad(RTYPE_BUILDINGC, MOVECMD_FLY,
	//		vStartPos, { .49f, -.19f, .85f },
	//		vDestPos, {.78f, .62f, .04f }, 0.f);
	
	//	m_bRoadTrigger[4] = false;
	//}
}

void CDisaster_Master::Moving_TargetBaum(_float fKirbyX)
{
	_float fKirbySpeed = m_pKirby->Get_KirbyInfo()->m_fMoveSpeed;

	// 그 위치에 커비는 약 2초후에 도달할 것이다.
	if (m_bBaumTrigger[0] == true && fKirbyX + (fKirbySpeed * 3.f) > 365.4f)
	{
		Make_OnTerrainBaum(_float4(365.4f, 39.3f, 1.93f, 1.f), true);
		m_bBaumTrigger[0] = false;
	}

	//맨 처음 도로
	else if (m_bBaumTrigger[1] == true && fKirbyX + (fKirbySpeed * 1.2f) > 471.f)
	{
		Make_OnTerrainBaum(_float4(471.f, 8.15f, -69.7f, 1.f), true);
		m_bBaumTrigger[1] = false;
	}

	else if (m_bBaumTrigger[2] == true && fKirbyX + (fKirbySpeed * 3.1f) > 585.25f)
	{
		Make_OnTerrainBaum(_float4(585.25f, -.9f, -84.f, 1.f), false);
		m_bBaumTrigger[2] = false;
	}

	//떨어지는 놈
	else if (m_bBaumTrigger[3] == true && fKirbyX + (fKirbySpeed * 2.2f) > 689.f)
	{
		Make_OnTerrainBaum(_float4(689.f, -25.9f, -93.f, 1.f), false);
		m_bBaumTrigger[3] = false;
	}

	//밑 도로
	else if (m_bBaumTrigger[4] == true && fKirbyX + (fKirbySpeed * 2.f) > 782.f)
	{
		Make_OnTerrainBaum(_float4(782.f, -151.8f, -122.6f, 1.f), false);
		m_bBaumTrigger[4] = false;
	}

	else if (m_bBaumTrigger[5] == true && fKirbyX + (fKirbySpeed * 4.5f) > 916.f)
	{
		Make_OnTerrainBaum(_float4(916.f, -150.8f, -120.f, 1.f), false);
		m_bBaumTrigger[5] = false;
	}


	//else if (m_bBaumTrigger[6] == true && fKirbyX + (fKirbySpeed * 2.5f) > 689.f)
	//{
	//	Make_OnTerrainBaum(_float4(689.f, -25.9f, -93.f, 1.f), false);
	//	m_bBaumTrigger[6] = false;
	//}

	//else if (m_bBaumTrigger[7] == true && fKirbyX + (fKirbySpeed * 2.5f) > 689.f)
	//{
	//	Make_OnTerrainBaum(_float4(689.f, -25.9f, -93.f, 1.f), false);
	//	m_bBaumTrigger[7] = false;
	//}
}

CDisaster_Master* CDisaster_Master::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDisaster_Master* pInstance = new CDisaster_Master(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDisaster_Master"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDisaster_Master::Clone(void* pArg)
{
	CDisaster_Master* pInstance = new CDisaster_Master(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDisaster_Master"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDisaster_Master::Free()
{
	__super::Free();
	Safe_Release(m_pKirby);
	Safe_Release(m_pLight);
	Safe_Release(m_pMaker);
}
