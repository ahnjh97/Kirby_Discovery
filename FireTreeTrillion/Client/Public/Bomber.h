#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CBomber final : public CMonster
{
public:
	enum BOMBER_ANIM {
		BOMBER_CLIFFFALL, BOMBER_CLIFFFALLEND, BOMBER_CLIFFFALLLANDING, BOMBER_CLIFFFALLSTART, BOMBER_DAMAGE, BOMBER_EXPLOSION, BOMBER_FALL, BOMBER_FIND, 
		BOMBER_FINDWAIT, BOMBER_WAIT, BOMBER_WALK, BOMBER_WALKFALL, BOMBER_END
	};

	//struct BOMBER_DESC : public CMonster::MONSTER_DESC {
	//};

private:
	CBomber(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBomber(const CBomber& rhs);
	virtual ~CBomber() = default;

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
	virtual void	Add_AnimEvent() override;

public:
	void Change_State(BOMBER_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	BOMBER_ANIM		m_eCurrentState = { BOMBER_END };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CBomber* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END