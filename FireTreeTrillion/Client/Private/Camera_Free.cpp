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

	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_GAMEPLAY) {
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
	Orbit_Target(fTimeDelta);
	if (true == m_bLerpByTriggerInfo)
		LerpByTriggerInfo(m_iMatrixIndex);

	/*
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_PRESS))
	{
		m_pTransformCom->Go_Left(fTimeDelta);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_D, KEY_PRESS))
	{
		m_pTransformCom->Go_Right(fTimeDelta);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_W, KEY_PRESS))
	{
		m_pTransformCom->Go_Straight(fTimeDelta);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_S, KEY_PRESS))
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	_long	MouseMove = { 0 };
	if (m_pGameInstance->Get_DIKeyState(DIK_LSHIFT, KEY_PRESS))
	{
		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * MouseMove * m_fMouseSensor);
		}
		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * m_fMouseSensor);
		}
	}
	*/

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0.f, 0.f, 0.f, 1.f));

	//m_fFovy = XMConvertToRadians(120.f);

	//__super::Bind_PipeLines();


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


	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._11, WorldMat._12, WorldMat._13, WorldMat._14 );
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
	else if(CAM_REAR == iCamType)
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

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, pKirbyPos - m_vSlerpedDir * m_fLerpedRadius);
	m_pTransformCom->Look_At(pKirbyPos);
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
	//vTargetPos.y = m_pTransformCom->Get_State(CTransform::STATE_POSITION).y;
	if (*m_pCurrentLevelID == LEVEL_GAMEPLAY)
	{
		_float4 vTerrainPos = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0))->Compute_TerrainPosition();
		vTargetPos.y = vTerrainPos.y;
	}

	_float4 vBackDir = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vBackDir.Normalize();
	vBackDir *= m_fTrackDistance;

	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	_float4 vDestDir = (vTargetPos + vBackDir) - vCurPos;

	if (.1f <= vDestDir.Length())
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(vCurPos + (vDestDir * .06f)));
}

void CCamera_Free::Orbit_Target(_float fTimeDelta)
{
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
