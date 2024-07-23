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
		SEQ_BREAKRACINGMAP,
		SEQ_PARTTIMESTART,
		SEQ_FINALESTART,
		SEQ_LUNCHTIME,

		SEQ_FINALECUT1,
		SEQ_FINALECUT2,
		SEQ_FINALECUT3,
		SEQ_FINALECUT4,
		SEQ_FINALECUT5,
		SEQ_FINALECUT6,
		SEQ_FINALECUT7,
		SEQ_FINALECUT8,
		SEQ_FINALECUT9,
		SEQ_FINALECUT10,
		SEQ_FINALECUT11,
		SEQ_FINALECUT12,
		SEQ_FINALECUT13,
		SEQ_FINALECUT14,
		SEQ_FINALECUT15,
		SEQ_FINALECUT16,
		SEQ_FINALECUT17,
		SEQ_FINALECUT18,
		SEQ_FINALECUT19,
		SEQ_FINALECUT20,

		SEQ_SIMBA_START,

		//심바
		SEQ_SIMBA_SHOULDER,
		SEQ_SIMBA_TONG,
		SEQ_SIMBA_FRONTVIEW,
		SEQ_SIMBA_LOW,

		SEQ_SIMBA_BATTLESTART,

		SEQ_SIMBA_GLASSBREAK,
		SEQ_SIMBA_BOSSORIGIN,
		SEQ_SIMBA_CAGEBREAK,

		//파이널보스
		SEQ_FINALBOSS_APPEAR,

		//단독으로 하나를 즉시 넣어 사용하는 시퀀스
		SEQ_ONE,
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


//카메라 세팅(타겟, 기타 카메라 값) 관련 함수
public:
	virtual void Set_Target(CTransform* pTarget, CAMTARGET eTarget, CAMFOCUS eFocus, _float3 vAnchorOffset = _float3{ 0.f, 0.f, 0.f }, _float fInterpolateSpeed = -1.f) override;

	void Set_TargetAnchor(_float3 vAnchorOffset) { m_vAnchorOffset = vAnchorOffset; }

	void Set_CamFocus(CAMFOCUS eFocus)
	{
		m_eCamFocus = eFocus;
	}

	//FOV를 세팅한다.
	void Set_FOVY(_float fFOVYDegree) { m_fDestFovy = XMConvertToRadians(fFOVYDegree); }

	//카메라에 마지막으로 보정할 쌩 오프셋.
	void Set_FinalOffset(_float3 vOffset, _float fInterpolateSpeed = -1.f)
	{
		m_fZAngleInterpolateSpeed = (fInterpolateSpeed != -1.f) ?
			3.f : fInterpolateSpeed;

		m_vDestFinalOffset = vOffset;
	}

	//z 각도와, z 각도 보간 속도를 설정한다.
	void Set_ZAngle(_float fZAngle, _float fInterpolateSpeed = -1.f)
	{
		m_fZAngleInterpolateSpeed = (fInterpolateSpeed != -1.f) ?
									3.f : fInterpolateSpeed;

		m_fDestZAngle = fZAngle;
	}

	//줌 수치를 설정한다.
	void Zoom(_float fZoom)	{ m_fCurZoomOffset = fZoom; }

	//(두 타겟팅일 경우) 기준점 세팅 비율을 설정한다.
	void Set_BothFocusRatio(_float fRatio)
	{
		m_fBothFocusRatio = SATURATE(fRatio);
	}

	//카메라 쉐이크 주기
	virtual void Make_Shake(_float fPower = 1.f, _float fTime = .5f, _float2 vDir = _float2(0.f, -1.f));

	//카메라에게 특정 동작들을 시퀀스로 선예약한다.
	void Make_Sequence(CAMSEQ eSeq);

	void Set_DOFMode(_bool bAuto) { m_bAutoDOF = bAuto; }

	_bool IsSequencePlaying() { return m_eSpecialSeq == SEQ_END; }

	//카메라에게 동작을 수행시킨다.
	void Make_One_Sequence(CAMACTION newAction);

	//카메라의 이벤트 함수들
	void Start_ShutterSeq(CGameObject* pNotifier);
	void Start_BridgeSeq(CGameObject* pNotifier);

	void Ready_Cam_DeeDeeDee(CGameObject* pNotifier);

	void Ready_Monsters_Leongar(CGameObject* pNotifier);

	void Ready_Dialog1_Leongar(CGameObject* pNotifier);
	void Ready_Dialog2_Leongar(CGameObject* pNotifier);
	void Ready_Dialog3_Leongar(CGameObject* pNotifier);

	void Ready_Cam_GlassBreak(CGameObject* pNotifier);
	void Ready_Cam_BossOrigin(CGameObject* pNotifier);
	void Ready_Cam_CageBreak(CGameObject* pNotifier);

	void Ready_Cam_FinalBoss(CGameObject* pNotifier);

	//이벤트를 받기 위해 만든 테스트 함수입니다. 인자는 CGameObject* 로 한정
	void EventFunc(CGameObject* pObj);


	virtual void Lock_Position(_float3 vPos = { -1.f, -1.f, -1.f }, _bool bInterpolate = false) override;
	virtual void Lock_Direction(_float3 vLook = { -1.f, -1.f, -1.f }, _bool bInterpolate = false) override;
	virtual void Lock_All(_float3 vPos = { -1.f, -1.f, -1.f }, _float3 vLook = { -1.f, -1.f, -1.f }, _bool bInterpolate = false) override;


	//카메라 목표 수치 계산
	void Compute_Set_BothFocus(_float fTimeDelta);
	void Compute_Set_BattleFocus(_float fTimeDelta);
	void Compute_Set_FinalBossFocus(_float fTimeDelta);
	void Compute_Set_CamLock(_float fTimeDelta);
	void Compute_Set_Trigger(_int iTriggerIndex);


//카메라 트리거 관련 함수
public:
	void Set_MatrixIndex(_int iMatrixIndex);
	void EmplaceBackCamMatrix(const _float4x4& matWorld);
	void EmplaceBackDirRadius(_int iCamType, _fvector vDir, _float fRadius);

	void EmplaceBackTriggerInfo(const _float4x4& matWorld, _float fScale);

	_float Compute_TriggerPosRatio(_int iTriggerIndex);

	void StartLerpByTriggerInfo(_int iTriggerIndex) { m_bLerpByTriggerInfo = true; m_iMatrixIndex = iTriggerIndex; }
	void EndLerpByTriggerInfo() { m_bLerpByTriggerInfo = false; };

	_vector SlerpDirVec(_fvector vStart, _fvector vEnd, _float fRatio);

	// 레벨 내에서 커비 이동제어
	void	Move_ForTrigger(_float fTimeDelta, _float3 vPos = _float3(), _float3 vDir = _float3());
	//void	_ForTrigger(_float fTimeDelta, _float3 vPos = _float3(), _float3 vDir = _float3());


	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
#ifdef _DEBUG
	virtual void Render_IMGUI() override;
	void Render_GraphicIMGUI(_float4x4 worldMat);
#endif



private:

	//for Finale
	_int				m_iPreSceneIdx =	{ 0 };
	_int				m_iCurSceneIdx =	{ 0 };
	_bool				m_bChangedScene = { false };

	_int				m_iSceneCnt =		{ -1 };


/*카메라 트리거*/
	vector<_float4x4>	m_vecCamMatrices;
	_int				m_iMatrixIndex = { -1 };


	list<_float>		m_FinaleSeqATime;
	list<_float>		m_FinaleSeqBTime;
	list<_float>		m_FinaleSeqCTime;
	list<_float>		m_FinaleSeqDTime;


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
	vector<vector<_float>>	m_CamTriggerUpOffsets;

/*타겟 트래킹*/

	//현재 트래킹 설정
	CAMFOCUS m_eCamFocus = { FOCUS_END };
	
	_bool	m_bAutoDOF = { true };

	//카메라가 포커징할 기준점 
	_float3 m_vAnchor = { 0.f, 0.f, 0.f };

	//두 타겟을 포커징할 때, 어디 부분으로 가중치 줄 것이냐?
	_float m_fBothFocusRatio = { .5f };

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


#pragma region 후보정

/*마지막 쌩 이동값 보정*/
	_float3	m_vPreFinalOffset = { 0.f, 0.f, 0.f };
	_float3	m_vCurFinalOffset = { 0.f, 0.f, 0.f };
	_float3	m_vDestFinalOffset = { 0.f, 0.f, 0.f };

	_float m_fFinalOffsetInterpolateSpeed = { 3.f };


/*Z 앵글*/
	//이전 프레임의 z 값을 저장.
	_float m_fPreZAngle = { 0.f };

	_float m_fCurZAngle = { 0.f };
	_float m_fStartZAngle = { 0.f };
	_float m_fDestZAngle = { 0.f };

	_float m_fZAngleInterpolateSpeed = { 3.f };

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
	_float m_fShakeAmplitude = { .3f };
	_float m_fShakeFrequency = { 60.f };
	_float m_fInitialShakeTime = { 0.f };
	_float m_fCurShakeTime = { 0.f };

#pragma endregion

/*카메라 시퀀스*/

	//현재 시퀀스 모드
	CAMSEQ m_eSpecialSeq = { SEQ_END };
	_float m_fSeqEventTime = { 0.f };
	vector<_bool> m_SeqEventTriggers;

	//시퀀스 웨이팅 목록
	list<CAMACTION> m_CamSeq;

	CAMCUT m_eCamCut = { CUT_END };

	//보간 시간
	EASING m_eCurSeqEase = { EASE_END };
	pair<_float, _float> m_fSeqInterpolateTime = { 0.f, 0.f };

	//이펙트 소켓
	_float4x4		m_EffectSocket;

	_float			m_fSeqCheckTime = { 0.f };
	_float			m_fSeqPlayedTime = { 0.f };
	_float			m_fSeqTotalTime = { 0.f };

private:
	void Reset_DeferredCamSet();
	void Set_DeferredCamSet(_float fTimeDelta);

	void Play_Sequence(_float fTimeDelta);
	void Control(_float fTimeDelta);


	void Subscribe_Events();

	void Track_Anchor(_float fTimeDelta);

	//포커징 기준점을 업데이트한다.
	void Update_Anchor(_float fTimeDelta);

	//타겟 위치를 받는다
	_float3 Make_TargetPos();

	void Interpolate_CamSet(_float fTimeDelta);
	void Update_CurCamPos(_float fTimeDelta);

	_float3 Make_ShakeDir(_float fTimeDelta);
	void MoveTo_CurCamPos_Interpolate(_float fTimeDelta);
	void MoveTo_CurCamPos_Absolute(_float fTimeDelta);
	void Snap_CamSet(_float fTimeDelta);

	void System_Tick(_float fTimeDelta);
	void Check_FinaleScene(_float fTimeDelta);
	void Check_FinaleTime(_float fTimeDelta);
	void Deferred_Blackoperation(CAMSEQ eSEQ);

	void Fill_HardCutSet(CAMACTION& Action, _float fTime);
	void Fill_InterpolateCutSet(CAMACTION& Action, _float fTime, EASING eEase, _float fInterpolateSpeed);

	void Fill_ActionPos(CAMACTION& Action, CAMPOS eCamPos, _float3 vPos);
	void Fill_ActionDir(CAMACTION& Action, CAMDIR eCamDir, _float3 vDir);

	// [임시] 레이싱맵에서 level전환하는 함수 written by JYWI
	void Change_LevelTrigger();

#ifdef _DEBUG
	_float		m_fFovyTemp = _float();
#endif
public:
	static CCamera_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END