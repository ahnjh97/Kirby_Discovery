#pragma once

#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
class CRigidBody;
class CFSM;
class CCharacterController;
END

BEGIN(Client)

class CTestModel final : public CCharacter
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
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	void IsJump(_bool _isJump) {
		m_isJump = _isJump;
	}

public:
	void SetOn_Slope(_float fTimeDelta);
	void Lerp_UpVector(_fvector _vOriginUp, _fvector _vTargetUp, _float _maxAngle, _float fTimeDelta);

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
	//CShader*		m_pShaderCom = { nullptr };
	//class CLight*	m_pLight = { nullptr };

	//CRigidBody*		m_pRigidBodyCom = { nullptr };
	//map<string, CRigidBody*> m_mapRigidBodies;
	//CCharacterController* m_pControllerCom = { nullptr };

	// Player FSM 및 Jump 관련 변수들
	//CFSM*			m_pFSM = { nullptr };
	STATE			m_eCurrentState = { STATE_END };

	_int			m_iTestAnim = { 0 };
	_bool			m_isJump = { false };
	_float			m_fJumpVelocity = { 0.f };
	_float			m_fOffsetTurn = { 7.f };

	_vector			m_vPos = {};

public:
	static CTestModel*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END