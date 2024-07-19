#include "stdafx.h"
#include "DimensionClaw.h"
#include "HitBox.h"

CDimensionClaw::CDimensionClaw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CDimensionClaw::CDimensionClaw(const CDimensionClaw& rhs)
	: CGameObject{ rhs }
{
}

void CDimensionClaw::MoveToStrangePos()
{
	_float fPos = 3000.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(fPos, fPos, fPos, 1));
}

_bool CDimensionClaw::CheckCollsion(_float4 vPos) // 커비를 내 로컬로 데려와서 충돌 판정
{
	_float4 vLocalPos = _float4::Transform(vPos + _float4(0, m_fKirbyOffsetY, 0, 0), m_pTransformCom->Get_WorldMatrix_Inverse());
	
	if (5.f < vLocalPos.z || -4.f > vLocalPos.z) // z축 충돌범위
		return false;
	else
		_uint i = 0;

	// 각 직선에 대한 거리 계산
	auto distanceToLine = [](_float x, _float y, _float A, _float B, _float C) { // x, y는 직선과의 거리 비교
		return fabs(A * x + B * y + C) / sqrt(A * A + B * B);
		};

	// 직선 1: 0.7002x - y + b1 = 0
	if (distanceToLine(vLocalPos.x, vLocalPos.y, m_fGradient, -1.0f, m_fOffsetY) <= m_fKirbyRadius)
		return true;

	// 직선 2: 0.7002x - y + b2 = 0
	if (distanceToLine(vLocalPos.x, vLocalPos.y, m_fGradient, -1.0f, -m_fOffsetY) <= m_fKirbyRadius)
		return true;

	// 직선 3: -0.7002x - y + b3 = 0
	if (distanceToLine(vLocalPos.x, vLocalPos.y, -m_fGradient, -1.0f, m_fOffsetY) <= m_fKirbyRadius)
		return true;

	// 직선 4: -0.7002x - y + b4 = 0
	if (distanceToLine(vLocalPos.x, vLocalPos.y, -m_fGradient, -1.0f, -m_fOffsetY) <= m_fKirbyRadius)
		return true;

	return false;
}

HRESULT CDimensionClaw::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDimensionClaw::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr) {
		Desc = (GAMEOBJECT_DESC*)pArg;
		Desc->fSpeedPerSec = 10.f;
		Desc->fRotationPerSec = XMConvertToRadians(90.f);
	}

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	m_fKirbyOffsetY = 0.7f;
	m_fKirbyRadius = 0.6f;

	m_fGradient = tanf(XMConvertToRadians(35.0f));
	m_fOffsetY = 3.5f;

	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = MONSTERBULLET;
	
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;

	Set_BodyCollider(COLLIDER_SPHERE, 0.f, 0.f, 3.f);

	return S_OK;
}

_int CDimensionClaw::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_pTransformCom->Go_Straight(m_pGameInstance->Get_SecondTimer() * 10.f);

	return OBJ_NOEVENT;
}

HRESULT CDimensionClaw::Render()
{
	if (m_pGameInstance->Get_HitBoxRender() == false)
		return S_OK;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW);
	ProjMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ);
	_float4x4 VPMatrix = ViewMatrix * ProjMatrix;

	_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();

	// 로컬 좌표에서 월드 좌표로 변환
	auto LocalToWorld = [&](_vector localPos) {
		return XMVector3TransformCoord(localPos, matWorld);
		};

	auto TransformToScreen = [&](_vector worldPos)
		{
			_vector screenPos = XMVector3TransformCoord(worldPos, VPMatrix);
			screenPos = XMVectorMultiplyAdd(screenPos, XMVectorSet(0.5f, -0.5f, 1.0f, 0.0f), XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f));
			screenPos = XMVectorMultiply(screenPos, XMVectorSet(g_iWinSizeX, g_iWinSizeY, 1.f, 0.f));
			return ImVec2(XMVectorGetX(screenPos), XMVectorGetY(screenPos));
		};

	ImVec4 color = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);

	_float fLength = 50.f;
	// 직선의 두 점 계산
	auto ComputeLinePoints = [&](_float fGradient , _float offset) {
		_vector localPoint1 = XMVectorSet(-fLength, fGradient *  (-fLength) + offset, 0.0f, 1.0f); // 왼쪽 끝
		_vector localPoint2 = XMVectorSet(fLength, fGradient * fLength + offset, 0.0f, 1.0f); // 오른쪽 끝

		_vector worldPoint1 = LocalToWorld(localPoint1);
		_vector worldPoint2 = LocalToWorld(localPoint2);

		ImVec2 screenPoint1 = TransformToScreen(worldPoint1);
		ImVec2 screenPoint2 = TransformToScreen(worldPoint2);

		return make_pair(screenPoint1, screenPoint2);
		};

	// 직선 1: gradient * x - y + offsetY = 0
	auto [p1, p2] = ComputeLinePoints(m_fGradient, m_fOffsetY);
	drawList->AddLine(p1, p2, ImColor(color));

	// 직선 2: gradient * x - y - offsetY = 0
	auto [p3, p4] = ComputeLinePoints(m_fGradient, -m_fOffsetY);
	drawList->AddLine(p3, p4, ImColor(color));

	// 직선 3: -gradient * x - y + offsetY = 0
	auto [p5, p6] = ComputeLinePoints(-m_fGradient, m_fOffsetY);
	drawList->AddLine(p5, p6, ImColor(color));

	// 직선 4: -gradient * x - y - offsetY = 0
	auto [p7, p8] = ComputeLinePoints(-m_fGradient, -m_fOffsetY);
	drawList->AddLine(p7, p8, ImColor(color));

	return S_OK;
}

CDimensionClaw* CDimensionClaw::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDimensionClaw* pInstance = new CDimensionClaw(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDimensionClaw"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDimensionClaw::Clone(void* pArg)
{
	CDimensionClaw* pInstance = new CDimensionClaw(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDimensionClaw"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDimensionClaw::Free()
{
	__super::Free();
}
