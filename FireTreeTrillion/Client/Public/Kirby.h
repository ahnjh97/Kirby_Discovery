#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CFSM;
END

BEGIN(Client)

class CKirby :
    public CGameObject
{
public:
	// 다양한 STATE 에서 관리하여야 하는 구조체
	typedef struct INFODESC
	{
		_float			m_fMoveSpeed = { 0.f };
		_float			m_fZAngle = { 0.f };
		_float4			m_vMoveDir = { 0.f, 0.f, 0.f, 0.f };
		_float4			m_vTargetDir = { 0.f, 0.f, 0.f, 0.f };
	}KIRBY_INFODESC;

	enum DIR { DIR_LEFT, DIR_RIGHT, DIR_FRONT, DIR_BACK, DIR_LF, DIR_RF, DIR_LB, DIR_RB, DIR_END };


	enum STATE {
		STATE_IDLE, STATE_RUN = 172, STATE_END
    };

	enum EYESTATE { EYE_IDLE, EYE_ANGER, EYE_CLOSE, EYE_SADNESS, EYE_PUPIL, EYE_BLINK, EYE_END };
	enum MOUTHSTATE { MONTH_IDLE, MOUTH_ANGER, MOUTH_HAPPY, MOUTH_SMILE, MOUTH_SURPRISE, MOUTH_END };
	enum BODYSTATE { BODY_DEFAULT, BODY_VACUUM, BODY_BALLOON, BODY_END };

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

private:
	void			Setting_KirbyBalance();
	void			Key_Input(_float fTimeDelta);

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
	// 애니메이션 상태 제어를 한다.
	STATE			m_eCurrentState = { STATE_END };

	// 눈, 입, 몸체의 상태를 담당한다.
	EYESTATE		m_eEyeState = { EYE_END };
	MOUTHSTATE		m_eMouthState = { MOUTH_END };
	BODYSTATE		m_eBodyState = { BODY_END };

private:
	// 몸에 따라 모델이 달라진다.
	CModel*			m_pModelCom[BODY_END] = {nullptr};
	// 눈 텍스쳐
	CTexture*		m_pEyeTexture[EYE_END] = { nullptr };
	// 입 텍스쳐
	CTexture*		m_pMouthTexture[MOUTH_END] = { nullptr };
	CShader*		m_pShaderCom = { nullptr };
	class CCamera_Free* m_pCamera = { nullptr };
	KIRBY_INFODESC  m_tKirbyInfo;

public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END