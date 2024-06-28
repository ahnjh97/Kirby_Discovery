#pragma once	

#include "Client_Defines.h"
#include "Camera.h"

BEGIN(Client)

class CCamera_Main final : public CCamera
{
public:
	//정의한 카메라 시퀀스들.
	enum CAMSEQ
	{
		SEQ_ZOOMINOUT,
		SEQ_HARDCUT_TEST,
		SEQ_SOFTCUT_TEST,
		SEQ_BREAKCARSHOP,
		SEQ_END
	};

	enum CAMCUT { CUT_HARD, CUT_INTERPOLATE, CUT_END };

	enum CAMPOS { POS_ABSOLUTE, POS_RELATIVE, POS_END };
	enum CAMDIR	{ DIR_ABSOLUTE, DIR_RELATIVE, DIR_END };

	//카메라 시퀀스 구조체
	typedef struct
	{

		//시퀀스 시작부터 어느 시간에 재생할지?
		_float	fTime = { 0.f };

		//시퀀스를 어떻게 컷 할것인가? (하드 컷 / 보간)
		CAMCUT	eCamCut = { CUT_HARD };

		//하드 컷 모드라면, 필요없는 변수
		//easing 플래그. 보간 그래프를 정의.
		EASING eEase = { EASE_END };
		//보간 속도.
		_float fInterpolateSpeed = { -1.f };

		//카메라 위치가 (커비)타겟 기준인가, 월드 기준인가?
		CAMPOS eCamPos = { POS_END };

		//카메라 방향이 (커비)타겟 기준인가, 월드 기준인가?
		CAMDIR eCamDir = { DIR_END };

		//볼 방향과 위치. -1 이라면, 기존 값을 유지.
		_float3 vPos = { -1.f, -1.f, -1.f };
		//Dir이 -1이라면 타겟을 보도록 한다.
		_float3 vDir = { -1.f, -1.f, -1.f };

		//fov y 값. degree로 기입. -1 이라면, 기존 값을 유지.
		_float fFOVY = { -1.f };

		//z 앵글. -1이라면 기존 값을 유지.
		_float fZAngle = { -1.f };

		_float fZoomOffset = { -1.f };

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
	//이벤트를 받기 위해 만든 함수입니다. 인자는 CGameObject* 로 한정
	void EventFunc(CGameObject* pObj);


	virtual void Set_Target(CTransform* pTarget, CAMTARGET eTarget, CAMFOCUS eFocus, _float3 vAnchorOffset = _float3{ 0.f, 0.f, 0.f }, _float fInterpolateSpeed = -1.f) override;

	

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

	void Zoom(_float fZoom)	{ m_fCurZoomOffset = fZoom; }

	//카메라에게 특정 동작들을 시퀀스로 선예약한다.
	//인덱스 대신, enum으로 구별하게 하기
	
	void Make_Sequence(CAMSEQ eSeq);
	void Make_Shake(_float fPower = 1.f, _float fTime = .5f, _float2 vDir = _float2(0.f, -1.f));

	//카메라에게 동작을 수행시킨다.
	void Make_Sequence_FromAngle(EASING eEaseFlag, _float fDuration, _float3 fDestAngle, _float fDestZoom = -1.f);
	void Make_Sequence_FromDir(EASING eEaseFlag, _float fDuration, _float3 fDestDir, _float fDestZoom = -1.f);
	void Make_Sequence_FromQuat(EASING eEaseFlag, _float fDuration, _vector vDestQuat, _float fDestZoom = -1.f);

	//카메라의 이벤트 함수들
	void Start_ShutterSeq(CGameObject* pNotifier);


	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	//virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override {}
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
	//카메라의 시작 위치.(시퀀스 용)
	_float3 m_vStartCamPos = { 0.f, 0.f, 0.f };
	_float3 m_vCurCamPos = { 0.f, 0.f, 0.f };

	//기준점으로부터 현재 거리
	_float m_fCurDistance = { 0.f };
	//기준 거리
	_float m_fOrigDistance = { 0.f };


	_float m_fCurZoomOffset = { 0.f };
	_float m_fStartZoomOffset = { 0.f };
	_float m_fDestZoomOffset = { 0.f };

	//(시퀀스 시)시작, 목표 거리
	_float m_fStartDistance = { 0.f };
	_float m_fDestDistance = { 0.f };

	_float3 m_vOrigCamDir = { 0.f, 0.f, 0.f };
	_float3 m_vCurCamDir = {0.f, 0.f, 0.f};

	//(시퀀스) 카메라 방향이 타겟을 향하지 않는가?
	_bool	m_bSeqDestDirIsAbsolute = { false };

	//시작 dir과 목표 dir. 시퀀스 용과 상시 보간 용 둘 다 사용된다.
	_float3 m_vDestCamDir = { 0.f, 0.f, 0.f };
	_float3 m_vStartCamDir = { 0.f, 0.f, 0.f };

	_float m_fCurZAngle = { 0.f };
	_float m_fStartZAngle = { 0.f };
	_float m_fDestZAngle = { 0.f };

	//카메라 움직임 감도
	_float	m_fCamSensor = { 0.f };
	//카메라 움직임 누적값
	_float2	m_fCamOrbitDelta = { 0.f, 0.f };


/*FOV 조작*/

	_float		m_fDestFovy = { 0.f };
	//(시퀀스) 시작 fov 저장
	_float		m_fStartFovy = { 0.f };

/*카메라 쉐이킹*/

	//쉐이킹 트리거
	_bool m_bIsShaking = { false };
	//몇 번 흔들 것인가?
	_int m_iShakeCnt = { 0 };
	//얼마나 세게 흔들 것인가?
	_float m_fShakePower = { 0.f };

	//카메라 쉐이크 방향
	_float2 m_vShakeDir = { 0.f, 0.f };
	_float2 m_vPreShakeDir = { 0.f, 0.f };

	//카메라 움직임 관련 변수들
	_float m_fShakeAmplitude = { .5f };
	_float m_fShakeFrequency = { 20.f };
	_float m_fInitialShakeTime = { 0.f };
	_float m_fCurShakeTime = { 0.f };


/*카메라 시퀀스*/

	//현재 시퀀스 모드
	CAMSEQ m_eSpecialSeq = { SEQ_END };

	//시퀀스 웨이팅 목록
	list<CAMACTION> m_CamSeq;

	CAMCUT m_eCamCut = { CUT_END };

	//보간 시간
	EASING m_eCurSeqEase = { EASE_END };
	pair<_float, _float> m_fSeqInterpolateTime = { 0.f, 0.f };

private:
	void Reset_DeferredCamSet();

	void Play_Sequence(_float fTimeDelta);
	void Control(_float fTimeDelta);


	void Subscribe_Events();

	void UpdatePos_FromAnchor(_float fTimeDelta);

	//포커징 기준점을 업데이트한다.
	void Update_Anchor(_float fTimeDelta);

	void Interpolate_CamSet(_float fTimeDelta);
	void Update_CurCamPos(_float fTimeDelta);

	_float3 Make_ShakeDir(_float fTimeDelta);
	void MoveTo_CurCamPos_Interpolate(_float fTimeDelta);
	void MoveTo_CurCamPos_Absolute(_float fTimeDelta);

	//void Orbit_Target(_float fTimeDelta);


public:
	static CCamera_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END