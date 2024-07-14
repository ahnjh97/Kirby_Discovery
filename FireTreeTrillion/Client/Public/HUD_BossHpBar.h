#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CHUD_BossHpBar final : public CGameObject
{
private:
	CHUD_BossHpBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_BossHpBar(const CHUD_BossHpBar& rhs);
	virtual ~CHUD_BossHpBar() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

private:
	HRESULT						Add_Components();

	void						InitializeBar(_float fTimeDelta);
	_uint						m_uInitializeBar = { 0 };

	void						Compute_MyBossHp(_float fTimeDelta);

	void						ReleaseBar(_float fTimeDelta);


private:
	_float					m_fX, m_fY, m_fSizeX, m_fSizeY;
	enum TEXTURETYPE { UI_BARPLATE, UI_BARLOW, UI_BARMIDDLE, UI_BARHIGH, UI_MASK, UI_END };
	CTexture*				m_pTextureCom = { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom = { nullptr };
	CShader*				m_pShaderCom = { nullptr };
	_float4x4				m_BarMatrix[UI_END];
	_float3					m_vColor[UI_END];
	_float					m_fAlpha = { 0.f };

	CTexture*				m_pTextureNameCom = { nullptr };
	_float4x4				m_NameMatrix;
	_float					m_fNameOriginY = { 0.f };
	_float3					m_vNameColor = { 0.f, 0.f, 0.f };
private:
	class CMonster*			m_pMyMonster = { nullptr };
	_float4x4				m_ViewMatrix, m_ProjMatrix;

	_bool					m_bInitializeRatioSet = { true };
	_float					m_fCurBossHpRatio = { 0.f };
	_float					m_fPreBossHpRatio = { 0.f };

	_bool					m_bDamage = { false };
	_float					m_fSlowMovingTime = { 0.f };
	_bool					m_bComputeRatioDelta = { true };
	_float					m_fRatioDelta = { 0.f };

	_float					m_fBossHpBar = { 0.f };
	_float					m_fBossSlowHpBar = { 0.f };

	_bool					m_bShaking = { false };
	_float					m_fShakingTime = { 0.f };
	_float					m_fShakingAcc = { 0.f };
	_float					m_fAmplitude = { 0.f };
	_float					m_fOriginY[UI_END] = { 0.f, 0.f, 0.f, 0.f, 0.f };
	_float					m_fMoveY = { 0.f };


public:
	static CHUD_BossHpBar*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

};

END