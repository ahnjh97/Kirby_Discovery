#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CCappyHat final : public CMonster
{
public:
	enum CAPPYHAT_ANIM {
		CAPPYHAT_WAIT, CAPPYHAT_FLY, CAPPYHAT_LANDING, CAPPYHAT_END
	};

	//enum CAPPYBODYMOVING_STATE {
	//	CAPPYBODYMOVING_CIRCLE, CAPPYBODYMOVING_PATROL, CAPPYBODYMOVING_END
	//};

	struct CAPPYHAT_DESC : public CMonster::MONSTER_DESC {
		_float4x4		WorldMatrix = {};
		CGameObject*	pGameObject = { nullptr };
	};

private:
	CCappyHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCappyHat(const CCappyHat& rhs);
	virtual ~CCappyHat() = default;

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
	virtual void	Collision_Overlap(CGameObject* pGameObject);

public:
	void Change_State(CAPPYHAT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	CGameObject*			m_pGameObject = { nullptr };

	CAPPYHAT_ANIM			m_eCurrentState = { CAPPYHAT_END };
	//CAPPYBODYMOVING_STATE	m_eMoveState = { CAPPYMOVING_END };

	_float4x4				m_WorldMatrix = {};
	_float					m_fHeightTime = { 0.f };
	
	_float4					m_vPosition = {};

	_float		m_fFlyTime = { 0.f };
	_float		m_fDeadTime = { 0.f };
	_bool		m_bController = { false };
private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CCappyHat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END