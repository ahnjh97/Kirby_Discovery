#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBuffahorn final : public CMonster
{
public:
	enum BUFFAHORN_ANIM {
		BUFFAHORN_BOUNCETOTRUN, BUFFAHORN_BOUNCETOTURN, BUFFAHORN_BRAKE, BUFFAHORN_BRAKEEND, BUFFAHORN_CHARGEWAIT, BUFFAHORN_DAMAGE, BUFFAHORN_DAMAGESUB,
		BUFFAHORN_FALL, BUFFAHORN_FIND, BUFFAHORN_JUMP, BUFFAHORN_LANDING, BUFFAHORN_NORMALWAIT, BUFFAHORN_RETURNJUMP, BUFFAHORN_RETURNJUMPEND,
		BUFFAHORN_RETURNJUMPSTART, BUFFAHORN_RUN, BUFFAHORN_RUNSTART, BUFFAHORN_TURN, BUFFAHORN_TURNEND, BUFFAHORN_WAIT, BUFFAHORN_WALK,
		BUFFAHORN_END
	};

private:
	CBuffahorn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBuffahorn(const CBuffahorn& rhs);
	virtual ~CBuffahorn() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual void	Render_IMGUI() override;
	virtual void	Collision_Attack(CGameObject* pOtherObj) override;

public:
	void Change_State(BUFFAHORN_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_uint Get_State();

private:
	//CFSM*			m_pFSM = { nullptr };
	BUFFAHORN_ANIM	m_eCurrentState = { BUFFAHORN_END };

	void			Compute_MotionBlur();
	_float2			m_vPreScreenPos = { 0.f, 0.f };
	_float4			m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CBuffahorn* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END