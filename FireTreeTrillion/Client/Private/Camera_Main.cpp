#include "stdafx.h"
#include "Camera_Main.h"
#include "Kirby.h"
#include "PartTimerKirby.h"
#include "EventCenter.h"

CCamera_Main::CCamera_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CCamera_Main::CCamera_Main(const CCamera_Main& rhs)
	: CCamera{ rhs }
{
}

void CCamera_Main::EventFunc(CGameObject* pObj)
{
	//Make_Sequence(SEQ_SOFTCUT_TEST);
	_int a = 0;
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

	//트리거 세팅
	_uint iLevel = *m_pGameInstance->Get_CurrentLevelID();
	if (iLevel >= LEVEL_INTRO && iLevel <= LEVEL_FINALBOSS) {
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
	m_vDestCamDir = _float3(vLook);
	m_vDestCamDir.Normalize();
	m_vCurCamDir = m_vOrigCamDir = m_vDestCamDir;

	Subscribe_Events();

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
	//해당 시퀀스를 명시적으로 변수로 저장!
	m_eSpecialSeq = eSeq;

	switch (eSeq)
	{
	case SEQ_HARDCUT_TEST:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ -8.f, 6.f, -8.f };
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 1.4f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 8.f, 6.f, -8.f };
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		m_CamSeq.push_back(newAction);



		//newAction = {};
		//newAction.fTime = 1.f;
		//newAction.eCamCut = CUT_INTERPOLATE;
		//newAction.eEase = EASE_OUT;
		//newAction.fInterpolateSpeed = .4f;
		//newAction.eCamPos = POS_RELATIVE;
		//newAction.vPos = _float3{ 0.1f, 15.f, 0.1f };
		//newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		//m_CamSeq.push_back(newAction);

		//newAction = {};
		//newAction.fTime = 1.5f;
		//newAction.eCamCut = CUT_HARD;
		//newAction.eCamPos = POS_RELATIVE;
		//newAction.vPos = _float3{ 40.f, 60.f, 10.f };
		//newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		//m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 2.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 0.f, 3.f, 15.f };
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };
		m_CamSeq.push_back(newAction);
	}
	break;

	case SEQ_ZOOMINOUT:
	{
		//CAMACTION newAction{};

		//newAction.eEase = EASE_OUT;
		//newAction.fDist = m_fOrigDistance - 3.f;
		//m_CamSeq.push_back({ 0.f, newAction });

		//newAction = {};
		//newAction.eEase = EASE_OUT;
		//newAction.fDist = m_fOrigDistance;
		//m_CamSeq.push_back({ 1.f, newAction });
		//break;
	}
	break;

	case SEQ_SOFTCUT_TEST:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 8.f, 6.f, -8.f };
		m_CamSeq.push_back(newAction);


		newAction = {};
		newAction.fTime = .5f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ -8.f, 6.f, -8.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 1.f;
		newAction.eCamCut = CUT_HARD;
		//newAction.eEase = EASE_OUT;
		//newAction.fInterpolateSpeed =.5f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 10.5f, 55.f,10.5f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 1.5f;
		newAction.eCamCut = CUT_HARD;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_ABSOLUTE;
		newAction.vPos = _float3{ 40.f, 60.f, 10.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 2.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = .5f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 0.f, 3.f, 15.f };
		m_CamSeq.push_back(newAction);
	}
	break;

	case SEQ_BREAKCARSHOP:
	{
		CAMACTION newAction{};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_HARD;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 3.f, -4.f, 40.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 0.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 3.f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 3.f, -8.f, 40.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 3.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 2.f;
		newAction.eCamPos = POS_RELATIVE;
		newAction.vPos = _float3{ 2.f, 15.f, 15.f };
		m_CamSeq.push_back(newAction);

		newAction = {};
		newAction.fTime = 5.f;
		newAction.eCamCut = CUT_INTERPOLATE;
		newAction.eEase = EASE_OUT;
		newAction.fInterpolateSpeed = 1.f;
		newAction.eCamPos = POS_RELATIVE;

		newAction.vPos = _float3{ 0.f, 6.f, 30.f };
		newAction.eCamDir = DIR_ABSOLUTE;
		newAction.vDir = _float3{ 0.f, -.2f, 1.f };

		m_CamSeq.push_back(newAction);
	}
	break;
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

void CCamera_Main::Start_ShutterSeq(CGameObject* pNotifier)
{
	Make_Sequence(SEQ_BREAKCARSHOP);
}

_int CCamera_Main::Tick(_float fTimeDelta)
{

	Control(fTimeDelta);

	//특정 시퀀스가 세팅되어 있는 경우 업데이트한다.
	Play_Sequence(fTimeDelta);

	if (m_eSpecialSeq != SEQ_END)
		return OBJ_NOEVENT;

	UpdatePos_FromAnchor(fTimeDelta);

	return OBJ_NOEVENT;
}

HRESULT CCamera_Main::Render()
{
	return S_OK;
}

void CCamera_Main::Play_Sequence(_float fTimeDelta)
{


	if (m_eSpecialSeq == SEQ_END)
		return;

	//예약 동작이 모두 끝나면 다시 기본 상태로 만든다.
	if (m_CamSeq.empty() && m_fSeqInterpolateTime.first == m_fSeqInterpolateTime.second)
	{
		m_eSpecialSeq = SEQ_END;
		m_eCurSeqEase = EASE_END;
		m_fSeqInterpolateTime = { 0.f, 0.f };

		 m_vDestCamPos = m_vCurCamPos;
		 m_vDestCamDir = m_vCurCamDir;
		 m_fDestFovy = m_fFovy;
		 m_fDestZAngle = m_fCurZAngle;
		 m_fDestZoomOffset = m_fCurZoomOffset;

		return;
	}

	if (!m_CamSeq.empty())
	{
		//예약 리스트의 잔여 시간을 모두 깎는다.
		for (auto& seqKey : m_CamSeq)
		{
			seqKey.fTime -= fTimeDelta;
		}

		//시간이 다 되면, 맨 앞쪽에 있는 동작 정보를 읽는다.
		if (m_CamSeq.front().fTime <= 0.f)
		{
			CAMACTION curAction = m_CamSeq.front();

			Update_Anchor(fTimeDelta);

			if (curAction.eCamCut == CUT_HARD)
			{
				m_eCamCut = CUT_HARD;
				if (!ISDEFAULTFLOAT3(curAction.vPos))
				{
					if (curAction.eCamPos == POS_RELATIVE)
					{
						_float4x4 ToWorldMatrix = m_pFirstTarget->Get_WorldFloat4x4();

						_float4 vRight = ToWorldMatrix.Right();
						_float4 vUp = ToWorldMatrix.Up();
						_float4 vLook = ToWorldMatrix.Backward();
						vRight.Normalize();
						vUp.Normalize();
						vLook.Normalize();
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_RIGHT, Dir(vRight));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_UP, Dir(vUp));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_LOOK, Dir(vLook));

						_float3 vWorldPos = _float3::Transform(curAction.vPos, ToWorldMatrix);
						SET_POS(Pos(vWorldPos));
					}
					else
						SET_POS(Pos(curAction.vPos));
				}

				// 카메라 액션의 x, y, z가 모두 default일 경우, 타겟 위치를 곧바로 쳐다본다.
				if (ISDEFAULTFLOAT3(curAction.vDir))
				{
					m_pTransformCom->Look_At(Pos(m_vAnchor));
					m_vDestCamDir = m_vCurCamDir = (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK);
				}
				//default가 아닐 경우, 설정된 방향으로 쳐다본다.
				else
				{
					m_pTransformCom->Look_At_Axis(curAction.vDir);
					m_vDestCamDir = m_vCurCamDir = curAction.vDir;
				}

				if (!ISDEFAULTFLOAT(curAction.fFOVY))
					m_fFovy = m_fDestFovy = curAction.fFOVY;

				if (!ISDEFAULTFLOAT(curAction.fZAngle))
					m_fDestZAngle = m_fCurZAngle = curAction.fZAngle;

				if (!ISDEFAULTFLOAT(curAction.fZoomOffset))
					m_fDestZoomOffset = m_fCurZoomOffset = curAction.fZoomOffset;
			}
			else
			{
				m_eCamCut = CUT_INTERPOLATE;
				m_eCurSeqEase = curAction.eEase;
				//보간 시간을 계산할 친구를 초기화해준다.
				m_fSeqInterpolateTime = { 0.f, curAction.fInterpolateSpeed };

				if (!ISDEFAULTFLOAT3(curAction.vPos))
				{
					if (curAction.eCamPos == POS_RELATIVE)
					{
						_float4x4 ToWorldMatrix = m_pFirstTarget->Get_WorldFloat4x4();

						_float4 vRight = ToWorldMatrix.Right();
						_float4 vUp = ToWorldMatrix.Up();
						_float4 vLook = ToWorldMatrix.Backward();
						vRight.Normalize();
						vUp.Normalize();
						vLook.Normalize();
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_RIGHT, Dir(vRight));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_UP, Dir(vUp));
						CUtils::Set_State_Matrix(ToWorldMatrix, CUtils::STATE_LOOK, Dir(vLook));

						_float3 vWorldPos = _float3::Transform(curAction.vPos, ToWorldMatrix);

						m_vDestCamPos = vWorldPos;
					}
					else
						m_vDestCamPos = curAction.vPos;

					m_vStartCamPos = GET_POS;
				}

				// 카메라 목표 방향 값이 default일 경우와 아닐 경우를 구별한다.
				m_bSeqDestDirIsAbsolute = ISDEFAULTFLOAT3(curAction.vDir) ? false : true;

				m_vStartCamDir = (_float3)m_pTransformCom->Get_State(CTransform::STATE_LOOK);
				m_vDestCamDir = curAction.vDir;

				if (!ISDEFAULTFLOAT(curAction.fFOVY))
				{
					m_fStartFovy = m_fFovy;
					m_fDestFovy = curAction.fFOVY;
				}
				else
				{
					m_fDestFovy = m_fStartFovy = m_fFovy;
				}

				if (!ISDEFAULTFLOAT(curAction.fZAngle))
				{
					m_fStartZAngle = m_fCurZAngle;
					m_fDestZAngle = curAction.fZAngle;
				}

				if (!ISDEFAULTFLOAT(curAction.fZoomOffset))
				{
					m_fStartZoomOffset = m_fCurZoomOffset;
					m_fDestZoomOffset = curAction.fZoomOffset;
				}
			}

			m_CamSeq.pop_front();
		}
	}


	if (m_eCamCut != CUT_INTERPOLATE)
		return;


	//보간 카메라
	if (m_fSeqInterpolateTime.first < m_fSeqInterpolateTime.second)
	{
		m_fSeqInterpolateTime.first += fTimeDelta;

		if (m_fSeqInterpolateTime.second < m_fSeqInterpolateTime.first)
			m_fSeqInterpolateTime.first = m_fSeqInterpolateTime.second;
	}

	_float fInterpolateRatio = SATURATE(m_fSeqInterpolateTime.first / m_fSeqInterpolateTime.second);

	switch (m_eCurSeqEase)
	{
	case EASE_IN:
		fInterpolateRatio = EASE_IN(fInterpolateRatio);
		break;
	case EASE_IN_FAST:
		fInterpolateRatio = EASE_IN_FAST(fInterpolateRatio);
		break;
	case EASE_OUT:
		fInterpolateRatio = EASE_OUT(fInterpolateRatio);
		break;
	case EASE_OUT_FAST:
		fInterpolateRatio = EASE_OUT_FAST(fInterpolateRatio);
		break;
	case EASE_INOUT:
		fInterpolateRatio = EASE_INOUT(fInterpolateRatio);
		break;
	case EASE_INOUT_FAST:
		fInterpolateRatio = EASE_INOUT_FAST(fInterpolateRatio);
		break;
	default: //그냥 Linear도 여기 포함
		break;
	}


	_float3 vCamPos = _float3::Lerp(m_vStartCamPos, m_vDestCamPos, fInterpolateRatio);

	_float3 vCamDir = m_pFirstTarget->Get_State(CTransform::STATE_POSITION) - (_float4)GET_POS;
	if (m_bSeqDestDirIsAbsolute)
		vCamDir = _float3::Lerp(m_vStartCamDir, m_vDestCamDir, fInterpolateRatio);

	vCamDir.Normalize();


	_float fFOVY = LERP(m_fStartFovy, m_fDestFovy, fInterpolateRatio);
	_float fZAngle = LERP(m_fStartZAngle, m_fDestZAngle, fInterpolateRatio);
	_float fZoomOffset = LERP(m_fStartZoomOffset, m_fDestZoomOffset, fInterpolateRatio);


	m_vCurCamPos = vCamPos;
	m_vCurCamDir = vCamDir;
	m_fFovy = fFOVY;
	m_fCurZAngle = fZAngle;
	m_fCurZoomOffset = fZoomOffset;

	MoveTo_CurCamPos_Absolute(fTimeDelta);


	//SET_POS(Pos(m_vCurCamPos));
	//m_pTransformCom->Look_At_Axis(m_vCurCamDir);

}

void CCamera_Main::Control(_float fTimeDelta)
{

	if (m_pGameInstance->Get_KeyState(DIK_LSHIFT, KEY_PRESS))
	{
		if (m_pGameInstance->Get_KeyState(DIK_T, KEY_DOWN))
		{
			//SEQ_HARDCUT_TEST
			Make_Sequence(SEQ_HARDCUT_TEST);
			//Make_Sequence(SEQ_SOFTCUT_TEST);
		}
	}

	if (m_pGameInstance->Get_KeyState(DIK_9, KEY_DOWN))
	{
		CEventCenter::Get_Instance()->Notify(KEVENT_DDD_DEAD, this);
	}
}



void CCamera_Main::Update_Anchor(_float fTimeDelta)
{

	if (nullptr == m_pFirstTarget)
	{
		m_vAnchor = _float3(ZeroVecPos);
		return;
	}

	//타겟 위치를 정한다.
	_float4 vTargetPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	//첫번째 타겟 포커스
	if (m_eCamFocus == FOCUS_FIRST)
		vTargetPos = m_pFirstTarget->Get_State(CTransform::STATE_POSITION);

	//두번째 타겟 포커스
	else if (m_eCamFocus == FOCUS_SECOND)
		vTargetPos = m_pSecondTarget->Get_State(CTransform::STATE_POSITION);

	//두 타겟 사이의 중심점.
	else if (m_eCamFocus == FOCUS_BOTH)
		vTargetPos =
		(m_pFirstTarget->Get_State(CTransform::STATE_POSITION)
			+ m_pSecondTarget->Get_State(CTransform::STATE_POSITION)) * .5f;


	//지형 위치를 구하여 같이 쓰기
	_float4 vTerrainPos = static_cast<CCharacter*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"), 0))->Compute_TerrainPosition();

	if (vTerrainPos.y != 0.f)
		vTargetPos.y = (vTargetPos.y + vTerrainPos.y) * .5f;


	//실제 타겟 위치에서 조금 위로 기준점 정하기
	_float fYOffset = 2.f * (m_fCurDistance / 30.f) + m_fCurUpOffset;

	//기준점 저장
	m_vAnchor = F4toF3(vTargetPos) + _float3(0.f, fYOffset, 0.f);
}

void CCamera_Main::Update_CurCamPos(_float fTimeDelta)
{

	//실제 카메라 목표 위치를 저장한다.
	if (m_eCamFocus == FOCUS_BOTH)
		m_vCurCamPos = m_pFirstTarget->Get_State(CTransform::STATE_POSITION) - (m_vCurCamDir * m_fCurDistance);
	else
		m_vCurCamPos = m_vAnchor - (m_vCurCamDir * m_fCurDistance);
}

void CCamera_Main::Interpolate_CamSet(_float fTimeDelta)
{
	//떨어진 거리 보간
	if (abs((m_fDestDistance + m_fCurZoomOffset) - m_fCurDistance) > .1f)
		m_fCurDistance += ((m_fDestDistance + m_fCurZoomOffset) - m_fCurDistance) * fTimeDelta * 5.f;

	//y 오프셋 보간
	if (abs(m_fDestUpOffset - m_fCurUpOffset) > .001f)
		m_fCurUpOffset += (m_fDestUpOffset - m_fCurUpOffset) * fTimeDelta * 2.f;

	//fov y 보간
	if (.01f < abs(m_fFovy - m_fDestFovy))
		m_fFovy += (m_fDestFovy - m_fFovy) * fTimeDelta * 3.f;

	//각도 보간
	_float fSlerpSpeed = (m_eCamFocus == FOCUS_BOTH) ? 12.f : 4.f;
	m_vCurCamDir = SlerpDirVec(m_vCurCamDir, m_vDestCamDir, clamp(fTimeDelta * fSlerpSpeed, 0.f, 1.f));
}


void CCamera_Main::Subscribe_Events()
{
	CCamera_Main* pCam = this;


	//셔터 뿌수기
	function<void(CGameObject*)> func = bind(&CCamera_Main::Start_ShutterSeq, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_BREAK_CARSHOP, this, func, 0);

	//디디디 사망
	func = bind(&CCamera_Main::EventFunc, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_DDD_DEAD, this, func, 0);
}

//타겟 위치로부터 카메라 위치를 갱신, 보간한다.
void CCamera_Main::UpdatePos_FromAnchor(_float fTimeDelta)
{

	if (nullptr == m_pFirstTarget)
		return;

	_float fRealTimeDelta = fTimeDelta;


	//**** 타겟 위치를 만듬 ****//
	Update_Anchor(fTimeDelta);


	//_float3 vTargetProjPos = F4toF3(m_vAnchor);
	//CUtils::Make_World_ToScreen(vTargetProjPos);


	//**** 설정 값 보간 ****//

	//트리거 안에 들어가 있을 경우 트리거 사이에서의 목표 카메라 설정을 맞춘다.
	if (m_bLerpByTriggerInfo)
		LerpByTriggerInfo(m_iMatrixIndex);


	// 두 타겟을 잡을 때, 조정
	if (m_eCamFocus == FOCUS_BOTH)
	{
		_float3 vDir = _float3(m_pSecondTarget->Get_State(CTransform::STATE_POSITION) - m_pFirstTarget->Get_State(CTransform::STATE_POSITION));

		m_vDestCamDir = vDir;
		m_vDestCamDir.y = m_vOrigCamDir.y;

		_float fDist = vDir.Length();
		fDist = clamp(fDist, 12.f, 40.f);
		m_vDestCamDir.y += MAPVALUE(vDir.Length(), 12.f, 40.f, -4.5f, -3.f);
		m_vDestCamDir.Normalize();
	}


	if (.1f < fRealTimeDelta)
		fRealTimeDelta = 1.f / 30.f;


	//**** 카메라 세팅 값 보간 ****//
	Interpolate_CamSet(fRealTimeDelta);


	//**** 목표 위치 마지막 저장 ****//
	Update_CurCamPos(fRealTimeDelta);


	//**** 목표 위치로 이동 ****//
	MoveTo_CurCamPos_Interpolate(fRealTimeDelta);

}

_float3 CCamera_Main::Make_ShakeDir(_float fTimeDelta)
{
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
	return vShakeDir;
}

void CCamera_Main::MoveTo_CurCamPos_Interpolate(_float fTimeDelta)
{

	//**** 목표 위치를 따라간다 ****//
	_float4 vCurPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	_float4 vDestDir = Dir(Pos(m_vCurCamPos + Make_ShakeDir(fTimeDelta)) - Pos(vCurPos));
	_float4 vDestXZDir = { vDestDir.x, 0.f, vDestDir.z , 0.f };
	_float4 vDestYDir = { 0.f, vDestDir.y, 0.f , 0.f };


	//x 가기
	if (.1f <= vDestXZDir.Length())
		m_pTransformCom->Move(vDestXZDir * fTimeDelta * ((m_eCamFocus == FOCUS_BOTH) ? 12.f : 2.f));

	//y로 가기
	if (.1f <= vDestYDir.Length())
	{
		if (0.f < vDestYDir.y)
			m_pTransformCom->Move(vDestYDir * fTimeDelta * 2.5f);
		else
		{
			m_pTransformCom->Move((1.f < vDestYDir.Length()) ? vDestYDir * fTimeDelta * 3.f : vDestYDir * fTimeDelta * 2.f);
		}
	}

	//보간하여 바라보기
	m_pTransformCom->Look_At_Interpolate(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + Dir(m_vCurCamDir) + _float4{ 0.f, m_fCurUpOffset, 0.f, 0.f },
		fTimeDelta * (m_eCamFocus != FOCUS_BOTH ? 1.f : 10.f));

}

void CCamera_Main::MoveTo_CurCamPos_Absolute(_float fTimeDelta)
{
	SET_POS(Pos(m_vCurCamPos));
	m_pTransformCom->Look_At_Axis(Dir(m_vCurCamDir));
	//m_pTransformCom->Look_At_Interpolate(m_pTransformCom->Get_State(CTransform::STATE_POSITION) + Dir(m_vCurCamDir), fTimeDelta);

}

/*
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
}
*/

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


	ImGui::Text(u8"보간 시간: %.2f", m_fSeqInterpolateTime.first);
	ImGui::Text(u8"목표 보간 시간: %.2f", m_fSeqInterpolateTime.second);


	ImGui::Text(u8"보간 ratio: %.2f", m_fTriggerRatio);
	ImGui::Text(u8"현재 up offset: %.2f", m_fCurUpOffset);


	ImGui::Text(u8"현재 FOV: %.2f", ToDegree(m_fFovy));
	static _float fFOVY = ToDegree(m_fDestFovy);
	if (ImGui::DragFloat(u8"목표 FOV", &fFOVY, .1f, 10.f, 50.f, "%.1f"))
		m_fDestFovy = ToRadian(fFOVY);


	ImGui::DragFloat(u8"현재 줌 오프셋", &m_fCurZoomOffset, .05f, -20.f, 20.f, "%.1f");


	ImGui::Text(u8"현재 거리 : %.2f", m_fCurDistance);
	ImGui::DragFloat(u8"타겟까지의 목표 거리", &m_fDestDistance, .05f, 1.f, 50.f, "%.1f");



	ImGui::Dummy(ImVec2(0, 20));

	ImGui::Text(u8" 현재 Dir %.2f\t%.2f\t%.2f", m_vCurCamDir.x, m_vCurCamDir.y, m_vCurCamDir.z);
	ImGui::Text(u8" 목표 Dir %.2f\t%.2f\t%.2f", m_vDestCamDir.x, m_vDestCamDir.y, m_vDestCamDir.z);

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
	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();
}
