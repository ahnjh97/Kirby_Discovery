#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CSpookStep final : public CMonster
{
public:
	enum SPOOKSTEP_ANIM {
		SPOOKSTEP_APPEAR, SPOOKSTEP_DAMAGE, SPOOKSTEP_DISAPPEAR, SPOOKSTEP_FALL, SPOOKSTEP_FIND, SPOOKSTEP_LANDING, SPOOKSTEP_MOVE, SPOOKSTEP_WAIT, SPOOKSTEP_END
	};

	//struct PHANTA_DESC : public CMonster::MONSTER_DESC {
	//	//POPPY_STATE ePoppyState = { PS_END };
	//};

private:
	CSpookStep(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSpookStep(const CSpookStep& rhs);
	virtual ~CSpookStep() = default;

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
	void Change_State(SPOOKSTEP_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	_bool IsAnimFinished();

private:
	SPOOKSTEP_ANIM		m_eCurrentState = { SPOOKSTEP_END };

	_uint				m_iEyeNum = { 0 };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	// FSM
	void SetUp_FSM();
	void SpawnEffect(CEffect::FX_DESC FXDesc, _fvector vRight, _fvector vUp, string strName);

public:
	static CSpookStep* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END