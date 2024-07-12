#pragma once

#include "Client_Defines.h"
#include "Character.h"
#include "Effect.h"
#include "BombOrbitGlow.h"
#include "BombOrbit.h"
#include "Light.h"

#define	INFO(state) m_tKirbyInfo.state

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CPartObject;
END

BEGIN(Client)

class CKirby :
    public CCharacter
{
public:

// Enum을 모아놓은 헤더파일이다.
#include "Kirby_Enums.h"

	// 다양한 STATE 에서 관리하여야 하는 구조체
	typedef struct INFODESC
	{
		_float			m_fMoveSpeed = { 0.f };
		_float			m_fZAngle = { 0.f };
		_float4			m_vMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vDodgeDir = { 0.f, 0.f, 0.f, 0.f };

		// 다양한 무기 움직임에 사용 될 것이다.
		_float4			m_vAttackDir = { 0.f, 0.f, 0.f, 0.f };
		_float			m_fAttackMoveSpeed = { 0.f };

		// 눈, 입, 몸체의 상태를 담당한다.
		EYESTATE		m_eEyeState = { EYE_END };
		MOUTHSTATE		m_eMouthState = { MOUTH_END };
		BODYSTATE		m_eBodyState = { BODY_END };
		DIR				m_eKirbyDir = { DIR_END };
		STATE			m_eJumpState = { STATE_JUMPR };

		// 먹은 놈을 소화시키기 전에 잠깐 저장해놓는 곳. (현재 입에 있는 놈)
		ABILITYTYPE		m_eTemporaryEatType = { ABILITY_END };

		// 중력 및 점프
		_float			m_fJumpVelocity = { 0.f };
		_float			m_fGravityOffset = { 6.f };
		_float			m_fJumpHoldTime = { 0.f };
		_float			m_fChangeVelocityZeroTime = { 0.f };
		_float			m_fHoldAirTime = { 0.f };

		// 사다리
		_bool			m_bCanLadder = { false };
		_float4			m_vLadderPoint = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vLadderLook = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vLadderOriginalPos = { 0.f, 0.f, 0.f, 0.f };
		// 다시 바로 사다리를 타는 행위를 막는다.
		_bool			m_bBlockLadder = { false };

		// 점프 중 재입력 방지
		_bool			m_bRePressBlock = { false };

		// 점프 예약 입력
		_bool			m_bReserveJumpKey = { false };

		// 먹은 상태에서의 FALL 애님은 없기 때문에 같은 애니메이션으로 구분하기 위헤 부울값 선언
		_bool			m_isEatFall = { false };
		// 현재 먹은 상황임을 표현한다.
		_bool			m_isEat = { false };

		// Vacuum
		_float			m_fVacuumTime = { 0.f };
		CPhysXObject*	m_pObject = { nullptr };
		_float3			m_vObjectScale = { 0.f, 0.f, 0.f };
		_float			m_fObjectDistance = { 0.f };

		// 방향 키 컨트롤러를 만지고 있는가?
		//_bool			m_isController = { false };

		// Fly
		_float			m_fFlyTime = { 0.f };

		// 땅에 능력 버리는 시간
		_float			m_fDumpAbilityTime = { 0.f };
		_bool			m_bDumpAbilityPress = { false };

		// Ability Sword
		// PRESS 시, 차지시간을 정해주는 변수
		_float			m_fChargeTime = { 0.f };
		// 어택 시, 다음 어택모션이 정해질 시간변수
		_float			m_fAttackTime = { 0.f };
		STATE			m_ePreAttackState = { SWORDSTATE_DECISIVESLASH };
		_bool			m_bWalkingCharge = { true };
		_bool			m_bUpWardSlash = { false };

		// Ability Bomb
		// 폭탄을 들고 있어야하는 상황인가?
		_bool			m_bBombHold = { true };
		// 포물선 점선을 랜더준비 할 상황인가?
		_bool			m_bBombOrbit = { false };
		// 폭탄의 목표 타겟 (커비 기준 방향벡터이다)
		_float4			m_vBombTargetDir = { 0.f, 0.f, 0.f, 0.f };
		// 폭탄의 목표 최종 타겟
		_float4			m_vBombTargetPos = { 0.f, 0.f, 0.f, 0.f };
		// 폭탄을 던지는 방향
		_float4			m_vBombThrowDir = { 0.f, 0.f, 0.f, 0.f };
		// 폭탄을 던지는 힘
		_float			m_fBombPower = { 0.f };
		// 조준 했을 때 들어오는 bool 값
		_bool			m_bBombAimming = { false };


		_bool			m_bBlockOtherVacuum = { false };
		// Deform 중일 때, true하여 Vacuuming 상태를 분기할 수 있게 만든다.
		_bool			m_bisDeforming = { false };
		// 자동차 전용 점퍼
		_bool			m_bCarJump = { false };
		// 자동차 부스터 타임
		_bool			m_bBooster = { false };
		_float			m_fBoosterTime = { 0.f };


		// 해머 폼
		_int			m_iHammerHit = { 0 };
		_float			m_fHammerChargeTime = { 0.f };

		// 전구 폼
		_bool			m_bLightOn = { false };
		_bool			m_bBulbJump = { false };
		class CLight*	m_pLight = { nullptr };

		// 크래쉬 폼
		_float			m_fTimeRatio = { 0.f };
		_int			m_iCrashTimeSlow = { 0 };
		_float			m_fCrashChargeTime = { 0.f };
	}KIRBY_INFODESC;


private:
	CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby(const CKirby& rhs);
	virtual ~CKirby() = default;

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

	KIRBY_INFODESC* Get_KirbyInfo() { return &m_tKirbyInfo; }
	void			Set_KirbyInfo(KIRBY_INFODESC _tInfo) {
		m_tKirbyInfo = _tInfo;
	}

	void			Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody, _uint iOffSet = 0);
	void			Set_Animation(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	void			Set_Animation(_int iAnimIndex);

	_uint			Get_Coin() const { return m_uCoin; }
	void			Set_Coin(_uint uCoin) { m_uCoin = uCoin; }
	void			Plus_Coin(_uint uCoin) { m_uCoin += uCoin;	}

	_bool			isAnimFinish();
	_float			Get_AnimTrackPosition();
	void			DefaultIdle();

	// 현재 커비가 무적상태인지 아닌지 판별하는 부울 값
	_bool			isOverPower() { return m_bOverPower; }
	// 손에 쥐고있어야 할 때, 필요한 행렬 포인터
	_float4x4*		Get_HandsMatrix()  { return &m_ArmourMatrix; }

	void			Set_HitStop(_float fHitStopMaxTime = 0.12f) { m_bHitStop = true; m_fHitStopMaxTime = fHitStopMaxTime; }
	_bool			Is_Attacking() { return m_isKirbyAttacking; }
	void			RegisterActorToPlayer(PxRigidActor* pActor, CGameObject* pGameObject) { 
		m_mapToppleableBridges.insert_or_assign(pActor, pGameObject);
		Safe_AddRef(pGameObject);
	}
	void			RegisterActorsToPlayer_ForStarBox(PxRigidActor* pActor, CGameObject* pGameObject) {
		m_mapStarBoxes.insert_or_assign(pActor, pGameObject);
		Safe_AddRef(pGameObject);
	}
	void			RegisterActorsToPlayer_ForBox(PxRigidActor* pActor, CGameObject* pGameObject) {
		m_mapBoxes.insert_or_assign(pActor, pGameObject);
		Safe_AddRef(pGameObject);
	}

	CGameObject*	FindToppleableBridge(PxRigidActor* pActor);
	CGameObject*	FindStarBox(PxRigidActor* pActor);
	CGameObject*	FindBox(PxRigidActor* pActor);
	void			Set_WeaponAnim(_uint index);
	_float4			Get_BulbLightPos();

	// 기타 세부적인 제어
private:
	// 커비의 움직임을 담은 구조체
	KIRBY_INFODESC  m_tKirbyInfo;
	void			Setting_KirbyBalance();
	void			Key_Input(_float fTimeDelta);
	void			Kirby_SystemTick(_float fTimeDelta);
	HRESULT			Kirby_SystemInitialize();
	void			Kirby_LookInitialize();
	void			Kirby_StateInitialize();

private:
	HRESULT			Make_TargetToCams();
	HRESULT			Add_Components();
	HRESULT			Add_PartObjects();
	HRESULT			Bind_ShaderResources();
	_bool			Kirby_FaceCustom(BODYSTATE _eBodyState, _uint _iMeshIndex);
	// FSM
	void			SetUp_FSM();
	// 구독서비스
	void			SetUp_Event();
	void			Event_Racing_Cut1(CGameObject* pObj);
	void			Event_Racing_Cut2(CGameObject* pObj);

	// 히트박스 체인저
	void			HitBoxChanger(_uint eState);
	_bool			m_isKirbyAttacking = { false };
	_float			m_fIsAttackTime = { 0.f };

	// 파크에서의 커비 행동 감지
	void			RayCast_Crumbles();

private:
	CModel*					m_pModelCom[BODY_END] = {nullptr};

	CTexture*				m_pEyeTexture[EYE_END] = { nullptr };
	CTexture*				m_pMouthTexture[MOUTH_END] = { nullptr };
	class CCamera*			m_pCamera = { nullptr };

private:
	void			Update_PartObjectMatrix();
	void			Bone_Rotation(_float fTimeDelta);

	class CKirbyWeapons*  m_pWeapons = { nullptr };
	class CKirbyArmours*  m_pArmours = { nullptr };

	_float4x4			  m_WeaponMatrix;
	_float4x4			  m_ArmourMatrix;

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

	// For Bulb
	_int				  m_iRenderCount = { 0 };
	_float4				  m_vBulbColor = { 0.f, 0.f, 0.f, 0.f };
	class CBulbFlare*	  m_pBulbFlare = { nullptr };

	// For Bomb
	vector<CBombOrbitGlow*> m_OrbitGlows;
	CBombOrbit* m_pOrbit = { nullptr };
	void Ready_BombOrbit();
	void Update_BombOrbit(_float fTimeDelta);
	_float4 Compute_Parabola(_float fOrbitTime, _float4 vStartPos, _float4 vEndPos);
	_bool				  m_bInitializeTargetPos = { true };
	_float				  m_fOrbitTime = { 0.f };
	// 폭탄 조준시간. 0.5초가 넘어가면 그제서야 진짜 찐 조준을 한것이다.
	_float				  m_fOrbitRenderDelay = { 0.f };

	_int				  m_iTestAnim = { 0 };

	// For Crash
	_float				  m_fCrashRestoreTime = { 0.f };

	unordered_map<PxRigidActor*, CGameObject*> m_mapToppleableBridges;
	unordered_map<PxRigidActor*, CGameObject*> m_mapStarBoxes;
	unordered_map<PxRigidActor*, CGameObject*> m_mapBoxes;
	void ReleaseAndClearMap(unordered_map<PxRigidActor*, CGameObject*> _map);

public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END