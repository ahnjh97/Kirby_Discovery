#pragma once

#include "Client_Defines.h"
#include "Monster.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CPoppyBomb final : public CMonster
{
public:
	struct POPPYBOMB_DESC : public CMonster::MONSTER_DESC {
		_float4 vPosition = {};
		_float4 vLook = {};
		_vector vTargetPosition = {};
		CGameObject* pGameObject = { nullptr };
	};

private:
	CPoppyBomb(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPoppyBomb(const CPoppyBomb& rhs);
	virtual ~CPoppyBomb() = default;

public:
	void Set_BombActivate(_bool bBomb) {
		m_bBomb = bBomb;
	}
	void Set_BombPos(_float4 vPosition) {
		m_vPosition = vPosition;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_LightDepth() override;

private:
	//CModel*					m_pModelCom = { nullptr };
	//CShader*				m_pShaderCom = { nullptr };
	//CCharacterController*	m_pControllerCom = { nullptr };

private:
	CGameObject* m_pGameObject = { nullptr };

	_float		m_fJumpTimeDelta = { 0.f };
	_float		m_fMoveTime = { 0.f };
	_float		m_fLifeTime = { 0.f };

	_float4		m_vPosition = {};
	_float4		m_vLook = {};

	_vector		m_vTargetPosition = {};
	_vector		m_vBeforePos = {};
	_vector		m_vLookDir = {};

	_bool		m_bBomb = { false };
	_bool		m_bJump = { false };
	_bool		m_bPhysx = { false };

	// 포물선 공식에 필요한 변수
	_float4		m_vStartPos = {};			// 스킬 시작했을 때의 초기지점
	_float4		m_vEndPos = {};				// 스킬의 목적지 지점
	_float		m_fAxisX = { 0.f };			// X축으로의 속도
	_float		m_fAxisY = { 0.f };			// Y축으로의 속도
	_float		m_fAxisZ = { 0.f };			// Z축으로의 속도

	_float		m_fGravity = { 0.f };		// Y축으로의 중력가속도
	_float		m_fEndTime = { 0.f };		// 도착지점까지 도달 시간
	_float		m_fMaxHeight = { 0.f };		// 최대 높이
	_float		m_fHeight = { 0.f };		// 최대 높이Y- 시작지점높이의 Y
	_float		m_fEndHight = { 0.f };		// 도착지점 높이 Y - 시작지점 높이 Y
	_float		m_fTime = { 0.f };			// 흐르는 시간
	_float		m_fMaxTime = { 0.8f };		// 최대높이 까지 가는 시간

	// 점프공격
	_float4		m_vGoPos = {}; // 포물선을 그릴 때 포물선 공식을 계속 더해줄 값

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void	Compute_MotionBlur();

	void Compute_Parabola(_vector vEndPos);
	_vector JumpAttak(_float fTimeDelta);

public:
	static CPoppyBomb* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END