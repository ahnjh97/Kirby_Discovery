#include "stdafx.h"
#include "Camera_Main.h"
#include "Kirby.h"

CCamera_Main::CCamera_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CCamera_Main::CCamera_Main(const CCamera_Main& rhs)
	: CCamera{ rhs }
{
}

HRESULT CCamera_Main::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Main::Initialize(void* pArg)
{
	CAMERA_KIRBY_DESC pCamDesc{};

	if (nullptr != pArg)
	{
		pCamDesc = *(CAMERA_KIRBY_DESC*)pArg;

		m_fCamSensor = pCamDesc.fCamSensor;
		m_fDestFovy = pCamDesc.fFovy;
		m_fOrigDistance = pCamDesc.fOrigDistance;


		HRESULT hr = __super::Initialize(pArg);
		CHECK_FAILED_MSG(hr, "Failed To Initialize : CCamera_Main");
	}

	m_pGameInstance->Add_Camera(this);

	//m_vecCamMatrices.emplace_back(_float4x4::Identity);

	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_INTRO ||
		*m_pGameInstance->Get_CurrentLevelID() == LEVEL_GAMEPLAY) {
		function<void(_int)> func = bind(&CCamera_Main::StartLerpByTriggerInfo, this, placeholders::_1);
		m_pGameInstance->Emplace_TriggerFunc(TRIGGER_CAMERA, func);

		function<void(void)> exitFunc = bind(&CCamera_Main::EndLerpByTriggerInfo, this);
		m_pGameInstance->Emplace_ExitFunc(TRIGGER_CAMERA, exitFunc);
	}

	m_fDestDistance = m_fOrigDistance;
	m_fCurDistance = m_fOrigDistance;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Pos(pCamDesc.vEye));
	m_pTransformCom->Look_At(pCamDesc.vAt);

	_float4 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	m_vDestCamDir = _float3{ 0.f, -.15f, 1.f };
	m_vDestCamDir.Normalize();
	m_vCurCamDir = m_vDestCamDir;

	//m_vDestCamDir = _float3{ vLook.x, vLook.y, vLook.z };
	//m_vDestCamDir.Normalize();

	return S_OK;
}



void CCamera_Main::Set_MatrixIndex(_int iMatrixIndex)
{
	if (nullptr == m_pTransformCom || m_vecCamMatrices.empty())
		return;

	if (iMatrixIndex < 0 || iMatrixIndex == m_iMatrixIndex || iMatrixIndex >= m_vecCamMatrices.size())
		return;

	m_pTransformCom->Set_WorldMatrix(m_vecCamMatrices[iMatrixIndex]);
	m_iMatrixIndex = iMatrixIndex;
}



void CCamera_Main::EmplaceBackDirRadius(_int iCamType, _fvector vDir, _float fRadius)
{
	if (CAM_FRONT == iCamType)
		m_vecFrontDirRadius.emplace_back(vDir, fRadius);
	else if (CAM_REAR == iCamType)
		m_vecRearDirRadius.emplace_back(vDir, fRadius);
}

void CCamera_Main::LerpByTriggerInfo(_int iTriggerIndex)
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

	m_vSlerpedDir = SlerpDirVec(m_vecFrontDirRadius[m_iMatrixIndex].first, m_vecRearDirRadius[m_iMatrixIndex].first, m_fTriggerRatio);
	m_fLerpedRadius = LERP(m_vecFrontDirRadius[m_iMatrixIndex].second, m_vecRearDirRadius[m_iMatrixIndex].second, m_fTriggerRatio);

	m_fDestUpOffset = m_CamTriggerUpOffsets[m_iMatrixIndex];

	m_vDestCamDir = m_vSlerpedDir;
	m_fDestDistance = m_fLerpedRadius;
}

void CCamera_Main::EmplaceBackCamMatrix(const _float4x4& matWorld)
{
	_float4x4 mat = matWorld;
	m_vecCamMatrices.emplace_back(mat);
}

void CCamera_Main::EmplaceBackTriggerInfo(const _float4x4& matWorld, _float fScale)
{
	pair<_float4x4, _float> vPair = make_pair(matWorld, fScale);
	m_vecTriggerInfo.emplace_back(vPair);
}

_float CCamera_Main::Compute_TriggerPosRatio(_int iTriggerIndex)
{
	if (m_vecTriggerInfo.empty())
		return _float();


	_float3 vLocalTargetPos = m_pFirstTarget->Get_State(CTransform::STATE_POSITION);
	_float fZ = _float3::Transform(vLocalTargetPos, m_vecTriggerInfo[iTriggerIndex].first).z;

	// rear : 0, middle : 0.5, front: 1
	_float fRatio = 0.5f * fZ + 0.5f;
	return fRatio;
}

_vector CCamera_Main::SlerpDirVec(_fvector vStart, _fvector vEnd, _float fRatio)
{
	_float fDot = ::XMVectorGetX(::XMVector3Dot(vStart, vEnd));
	fDot = clamp(fDot, -1.f, 1.f);
	_float fTheta = acosf(fDot) * fRatio;
	_vector vRelative = ::XMVector3Normalize(vEnd - vStart * fRatio);

	return XMVector3Normalize(vStart * cosf(fTheta) + vRelative * sinf(fTheta));
}


//일련의 동작을 하나의 시퀀스로 선예약한다.
void CCamera_Main::Make_Sequence(CAMSEQ eSeq)
{
	m_eSpecialSeq = eSeq;

	switch (eSeq)
	{
	case SEQ_ZOOMINOUT:
	{
		CAMACTION newAction{};

		newAction.eEase = EASE_OUT;
		newAction.fDist = m_fOrigDistance - 3.f;
		m_SeqList.push_back({ 0.f, newAction });

		newAction = {};
		newAction.eEase = EASE_OUT;
		newAction.fDist = m_fOrigDistance;
		m_SeqList.push_back({ 1.f, newAction });
		break;
	}
	default:
		break;
	}
}

//원하는 정도의 카메라 쉐이킹을 세팅한다.
void CCamera_Main::Make_Shake(_float fPower, _float fTime, _float2 vDir)
{
	m_bIsShaking = true;
	m_fShakePower = fPower;
	m_fShakeTime = fTime;

	vDir.Normalize();
	m_vShakeDir = vDir;
	//m_iShakeCnt = iShakeCnt;
}

void CCamera_Main::Make_Sequence_FromAngle(EASING eEaseFlag, _float fDuration, _float3 fDestAngle, _float fDestZoom)
{

}

void CCamera_Main::Make_Sequence_FromDir(EASING eEaseFlag, _float fDuration, _float3 fDestDir, _float fDestZoom)
{

}

void CCamera_Main::Make_Sequence_FromQuat(EASING eEaseFlag, _float fDuration, _vector vDestQuat, _float fDestZoom)
{

}


_int CCamera_Main::Tick(_float fTimeDelta)
{

	Control(fTimeDelta);

	UpdatePos_FromAnchor(fTimeDelta);


	return OBJ_NOEVENT;
}

void CCamera_Main::Late_Tick(_float fTimeDelta)
{
}

HRESULT CCamera_Main::Render()
{
	return S_OK;
}

void CCamera_Main::Control(_float fTimeDelta)
{

	if (m_pGameInstance->Get_KeyState(DIK_F9, KEY_DOWN))
	{
		Make_Shake();
	}

	//특정 시퀀스가 세팅되어 있는 경우 업데이트한다.
	if (m_eSpecialSeq != SEQ_END)
	{
		//예약 동작이 모두 끝나면 다시 기본 상태로 만든다.
		if (m_SeqList.empty())
		{
			m_eSpecialSeq = SEQ_END;
			return;
		}

		//예약 리스트의 잔여 시간을 모두 깎는다.
		for (auto& seqKey : m_SeqList)
		{
			seqKey.first -= fTimeDelta;
		}

		//시간이 다 되면 동작 실행
		if (m_SeqList.front().first <= 0.f)
		{
			CAMACTION curAction = m_SeqList.front().second;

			//목표 거리 기입되어 있는 경우 set 
			//if (curAction.fDist != -1.f)
			//	Zoom_Absolute(curAction.fDist);

			//목표 방향 기입되어 있는 경우 set
			if (0.f < XMVector3Length(XMLoadFloat3(&curAction.fDir)).m128_f32[0])
			{

			}

			m_SeqList.pop_front();
		}
	}

	//fov y 를 보간하여 갱신한다.
	if (.01f < abs(m_fFovy - m_fDestFovy))
		m_fFovy += (m_fDestFovy - m_fFovy) * fTimeDelta * 3.f;
}



//타겟 위치로부터 카메라 위치를 갱신, 보간한다.
void CCamera_Main::UpdatePos_FromAnchor(_float fTimeDelta)
{
	if (nullptr == m_pFirstTarget)
		return;

	//**** 타겟 위치를 만듬 ****//

	//타겟 위치를 정한다.
	_float4 vTargetPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);


	if (m_eCamFocus == FOCUS_FIRST)
		vTargetPos = m_pFirstTarget->Get_State(CTransform::STATE_POSITION);

	else if (m_eCamFocus == FOCUS_SECOND)
		vTargetPos = m_pSecondTarget->Get_State(CTransform::STATE_POSITION);

	//플레이어와 타겟 사이의 중심점.
	else if (m_eCamFocus == FOCUS_BOTH)
		vTargetPos =
		(m_pFirstTarget->Get_State(CTransform::STATE_POSITION)
			+ m_pSecondTarget->Get_State(CTransform::STATE_POSITION)) * .5f;

	//y 위치 보정
	_float3 vTerrainPos = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0))->Compute_TerrainPosition();
	vTargetPos.y = (vTargetPos.y + vTerrainPos.y) * .5f;


	//실제 타겟 위치에서 조금 위로 기준점 정하기

	//기준점 저장
	_float fYOffset = 2.f * (m_fCurDistance / 30.f) + m_fCurUpOffset;


	m_vAnchor = F4toF3(vTargetPos) + _float3(0.f, fYOffset, 0.f);


	_float3 vTargetProjPos = F4toF3(vTargetPos) + _float3(0.f, fYOffset, 0.f);
	CUtils::Make_World_ToScreen(vTargetProjPos);


	//타겟 포지션을 조절
	//if (.3f < static_cast<_float2>(vTargetProjPos).Length())

		// 타겟 포즈를 중심으로 조절
	//	_float2 vDir = static_cast<_float2>(vTargetProjPos);
	//vDir.Normalize();  // 방향 벡터를 정규화

	//// 임계값 범위 내로 조정
	//vTargetProjPos = _float3((vDir * .3f).x, (vDir * .3f).y, vTargetProjPos.z);

	//// 스크린 좌표를 다시 월드 좌표로 변환
	//CUtils::Make_Screen_ToWorld(vTargetProjPos);


	//m_vAnchor = vTargetProjPos;
	//타겟 포즈가 중심점에 가까운 거리라면 카메라 이동 하지 않고 유지
	//if (static_cast<_float2>(vTargetProjPos).Length() < .2f)
	//{
	//	m_vAnchor = static_cast<_float3>(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	//}



	//**** 설정 값 보간 ****//

	//트리거 안에 들어가 있을 경우 트리거 사이에서의 목표 카메라 설정을 맞춘다.
	if (m_bLerpByTriggerInfo)
		LerpByTriggerInfo(m_iMatrixIndex);



	//떨어진 거리 보간
	if (abs((m_fDestDistance + m_fZoomOffset) - m_fCurDistance) > .1f)
		m_fCurDistance += ((m_fDestDistance + m_fZoomOffset) - m_fCurDistance) * fTimeDelta * 5.f;

	//y 오프셋 보간
	if (abs(m_fDestUpOffset - m_fCurUpOffset) > .001f)
		m_fCurUpOffset += (m_fDestUpOffset - m_fCurUpOffset) * fTimeDelta * 2.f;

	//if (abs(m_fDestUpOffset - m_fCurUpOffset) > .1f)
	//	LERP(m_fCurUpOffset, m_fDestUpOffset, fTimeDelta * 5.f);

	//각도 보간
	m_vCurCamDir = SlerpDirVec(m_vCurCamDir, m_vDestCamDir, fTimeDelta * 4.f);



	//**** 목표 위치 마지막 저장 ****//

	//타겟 기준으로 뒤 위치를 목표한다.
	_float3 vBackDir = -m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vBackDir.Normalize();

	//실제 카메라 목표 위치를 저장한다.
	m_vDestCamPos = m_vAnchor - (m_vCurCamDir * m_fCurDistance);


	//**** 카메라 쉐이킹 오프셋 ****//

	_float3 vShakeDir = XMVectorZero();

	//쉐이크 세팅 존재 시, 그만큼 팅궈준다.
	if (m_bIsShaking)
	{
		_float fSinOffset = sinf(m_fShakeTime * m_fShakeFrequency) * m_fShakePower * m_fShakeAmplitude * m_fShakeTime;

		vShakeDir = static_cast<_float3>(m_vShakeDir * fSinOffset);

		if (m_fShakeTime <= 0.f)
		{
			m_bIsShaking = false;
			m_fShakeTime = 0.f;
		}
		m_fShakeTime -= fTimeDelta;
	}

	//**** 목표 위치를 따라간다 ****//

	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	_float4 vDestDir = Dir(Pos(m_vDestCamPos + vShakeDir) - Pos(vCurPos));
	_float4 vDestXZDir = { vDestDir.x, 0.f, vDestDir.z , 0.f };
	_float4 vDestYDir = { 0.f, vDestDir.y, 0.f , 0.f };




	//x 가기
	if (.1f <= vDestXZDir.Length() /*&& .2f < static_cast<_float2>(vTargetProjPos).Length()*/)
		m_pTransformCom->Move(vDestXZDir * fTimeDelta * 4.f);

	//y로 가기
	if (.1f <= vDestYDir.Length())
	{
		if (0.f < vDestYDir.y)
			m_pTransformCom->Move(vDestYDir * fTimeDelta * 2.5f);
		else
		{
			m_pTransformCom->Move((1.f < vDestYDir.Length()) ? _float4{ 0.f, -1.f, 0.f, 0.f } *fTimeDelta * 3.f : vDestYDir * fTimeDelta * 2.5f);

		}

	}


	m_pTransformCom->Look_At_Interpolate(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + Dir(m_vCurCamDir) + _float4{0.f, m_fCurUpOffset, 0.f, 0.f}, fTimeDelta);

}

void CCamera_Main::Orbit_Target(_float fTimeDelta)
{
	if (nullptr == m_pFirstTarget)
		return;

	//Transform 다른 부분 건드려야 되서 일단 대기 용으로 주석 처리해 둡니다
/*
	_float fLen = XMVector2Length(XMLoadFloat2(&m_fCamOrbitDelta)).m128_f32[0];

	if (0.f <= fLen)
	{
		m_pTransformCom->Orbit(F3ToVec(m_vAnchor), XMVectorSet(0.f, 1.f, 0.f, 1.f), fTimeDelta * (m_fCamOrbitDelta.x * .15f) * m_fMouseSensor);


		if (5.f <= m_fCurAngle.second && m_fCurAngle.second < 80.f)
			m_pTransformCom->Orbit(F3ToVec(m_vAnchor), m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * (m_fCamOrbitDelta.y * .15f) * m_fMouseSensor);
		else
		{
			if (5.f <= m_fCurAngle.second)
				m_fCamOrbitDelta.y = -5.f;

			else
				m_fCamOrbitDelta.y = 5.f;

			m_pTransformCom->Orbit(F3ToVec(m_vAnchor), m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * (m_fCamOrbitDelta.y * .15f) * m_fMouseSensor);
		}
	}


	//}

	fLen = XMVector2Length(XMLoadFloat2(&m_fCamOrbitDelta)).m128_f32[0];
	m_fCamOrbitDelta.x -= (m_fCamOrbitDelta.x * .15f);
	m_fCamOrbitDelta.y -= (m_fCamOrbitDelta.y * .15f);

	if (fLen <= 5.f)
		m_fCamOrbitDelta = { 0.f, 0.f };
*/
}

#ifdef _DEBUG
void CCamera_Main::Render_IMGUI()
{

	_float4x4 WorldMat = m_pTransformCom->Get_WorldMatrix();
	_float4 vPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);


	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._11, WorldMat._12, WorldMat._13, WorldMat._14);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._21, WorldMat._22, WorldMat._23, WorldMat._24);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._31, WorldMat._32, WorldMat._33, WorldMat._34);
	ImGui::Text("%.2f\t%.2f\t%.2f\t%.2f", WorldMat._41, WorldMat._42, WorldMat._43, WorldMat._44);


	//ImGui::Text("TriggerRatio: %.2f", m_fTriggerRatio);
	//ImGui::Text("SlerpedDir: %.2f, %.2f, %.2f", XMVectorGetX(m_vSlerpedDir), XMVectorGetY(m_vSlerpedDir), XMVectorGetZ(m_vSlerpedDir));
	//ImGui::Text("LerpedRadius: %.2f", m_fLerpedRadius);
	ImGui::Dummy(ImVec2(0, 20));

	ImGui::Text(u8"보간 ratio: %.2f", m_fTriggerRatio);
	ImGui::Text(u8"현재 up offset: %.2f", m_fCurUpOffset);


	static _float fFOVY = ToDegree(m_fDestFovy);

	if (ImGui::DragFloat(u8"FOV", &fFOVY, .1f, 10.f, 50.f, "%.1f"))
	{
		m_fDestFovy = ToRadian(fFOVY);
	}




	ImGui::DragFloat(u8"타겟까지의 목표 거리", &m_fDestDistance, .05f, 1.f, 50.f, "%.1f");
	ImGui::DragFloat(u8"줌 오프셋", &m_fZoomOffset, .05f, -20.f, 20.f, "%.1f");

	ImGui::Text(u8"현재 거리: %.2f", m_fCurDistance);

	ImGui::Dummy(ImVec2(0, 20));

	ImGui::Text(u8" 목표 Dir %.2f\t%.2f\t%.2f", m_vDestCamDir.x, m_vDestCamDir.y, m_vDestCamDir.z);
	ImGui::Text(u8" 현재 Dir %.2f\t%.2f\t%.2f", m_vCurCamDir.x, m_vCurCamDir.y, m_vCurCamDir.z);

}
#endif

CCamera_Main* CCamera_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Main* pInstance = new CCamera_Main(pDevice, pContext);

	HRESULT hr = pInstance->Initialize_Prototype();

	if (FAILED(hr))
	{
		CHECK_FAILED_MSG(hr, "Failed To Clone : CCamera_Main");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCamera_Main::Clone(void* pArg)
{
	CCamera_Main* pInstance = new CCamera_Main(*this);


	HRESULT hr = pInstance->Initialize(pArg);
	CHECK_FAILED_MSG(hr, "Failed To Clone : CCamera_Main");

	return pInstance;
}

void CCamera_Main::Free()
{
	__super::Free();
}
