#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CKabu final : public CMonster
{
public:
	enum KABU_ANIM {
		KABU_LANDING, KABU_WAIT, KABU_DAMAGE, KABU_FALL, KABU_JUMP, KABU_JUMPSTART, KABU_WARP1, KABU_WARP2,
		KABU_END
	};

	enum KABUMOVING_STATE {
		KABUMOVING_CIRCLE, KABUMOVING_PATROL, KABUMOVING_END
	};

	struct KABU_DESC : public CMonster::MONSTER_DESC {
		KABUMOVING_STATE eMoveState = { KABUMOVING_END };
	};

private:
	CKabu(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKabu(const CKabu& rhs);
	virtual ~CKabu() = default;

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
	void Change_State(KABU_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	KABU_ANIM			m_eCurrentState = { KABU_END };
	KABUMOVING_STATE	m_eMoveState = { KABUMOVING_END };

	// 회전 상태의 카부
	_float				m_fDistance = { 0.f };
	_float				m_fAngle = { 0.f };

	_float4				m_vOriginPos = {};
	_float4				m_vRotatePos = {};

	// 와리가리 상태의 카부
	_float4				m_vRally = {};
	_uint				m_iCnt = { 0 };
	_bool				m_bConvert = { false };

	vector<_float4>		m_vecRallyPoint = {};

	_float				m_fMoveTime = { 0.f };
	_float				m_fSpeed = { 0.f };

	_float2				m_vPreScreenPos = { 0.f, 0.f };
	_float4				m_vMotionVelocity = { 0.f, 0.f, 0.f, 0.f };


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

	// FSM
	void SetUp_FSM();

public:
	static CKabu* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END