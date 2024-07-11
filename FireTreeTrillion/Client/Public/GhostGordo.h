#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CGhostGordo final : public CMonster
{
public:
	enum GORDO_ANIM {
		GORDO_APPEAR, GORDO_EYECLOSESTART, GORDO_EYECLOSEWAIT, GORDO_EYEOPENSTART, GORDO_EYEOPENWAIT, GORDO_HIGHTHORNSUB, GORDO_LOOK, GORDO_LOWTHORNSUB, GORDO_LOWTOHIGHSUB,
		GORDO_END
	};

	//struct GORDO_DESC : public CMonster::MONSTER_DESC {
	//};

private:
	CGhostGordo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGhostGordo(const CGhostGordo& rhs);
	virtual ~CGhostGordo() = default;

public:
	void Set_R2B(_bool bR2b) { m_bR2b = bR2b; }
	void Set_Look(_vector vLook) { m_vOriginLook = vLook; }

	_bool Get_Light() { return m_bLight; }
	_bool Get_R2B() { return m_bR2b; }
	_vector Get_Position() { return m_vOriginPosition; }
	_vector Get_Look() { return m_vOriginLook; }

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
	void Change_State(GORDO_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	GORDO_ANIM		m_eCurrentState = { GORDO_END };

	_bool			m_bLight = { false };
	_bool			m_bR2b = { false };

	_vector			m_vOriginPosition = {};
	_vector			m_vOriginLook = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();

public:
	static CGhostGordo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END