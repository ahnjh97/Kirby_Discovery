#pragma once
#include "Client_Defines.h"
#include "Character.h"
#include "Effect.h"
#define	INFO(state) m_tKirbyInfo.state

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CPartObject;
END

BEGIN(Client)

class CFinaleKirby : public CCharacter
{
public:
	enum DIR { DIR_LEFT, DIR_RIGHT, DIR_FRONT, DIR_BACK, DIR_LF, DIR_RF, DIR_LB, DIR_RB, DIR_END };
	enum STATE {
		STATE_INHALESTART = 123,
		STATE_INHALE = 117,
		STATE_SUPERINHALESTART = 225,
		STATE_VACUUM = 252,

		STATE_IDLE = 256,
		STATE_RUN = 172,

		// 덤프 폼
		DUMPTSTATE_CUT = 550, DUMPSTATE_DAMAGE, DUMPSTATE_IDLING, DUMPSTATE_JUMP, DUMPSTATE_LANDING, DUMPSTATE_WAIT,

		// 덤프 흡수 폼
		DUMPSTATE_CUTDEMOKIRBY = 600,

		// 덤프 컷씬
		DUMPCUTSTATE_CUT1 = 650, DUMPCUTSTATE_CUT10, DUMPCUTSTATE_CUT11, DUMPCUTSTATE_CUT12, DUMPCUTSTATE_CUT13,
		DUMPCUTSTATE_CUT14, DUMPCUTSTATE_CUT15, DUMPCUTSTATE_CUT16, DUMPCUTSTATE_CUT17, DUMPCUTSTATE_CUT18,
		DUMPCUTSTATE_CUT19, 
		DUMPCUTSTATE_CUT2, DUMPCUTSTATE_CUT20, DUMPCUTSTATE_CUT21, DUMPCUTSTATE_CUT22, DUMPCUTSTATE_CUT23,
		DUMPCUTSTATE_CUT24, DUMPCUTSTATE_CUT25, DUMPCUTSTATE_CUT6, DUMPCUTSTATE_CUT7, DUMPCUTSTATE_CUT8, DUMPCUTSTATE_CUT9,
		DUMPCUTSTATE_MISS,

		STATE_END = 5000
	};
	enum EYESTATE { EYE_IDLE, EYE_ANGER, EYE_CLOSE, EYE_SADNESS, EYE_PUPIL, EYE_BLINK, EYE_DOUBT, EYE_END };
	enum MOUTHSTATE { MOUTH_IDLE, MOUTH_ANGER, MOUTH_HAPPY, MOUTH_SMILE, MOUTH_SURPRISE, MOUTH_END };
	enum BODYSTATE {
		BODY_DEFAULT, BODY_VACUUM,
		BODY_DUMPDEFAULT, BODY_DUMPVACUUM, BODY_DUMPCUT,
		BODY_END
	};
	enum KIRBYOFFSET {
		OFFSET_DUMP = 550, OFFSET_DUMPVACUUM = 600, OFFSET_DUMPCUT = 650,
		OFFSET_END
	};

	// 다양한 STATE 에서 관리하여야 하는 구조체
	typedef struct INFODESC
	{
		_float			m_fMoveSpeed = { 0.f };
		_float			m_fZAngle = { 0.f };
		_float4			m_vMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vHandleDir = { 0.f, 0.f, 0.f, 0.f };

		// 눈, 입, 몸체의 상태를 담당한다.
		EYESTATE		m_eEyeState = { EYE_END };
		MOUTHSTATE		m_eMouthState = { MOUTH_END };
		BODYSTATE		m_eBodyState = { BODY_END };
		DIR				m_eKirbyDir = { DIR_END };

		// 중력 및 점프
		_float			m_fJumpVelocity = { 0.f };
		_float			m_fGravityOffset = { 6.f };
		// 자동차 전용 점퍼
		_bool			m_bCarJump = { false };
		// 자동차 부스터 타임
		_bool			m_bBooster = { false };
		_float			m_fBoosterTime = { 0.f };

		// 트럭이 전부 빨렸을 때, 이 불값은 트루가된다.
		_bool			m_bVacuumComplete = { false };
	}FINALEKIRBY_INFODESC;

private:
	CFinaleKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinaleKirby(const CFinaleKirby& rhs);
	virtual ~CFinaleKirby() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual HRESULT Render_LightDepth()							override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif
	virtual HRESULT	Render_DeferredInfo()						override;

	virtual void	Add_AnimEvent()								override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;


	FINALEKIRBY_INFODESC*	Get_KirbyInfo() { return &m_tKirbyInfo; }
	void					Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody, _uint iOffSet = 0);

	_uint					Get_Coin() const { return m_uCoin; }
	void					Set_Coin(_uint uCoin) { m_uCoin = uCoin; }
	void					Plus_Coin(_uint uCoin) { m_uCoin += uCoin; }

	_bool					isAnimFinish();
	_float					Get_AnimTrackPosition();
	void					Bone_Rotation(_float fTimeDelta);

	void					Set_HitStop(_float fHitStopMaxTime = 0.12f) { 
		m_bHitStop = true; m_fHitStopMaxTime = fHitStopMaxTime; }

	void					Start_CutScene();
	_float4					m_vBonePos = { 0.f, 0.f, 0.f, 0.f };
	_float4					Compute_RootPos();

private:
	// 커비의 움직임을 담은 구조체
	FINALEKIRBY_INFODESC	m_tKirbyInfo;
	void			Setting_KirbyBalance();
	void			Kirby_SystemTick(_float fTimeDelta);
	HRESULT			Kirby_SystemInitialize();
	void			Kirby_LookInitialize();
	void			Kirby_StateInitialize();

private:
	HRESULT			Make_TargetToCams();
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();
	_bool			Kirby_FaceCustom(BODYSTATE _eBodyState, _uint _iMeshIndex);
	// FSM
	void			SetUp_FSM();
	// 구독서비스
	void			SetUp_Event();

private:
	CModel* m_pModelCom[BODY_END] = { nullptr };
	CTexture* m_pEyeTexture[EYE_END] = { nullptr };
	CTexture* m_pMouthTexture[MOUTH_END] = { nullptr };
	class CCamera* m_pCamera = { nullptr };

	void				  OverPower();
	_bool				  m_bOverPower = { false };
	_float				  m_fOverPowerColor = { 0.f };
	_float				  m_fOverPowerTime = { 0.f };
	_float				  m_fFlashOverPowerTime = { 0.f };
	_float				  m_fPreHp = { 0.f };
	_uint				  m_uCoin = { 0 };

	void				  HitStop_System(_float fTimeDelta);
	_bool				  m_bHitStop = { false };
	_float				  m_fHitStopTime = { 0.f };
	_float				  m_fHitStopMaxTime = { 0.f };

	_bool				  m_bMakeDisaster = { true };

public:
	static CFinaleKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END