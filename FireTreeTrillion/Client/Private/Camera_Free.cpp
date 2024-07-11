#include "stdafx.h"
#include "Camera_Free.h"
#include "Kirby.h"
#include "EventCenter.h"
#include "PartTimeHelper.h"

CCamera_Free::CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{

}

CCamera_Free::CCamera_Free(const CCamera_Free& rhs)
	: CCamera{ rhs }
{

}



HRESULT CCamera_Free::Initialize_Prototype()
{


	return S_OK;
}

HRESULT CCamera_Free::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_FREE_DESC* pCameraFree = (CAMERA_FREE_DESC*)pArg;
	m_fMouseSensor = pCameraFree->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	m_pGameInstance->Add_Camera(this);

	m_vDestCamDir = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

	if (*m_pCurrentLevelID == LEVEL_PARTTIME)
	{
		__super::Lock_Camera({ 16.4f, 25.7f, 35.75f }, { .16f, -.08f, -1.f }, 38.f);
	}

	//// 파트타임헬퍼에 옵저버로 카메라를 알게하고 있습니다. JYWI's ps : 카메라 클래스 하나 더 팔걸~~
	//if(LEVEL_PARTTIME == *m_pGameInstance->Get_CurrentLevelID())
	//	CPartTimeHelper::Get_Instance()->Register_Camera(this);

	return S_OK;
}

_int CCamera_Free::Tick(_float fTimeDelta)
{
	if ( m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS) && m_pGameInstance->Get_KeyState(DIK_L, KEY_DOWN))
	{
		LockToggle();
	}

	//내가 현재 카메라가 아니라면 바쁘게 타겟 따라가기
	if (m_pGameInstance->Get_CurCameraPtr() != this)
		m_bTrackTarget = true;
	else
		m_bTrackTarget = false;


	if (!m_bLockCamera)
	{
		Control(fTimeDelta);

		//fov y 를 보간하여 갱신한다.
		if (.01f < abs(m_fFovy - m_fDestFovy))
			m_fFovy += (m_fDestFovy - m_fFovy) * fTimeDelta * 3.f;
	}

	m_bWasMainCamera = (m_pGameInstance->Get_CurCameraPtr() != this);

	return OBJ_NOEVENT;
}

void CCamera_Free::Late_Tick(_float fTimeDelta)
{
}

HRESULT CCamera_Free::Render()
{
	return S_OK;
}

#ifdef _DEBUG
void CCamera_Free::Render_IMGUI()
{

	ImGui::DragFloat(u8"카메라 이동 속도", &m_fCamSpeed, .01f, 0.f, 500.f, "%.2f");

	ImGui::Separator();

	static _float fSpeed = 10.f;

	_float4x4 WorldMat = m_pTransformCom->Get_WorldMatrix();
	_float4 vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);


	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._11, WorldMat._12, WorldMat._13, WorldMat._14);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._21, WorldMat._22, WorldMat._23, WorldMat._24);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._31, WorldMat._32, WorldMat._33, WorldMat._34);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._41, WorldMat._42, WorldMat._43, WorldMat._44);


	ImGui::Separator();


	static _float fFOVY = ToDegree(m_fDestFovy);

	if (ImGui::DragFloat(u8"FOV", &fFOVY, .1f, 10.f, 50.f, "%.1f"))
	{
		m_fDestFovy = ToRadian(fFOVY);
	}


	ImGui::SliderFloat("CameraFree Speed", &fSpeed, 0.f, 200.f);

	ImGui::Checkbox(u8"타겟 따라가기", &m_bTrackTarget);

	ImGui::Text("TriggerRatio: %.2f", m_fTriggerRatio);
	ImGui::Text("SlerpedDir: %.2f, %.2f, %.2f", XMVectorGetX(m_vSlerpedDir), XMVectorGetY(m_vSlerpedDir), XMVectorGetZ(m_vSlerpedDir));
	ImGui::Text("LerpedRadius: %.2f", m_fLerpedRadius);


	ImGui::DragFloat(u8"목표 각도 보간 ratio", &m_fSlerpRatio, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat(u8"타겟까지의 거리", &m_fTrackDistance, .1f, 10.f, 50.f, "%.1f");
	ImGui::Text(u8" 목표 Dir %.2f\t%.2f\t%.2f", m_vDestCamDir.x, m_vDestCamDir.y, m_vDestCamDir.z);

}
#endif

void CCamera_Free::Set_MatrixIndex(_int iMatrixIndex)
{
	if (nullptr == m_pTransformCom || m_vecCamMatrices.empty())
		return;

	if (iMatrixIndex < 0 || iMatrixIndex == m_iMatrixIndex || iMatrixIndex >= m_vecCamMatrices.size())
		return;

	m_pTransformCom->Set_WorldMatrix(m_vecCamMatrices[iMatrixIndex]);
	m_iMatrixIndex = iMatrixIndex;
}

void CCamera_Free::EmplaceBackDirRadius(_int iCamType, _fvector vDir, _float fRadius)
{
	if (CAM_FRONT == iCamType)
		m_vecFrontDirRadius.emplace_back(vDir, fRadius);
	else if (CAM_REAR == iCamType)
		m_vecRearDirRadius.emplace_back(vDir, fRadius);
}

void CCamera_Free::LerpByTriggerInfo(_int iTriggerIndex)
{
	if (nullptr == m_pTransformCom || m_vecFrontDirRadius.empty() || m_vecRearDirRadius.empty())
		return;

	if (m_iMatrixIndex < 0 || m_iMatrixIndex >= m_vecFrontDirRadius.size())
		return;

	m_fTriggerRatio = Compute_TriggerPosRatio(m_iMatrixIndex);
	if (0 > m_fTriggerRatio || 1 < m_fTriggerRatio)
		return;

	CGameObject* pKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0);
	if (nullptr == pKirby)
		return;
	CTransform* pKirbyTransform = static_cast<CTransform*>(pKirby->Get_Component(g_strTransformTag));
	if (nullptr == pKirbyTransform)
		return;
	_vector pKirbyPos = pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION);

	m_vSlerpedDir = SlerpDirVec(m_vecRearDirRadius[m_iMatrixIndex].first, m_vecFrontDirRadius[m_iMatrixIndex].first, m_fTriggerRatio);
	m_fLerpedRadius = LerpRadius(m_vecRearDirRadius[m_iMatrixIndex].second, m_vecFrontDirRadius[m_iMatrixIndex].second, m_fTriggerRatio);

	//m_vDestCamPos = pKirbyPos - m_vSlerpedDir * m_fLerpedRadius;

	m_vDestCamDir = m_vSlerpedDir;
	m_fTrackDistance = m_fLerpedRadius;
}

_float CCamera_Free::Compute_TriggerPosRatio(_int iTriggerIndex)
{
	if (m_vecTriggerInfo.empty())
		return _float();

	CGameObject* pKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0);
	CTransform* pKirbyTransform = static_cast<CTransform*>(pKirby->Get_Component(g_strTransformTag));
	_float fZ = XMVectorGetZ(XMVector4Transform(pKirbyTransform->Get_State_Vector(CTransform::STATE_POSITION)
		, m_vecTriggerInfo[iTriggerIndex].first));

	// rear : 0, middle : 0.5, front: 1
	_float fRatio = 0.5f * fZ + 0.5f;
	return fRatio;
}

_vector CCamera_Free::SlerpDirVec(_fvector vStart, _fvector vEnd, _float fRatio)
{
	_float fDot = ::XMVectorGetX(::XMVector3Dot(vStart, vEnd));
	fDot = clamp(fDot, -1.f, 1.f);
	_float fTheta = acosf(fDot) * fRatio;
	_vector vRelative = ::XMVector3Normalize(vEnd - vStart * fRatio);

	return XMVector3Normalize(vStart * cosf(fTheta) + vRelative * sinf(fTheta));
}

_float CCamera_Free::LerpRadius(_float fStart, _float fEnd, _float fRatio)
{
	return fStart + fRatio * (fEnd - fStart);
}

void CCamera_Free::Track_Target(_float fTimeDelta)
{
	if (nullptr == m_pFirstTarget)
		return;

	_float3 vTargetPos = m_pFirstTarget->Get_State(CTransform::STATE_POSITION);


	if (*m_pCurrentLevelID == LEVEL_GAMEPLAY)
	{
		_float4 vTerrainPos = static_cast<CCharacter*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0))->Compute_TerrainPosition();
		vTargetPos.y = vTerrainPos.y;
	}

	//**** 목표 위치 마지막 저장 ****//

	//타겟 기준으로 뒤 위치를 목표한다.
	_float3 vBackDir = -m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vBackDir.Normalize();
	//실제 카메라 목표 위치를 저장한다.
	m_vDestCamPos = Pos(vTargetPos + _float3(0.f, 10.f, 0.f) - (m_vDestCamDir * m_fTrackDistance));



	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);


	//1차. x 좌표 보간.
	_float4 vDestDir = m_vDestCamPos - vCurPos;

	_float4 vDestXZDir = { vDestDir.x, 0.f, vDestDir.z , 0.f };
	_float4 vDestYDir = { 0.f, vDestDir.y, 0.f , 0.f };


	if (.1f <= vDestXZDir.Length())
		m_pTransformCom->Move(vDestXZDir * .04f);


	if (.1f <= vDestYDir.Length())
		m_pTransformCom->Move(vDestYDir * .02f);


}


void CCamera_Free::Control(_float fTimeDelta)
{

	if (m_bTrackTarget)
		Track_Target(fTimeDelta);


	//내가 현재 카메라가 아니라면 컨트롤은 못하게 하기
	if (m_pGameInstance->Get_CurCameraPtr() != this)
	{
		if (m_pFirstTarget == nullptr)
			return;

		if (m_bWasMainCamera)
			m_pTransformCom->Look_At(m_pFirstTarget->Get_State(CTransform::STATE_POSITION));

		return;
	}

	if (m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{
		_long	MouseMove = { 0 };

		//휠 누른 채로 상하좌우 이동
		if (m_pGameInstance->Get_KeyState(DIMKS_WHEEL, KEY_PRESS))
		{
			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
				m_pTransformCom->Go_Right(fTimeDelta * -MouseMove * m_fCamSpeed * m_fMouseSensor);

			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
				m_pTransformCom->Go_Up(fTimeDelta * MouseMove  * m_fCamSpeed * m_fMouseSensor);
		}

		if (m_pGameInstance->Get_KeyState(DIK_W, KEY_PRESS))
			m_pTransformCom->Go_Straight(fTimeDelta * m_fCamSpeed);
		if (m_pGameInstance->Get_KeyState(DIK_A, KEY_PRESS))
			m_pTransformCom->Go_Left(fTimeDelta * m_fCamSpeed);
		if (m_pGameInstance->Get_KeyState(DIK_S, KEY_PRESS))
			m_pTransformCom->Go_Backward(fTimeDelta * m_fCamSpeed);
		if (m_pGameInstance->Get_KeyState(DIK_D, KEY_PRESS))
			m_pTransformCom->Go_Right(fTimeDelta * m_fCamSpeed);

		//전후진
		if ((MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_WHEEL)) && m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
		{
			m_pTransformCom->Go_Straight(fTimeDelta * MouseMove * m_fCamSpeed * m_fMouseSensor * .5f);

			if (m_pTarget != nullptr && m_bTrackTarget)
				m_fTrackDistance -= MouseMove * m_fMouseSensor * .1f;
		}

		//우측 마우스 누른 채로 공전
		if (m_pGameInstance->Get_KeyState(DIMKS_RBUTTON, KEY_PRESS))
		{
			_float3 vTargetPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION) + m_pTransformCom->Get_State_Float4(CTransform::STATE_LOOK) * 10.f;
			if (*m_pCurrentLevelID == LEVEL_TOOL_FX)
				vTargetPos = _float3::Zero;

			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
			{
				m_pTransformCom->Orbit(vTargetPos, XMVectorSet(0.f, 1.f, 0.f, 1.f), fTimeDelta * MouseMove * m_fMouseSensor);
			}
			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
			{
				m_pTransformCom->Orbit(vTargetPos, m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * m_fMouseSensor);
			}
		}
	}
}

CCamera_Free* CCamera_Free::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Free* pInstance = new CCamera_Free(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CCamera_Free::Clone(void* pArg)
{
	CCamera_Free* pInstance = new CCamera_Free(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCamera_Free"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Free::Free()
{
	Safe_Release(m_pTarget);

	__super::Free();

}
