#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CPoppyBrosJr final : public CMonster
{
public:
	enum POPPY_ANIM {
		POPPY_DAMAGE, POPPY_ENEMYATTACK, POPPY_ENEMYWAIT1, POPPY_ENEMYWAIT2, POPPY_FALL, POPPY_FIND, POPPY_FINDWAIT, POPPY_LANDING, POPPY_THROW, POPPY_WAIT, POPPY_WALK,
		POPPY_END
	};

	enum POPPY_STATE {
		PS_TARGET, PS_NONTARGET, PS_END
	};

	//struct POPPY_DESC : public CMonster::MONSTER_DESC {
	//	POPPY_STATE ePoppyState = { PS_END };
	//};

	enum POPPYEYE_STATE { POPPYEYE_IDLE, POPPYEYE_HALF, POPPYEYE_SLEEP, POPPYEYE_SURPRISE, POPPYEYE_DAMAGE, POPPYEYE_END };

private:
	CPoppyBrosJr(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPoppyBrosJr(const CPoppyBrosJr& rhs);
	virtual ~CPoppyBrosJr() = default;

public:
	void Set_BombActivate(_bool bBomb) {
		m_bBomb = bBomb;
	}
	void Set_PoppyEye(POPPYEYE_STATE eEyeState) {
		m_eEyeState = eEyeState;
	}

	_float Get_AnimRatio() {
		return m_pModelCom->Get_AnimRatio();
	}
	POPPY_STATE Get_PoppyState() {
		return m_ePoppyState;
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

public:
	void Change_State(POPPY_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();
	_float4 Compute_BoneWorldMatrix();

private:
	CTexture* m_pEyeTextureCom = { nullptr };

	POPPY_ANIM	m_eCurrentState = { POPPY_END };
	POPPY_STATE	m_ePoppyState = { PS_END };
	POPPYEYE_STATE	m_eEyeState = { POPPYEYE_END };

	_float			m_fJumpTime = { 0.f };

	_bool			m_bBomb = { false };

	_uint				m_iEyeMeshIdx = { 2 };

	_float4x4		m_WorldMatrix = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();
	_bool Custom_Face(_uint iMeshIndex);

public:
	static CPoppyBrosJr* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END