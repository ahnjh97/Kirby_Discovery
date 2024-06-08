#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
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

	enum BUFFAHORNEYE_STATE { BUFFAHORNEYE_IDLE, BUFFAHORNEYE_HALF, BUFFAHORNEYE_SLEEP, BUFFAHORNEYE_SURPRISE, BUFFAHORNEYE_END };

private:
	CBuffahorn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBuffahorn(const CBuffahorn& rhs);
	virtual ~CBuffahorn() = default;

public:
	void Set_BuffahornEye(BUFFAHORNEYE_STATE eEyeState) {
		m_eEyeState = eEyeState;
	}
	void Set_JumpTime(_float fTimeDelta) {
		m_fJumpTime = fTimeDelta;
	}

	_float Get_AnimRatio() {
		return m_pModelCom->Get_AnimRatio();
	}
	_float Get_JumpTime() {
		return m_fJumpTime;
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
	void Change_State(BUFFAHORN_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	CTexture*		m_pEyeTextureCom = { nullptr };

	BUFFAHORN_ANIM	m_eCurrentState = { BUFFAHORN_END };
	BUFFAHORNEYE_STATE	m_eEyeState = { BUFFAHORNEYE_END };

	_float			m_fJumpTime = { 0.f };


private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

	// FSM
	void SetUp_FSM();
	_bool Custom_Face(_uint iMeshIndex);

public:
	static CBuffahorn* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END