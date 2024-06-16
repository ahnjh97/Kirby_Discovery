#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CHUD_KirbyStatus : public CUIObject
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

	void						Compute_Player_Hp(_float fTimeDelta);

	// 마스킹을 위한 텍스쳐
	CTexture* m_pTextureMask = { nullptr };


	// 분홍색 게이지에 대한 마스킹 비율
	_float						m_fHpRatio = { 1.f };


	// 노란색 게이지에 대한 마스킹 비율
	_float						m_fHpSlowRatio = { 1.f };
	// 깜빡임을 셰이더에 던져서 표현하는 변수
	_float						m_fAlarmColor = { 0.f };

	_float						m_fAccDamageTime = { 0.f };
	//_bool						m_bInitializeRatio = { true };
	_bool						m_bComputeDeltaGauge = { true };

	_float						m_fDistanceGauge = { 0.f };

	_float						m_fAlarmTime = { 0.f };
	_bool						m_bAlarm = { false };

	_bool						m_bShakingTrigger = { true };
	_bool						m_bShaking = { false };
	_float						m_fShakingTime = { 0.f };
	_float						m_fShakingAcc = { 0.f };

	_float						m_fSaveMyY = { 0.f };
	_float						m_fAmplitude = { 0.f };




public:
	static CHUD_KirbyStatus* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void				Free() override;
};


END