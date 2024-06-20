#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "HUD.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CHUD_KirbyStatus : public CHUD
{
private:
	CHUD_KirbyStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_KirbyStatus(const CHUD_KirbyStatus& rhs);
	virtual ~CHUD_KirbyStatus() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	//virtual void				Render_IMGUI()								override;
#endif

private:
	HRESULT						Add_Components();
	HRESULT						Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom);
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

	void						Update_UIState(_float _fTimeDelta);
	void						Play_Animation(_float _fAccTime, KIRBYHP_STATE _eCurState);

	void						Compute_Player_Hp(_float fTimeDelta, class CKirby* pKirby);
	void						Disappear_HpBar(_float fTimeDelta);

private:
	_bool							m_IsMovingUP = { TRUE };
	_bool							m_IsKirbyExistence = { FALSE };

	KIRBYHP_STATE					m_ePreState = { KIRBYHP_NONE };
	KIRBYHP_STATE					m_eCurState = { KIRBYHP_NONE };

	// 마스킹을 위한 텍스쳐
	CTexture*					m_pTexMask = { nullptr };


#pragma region 피통이 까이거나, 회복되는 로직에 사용되는 변수
	// 레벨이 넘어가거나 할때, true로 만든다.
	_bool						m_bInitializeHp = { true };
	// 진짜 실제 HP 비율
	_float						m_fCurHpRatio = { 0.f };
	// 전틱 진짜 실제 HP 비율
	_float						m_fPreHpRatio = { 0.f };
	// 분홍 피통
	_float						m_fHpRatio = { 0.f };
	// 노란 피통
	_float						m_fSlowHpRatio = { 0.f };
	// 피가 닳거나 또는 회복되었을 때 틱당 이동해야하는 비율을 계산한 값이다.
	_float						m_fDeltaRatio = { 0.f };
	_bool						m_bDeltaRatio = { true };

	// 현재 피통의 상황을 정의하는 불 값
	_bool						m_isHealing = { false };
	_bool						m_isDamage = { false };

	// 피가 닳거나, 회복되었을 때 대기하는 시간
	_float						m_fDamageHoleTime = { 0.f };
	_float						m_fHealHoleTime = { 0.f };
#pragma endregion

#pragma region 깜빡임을 나타내거나, 이동, 쉐이킹을 표현하는 변수들
	// 깜빡임을 셰이더에 던져서 표현하는 변수
	_float						m_fAlarmColor = { 0.f };

	_float						m_fAlarmTime = { 0.f };
	_bool						m_bAlarm = { false };

	// 쉐이킹 담당 변수
	_bool						m_bShakingTrigger = { true };
	_bool						m_bShaking = { false };
	_float						m_fShakingTime = { 0.f };
	_float						m_fShakingAcc = { 0.f };

	// 이동할 때, 다시 원래 자리로 돌아오기 위한 변수
	_float						m_fSaveMyX = { 0.f };
	_float						m_fSaveMyY = { 0.f };
	_float						m_fAmplitude = { 0.f };
#pragma endregion

	// 아무 신호가 없을 경우, 돌아가는 타이머
	_float						m_fIdleTime = { 0.f };
	_bool						m_bCustomRenderHpbar = { false };

	_bool						m_bRenderHpbar = { false };
	_float						m_fAlpha = { 0.f };

	_float2						m_vFontPos = { 0.f, 0.f };
	_float						m_fFontSavePosX = { 0.f };
	_float						m_fTimeDelta = { 0.f };




public:
	static CHUD_KirbyStatus*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};


END