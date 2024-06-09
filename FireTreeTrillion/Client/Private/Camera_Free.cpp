#include "stdafx.h"
#include "Camera_Free.h"
#include "Kirby.h"

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

	//pCameraFree->fRotationPerSec = ToRadian(45.f);


	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pGameInstance->Add_Camera(this);

	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_INTRO || 
		*m_pGameInstance->Get_CurrentLevelID() == LEVEL_GAMEPLAY) {
		function<void(_int)> func = bind(&CCamera_Free::StartLerpByTriggerInfo, this, placeholders::_1);
		m_pGameInstance->Emplace_TriggerFunc(TRIGGER_CAMERA, func);

		function<void(void)> exitFunc = bind(&CCamera_Free::EndLerpByTriggerInfo, this);
		m_pGameInstance->Emplace_ExitFunc(TRIGGER_CAMERA, exitFunc);
	}
	return S_OK;
}

_int CCamera_Free::Tick(_float fTimeDelta)
{
	Control(fTimeDelta);
	//Orbit_Target(fTimeDelta);




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
	static _float fSpeed = 10.f;

	_float4x4 WorldMat = m_pTransformCom->Get_WorldMatrix();
	_float4 vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);


	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._11, WorldMat._12, WorldMat._13, WorldMat._14);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._21, WorldMat._22, WorldMat._23, WorldMat._24);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._31, WorldMat._32, WorldMat._33, WorldMat._34);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._41, WorldMat._42, WorldMat._43, WorldMat._44);

	//ImGui::Text("X: %.2f", vPosition.x);
	//ImGui::SameLine();
	//ImGui::Text("Y: %.2f", vPosition.y);
	//ImGui::SameLine();
	//ImGui::Text("Z: %.2f", vPosition.z);

	ImGui::SliderFloat("CameraFree Speed", &fSpeed, 0.f, 200.f);

	ImGui::Checkbox(u8"타겟 따라가기", &m_bTrackTarget);

	ImGui::Text("TriggerRatio: %.2f", m_fTriggerRatio);
	ImGui::Text("SlerpedDir: %.2f, %.2f, %.2f", XMVectorGetX(m_vSlerpedDir), XMVectorGetY(m_vSlerpedDir), XMVectorGetZ(m_vSlerpedDir));
	ImGui::Text("LerpedRadius: %.2f", m_fLerpedRadius);


	ImGui::DragFloat(u8"목표 각도 보간 ratio", &m_fSlerpRatio, .01f, 0.f, 1.f, "%.2f");
	ImGui::DragFloat(u8"타겟까지의 거리", &m_fTrackDistance, .1f, 10.f, 50.f, "%.1f");
	ImGui::Text(u8" 목표 Dir %.2f\t%.2f\t%.2f", m_vDestCamDir.x, m_vDestCamDir.y, m_vDestCamDir.z);

	//m_pTransformCom->Set_Speed(fSpeed);
	/*
	ImGui::SliderFloat("CameraFree Smooth Speed", &m_fSmoothSpeed, 0.f, 0.3f);
	m_pTransformCom->Set_Speed(fSpeed);

	ImGui::Separator();
	ImGui::DragFloat3("CameraFree Position", &fPosition.x);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, fPosition);

	ImGui::Separator();
	ImGui::DragFloat3("CameraFree Offset", &m_vOffset.x);*/
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

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0));
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
	//m_vDestCamPos = pKirbyPos - m_vSlerpedDir * m_fLerpedRadius;
	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, pKirbyPos - m_vSlerpedDir * m_fLerpedRadius);
	//m_pTransformCom->Look_At(pKirbyPos);
}

_float CCamera_Free::Compute_TriggerPosRatio(_int iTriggerIndex)
{
	if (m_vecTriggerInfo.empty())
		return _float();

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0));
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
	if (nullptr == m_pTarget)
		return;

		_float4 vTargetPos = m_pTarget->Get_State(CTransform::STATE_POSITION);

	if (m_bLerpByTriggerInfo)
		LerpByTriggerInfo(m_iMatrixIndex);
	else
	{

		if (*m_pCurrentLevelID == LEVEL_GAMEPLAY)
		{
			_float4 vTerrainPos = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0))->Compute_TerrainPosition();
			vTargetPos.y = vTerrainPos.y;
		}

		_float4 vBackDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		vBackDir.Normalize();
		vBackDir *= m_fTrackDistance;
		m_vDestCamDir = vBackDir;
		m_vDestCamPos = vTargetPos + vBackDir;
		m_vDestCamDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);

	}

	//m_vDestCamDir = m_vSlerpedDir;
	//m_fTrackDistance = m_fLerpedRadius;

	m_vDestCamPos = vTargetPos - m_vDestCamDir * m_fTrackDistance;
;	//if (m_bLerpByTriggerInfo)
	//{

		//_float4 vCamLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		//_float4 vDestLook = m_pTarget->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION);

#pragma region 방법 1 - 절대 쿼터니언
		/*
		//_float4
		Quaternion vStartQuat = CUtils::Make_Quat_FromDir(vCamLook);
		Quaternion vDestQuat = CUtils::Make_Quat_FromDir(vDestLook);

		Quaternion vResultQuat = Quaternion::Slerp(vStartQuat, vDestQuat, m_fSlerpRatio);


		float dot = vCamLook.Dot(vDestLook);
		if (dot < 0.9999f) {

			//커비 위치와 내 위치를 비교하여 방향 보간하기
			m_pTransformCom->Turn_Absolute(vResultQuat);
		}
		*/
#pragma endregion

#pragma region 방법 2 - 변환 쿼터니언
		/*_float3 vCamLook3 = vCamLook;
		_float3 vDestLook3 = vDestLook;
		vCamLook3.Normalize();
		vDestLook3.Normalize();
		Quaternion vDestQuat = Quaternion::FromToRotation(vCamLook3, vDestLook3);
		Quaternion vSlerpedQuat = Quaternion::Slerp(Quaternion::Identity, vDestQuat, fTimeDelta);
		m_pTransformCom->Turn(vSlerpedQuat);*/

#pragma endregion

#pragma region 방법 3 - 방향 벡터 lerp

		//_float4 vCamLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		//_float4 vDestLook = m_pTarget->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		//if (vCamLook.Dot(vDestLook) < .95f)
		//{
		//	_float4 vCamDestDir = _float4::Lerp(vCamLook, vDestLook, m_fSlerpRatio);
		//	//Quaternion vDestQuat = Quaternion::FromToRotation((_float3)vCamLook, (_float3)vCamDestDir);
		//	//m_pTransformCom->Turn(vDestQuat);

		//	m_pTransformCom->Look_At_Interpolate( m_pTransformCom->Get_State(CTransform::STATE_POSITION) + Dir(vCamDestDir), fTimeDelta);

		//}

#pragma endregion



#pragma region 방법 4
		/*
		_float4 vCamLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		_float4 vDestLook = m_pTarget->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION);


		if (vCamLook == vDestLook)
			return;


		///////// 보간 속도 조정임
		//_float fInterpolate = fTimeDelta * 12.f;
		_vector vTargetDir = vDestLook;
		_vector vMoveDir = vDestLook;

		vTargetDir = XMVector3Normalize(vTargetDir);
		vMoveDir = XMVector3Normalize(vMoveDir);
		_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDir, vMoveDir));

		if (fcosTheta < -0.9995f || fcosTheta > 0.9995f)
		{
			// 180도로 NaN 방지 랜덤으로 -1, 1도 틀어줌
			_float4x4 rotationMatrix;
			XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
			CUtils::Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomInt(0, 1) == 1 ? 1.f : -1.f);
			vCamLook = XMVector3Transform(vCamLook, XMLoadFloat4x4(&rotationMatrix));
			vCamLook = XMVectorSetW(vCamLook, 0.0f);
		}
		else
		{
			_float ftheta = acos(fcosTheta);
			_float fAngleDegrees = XMConvertToDegrees(ftheta);

			if (fAngleDegrees < 10.0f)
			{
				vCamLook = vDestLook;
			}
			else
			{
				_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
				_float fAlpha = sin((1 - m_fSlerpRatio) * ftheta) / fsinTheta;
				_float fBeta = sin(m_fSlerpRatio * ftheta) / fsinTheta;
				_float4 vResult = vMoveDir * fAlpha + vTargetDir * fBeta;
				//vCamLook = XMVector4Normalize(vResult);
				vCamLook = XMVector3Normalize(vResult);

			}
		}

		m_pTransformCom->Look_At_Dir(vCamLook);
		///////////
		*/
#pragma endregion
		//}


	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);


	//1차. x 좌표 보간.
	_float4 vDestDir = m_vDestCamPos - vCurPos;

	_float4 vDestXZDir = { vDestDir.x, 0.f, vDestDir.z , 0.f };
	_float4 vDestYDir = { 0.f, vDestDir.y, 0.f , 0.f };

	//m_pTransformCom->Look_At(m_pt);


	if (.1f <= vDestXZDir.Length())
		m_pTransformCom->Move(vDestXZDir * .04f);


	if (.1f <= vDestYDir.Length())
		m_pTransformCom->Move(vDestYDir * .02f);



	//if (m_bLerpByTriggerInfo)
	//{
		_float4 vCamLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		_float4 vDestLook = m_vDestCamDir;

	if (vCamLook.Dot(vDestLook) < .95f)
	{
		_float4 vCamDestDir = _float4::Lerp(vCamLook, vDestLook, m_fSlerpRatio);
		//Quaternion vDestQuat = Quaternion::FromToRotation((_float3)vCamLook, (_float3)vCamDestDir);
		//m_pTransformCom->Turn(vDestQuat);

		m_pTransformCom->Look_At_Interpolate(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + Dir(m_vDestCamDir), fTimeDelta);

	}

	//}
	/*


	Quaternion vResultQuat;


	float dot = vCamLook.Dot(vDestLook);
	//if (dot > 0.9999f) {
	//	// 벡터들이 너무 가까울 때, 단위 쿼터니언 반환
	//	vResultQuat =  Quaternion::Identity;
	//}
	//else if (dot < -0.9999f)
	//{
	//	// 벡터들이 반대 방향을 가리킬 때, 180도 회전 쿼터니언 반환
	//	Vector3 orthogonal = Vector3::Right.Cross(vCamLook);
	//	if (orthogonal.LengthSquared() < 0.01f)
	//	{
	//		orthogonal = Vector3::Up.Cross(vCamLook);
	//	}
	//	orthogonal.Normalize();
	//	vResultQuat =  Quaternion(orthogonal, XM_PI);
	//}
	//else
	//{
	if ( -.999f <= dot && dot < .999f)
	{
		vResultQuat = Quaternion::FromToRotation(vCamLook, vDestLook);
		m_pTransformCom->Turn_Absolute(vResultQuat);
	}
	*/


	//}
	//
	/*if (m_bLerpByTriggerInfo)
	{

		_float4 vLerpedDir = SlerpDirVec(vCamLook, vDestLook, m_fSlerpRatio);
		m_pTransformCom->Look_At_Axis(vLerpedDir);

	}*/



	//m_pTransformCom->Look_At_Axis(vLerpedDir);
	/*
	Quaternion vStartQuat = CUtils::Make_Quat_FromDir(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	Quaternion vDestQuat = CUtils::Make_Quat_FromDir(m_pTarget->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	Quaternion vInterpolateQuat = Quaternion::Slerp(vStartQuat, vDestQuat, 1.f);

	*/


}


void CCamera_Free::Control(_float fTimeDelta)
{

	if (m_pGameInstance->Get_KeyState(DIK_TAB, KEY_DOWN))
		m_bTrackTarget = !m_bTrackTarget;

	if (m_bTrackTarget)
		Track_Target(fTimeDelta);


	if (/**m_pCurrentLevelID == LEVEL_TOOL_MAP ||*/ m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{
		_long	MouseMove = { 0 };

		//휠 누른 채로 상하좌우 이동
		if (m_pGameInstance->Get_KeyState(DIMKS_WHEEL, KEY_PRESS))
		{
			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
				m_pTransformCom->Go_Right(fTimeDelta * -MouseMove * m_fMouseSensor);

			if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
				m_pTransformCom->Go_Up(fTimeDelta * MouseMove * m_fMouseSensor);
		}

		//전후진
		if ((MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_WHEEL)) && m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
		{
			m_pTransformCom->Go_Straight(fTimeDelta * MouseMove * m_fMouseSensor * .5f);

			if (m_pTarget != nullptr && m_bTrackTarget)
				m_fTrackDistance -= MouseMove * .01f;
		}

		//우측 마우스 누른 채로 공전

		if (m_pGameInstance->Get_KeyState(DIMKS_RBUTTON, KEY_PRESS))
		{
			_float3 vTargetPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION) + m_pTransformCom->Get_State_Float4(CTransform::STATE_LOOK) * 10.f;


			// 05.22) LEVEL_TOOL_UI에는 카메라 회전 기능 제외
			//if (*m_pCurrentLevelID == LEVEL_TOOL_UI)
			//	return;

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


	//if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_PRESS))
	//{
	//	m_pTransformCom->Go_Left(fTimeDelta);

	//}

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
