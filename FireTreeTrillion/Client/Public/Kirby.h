#pragma once

#include "Client_Defines.h"
#include "Character.h"
#include "Effect.h"

#define	INFO(state) m_tKirbyInfo.state

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CPartObject;
END

BEGIN(Client)

class CKirby :
    public CCharacter
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
		_float4			m_vDodgeDir = { 0.f, 0.f, 0.f, 0.f };

		// 다양한 무기 움직임에 사용 될 것이다.
		_float4			m_vAttackDir = { 0.f, 0.f, 0.f, 0.f };
		_float			m_fAttackMoveSpeed = { 0.f };

		// 눈, 입, 몸체의 상태를 담당한다.
		EYESTATE		m_eEyeState = { EYE_END };
		MOUTHSTATE		m_eMouthState = { MOUTH_END };
		BODYSTATE		m_eBodyState = { BODY_END };
		DIR				m_eKirbyDir = { DIR_END };
		STATE			m_eJumpState = { STATE_JUMPR };

		// 먹은 놈을 소화시키기 전에 잠깐 저장해놓는 곳. (현재 입에 있는 놈)
		ABILITYTYPE		m_eTemporaryEatType = { ABILITY_END };

		// 중력 및 점프
		_float			m_fJumpVelocity = { 0.f };
		_float			m_fGravityOffset = { 6.f };
		_float			m_fJumpHoldTime = { 0.f };
		_float			m_fChangeVelocityZeroTime = { 0.f };
		_float			m_fHoldAirTime = { 0.f };

		// 점프 중 재입력 방지
		_bool			m_bRePressBlock = { false };

		// 점프 예약 입력
		_bool			m_bReserveJumpKey = { false };

		// 먹은 상태에서의 FALL 애님은 없기 때문에 같은 애니메이션으로 구분하기 위헤 부울값 선언
		_bool			m_isEatFall = { false };
		// 현재 먹은 상황임을 표현한다.
		_bool			m_isEat = { false };

		// Vacuum
		_float			m_fVacuumTime = { 0.f };
		CPhysXObject*	m_pObject = { nullptr };
		_float3			m_vObjectScale = { 0.f, 0.f, 0.f };
		_float			m_fObjectDistance = { 0.f };

		// 방향 키 컨트롤러를 만지고 있는가?
		_bool			m_isController = { false };

		// Fly
		_float			m_fFlyTime = { 0.f };

		// 땅에 능력 버리는 시간
		_float			m_fDumpAbilityTime = { 0.f };

		// Ability Sword
		// PRESS 시, 차지시간을 정해주는 변수
		_float			m_fChargeTime = { 0.f };
		// 어택 시, 다음 어택모션이 정해질 시간변수
		_float			m_fAttackTime = { 0.f };
		STATE			m_ePreAttackState = { SWORDSTATE_DECISIVESLASH };

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
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif
	virtual HRESULT	Render_DeferredInfo() override;
	virtual void	Collision_Attack(CGameObject* pOtherObj) override;

	KIRBY_INFODESC* Get_KirbyInfo() { return &m_tKirbyInfo; }
	void			Set_KirbyInfo(KIRBY_INFODESC _tInfo) {
		m_tKirbyInfo = _tInfo;
	}

	void			Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, BODYSTATE eBody, _uint iOffSet = 0);
	void			Set_Animation(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	void			Set_Animation(_int iAnimIndex);

	_bool			isAnimFinish();
	void			DefaultIdle();

	_float4			Compute_TerrainPosition();

#pragma region 이펙트 리스트 실험
	//안녕하세요 디버깅 용으로 이펙트 리스트를 추가했습니다
	void			Add_KirbyEffect(CEffect* pEffect)
	{
		m_KirbyFXList.emplace_back(pEffect);
		Safe_AddRef(pEffect);
	}
	void			Delete_KirbyEffect()
	{
		if (m_KirbyFXList.empty())
			return;

		for (auto& FX : m_KirbyFXList)
		{
			FX->Set_Dead();
			Safe_Release(FX);
		}
		m_KirbyFXList.clear();
	}

private:
	list<CEffect*> m_KirbyFXList;

#pragma endregion


	// 기타 세부적인 제어
private:
	// 커비의 움직임을 담은 구조체
	KIRBY_INFODESC  m_tKirbyInfo;
	void			Setting_KirbyBalance();
	void			Key_Input(_float fTimeDelta);
	void			Kirby_SystemTick(_float fTimeDelta);

	_float3			Make_RepulsiveDir(CPhysXObject* pObject);

private:
	HRESULT			Add_Components();
	HRESULT			Add_PartObjects();
	HRESULT			Bind_ShaderResources();
	_bool			Kirby_FaceCustom(BODYSTATE _eBodyState, _uint _iMeshIndex);
	// FSM
	void			SetUp_FSM();

private:
	CModel*					m_pModelCom[BODY_END] = {nullptr};
	CTexture*				m_pEyeTexture[EYE_END] = { nullptr };
	CTexture*				m_pMouthTexture[MOUTH_END] = { nullptr };
	class CCamera_Free*		m_pCamera = { nullptr };

private:
	void			Update_PartObjectMatrix();
	class CKirbyWeapons* m_pWeapons = { nullptr };
	class CKirbyArmours* m_pArmours = { nullptr };
	_float4x4			 m_WeaponMatrix;
	_float4x4			 m_ArmourMatrix;

	_int					m_iTestAnim = { 0 };


public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END