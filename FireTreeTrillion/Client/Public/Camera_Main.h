#pragma once	

#include "Client_Defines.h"
#include "Camera.h"

BEGIN(Client)

class CCamera_Main final : public CCamera
{
public:
	enum CAMSEQ { SEQ_ZOOMINOUT, SEQ_END };

	//카메라 시퀀스 구조체
	typedef struct
	{
		//easing 플래그
		EASING eEase = { EASE_LINEAR };
		//지속 시간
		_float fDuration = { .3f };
		//거리
		_float fDist = { -1.f };
		//볼 방향과 피치(x) 앵글
		_float3 fDir = { 0.f, 0.f, 0.f };
		_float fPitchAngle = { -1.f };

	}CAMACTION;


	//initialize desc
	typedef struct : public CCamera::CAMERA_DESC
	{
		_float fCamSensor = { 0.f };
		_float fOrigDistance = { 0.f };

	}CAMERA_KIRBY_DESC;

private:
	CCamera_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Main(const CCamera_Main& rhs);
	virtual ~CCamera_Main() = default;


//카메라 트리거 관련 함수
public:
	virtual void Set_Target(CTransform* pTarget, CAMFOCUS eFocus = FOCUS_FIRST) override
	{
		if (nullptr == pTarget)
			return;

		if (eFocus == FOCUS_FIRST)
		{
			if (nullptr != m_pFirstTarget)
				Safe_Release(m_pFirstTarget);

			m_pFirstTarget = pTarget;
			Safe_AddRef(pTarget);
			m_eCamFocus = eFocus;
		}
		else if(eFocus == FOCUS_SECOND)
		{
			if (nullptr != m_pSecondTarget)
				Safe_Release(m_pSecondTarget);

			m_pSecondTarget = pTarget;
			Safe_AddRef(pTarget);
			m_eCamFocus = eFocus;
		}
	}


	void Set_MatrixIndex(_int iMatrixIndex);
	void EmplaceBackCamMatrix(const _float4x4& matWorld);
	void EmplaceBackDirRadius(_int iCamType, _fvector vDir, _float fRadius);
	void LerpByTriggerInfo(_int iTriggerIndex);

	void EmplaceBackTriggerInfo(const _float4x4& matWorld, _float fScale);

	_float Compute_TriggerPosRatio(_int iTriggerIndex);

	void StartLerpByTriggerInfo(_int iTriggerIndex) { m_bLerpByTriggerInfo = true; m_iMatrixIndex = iTriggerIndex; }
	void EndLerpByTriggerInfo() { m_bLerpByTriggerInfo = false; };

	_vector SlerpDirVec(_fvector vStart, _fvector vEnd, _float fRatio);



//카메라 세팅(타겟, 기타 카메라 값) 관련 함수
public:
	void Set_CamFocus(CAMFOCUS eFocus) { m_eCamFocus = eFocus; }

	//FOV를 세팅한다.
	void Set_FOVY(_float fFOVYDegree) { m_fDestFovy = XMConvertToRadians(fFOVYDegree); }

	void Zoom(_float fZoom)	{ m_fZoomOffset = fZoom; }

	//카메라에게 특정 동작들을 시퀀스로 선예약한다.
	//인덱스 대신, enum으로 구별하게 하기
	void Make_Sequence(CAMSEQ eSeq);
	void Make_Shake(_float fPower = 1.f, _float fTime = .5f, _float2 vDir = _float2(0.f, -1.f));

	//카메라에게 동작을 수행시킨다.
	void Make_Sequence_FromAngle(EASING eEaseFlag, _float fDuration, _float3 fDestAngle, _float fDestZoom = -1.f);
	void Make_Sequence_FromDir(EASING eEaseFlag, _float fDuration, _float3 fDestDir, _float fDestZoom = -1.f);
	void Make_Sequence_FromQuat(EASING eEaseFlag, _float fDuration, _vector vDestQuat, _float fDestZoom = -1.f);




	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	//virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
#ifdef _DEBUG
	virtual void Render_IMGUI() override;
#endif
private:
/*카메라 트리거*/
	vector<_float4x4>	m_vecCamMatrices;
	_int				m_iMatrixIndex = { -1 };


	_float3 m_vCurKirbyTriggerLocalPos = { 0.f, 0.f, 0.f };
	vector<pair<_vector, _float>>	m_vecFrontDirRadius;
	vector<pair<_vector, _float>>	m_vecRearDirRadius;
	vector<pair<_float4x4, _float>>	m_vecTriggerInfo; // Trigger InverseMatrix and Scale

	_float m_fTriggerRatio = {};
	_bool m_bLerpByTriggerInfo = { false };
	_bool m_bPreLerpByTriggerInfo = { false };

	_vector m_vSlerpedDir = {};
	_float m_fLerpedRadius = {};


	_float			m_fDestUpOffset = { 0.f };
	_float			m_fCurUpOffset = { 0.f };
	vector<_float>	m_CamTriggerUpOffsets = { 0.f, 0.f, 0.f, .15f, .15f, 0.f, 0.f, 0.f };

/*타겟 트래킹*/

	//현재 트래킹 설정
	CAMFOCUS m_eCamFocus = { FOCUS_END };
	

	//카메라가 포커징할 기준점 
	_float3 m_vAnchor = { 0.f, 0.f, 0.f };
	//카메라의 실제 목표 위치
	_float3 m_vDestCamPos = { 0.f, 0.f, 0.f };


	//기준점으로부터 현재 거리
	_float m_fCurDistance = { 0.f };
	//기준 거리
	_float m_fOrigDistance = { 0.f };


	_float m_fZoomOffset = { 0.f };
	//_float m_fAbsoluteZoomOffset = { 0.f };

	//(시퀀스 시)시작, 목표 거리
	_float m_fStartDistance = { 0.f };
	_float m_fDestDistance = { 0.f };

	_float3 m_vCurCamDir = {0.f, 0.f, 0.f};
	_float3 m_vDestCamDir = { 0.f, 0.f, 0.f };
	_float3 m_vOrigCamDir = { 0.f, 0.f, 0.f };


	//카메라 움직임 감도
	_float	m_fCamSensor = { 0.f };
	//카메라 움직임 누적값
	_float2	m_fCamOrbitDelta = { 0.f, 0.f };


/*FOV 조작*/

	_float		m_fDestFovy = { 0.f };


/*카메라 쉐이킹*/

	//쉐이킹 트리거
	_bool m_bIsShaking = { false };
	//몇 번 흔들 것인가?
	_int m_iShakeCnt = { 0 };
	//얼마나 세게 흔들 것인가?
	_float m_fShakePower = { 0.f };
	//카메라 쉐이크 방향
	_float2 m_vShakeDir = { 0.f, 0.f };

	//카메라 움직임 관련 변수들
	_float m_fShakeAmplitude = { 5.f };
	_float m_fShakeFrequency = { 50.f };
	_float m_fShakeTime = { 0.f };


/*카메라 시퀀스*/

	//현재 시퀀스 모드
	CAMSEQ m_eSpecialSeq = { SEQ_END };
	//시퀀스 웨이팅 목록
	list<pair<_float, CAMACTION>> m_SeqList;
	//보간 시간
	pair<_float, _float> m_fSeqTime = { 0.f, 0.f };

private:
	void Control(_float fTimeDelta);
	void UpdatePos_FromAnchor(_float fTimeDelta);
	void Orbit_Target(_float fTimeDelta);


public:
	static CCamera_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END