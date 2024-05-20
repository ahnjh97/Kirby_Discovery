#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CRigidBody;
class CFSM;
END

BEGIN(Client)

class CTestModel final : public CGameObject
{
public:
	enum STATE {
		ATTACK = 0,
		IDLE,
		RUN,
		STATE_END
	};

private:
	CTestModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestModel(const CTestModel& rhs);
	virtual ~CTestModel() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual void	Render_IMGUI() override;

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();
	void			Add_RigidBody(const wstring& KeyName, void* pArg);

	// FSM
	void			SetUp_FSM();
	void			Update_FSMState(_float fTimeDelta);
	void			Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);

private:
	CModel*			m_pModelCom = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	class CLight*	m_pLight = { nullptr };

	CRigidBody*		m_pRigidBodyCom = { nullptr };
	//map<string, CRigidBody*> m_mapRigidBodies;

	// Player FSM 및 Jump 관련 변수들
	CFSM*			m_pFSM = { nullptr };
	STATE			m_eCurrentState = { STATE_END };

	_int			m_iTestAnim = { 0 };

public:
	static CTestModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END