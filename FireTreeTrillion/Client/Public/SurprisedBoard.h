#pragma once
#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CFSM;
END

BEGIN(Client)
 
class CSurprisedBoard final : public CMonster
{
public:
	// L은 왼쪽으로 튀어나오는 친구일 경우 사용하는 약어
	enum ANIM_STATE	  { ARM_MOVE_L, ARM_MOVE_R, 
						//BREAK_L, BREAK_R, 
						POP_OUT_L = 4, POP_OUT_R,
						PREPOP_OUT_L, PREPOP_OUT_R,
						RETURN_L, RETURN_R,
						WAIT_L, WAIT_R, ANIM_STATE_END };
	enum STATE { WAIT, POPOUT, RETURN, STATE_END };
	enum MODEL_COLOR { RED, GREEN, BLUE, COLOR_END };

public:
	struct SURPRISED_DESC : public CMonster::MONSTER_DESC
	{ 
		MODEL_COLOR eColor = RED;
		ANIM_STATE	eStartState;
		_float3		vPosition = _float3();
	};

private:
	CSurprisedBoard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSurprisedBoard(const CSurprisedBoard& rhs);
	virtual ~CSurprisedBoard() = default;

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
	void				SetUp_FSM(ANIM_STATE eSta);

private:
	array<CModel*, 3>	m_arrModelCom;
	MODEL_COLOR			m_eModelColor;

public:
	static CSurprisedBoard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END