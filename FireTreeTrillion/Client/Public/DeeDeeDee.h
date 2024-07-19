#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CDeeDeeDee final : public CMonster
{
public:
	enum STATE_TYPE {
		STATE_COMMAND = 3, //
		STATE_DEATH = 6, STATE_DEATHWAIT = 7, STATE_DEATHWAITEND = 8,

		STATE_FALL = 22, STATE_JUMP = 45, STATE_JUMPEND = 46, STATE_JUMPSTART = 47, STATE_LANDING = 48, //
		STATE_SLIDINGSTART = 55, STATE_SLIDING = 53, STATE_SLIDINGEND = 54, //
		STATE_SHOUTSTART = 52, STATE_SHOUT = 50, STATE_SHOUTEND = 51, //

		STATE_HAMMERATTACK = 27, STATE_HAMMERATTACKCHARGE = 26, STATE_HAMMERATTACKHIT = 28, //

		STATE_HAMMERSIDE = 37, STATE_HAMMERSIDEATTACK = 38, STATE_HAMMERSIDECHARGE = 39, STATE_HAMMERSIDEWALK = 40, //

		STATE_WAIT = 91, STATE_WALK = 92, STATE_RUN = 49, //
		STATE_END = 999
	};

	enum MYPATTERN {
		PATTERN_BATTLE,
		PATTERN_CENTER,
		PATTERN_END
	};

	struct DDDDESC {
		// 현재 싸움중인가? 이것이 켜져 있으면, 디디디가 커비를 바라보게된다.
		_bool			m_isBattle = { false };

		_float			m_fMoveSpeed = { 0.f };
		_float4			m_vMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };

		_float			m_fJumpVelocity = { 0.f };
		_float			m_fGravityOffset = { 15.f };

		// 다시 돌아가야 한다면, 이것이 트루가된다.
		_bool			m_bBackToOriginPos = { false };
		// 플레이어가 범위 안에 있다면
		_bool			m_bInRangePlayer = { false };



		_float4			m_vOriginPos = { 0.f, 0.f, 0.f, 0.f };
		_bool			m_bJumpTarget = { false };
		_float4			m_vJumpTargetDir = { 0.f, 0.f, 0.f, 0.f };
		MYPATTERN		m_ePattern = { PATTERN_END };
	};



private:
	CDeeDeeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDeeDeeDee(const CDeeDeeDee& rhs);
	virtual ~CDeeDeeDee() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual void	Add_AnimEvent()			override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void Change_State(STATE_TYPE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	void Look_Player(_float fTimeDelta);
	void Bone_Turn_Interpolate(_float4& vMoveDir, const _float4& vTargetDir, _float fTimeDelta);
	_bool IsAnimFinished();
	_float IsAnimRatio();

	DDDDESC* Get_Info() { return &m_tInfo; }

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	// FSM
	void SetUp_FSM();
	// 히트박스 체인저
	void HitBoxChanger(_uint eState);

	void Set_HitStop() { m_bHitStop = true; }
	_bool				  m_bHitStop = { false };
	_float				  m_fHitStopTime = { 0.f };
	_bool				  m_isDead = { false };
	_float				  m_fDeadTime = { 0.f };
	_bool				  m_bRestoreTrigger = { true };


	HRESULT	Make_TargetToCams();

	// 디디디의 시스템적인 틱을 담당한다.
	void DeeDeeDee_SystemTick(_float fTimeDelta);
	MYPATTERN Now_Pattern();
	void Hammer_BoneUpdate();
	void Dead_Animation(_float fTimeDelta);

	_float4 m_vNeckLook = {};
	_float4 m_vLEyeLook = {};
	_float4 m_vREyeLook = {};
	DDDDESC m_tInfo = {};

	_bool					m_bInitializeAnim = { true };
	_float4x4				m_WeaponMatrix;
	class CDeeDeeDeeHammer* m_pWeapons = { nullptr };
	class CUI_Interactable* m_pUI_Interactable = nullptr;


	_float					m_ftext = { 0.f };
	_bool					m_bCheckCollision = false;



public:
	static CDeeDeeDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END