#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPartObject;
END

BEGIN(Client)

class CBladeKnight final : public CMonster
{
public:
	enum BLADEKNIGHT_ANIM {
		BLADEKNIGHT_ATTACK, BLADEKNIGHT_ATTACKSTART, BLADEKNIGHT_DAMAGE, BLADEKNIGHT_DOUBLEATTACK, BLADEKNIGHT_FALL, BLADEKNIGHT_FIND, BLADEKNIGHT_FINDWAIT,
		BLADEKNIGHT_FINDWAITSUB, BLADEKNIGHT_LANDING, BLADEKNIGHT_MOVE, BLADEKNIGHT_RETREAT, BLADEKNIGHT_TRUST, BLADEKNIGHT_TRUSTEND, BLADEKNIGHT_TRUSTLOOP, BLADEKNIGHT_TRUSTSTART, BLADEKNIGHT_THRUSTSTARWAIT,
		BLADEKNIGHT_THRUSTWAIT, BLADEKNIGHT_TORNADOATTACK, BLADEKNIGHT_TORNADOATTACKCHARGE, BLADEKNIGHT_TORNADOATTACKCHARGE2, BLADEKNIGHT_TORNADOATTACKCHARGEMAX, BLADEKNIGHT_WAIT, 
		BLADEKNIGHT_END
	};

private:
	CBladeKnight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBladeKnight(const CBladeKnight& rhs);
	virtual ~CBladeKnight() = default;

public:
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg)				override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;
	virtual HRESULT Render_LightDepth()					override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()						override;
#endif
	virtual void	Add_AnimEvent()						override;
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void Change_State(BLADEKNIGHT_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	map<const wstring, CPartObject*>	m_PartObjects;
	class CBladeKnightSword*			m_pSword = { nullptr };

	BLADEKNIGHT_ANIM					m_eCurrentState = { BLADEKNIGHT_END };

private:
	HRESULT Add_Components();
	HRESULT Add_PartObjects();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CBladeKnight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END