#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CCappyBody final : public CMonster
{
public:
	enum CAPPYBODY_ANIM {
		CAPPYBODY_DAMAGE, CAPPYBODY_FALL, CAPPYBODY_FIND, CAPPYBODY_HATLOSE, CAPPYBODY_HATLOSELOOP, CAPPYBODY_HIDINGJUMPA, CAPPYBODY_HIDINGJUMPB, CAPPYBODY_HIDINGWAITA, CAPPYBODY_HIDINGWAITB, CAPPYBODY_JUMP,
		CAPPYBODY_JUMPSTART, CAPPYBODY_KASACATCH, CAPPYBODY_KASAUP1, CAPPYBODY_KASAUP1WAIT, CAPPYBODY_KASAUP2, CAPPYBODY_KASAUP2WAIT, CAPPYBODY_KASAUP3, CAPPYBODY_KASAUP3WAIT, CAPPYBODY_LANDING, CAPPYBODY_WAIT,
		CAPPYBODY_END
	};

	//enum CAPPYBODYMOVING_STATE {
	//	CAPPYBODYMOVING_CIRCLE, CAPPYBODYMOVING_PATROL, CAPPYBODYMOVING_END
	//};

	//struct CAPPYBODY_DESC : public CMonster::MONSTER_DESC {
	//	CAPPYBODYMOVING_STATE eMoveState = { CAPPYBODYMOVING_END };
	//	vector<_float4> vecRallyPoints;
	//};

private:
	CCappyBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCappyBody(const CCappyBody& rhs);
	virtual ~CCappyBody() = default;

public:
	void Set_Render(_bool bRender) {
		m_bRender = bRender;
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
	virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;
	virtual void	Collision_Overlap(CGameObject* pGameObject);

public:
	void Change_State(CAPPYBODY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_float4x4 Compute_BoneWorldMatrix(_bool bDecompose = false);

private:
	CAPPYBODY_ANIM			m_eCurrentState = { CAPPYBODY_END };
	//CAPPYBODYMOVING_STATE	m_eMoveState = { CAPPYMOVING_END };

	_float4x4				m_WorldMatrix = {};

	_bool					m_bRender = { false };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CCappyBody* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END