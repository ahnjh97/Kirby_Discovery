#pragma once
#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CPartTimerKirby final : public CCharacter
{
public:
	enum PARTTIMER_KIRBY_ANIM {
		PARTTIMER_KIRBY_HI,
		PARTTIMER_KIRBY_IDLE,
		PARTTIMER_KIRBY_PULL,
		PARTTIMER_KIRBY_GRAB,
		PARTTIMER_KIRBY_SMILE,
		PARTTIMER_KIRBY_PUSH,
		PARTTIMER_KIRBY_END
	};

private:
	CPartTimerKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPartTimerKirby(const CPartTimerKirby& rhs);
	virtual ~CPartTimerKirby() = default;

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
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;
	virtual void	Collision_Hitbox(CPhysXObject* pGameObject) override;

public:
	void			Change_State(PARTTIMER_KIRBY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool			IsAnimFinished();

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

	// FSM
	void			SetUp_FSM();

private:
	CModel*					m_pModelCom = { nullptr };
	PARTTIMER_KIRBY_ANIM	m_eCurrentState = { KABU_END };

	_float					m_fScore = _float();

public:
	static CPartTimerKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END