#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#define	INFO(state) m_tKirbyInfo.state

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CFSM;
class CCharacterController;
END

BEGIN(Client)

class CKirby :
    public CGameObject
{
public:

// Enum을 모아놓은 헤더파일이다.
#include "Kirby_Enums.h"

	// 다양한 STATE 에서 관리하여야 하는 구조체
	typedef struct INFODESC
	{
		_float			m_fMoveSpeed = { 0.f };
		_float			m_fZAngle = { 0.f };
		_float4			m_vMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };
		// 눈, 입, 몸체의 상태를 담당한다.
		EYESTATE		m_eEyeState = { EYE_END };
		MOUTHSTATE		m_eMouthState = { MOUTH_END };
		BODYSTATE		m_eBodyState = { BODY_END };

		// 중력 및 점프
		_float m_fJumpVelocity = { 0.f };
		_bool	m_isJump = { false };
		_bool	m_isLanding = { false };

		// 방향 키 컨트롤러를 만지고 있는가?
		_bool	m_isController = { false };
	}KIRBY_INFODESC;


private:
	CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby(const CKirby& rhs);
	virtual ~CKirby() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;
	virtual void	Render_IMGUI() override;

	KIRBY_INFODESC* Get_KirbyInfo() { return &m_tKirbyInfo; }
	void			Set_KirbyInfo(KIRBY_INFODESC _tInfo) {
		m_tKirbyInfo = _tInfo;
	}
	_uint			Get_State();


	// 기타 세부적인 제어
private:
	// 커비의 움직임을 담은 구조체
	KIRBY_INFODESC  m_tKirbyInfo;
	DIR				m_eKirbyDir = { DIR_END };

	void			SetOn_Slope(_float fTimeDelta);
	void			Lerp_UpVector(_fvector _vTargetUp, _float _maxAngle, _float fTimeDelta);
	void			Setting_KirbyBalance();
	void			Key_Input(_float fTimeDelta);
	void			JoyStick_Input(_float fTimeDelta);
	// 일반 움직임을 할 수 있는 상황이여야 하니, 전부 false 여야 true를 반환하여야 한다.
	_bool			Can_JoyStickUsing() { 
		return !m_tKirbyInfo.m_isJump && !m_tKirbyInfo.m_isLanding;
	}
	void			ZXCV_Input(_float fTimeDelta);
	void			Kirby_SystemTick(_float fTimeDelta);

	void			Idle_Animation(_float fTimeDelta);
	_float			m_fIdleStreachTime = { 0.f };
	_bool			m_bKirbyIdleChangeTrigger = { true };
	_uint			m_iIdleChoose = { 0 };

	// JUMP를 위해 L와 R가 번갈아서 진행된다.
	void			Kirby_Jump(_float fTimeDelta);
	STATE			m_eJumpState = { STATE_END };
	_float			m_fJumpHoldTime = { 0.f };
	_float			m_fChangeVelocityZeroTime = { 0.f };
	_float			m_fHoldAirTime = { 0.f };
	_float			m_fChangeRunTime = { 0.f };


private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();
	_bool			Kirby_FaceCustom(BODYSTATE _eBodyState, _uint _iMeshIndex);
	_float4			Make_TargetDir(DIR _eDir);

	// FSM
	void			SetUp_FSM();
	void			Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	// Player FSM 및 Jump 관련 변수들
	CFSM*			m_pFSM = { nullptr };

private:
	CModel*					m_pModelCom[BODY_END] = {nullptr};
	CTexture*				m_pEyeTexture[EYE_END] = { nullptr };
	CTexture*				m_pMouthTexture[MOUTH_END] = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	class CCamera_Free*		m_pCamera = { nullptr };
	CCharacterController*	m_pControllerCom = { nullptr };

	_int			m_iTestAnim = { 0 };

	// For_PhysX
	_float			m_fOffsetTurn = { 7.f };
	_float4			m_vOriginUp = { 0.f, 1.f, 0.f, 0.f };


public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END