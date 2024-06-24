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
		STATE_CHANGEDIRECTION = 0,
		STATE_COMMAND = 4,
		STATE_DEATH = 6, STATE_DEATHWAIT = 7, STATE_DEATHWAITEND = 8,
		STATE_FALL = 22, STATE_JUMP = 45, STATE_JUMPEND = 46, STATE_JUMPSTART = 47, STATE_LANDING = 48,
		STATE_SLIDINGSTART = 55, STATE_SLIDING = 53, STATE_SLIDINGEND = 54,
		STATE_SHOUTSTART = 52, STATE_SHOUT = 50, STATE_SHOUTEND = 51,

		STATE_HAMMERATTACK = 26, STATE_HAMMERATTACKCHARGE = 27, STATE_HAMMERATTACKHIT = 28,
		STATE_HAMMERSIDE = 37, STATE_HAMMERSIDEATTACK = 38, STATE_HAMMERSIDECHARGE = 39, STATE_HAMMERSIDEWALK = 40,

		STATE_WAIT = 91, STATE_WALK = 92, STATE_RUN = 49,
		STATE_END = 999
	};

	struct DDDDESC {
		// 현재 싸움중인가? 이것이 켜져 있으면, 디디디가 커비를 바라보게된다.
		_bool			m_isBattle = { false };

		_float			m_fMoveSpeed = { 0.f };
		_float4			m_vMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };

		_float			m_fJumpVelocity = { 0.f };
		_float			m_fGravityOffset = { 6.f };
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

	DDDDESC* Get_Info() { return &m_tInfo; }

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	// FSM
	void SetUp_FSM();

	void Set_Look_MoveDir();


	_float4 m_vNeckLook = {};
	_float4 m_vLEyeLook = {};
	_float4 m_vREyeLook = {};
	DDDDESC m_tInfo = {};



public:
	static CDeeDeeDee* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END