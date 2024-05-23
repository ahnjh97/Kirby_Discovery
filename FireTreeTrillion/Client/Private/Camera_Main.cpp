#include "stdafx.h"
#include "Camera_Main.h"


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
	CAMERA_KIRBY_DESC* pCamDesc{};

	if (nullptr != pArg)
	{
		pCamDesc = (CAMERA_KIRBY_DESC*)pArg;

		m_fCamSensor = pCamDesc->fCamSensor;
		m_fDestFovy = pCamDesc->fFovy;
		m_fOrigDistance = pCamDesc->fOrigDistance;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&pCamDesc->vEye));
		m_pTransformCom->Look_At(XMLoadFloat4(&pCamDesc->vAt));

		HRESULT hr = __super::Initialize(pArg);
		CHECK_FAILED_MSG(hr, "Failed To Initialize : CCamera_Main");

	}


	m_fCurDistance = m_fOrigDistance;
	ShowCursor(FALSE);

	return S_OK;
}

void CCamera_Main::Zoom_In(_float fZoom)
{
}

void CCamera_Main::Zoom_Out(_float fZoom)
{
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
void CCamera_Main::Make_Shake(_float fPower, _int iShakeCnt)
{
	m_bIsShaking = true;
	m_fShakePower = fPower;
	m_iShakeCnt = iShakeCnt;
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
	Orbit_Target(fTimeDelta);
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
			if (curAction.fDist != -1.f)
				Zoom_Absolute(curAction.fDist);

			//목표 방향 기입되어 있는 경우 set
			if (0.f < XMVector3Length(XMLoadFloat3(&curAction.fDir)).m128_f32[0])
			{

			}

			m_SeqList.pop_front();
		}
	}

	//fov y 를 보간하여 갱신한다.
	if (.05f < abs(m_fFovy - m_fDestFovy))
		m_fFovy += (m_fDestFovy - m_fFovy) * fTimeDelta * 3.f;
}

//타겟 위치로부터 카메라 위치를 갱신, 보간한다.
void CCamera_Main::UpdatePos_FromAnchor(_float fTimeDelta)
{
	if (nullptr == m_pFirstTarget)
		return;

	//타겟 위치
	_vector vTargetPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	if (m_eCamFocus == FOCUS_FIRST)
		vTargetPos = m_pFirstTarget->Get_State_Vector(CTransform::STATE_POSITION);
	
	else if (m_eCamFocus == FOCUS_SECOND)
		vTargetPos = m_pSecondTarget->Get_State_Vector(CTransform::STATE_POSITION);

	//플레이어와 타겟 사이의 중심점.
	else if (m_eCamFocus == FOCUS_BOTH)
		vTargetPos =
				(m_pFirstTarget->Get_State_Vector(CTransform::STATE_POSITION)
				+ m_pSecondTarget->Get_State_Vector(CTransform::STATE_POSITION))
				*.5f;


	//기준점은 실제 타겟 위치에서 조금 위로
	vTargetPos += XMVectorSet(0.f, 2.f, 0.f, 0.f);


	_vector vShakeDir = XMVectorZero();

	//쉐이크 세팅 존재 시, 팅구는 오프셋을 같이 넣어 준다.
	if (m_bIsShaking)
	{
		_vector vRight = m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
		_vector vUp = m_pTransformCom->Get_State_Vector(CTransform::STATE_UP);

		//0.3 ~ 0.7의 비율
		//_float fXOffset = m_pGameInstance->GetRandomInt(3, 7) * .1f;
		_float fXOffset = 5.f * .1f;
		_float fYOffset = 1.f - fXOffset;

		//_bool bOffset = _bool(m_pGameInstance->GetRandomNumber(0, 1));

		//vRight *= (m_fShakePower + fXOffset) * (_bool(m_pGameInstance->GetRandomInt(0, 1)) ? 1.f : -1.f);
		vRight *= (m_fShakePower + fXOffset) * 1.f;
		//vUp *= (m_fShakePower + fYOffset) * (_bool(m_pGameInstance->GetRandomInt(0, 1)) ? -1.f : 1.f);
		vUp *= (m_fShakePower + fYOffset) * 1.f;

		vShakeDir = XMVectorSetW(vRight + vUp, 0.f);

		if (m_iShakeCnt <= 0)
		{
			m_bIsShaking = false;
			m_iShakeCnt = 0;
		}
		--m_iShakeCnt;
	}

	//기준점 저장
	XMStoreFloat3(&m_vAnchor, vTargetPos);

	//기준점에서 뒤 방향으로 간 위치를 cam dest pos에 저장
	_vector vBackDir = XMVector3Normalize(-m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));
	XMStoreFloat3(&m_vCamDestPos, vTargetPos + (vBackDir * m_fCurDistance) + vShakeDir);

	//현재 위치에서 목표 지점으로의 방향을 계산한다.
	_vector vCurPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vDir = XMVectorSetW(XMLoadFloat3(&m_vCamDestPos), 1.f) - XMVectorSetW(vCurPos, 1.f);

	//보간한다.
	if (.1f <= XMVector3Length(vDir).m128_f32[0])
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vCurPos + (vDir * .1f));
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

CCamera_Main* CCamera_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Main* pInstance = new CCamera_Main(pDevice, pContext);

	pInstance->Initialize_Prototype();

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
	Safe_Release(m_pFirstTarget);
	Safe_Release(m_pSecondTarget);

	__super::Free();
}
