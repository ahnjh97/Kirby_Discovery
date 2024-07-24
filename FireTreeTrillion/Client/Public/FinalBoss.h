#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPartObject;
END

#define DEBRISCNT 6

BEGIN(Client)

class CFinalBoss final : public CMonster
{
public:
	enum FINALBOSS_ANIM {
		FINALBOSS_APPEAR,					FINALBOSS_AWAYFAST,						FINALBOSS_AWAYFASTEND,			FINALBOSS_AWAYFASTENDAIR,	
		FINALBOSS_AWAYFASTREADY,			FINALBOSS_AWAYFASTSTART,				FINALBOSS_BALLWAIT,				FINALBOSS_BIGAPPEAR,				FINALBOSS_BIGCHAINLR, 
		FINALBOSS_BIGCHAINRL,				FINALBOSS_BIGROAR,						FINALBOSS_BIGSWINGFINISHLEFT,	FINALBOSS_BIGSWINGFINISHLEFTEND,
		FINALBOSS_BIGSWINGLEFT,				FINALBOSS_BIGSWINGLEFTEND,				FINALBOSS_BIGSWINGLEFTSTART,	FINALBOSS_BIGSWINGRIGHT,			FINALBOSS_BIGSWINGRIGHTEND, 
		FINALBOSS_BIGSWINGRIGHTSTART,		FINALBOSS_BIGWAITAIR,					FINALBOSS_CHAINLR,				FINALBOSS_CHAINRL,					FINALBOSS_DAMAGE, DAMAGEAIR, 
		FINALBOSS_DAMAGEFACESUB,			FINALBOSS_DEMOAPPEARCUT1,				FINALBOSS_DEMOAPPEARCUT2,		FINALBOSS_DEMOAPPEARCUT3,			FINALBOSS_DEMOAPPEARCUT4,
		FINALBOSS_DEMOAPPEARCUT5,			FINALBOSS_DEMOAPPEARSOULCUT2,			FINALBOSS_DEMOAPPEARSOULCUT3,	FINALBOSS_DEMODISAPPEARCUT2,		FINALBOSS_DEMODISAPPEARCUT3,
		FINALBOSS_DEMODISAPPEARCUT8,		FINALBOSS_DEMODISAPPEARCUT8WAIT,		FINALBOSS_DEMODISAPPEARCUT9,	FINALBOSS_DEMODISAPPEARCUT9WAIT,	FINALBOSS_DEMODISAPPEARSOULCUT1,
		FINALBOSS_DIMENSIONLASER,			FINALBOSS_DIMENSIONLASERCHARGE,			FINALBOSS_DIMENSIONLASEREND,	FINALBOSS_DIMENSIONLASEREADY,		FINALBOSS_DIMENSIONLASERSTART,
		FINALBOSS_DIMENSIONSPIKE,			FINALBOSS_DIMENSIONSPIKEEND,			FINALBOSS_DIMENSIONSPIKEREADY,	FINALBOSS_DIMENSIONSPIKEREADYWAIT,	FINALBOSS_DIMENSIONSPIKESTART,
		FINALBOSS_DIMENSIONSPIKEWAIT,		FINALBOSS_FALL,							FINALBOSS_FLASHTHRUST,			FINALBOSS_FLASHTHRUSTEND,			FINALBOSS_FLASHTHRUSTREADY,
		FINALBOSS_FLASHTHRUSTSTART,			FINALBOSS_FLASHTHRUSTSWINGFINISHLEFT,	FINALBOSS_JUMPEND,				FINALBOSS_JUMPREADY,				FINALBOSS_JUMPSTART,
		FINALBOSS_LASTDAMAGEAIRFALL,		FINALBOSS_LASTDAMAGEAIRSTART,			FINALBOSS_LASTDAMAGESTART,		FINALBOSS_LASTDAMAGEWAIT,			FINALBOSS_MOVEFAST,
		FINALBOSS_MOVEFASTEND,				FINALBOSS_MOVEFASTREADY,				FINALBOSS_MOVEFASTSTART,		FINALBOSS_MOVESLOW,					FINALBOSS_RAYARROWEND,
		FINALBOSS_RAYARROWENDAIR,			FINALBOSS_RAYARROWREADY,				FINALBOSS_RAYARROWREADYAIR,		FINALBOSS_RAYARROWSTART,			FINALBOSS_RAYARROWSTARTAIR,
		FINALBOSS_RAYARROWWAIT,				FINALBOSS_RAYARROWWAITAIR,				FINALBOSS_RECOVERYEND,			FINALBOSS_RECOVERYSTART,			FINALBOSS_RECOVERYWAIT,
		FINALBOSS_RESIST,					FINALBOSS_RESISTFALL,					FINALBOSS_RESISTLOOP,			FINALBOSS_RESISTSTART,				FINALBOSS_RESISTSUCCESS,
		FINALBOSS_ROAR,						FINALBOSS_SLASH,						FINALBOSS_SLASHCHAINREADY,		FINALBOSS_SLASHCHAINSTABREADY,		FINALBOSS_SLASHEND, 
		FINALBOSS_SLASHREADY,				FINALBOSS_SLASHSTART,					FINALBOSS_STAB,					FINALBOSS_STABEND,					FINALBOSS_STABREADY,
		FINALBOSS_STABSTART,				FINALBOSS_STABWAIT,						FINALBOSS_SUMMON,				FINALBOSS_SUMMONEND,				FINALBOSS_SUMMONSTART,
		FINALBOSS_SUMMONWAIT,				FINALBOSS_SWINGFINISHLEFT,				FINALBOSS_SWINGFINISHLEFTEND,	FINALBOSS_SWINGLEFT,				FINALBOSS_SWINGLEFTEND, 
		FINALBOSS_SWINGLEFTSTART,			FINALBOSS_SWINGRIGHT,					FINALBOSS_SWINGRIGHTEND,		FINALBOSS_SWINGRIGHTSTART,			FINALBOSS_TESTANIM01,
		FINALBOSS_TURNLEFTAIR,				FINALBOSS_TURNLEFTAIREND,				FINALBOSS_TURNLEFTAIRSTART,		FINALBOSS_TURNRIGHTAIR,				FINALBOSS_TURNRIGHTAIREND,
		FINALBOSS_TURNRIGHTAIRSTART,		FINALBOSS_WAIT,							FINALBOSS_WAITAIR,
		FINALBOSS_END
	};

	enum FINALBOSS_STATE { STATE_FLYING, STATE_GROUND, STATE_2PAZE, STATE_END };

	struct FINALBOSS_DESC : public CMonster::MONSTER_DESC {
		vector<_float4> vecRallyPoints;
	};

private:
	CFinalBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFinalBoss(const CFinalBoss& rhs);
	virtual ~CFinalBoss() = default;

public:
	void Set_Direction(_vector vDir) { m_vDir = vDir; }
	void Set_BossState(FINALBOSS_STATE eBossState) { m_eBossState = eBossState; }
	void Set_Glide(_bool bGlide) { m_bGlide = bGlide; }
	void Set_Chain(_bool bChain) { m_bChain = bChain; }
	void Set_TickPerSecond(_float TickPerSecond) { m_pModelCom->Set_TickPerSecond(TickPerSecond); }
	void Set_Position(_vector vPosition) { m_vPosition = vPosition; }
	void Set_Gully(_bool bGully) { m_bGully = bGully; }
	void Set_Particle(_bool bParticle) { m_bEffect = bParticle; }

	_vector Get_Direction() { return m_vDir; }
	_vector Get_Position() { return m_vPosition; }
	vector<_float4> Get_RallyPoint() { return m_vecRallyPoint; }
	FINALBOSS_STATE Get_BossState() { return m_eBossState; }
	_bool Get_Chain() { return m_bChain; }

	// 임구이용
	void Set_Stab(_bool bStab) { m_bStab = bStab; }
	_bool Get_Stab() { return m_bStab; }

	void Set_Slash(_bool bSlash) { m_bSlash = bSlash; }
	_bool Get_Slash() { return m_bSlash; }

	void Set_Meteor(_bool bMeteor) { m_bMeteor = bMeteor; }
	_bool Get_Meteor() { return m_bMeteor; }

	void Set_Laser(_bool bLaser) { m_bLaser = bLaser; }
	_bool Get_Laser() { return m_bLaser; }

	void Set_Side(_bool bSide) { m_bSide = bSide; }
	_bool Get_Side() { return m_bSide; }

	void Set_AirArrow(_bool bArrowAir) { m_bArrowAir = bArrowAir; }
	_bool Get_AirArrow() { return m_bArrowAir; }

	void Set_GroundArrow(_bool bArrowGround) { m_bArrowGround = bArrowGround; }
	_bool Get_GroundArrow() { return m_bArrowGround; }

	void Set_BackStep(_bool bBackStep) { m_bBackStep = bBackStep; }
	_bool Get_BackStep() { return m_bBackStep; }

	void Set_Swing(_bool bSwing) { m_bSwing = bSwing; }
	_bool Get_Swing() { return m_bSwing; }

	void Set_Thrust(_bool bThrust) { m_bThrust = bThrust; }
	_bool Get_Thrust() { return m_bThrust; }

	void Set_Spike(_bool bSpike) { m_bSpike = bSpike; }
	_bool Get_Spike() { return m_bSpike; }

	void Set_BeforeHp(_float fBeforeHp) { m_fBeforeHp = fBeforeHp; }

	void Set_Auto(_float bAuto) { m_bAuto = bAuto; }
	_bool Get_Auto() { return m_bAuto; }

	void Set_BeforePos(_vector vBeforePos) { m_vBeforePos = vBeforePos; }
	_vector Get_BeforePos() { return m_vBeforePos; }

	//이벤트 함수
	void Appear_Event(CGameObject* pObj);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Add_AnimEvent() override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void Change_State(FINALBOSS_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	map<const wstring, CPartObject*>	m_PartObjects = {};
	vector<class CGully*>				m_vecGully = {};
	vector<class CDebris*>				m_vecDebris = {};
	vector<class CDebris*>				m_vecMagneticDebris = {};

	FINALBOSS_ANIM		m_eCurrentState = { FINALBOSS_END };
	FINALBOSS_STATE		m_eBossState = { STATE_END };

	vector<_float4>		m_vecRallyPoint;

	_vector				m_vDir = {};
	_vector				m_vPosition = {};
	_vector				m_vBeforePos = { 0.f };

	_bool				m_bGlide = { false };
	_bool				m_bChain = { false };
	_bool				m_bGully = { false };
	_bool				m_bShake = { false };
	_bool				m_bEffect = { false };
	_bool				m_bAuto = { false };
	_bool				m_bLastDamage = { false };

	// 임구이용
	_bool				m_bStab = { false };
	_bool				m_bSlash = { false };
	_bool				m_bMeteor = { false };
	_bool				m_bLaser = { false };
	_bool				m_bSide = { false };
	_bool				m_bArrowAir = { false };
	_bool				m_bArrowGround = { false };
	_bool				m_bBackStep = { false };
	_bool				m_bSwing = { false };
	_bool				m_bThrust = { false };
	_bool				m_bSpike = { false };

	//컷신 트리거
	_bool				m_bStartOpeningTrigger = { true };
	//2페이즈 트리거
	_bool				m_bStart2PhaseTrigger = { true };

	_float				m_fGlideTime = { 0.f };
	//_float				m_fGullyTime = { 0.f };
	_float				m_fTimeDelay = { 0.f };
	_float				m_fBeforeHp = { 0.f };

	_uint				m_iGullyCnt = { 0 };
	_uint				m_iDebrisCnt = { 0 };
	_uint				m_iDebrsiMaxCnt = { 0 };

private:
	HRESULT	Make_TargetToCams();

	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();
	void HitBoxChanger(_uint eState);
	void Create_Gully();

public:
	static CFinalBoss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END