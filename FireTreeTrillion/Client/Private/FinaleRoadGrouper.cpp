#include "stdafx.h"
#include "FinaleRoadGrouper.h"
#include "FinaleRoad.h"

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
	/*
	//내가 움직이는 놈 아니면 나가기
	if (m_eCollideMove == MOVECMD_END)
		return false;

	_bool bIsMyCollision{ false };

	for (auto& road : m_pRoads)
	{
		if (pRoad == road)
		{
			bIsMyCollision = true;
			break;
		}
	}

	//내 충돌 아니면 나가기
	if (!bIsMyCollision)
		return false;

*/

	switch (m_eCollideMove)
	{
	case MOVECMD_ROTATE:
		break;
	case MOVECMD_COLLIDE:
		m_bStartCollideEvent = true;
		break;
	default:
		break;
	}
	for (auto& road : m_pRoads)
	{
		road->Make_CollisionEvent();
	}

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

	}
	break;
	case RTYPE_BUILDINGC:
	{

	}
	break;
	case RTYPE_BUILDINGD:
	{

	}
	break;
	case RTYPE_ROADA:
	{
		//기본 도로
		CFinaleRoad::ROAD_DESC roadDesc{};
		//roadDesc.wstrModelName = TEXT("Road");
		_float4x4 InitMat = _float4x4::Identity;
		//InitMat.Translation({ 0.f, 0.f, -20.f });
		//roadDesc.matWorld = InitMat;
		//roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		//roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		//if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
		//	return E_FAIL;

		//CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		//if (pRoad != nullptr)
		//	m_pRoads.emplace_back(pRoad);


		////기본 도로
		//roadDesc = {};
		//roadDesc.wstrModelName = TEXT("Road");
		//InitMat = _float4x4::Identity;
		//InitMat.Translation({ 0.f, 0.f, 20.f });
		//roadDesc.matWorld = InitMat;
		//roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		//roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		//if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
		//	return E_FAIL;

		//pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		//if (pRoad != nullptr)
		//	m_pRoads.emplace_back(pRoad);


		//부서지는 도로
		roadDesc = {};
		roadDesc.wstrModelName = TEXT("WaddleDeeBase");
		roadDesc.bIsAnimModel = true;
		InitMat = _float4x4::Identity;
		InitMat.Translation({ 0.f, 0.f, 60.f });
		roadDesc.matWorld = InitMat;
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);
	}
	break;
	case RTYPE_ROADB:
	{

	}
	break;

	default:
		CFinaleRoad::ROAD_DESC roadDesc{};
		roadDesc.wstrModelName = TEXT("MovableBuildingA");
		roadDesc.pSocketMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();
		roadDesc.eCollideType = CFinaleRoad::CTYPE_NONE;
		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;

		CFinaleRoad* pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);


		roadDesc.wstrModelName = TEXT("Road");
		_float4x4 InitMat = _float4x4::Identity;
		InitMat.Translation({ 50.f, 0.f, -50.f });
		roadDesc.matWorld = InitMat;


		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"), TEXT("Prototype_GameObject_FinaleRoad"), &roadDesc)))
			return E_FAIL;


		pRoad = dynamic_cast<CFinaleRoad*>(m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad"))->back());
		if (pRoad != nullptr)
			m_pRoads.emplace_back(pRoad);

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
		_float3 vMyPos = GET_POS;
		m_vDestPos = vMyPos + _float3{ 0.f, -30.f, 0.f };
		m_vDestPos = RoadGroupDesc.vDestPos;
		m_vDestDir = RoadGroupDesc.vDestDir;
	}
	break;
	default:
		break;
	}

	m_eCollideMove = RoadGroupDesc.eMoveCommand;

	return S_OK;
}

_int CFinaleRoadGrouper::Tick(_float fTimeDelta)
{
	if (m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{
		if (m_pGameInstance->Get_KeyState(DIK_P, KEY_DOWN))
		{
			Make_CollideReaction();
		}
	}

	if (m_bStartCollideEvent)
	{

		_float3 vMyPos = GET_POS;
		_float fDist = _float3::Distance(vMyPos, m_vDestPos);

		if (.1f < fDist)
		{
			vMyPos += (m_vDestPos - vMyPos) * fTimeDelta;
			if (_float3::Distance(vMyPos, m_vDestPos) < .1f)
				vMyPos = m_vDestPos;

			SET_POS(Pos(vMyPos));
		}

		_float3 vMyDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		_float3 vCurRot = CUtils::SlerpDirVec(vMyDir, m_vDestDir, clamp(fTimeDelta * 3.f, 0.f, 1.f));
		m_pTransformCom->Set_State(CTransform::STATE_LOOK, Dir(vCurRot));
	}

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
