#pragma once

#include "Client_Defines.h"
#include "Monster.h"
#include <Kirby.h>

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)
 
class CAwoofy final : public CMonster
{
public:
	enum AWOOFY_ANIM { AWOOFY_BRAKE, AWOOFY_DAMAGE, AWOOFY_FALL, AWOOFY_FIND, AWOOFY_FINDSUB, AWOOFY_FINDWAIT, AWOOFY_GROOMING, 
		AWOOFY_HOWLING, AWOOFY_JUMP, AWOOFY_JUMPEND, AWOOFY_LANDING, AWOOFY_LOOKAROUND, AWOOFY_LOOKAROUNDAFTERBRAKE, AWOOFY_RUN, 
		AWOOFY_SLEEP, AWOOFY_SLEEPFALL, AWOOFY_WAIT, AWOOFY_WAKEUP, AWOOFY_WALK, AWOOFY_END };

private:
	CAwoofy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAwoofy(const CAwoofy& rhs);
	virtual ~CAwoofy() = default;

public:
	_vector Get_Origin() {
		return m_vOrginLook;
	}

	_vector Get_Target() {
		return m_vTargetLook;
	}

	_float Get_Angle() {
		return m_fAngle;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual void	Render_IMGUI() override;
	virtual void	Collision_Attack() override;

public:
	void Change_State(AWOOFY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_bool IsAnimFinished(_uint iCurrentAnimIndex);
	void Compute_Angle(_vector vOrginLook, _vector vTargetLook);
	
private:
	//CFSM*			m_pFSM = { nullptr };
	AWOOFY_ANIM	m_eCurrentState = { AWOOFY_END };

	_vector m_vOrginLook = {};
	_vector m_vTargetLook = {};

	_float	m_fAngle = { 0.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CAwoofy* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END