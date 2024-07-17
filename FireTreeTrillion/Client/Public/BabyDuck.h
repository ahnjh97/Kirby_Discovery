#pragma once
#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CFSM;
END

BEGIN(Client)
 
class CBabyDuck final : public CMonster
{
public:
	enum ANIM_STATE	  { IDLE, ANIM_STATE_END };
	enum STATE { WAIT, POPOUT, RETURN, STATE_END };

//public:
//	struct SURPRISED_DESC : public CMonster::MONSTER_DESC
//	{ 
//		ANIM_STATE	eStartState;
//		_float3		vPosition = _float3();
//	};

private:
	CBabyDuck(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBabyDuck(const CBabyDuck& rhs);
	virtual ~CBabyDuck() = default;

public:
	virtual HRESULT		Initialize_Prototype()			override;
	virtual HRESULT		Initialize(void* pArg)			override;
	virtual _int		Tick(_float fTimeDelta)			override;
	virtual void		Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT		Render()						override;
	virtual HRESULT		Render_LightDepth()				override;
#ifdef _DEBUG
	virtual void		Render_IMGUI()					override;
#endif
	virtual void		Add_AnimEvent()					override;
	virtual void		Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;

public:
	void				Change_State(ANIM_STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool				IsAnimFinished();
	_bool				IsAnimFinished(_uint iCurrentAnimIndex);

private:
	HRESULT				Add_Components();
	HRESULT				Add_HitBoxes(_float3 vPos);
	HRESULT				Bind_ShaderResources();

	// FOR FSM
	void				SetUp_FSM();

public:
	static CBabyDuck* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END