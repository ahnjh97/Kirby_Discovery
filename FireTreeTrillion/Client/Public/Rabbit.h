#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CRabbit final : public CMonster
{
public:
	enum RABBIT_ANIM {
		RABBIT_DAMAGE, RABBIT_FIND, RABBIT_JAMPBIG, RABBIT_JUMP, RABBIT_JUMPATTACK, RABBIT_JUMPBIG, RABBIT_JUMPEND, RABBIT_JUMPENDBIG,
		RABBIT_JUMPENDSMALL, RABBIT_JUMPFALL, RABBIT_JUMPFALLATTACT, RABBIT_JUMPFALLBIG, RABBIT_JUMPFALLSMALL, RABBIT_JUMPLANDING, RABBIT_JUMPLANDINGATTACK,
		RABBIT_JUNMPLANDINGBIG, RABBIT_JUMPLANDINGSMALL, RABBIT_JUMPSMALL, RABBIT_JUMPSTART, RABBIT_JUMPSTARTBIG, RABBIT_JUMPSTARTSMALL, RABBIT_LOOKAROUND, RABBIT_WAIT,
		RABBIT_END };

private:
	CRabbit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRabbit(const CRabbit& rhs);
	virtual ~CRabbit() = default;

public:
	void Set_Find(_bool bFind) {
		m_bFind = bFind;
	}
	void Set_TimeDelta(_float fTimeDelta) {
		m_fTimeDelta = fTimeDelta;
	}

	_bool Get_Find() {
		return m_bFind;
	}
	_float Get_TimeDelta() {
		return m_fTimeDelta;
	}

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
	virtual void	Collision_Attack(CGameObject* pOtherObj) override;

public:
	void Change_State(RABBIT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_uint Get_State();

	void Compute_Parabola(_vector vEndPos);
	_vector JumpAttak(_float fTimeDelta);

private:
	RABBIT_ANIM	m_eCurrentState = { RABBIT_END };


private:
	_float	m_fTimeDelta = { 0.f };

	_bool	m_bFind = { false };


	// 포물선 공식에 필요한 변수
	_float4						m_vStartPos = {};		// 스킬 시작했을 때의 초기지점
	_float4						m_vEndPos = {};			// 스킬의 목적지 지점
	_float						m_fAxisX = {};			// X축으로의 속도
	_float						m_fAxisY = {};			// Y축으로의 속도
	_float						m_fAxisZ = {};			// Z축으로의 속도

	_float						m_fGravity = {};		// Y축으로의 중력가속도
	_float						m_fEndTime = {};		// 도착지점까지 도달 시간
	_float						m_fMaxHeight = { 0.f };// 최대 높이
	_float						m_fHeight = {};			// 최대 높이Y- 시작지점높이의 Y
	_float						m_fEndHight = {};		// 도착지점 높이 Y - 시작지점 높이 Y
	_float						m_fTime = { 0.f };		// 흐르는 시간
	_float						m_fMaxTime = { 0.8f };	// 최대높이 까지 가는 시간

	// 점프공격
	_float4						m_vGoPos = {}; // 포물선을 그릴 때 포물선 공식을 계속 더해줄 값


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CRabbit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END