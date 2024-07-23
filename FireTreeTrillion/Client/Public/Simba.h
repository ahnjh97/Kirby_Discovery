#pragma once
#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CSimba final : public CMonster
{
public:
	enum SIMBA_ANIM {
		Simba_AttackJump, Simba_AttackJumpEnd, Simba_AttackJumpHit, Simba_AttackJumpPre, Simba_AttackJumpStart, Simba_AttackJumpWait,
		Simba_BackStep, Simba_BackStepEnd, Simba_BackStepLanding, Simba_BackStepStart,
		Simba_BiteRush, Simba_BiteRushEnd, 
		Simba_BiteRushFallL, Simba_BiteRushFallR,
		Simba_BiteRushJumpL, Simba_BiteRushJumpR, Simba_BiteRushJumpStartL, Simba_BiteRushJumpStartR,
		Simba_BiteRushLandingL, Simba_BiteRushLandingR,
		Simba_BiteRushStart, Simba_BiteRushStartStraight,
		Simba_BiteRushTiredAttackStart, Simba_BiteRushTiredAttackStartStraight,
		Simba_BiteRushTiredEnd, Simba_BiteRushTiredStart, Simba_BiteRushTiredWait,
		Simba_Command,
		Simba_Damage, Simba_DamageFaceSub,
		Simba_Death,
		Simba_DemoAppear1Cut10, Simba_DemoAppear1Cut10Wait,
		Simba_DemoAppear1Cut2, Simba_DemoAppear1Cut2Wait,
		Simba_DemoAppear1Cut3, Simba_DemoAppear1Cut3Wait,
		Simba_DemoAppear1Cut4, Simba_DemoAppear1Cut4Wait,
		Simba_DemoAppear1Cut6, Simba_DemoAppear1Cut6Wait,
		Simba_DemoAppear1Cut9, Simba_DemoAppear1Cut9Wait,
		Simba_DemoAppear2Cut1, Simba_DemoAppear2Cut2,
		Simba_DemoDeadCut1, Simba_DemoDeadCut2,
		Simba_DemoExAppearCut1, Simba_DemoExAppearCut2, Simba_DemoExAppearCut3, Simba_DemoExAppearCut4, Simba_DemoExAppearCut5,
		Simba_DemoExAppearCut6, Simba_DemoExAppearCut6Multi, Simba_DemoExAppearCut7, Simba_DemoExAppearCutPreWait,
		Simba_DemoMorphoKnightAppearCut1, Simba_DemoMorphoKnightAppearCut2, Simba_DemoMorphoKnightAppearCut3, Simba_DemoMorphoKnightAppearCut4, Simba_DemoMorphoKnightAppearCut5,
		Simba_DemoSoulAppearCut1, 
		Simba_DimensionClaw, Simba_DimensionClawContinue, Simba_DimensionClawEnd, Simba_DimensionClawStart, Simba_DimensionClawStartContinue, Simba_DimensionClawWait,
		Simba_DimensionLaser, Simba_DimensionLaserEnd, Simba_DimensionLaserStart, Simba_DimensionLaserWait,
		Simba_DoubleClaw, Simba_DoubleClawChargeStart, Simba_DoubleClawChargeWait, Simba_DoubleClawDash, Simba_DoubleClawDashStart, Simba_DoubleClawEnd,
		Simba_Fall,
		Simba_FinalCrusher, Simba_FinalCrusherEnd, Simba_FinalCrusherRepeat,
		Simba_FinalCrusherStart, Simba_FinalCrusherStartContinue, Simba_FinalCrusherStartRepeat, Simba_FinalCrusherStartRepeatEnd,
		Simba_Jump, Simba_JumpStart,
		Simba_Kneel, Simba_KneelEnd, Simba_KneelStart,
		Simba_Landing,
		Simba_LipSyncSub, Simba_LipSyncSubA,
		Simba_MouthSub,
		Simba_QuickClaw2L, Simba_QuickClaw2R, Simba_QuickClawAgainL, Simba_QuickClawAgainR, Simba_QuickClawChargeL, Simba_QuickClawChargeR,
		Simba_QuickClawEndL, Simba_QuickClawEndR, Simba_QuickClawL, Simba_QuickClawLFromStart, Simba_QuickClawR,
		Simba_QuickClawRepeatL, Simba_QuickClawRepeatR, Simba_QuickClawRFromStart, Simba_QuickClawStartL, Simba_QuickClawStartR,
		Simba_Roar, Simba_Roar2, Simba_Roar2Long, Simba_Roar2Soul,
		Simba_Sit,
		Simba_Wait1, Simba_Wait2,
		Simba_Walk,
		SIMBA_END
	};

	enum SIMBA_EYETEX { EYETEX_DIFFUSE, EYETEX_NORMAL, EYETEX_MRA, EYETEX_END };
	enum SIMBA_EYESTATE { SIMBAEYE_LONG, SIMBAEYE_SMALL, SIMBAEYE_BIG, SIMBAEYE_END };
	enum SIMBA_ROTATION { ATTACKJUMP, BITERUSH, BITERUSHJUMP, ROTATION_END };
	enum SIMBA_FINGER { INDEX, MIDDLE, PINKY, RING, THUMB, FINGER_END };

private:
	CSimba(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSimba(const CSimba& rhs);
	virtual ~CSimba() = default;

public:
	void Set_SimbaEye(SIMBA_EYESTATE eEyeState) { m_eEyeState = eEyeState; }

	void InsertHitboxActivationTiming(SIMBA_ANIM eAnimIdx, vector<tuple<_float, _bool, COLLISION_VALUE>>& _vecTimings);

	SIMBA_ANIM Get_PreState() { return m_ePreState; }
	void Set_PreState(_uint _ePreState) { m_ePreState = SIMBA_ANIM(_ePreState); }

	_bool Get_RenderMant() { return m_bRenderMant; }
	void Set_RenderMant(_bool _bRenderMant) { m_bRenderMant = _bRenderMant; }

	void RegisterMonstersToSimba(vector<MONSTER_DESC>& _vecMonsterDescs) { m_vecMonsterDescs = _vecMonsterDescs; }

	_bool Get_RenderEyeLid() { return m_bRenderEyeLid; }
	void Set_RenderEyeLid(_bool _bRenderEyeLid) { m_bRenderEyeLid = _bRenderEyeLid; }

	void SetCamSequence(_uint iCamSeq);
	void ResetStarCount() { m_iStarCount = 0; }
	_uint Get_StarCount() { return m_iStarCount; }

	void ChangeDimensionClawUpDown() { m_bDimensionClawUpAttack = !m_bDimensionClawUpAttack; }
	void Set_StarPosToRightHand() { m_bBiteRushSpawnStarAtLeft = false; }
	void Set_StarPosToLeftHand() { m_bBiteRushSpawnStarAtLeft = true; }
	void ResetRockCount() { m_iRockCount = 0; }
	_uint Get_RockCount() { return m_iRockCount; }
	void ResetDebrisCount() { m_iDebrisCount = 0; }
	_uint Get_DebrisCount() { return m_iDebrisCount; }

	_bool Get_Wave2Summoned() { return m_bWave2Summoned; }

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;

	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void			Change_State(SIMBA_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool			IsAnimFinished() { return m_pModelCom->IsFinished(); }
	_bool			IsAnimFinished(_uint iCurrentAnimIndex) { return m_pModelCom->IsFinished(iCurrentAnimIndex); }
	void			CreateHpBar();
	void			SpawnStar(_uint iAnimIdx);
	_bool			IsKirbyOnMyLeft();
	_bool			IsDamagable();

	void			SetUpDimensionClawWorldMatrix();
	void			MoveDimensionClaw(_float fTimeDelta);
	void			HideDimensionClawActor();
	void			HideDimensionLaserActor();
	void			LaserAttack(_float fTimeDelta);

	void			SpawnRocks(_uint iAnimIdx);
	void			SpawnDebris(_uint iAnimIdx);

	void			SetUpSecondTarget();

	void			CheckFinalCrusherRingCollision(_float fTimeDelta);

	// Simba Effects
	void			QuickClawNailFlash(_uint eSimbaAnim);
	void			QuickClawSlash();
	void			FinalCrusherCharge();
	void			FinalCrusherSwing();
	void			FinalCrusherSmash();
	void			FinalCrusherRing();
	void			JumpStartSmoke();
	void			LandingSmoke();
	void			AttackJumpWind();
	void			DoubleClawDashGround();
	void			DoubleClawGround();
	void			DoubleClawSweep();

private:
	CTexture*		m_pEyeTextureCom[EYETEX_END] = { nullptr, nullptr, nullptr };
	CGameObject*	m_pKirby = { nullptr };
	class CBone*	m_pLipBone = { nullptr };
	class CBone*	m_pLeftHandBone = { nullptr };
	class CBone*	m_pRightHandBone = { nullptr };
	class CBone*	m_pLaserBone = { nullptr };
	const _float4x4* m_pLaserBoneMatrix = { nullptr };
	CGameObject*	m_pSimbaLaser = { nullptr };
	CTransform*		m_pSimbaLaserTransform = { nullptr };
	PxRigidDynamic* m_pDimensionClawActor = { nullptr };

	vector<class CBone*> m_vecLeftNailBones;
	vector<class CBone*> m_vecRightNailBones;

	SIMBA_ANIM		m_eCurrentState = { SIMBA_END };
	SIMBA_ANIM		m_ePreState = { SIMBA_END };
	SIMBA_EYESTATE	m_eEyeState = { SIMBAEYE_END };

	vector<_uint>	m_vecMeshes;
	_uint			m_iEyeMesh = {};
	_uint			m_iEyeLidMesh = {};

	vector<_uint>	m_vecMantMeshes;
	_bool			m_bRenderMant = { true };

	_bool			m_bPlayPartialAnim = { false };

	unordered_map<SIMBA_ANIM, vector<tuple<_float, _bool, COLLISION_VALUE>>> m_mapHitBoxTiming; // 애니메이션 인덱스와, 활성화시킬 KeyFrame들.

	_float			m_fAngle = { 0.f };
	_float4x4		m_matDefault = {};

	_bool			m_bHpBarCreated = { false };

	unordered_set<SIMBA_ANIM> m_setAppear1Anims;
	unordered_set<SIMBA_ANIM> m_setUndamagableAnims;
	unordered_set<SIMBA_ANIM> m_setDimensionClawAnims;
	unordered_set<SIMBA_ANIM> m_setDimensionLaserAnims;

	unordered_set<SIMBA_ANIM> m_mapRotation[ROTATION_END];			

	vector<MONSTER_DESC> m_vecMonsterDescs;

	_float			m_fHpRatio = { 1.f };
	_bool			m_bPhaseTwo = { false };
	_bool			m_bDeathAnimPlayed = { false };
	_bool			m_bRenderEyeLid = { false };
	_bool			m_bSummon1 = { false };
	_bool			m_bSummon2 = { false };
	_float			m_fSpawnTime = {};
	_float			m_fMonsterTime = {};
	_uint			m_iEffectCount = {};
	_uint			m_iMonsterCount = {};

	_uint			m_iStarCount = {};
	_bool			m_bDimensionClawUpAttack = { false };
	_bool			m_bBiteRushSpawnStarAtLeft = { true };

	_bool			m_bLaserActivated = { false };
	_bool			m_bRenderDimensionClaw = { false };

	_uint			m_iRockCount = {};
	_uint			m_iDebrisCount = {};
	vector<class CSimbaRock*>	m_vecSimbaRocks;
	vector<class CDebris*>		m_vecDebris;
	list<_uint>		m_listUsedRocks;
	list<_uint>		m_listUsedDebris;

	unordered_set<SIMBA_ANIM>	m_setResetRequiredAnims;
	_uint			m_iNextDebrisIndex = {};
	_bool			m_bStateChanged = { false };
	_bool			m_bWave2Summoned = { false };

	_float3			m_vRingPos = { };
	_float			m_fRingInnerRadius = {};
	_float			m_fRingOuterRadius = {};

	_bool			m_bRenderRing;

private:
	HRESULT		Add_Components();
	HRESULT		Bind_ShaderResources();

	void		PlayPartialAnimation();

	// FSM
	void		SetUp_FSM();
	void		SetUpHitBoxTimings();

	void		Check_HitBoxActivation();
	void		Reset_HitBoxTimingMap(SIMBA_ANIM eAnimIdx);
	void		PlayLipSinc();

	void		TransformToDefault(_float fOffsetY);

	void		OnAppearStart(CGameObject* pObj);
	void		OnNextDialog1(CGameObject* pObj);
	void		OnNextDialog2(CGameObject* pObj);
	void		OnLastDialog(CGameObject* pObj);
	void		OnAppearEnd(CGameObject* pObj);
	void		OnWave1Dead(CGameObject* pObj);
	void		OnWave2Dead(CGameObject* pObj);

	void		TriggerMonsterSpawning(_uint iTriggerIndex);
	void		SpawnMonsters(_uint iTriggerIndex);
	void		SpawnEffects(_uint iTriggerIndex);
	void		CheckSpawning();

	void		DetermineSimbaRotation();
	void		TurnSimba(_float fAngle);
	void		ResetRotation();

	void		CreateDimensionClawActor();
	void		OnSimbaAttackTrigger();
	void		RemoveDeadRocksFromList();
	void		RemoveDeadDebrisFromList();

	void		RenderRing();
	void		RenderPolygon(vector<_vector>& worldPoints);

public:
	static CSimba* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
