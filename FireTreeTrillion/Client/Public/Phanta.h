#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CPhanta final : public CMonster
{
public:
	enum PHANTA_ANIM {
		PHANTA_APPEAR, PHANTA_ATTACK, PHANTA_BRAKE, PHANTA_DAMAGE, PHANTA_DISAPPEAR, PHANTA_FIND, PHANTA_FLYINGATTACK, PHANTA_FLYINGFIND, PHANTA_MOVE, PHANTA_WAIT, PHANTA_END
	};

	struct PHANTA_DESC : public CMonster::MONSTER_DESC {
		//POPPY_STATE ePoppyState = { PS_END };
	};

private:
	CPhanta(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPhanta(const CPhanta& rhs);
	virtual ~CPhanta() = default;

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
	void Change_State(PHANTA_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	PHANTA_ANIM		m_eCurrentState = { PHANTA_END };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CPhanta* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END