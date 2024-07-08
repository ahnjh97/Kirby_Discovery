#include "stdafx.h"
#include "FinaleRoadGrouper.h"
#include "FinaleRoad.h"
#include "Camera_Main.h"

CFinaleRoadGrouper::CFinaleRoadGrouper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CFinaleRoadGrouper::CFinaleRoadGrouper(const CFinaleRoadGrouper& rhs)
	:CGameObject{ rhs }
{
}

_bool CFinaleRoadGrouper::Make_CollideReaction(CFinaleRoad* pRoad)
{

	//내가 움직이는 놈 아니면 나가기
	if (m_eCollideMove == MOVECMD_END)
		return false;

	_bool bIsMyCollision{ false };

	for (auto& road : m_pRoads)
	{
		if (_float4::Distance(road->Get_WorldPos(), pRoad->Get_WorldPos()) < 10.f)
		{
			bIsMyCollision = true;
			break;
		}
	}

	//내 충돌 아니면 나가기
	if (!bIsMyCollision)
		return false;


	switch (m_eCollideMove)
	{
	case MOVECMD_ROTATE:
		break;
	case MOVECMD_COLLIDE:
		m_bStartCollideEvent = true;
		m_fCollideTime = 1.f;
		m_fMaxDuration = m_fCollideTime;

	//m_fDestZAngle = CUtils::Make_RandomFloat(-15.f, 15.f);

		break;
	default:
		break;
	}

	for (auto& road : m_pRoads)
	{
		road->Make_CollisionEvent();
	}

	//m_fDestZAngle = CUtils::Make_RandomInt(0, 1) ? 10.f : -10.f;

	return true;
}

HRESULT CFinaleRoadGrouper::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinaleRoadGrouper::Initialize(void* pArg)
{
	ROADGROUPER_DESC RoadGroupDesc{};

	if (nullptr != pArg)
		RoadGroupDesc = *(ROADGROUPER_DESC*)pArg;

	RoadGroupDesc.fSpeedPerSec = 5.f;
	RoadGroupDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	HRESULT hr;

	hr = __super::Initialize(&RoadGroupDesc);
	CHECK_FAILED(hr);

	switch (RoadGroupDesc.eRoadType)
	{
	case RTYPE_BUILDINGA:
	{
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("MovableBuildingA");
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;


		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

	}
	break;
	case RTYPE_BUILDINGB:
	{
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("MovableBuildingB");
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;


		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	case RTYPE_BUILDINGC:
	{
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("MovableBuildingC");
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;


		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	case RTYPE_BUILDINGD:
	{
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("MovableBuildingD");
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;


		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	case RTYPE_ROADA:
	{
		//기본 도로
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("Road");
		_float4x4 InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, -150.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);


		//기본 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("Road");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, -90.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);


		//부서지는 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("RoadLBreak");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, -30.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();

		roadDesc.bIsAnimModel = true;
		roadDesc.eCollideType = CFinaleRoad::CTYPE_BREAK;

		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

		//기본 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("RoadLongBreak");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 60.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();

		roadDesc.bIsAnimModel = true;
		roadDesc.eCollideType = CFinaleRoad::CTYPE_BREAK;

		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

		roadDesc = {};
		roadDesc.wstrModelName = TEXT("RoadEnd");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 150.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	case RTYPE_ROADB:
	{
		//기본 도로
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("Road");
		_float4x4 InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, -60.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);


		//기본 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("Road");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 0.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);


		//부서지는 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("RoadLBreak");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 60.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();

		roadDesc.bIsAnimModel = true;
		roadDesc.eCollideType = CFinaleRoad::CTYPE_BREAK;

		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	case RTYPE_ROADC:
	{
		//기본 도로
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("Road");
		_float4x4 InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, -90.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

		//부서지는 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("RoadBreak");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, -30.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();

		roadDesc.bIsAnimModel = true;
		roadDesc.eCollideType = CFinaleRoad::CTYPE_BREAK;

		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

		//기본 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("Road");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 30.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

		roadDesc = {};
		roadDesc.wstrModelName = TEXT("RoadEnd");
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 90.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	default:
		break;
	}

	switch (RoadGroupDesc.eMoveCommand)
	{
	case MOVECMD_ROTATE:
	{

	}
	break;
	case MOVECMD_COLLIDE:
	{
		//부딪혔을 때 만들 상태
		m_vStartPos = GET_POS;
		m_vDestPos = RoadGroupDesc.vDestPos;

		m_vStartDir = (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		m_vDestDir = RoadGroupDesc.vDestDir;
		m_fDestZAngle = CUtils::Make_RandomFloat(-15.f, 15.f);

	}
	break;
	//등장 시에 날라오는 놈
	case MOVECMD_FLY:
	{
		//부딪혔을 때 만들 상태
		m_vStartPos = GET_POS;
		m_vDestPos = RoadGroupDesc.vDestPos;

		m_vStartDir = (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		m_vDestDir = RoadGroupDesc.vDestDir;

		m_bStartCollideEvent = true;
		m_fCollideTime = 1.5f;
		m_fMaxDuration = m_fCollideTime;
		m_fDestZAngle = CUtils::Make_RandomFloat(-15.f, 15.f);

	}
	break;
	default:
		break;
	}

	//충돌 판정 시 어떻게 움직이는가?
	m_eCollideMove = RoadGroupDesc.eMoveCommand;

	return S_OK;
}

_int CFinaleRoadGrouper::Tick(_float fTimeDelta)
{
	_float fRealTimeDelta = m_pGameInstance->Get_SecondTimer();


	//z 앵글을 초기화한다.
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 1.f, -m_fPreZAngle);


	//특정 목표로 가야 할 때
	if (m_bStartCollideEvent && 0.f < m_fCollideTime)
	{
		//충돌 시간을 깎는다.
		m_fCollideTime -= fRealTimeDelta;
		if (m_fCollideTime < 0.f)
		{
			m_fCollideTime = 0.f;

			//날아와서 부딪히는 놈이면 shake 하기
			if (m_eCollideMove == MOVECMD_FLY)
			{
				CCamera_Main* pCamera = dynamic_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());

				if (nullptr != pCamera)
					pCamera->Make_Shake(3.f, .8f);
			}
		}


		_float3 vMyPos = GET_POS;
		_float fDist = _float3::Distance(vMyPos, m_vDestPos);


		_float fTime = 0.f;

		if (m_eCollideMove == MOVECMD_FLY)
			fTime = (m_fMaxDuration - m_fCollideTime) / m_fMaxDuration;
		else if (m_eCollideMove == MOVECMD_COLLIDE)
			fTime = EASE_OUT((m_fMaxDuration - m_fCollideTime) / m_fMaxDuration);


		//거리 스냅
		if (.1f < fDist)
		{
			_float3 vResultPos = _float3::Lerp(m_vStartPos, m_vDestPos, fTime);

			if (_float3::Distance(vResultPos, m_vDestPos) < .1f)
				vResultPos = m_vDestPos;

			SET_POS(Pos(vResultPos));
		}

		//회전 보간
		Quaternion vFirstQuat, vSecondQuat, vResultQuat;

		vFirstQuat = CUtils::Make_Quat_FromDir(m_vStartDir);
		vSecondQuat = CUtils::Make_Quat_FromDir(m_vDestDir);

		vResultQuat = Quaternion::Slerp(vFirstQuat, vSecondQuat, clamp(fTime, 0.f, 1.f));
		m_pTransformCom->Turn_Absolute(vResultQuat);

		//z 앵글 보간
		if (.05f < abs(m_fCurZAngle - m_fDestZAngle))
		{
			m_fCurZAngle = LERP(m_fStartZAngle, m_fDestZAngle, fTime);

			if (abs(m_fCurZAngle - m_fDestZAngle) < .05f)
				m_fCurZAngle = m_fDestZAngle;
		}



	}
	else
	{
		//평소
		if (m_eCollideMove == MOVECMD_FLY)
		{

		}
	}



	//Z 앵글
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), 1.f, m_fCurZAngle);
	m_fPreZAngle = m_fCurZAngle;

	return OBJ_NOEVENT;
}

void CFinaleRoadGrouper::Late_Tick(_float fTimeDelta)
{
}

HRESULT CFinaleRoadGrouper::Render()
{
	return S_OK;
}

#ifdef DEBUG
void CFinaleRoadGrouper::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	ImGui::Separator(); ImGui::NewLine();

	_float4x4 WorldMat = m_pTransformCom->Get_WorldMatrix();

	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._11, WorldMat._12, WorldMat._13, WorldMat._14);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._21, WorldMat._22, WorldMat._23, WorldMat._24);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._31, WorldMat._32, WorldMat._33, WorldMat._34);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._41, WorldMat._42, WorldMat._43, WorldMat._44);


	__super::Render_IMGUI();
}
#endif

CFinaleRoadGrouper* CFinaleRoadGrouper::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinaleRoadGrouper* pInstance = new CFinaleRoadGrouper(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinaleRoadGrouper"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinaleRoadGrouper::Clone(void* pArg)
{
	CFinaleRoadGrouper* pInstance = new CFinaleRoadGrouper(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinaleRoad"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinaleRoadGrouper::Free()
{
	for (auto* road : m_pRoads)
		Safe_Release(road);
	m_pRoads.clear();

	__super::Free();
}
