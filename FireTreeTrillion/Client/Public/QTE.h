#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END


BEGIN(Client)

class CQTE final : public CGameObject
{
public:
	enum QTETYPE { QTE_A, QTE_B, QTE_JOYSTICK, QTE_END };

	struct QTEDESC : public GAMEOBJECT_DESC
	{
		QTETYPE eType = { QTE_END };
		_float3 vOffSet = { 0.f, 0.f, 0.f };
	};



private:
	CQTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE(const CQTE& rhs);
	virtual ~CQTE() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

	void						Set_QTE_End() {
		m_iQTE_State = 2;
	}

private:
	HRESULT						Add_Components();
	void						Update_Pos(_float fTimeDelta);
	void						QTE_Animation(_float fTimeDelta);
	void						QTE_End(_float fTimeDelta);
	_bool						m_bEndEffectTrigger = { true };

private:
	_float							m_fX, m_fY, m_fSizeX, m_fSizeY;
	_float3							m_vOffSet = { 0.f, 0.f, 0.f };

	enum TEXTURE { TEXTURE_PLATE, TEXTURE_BASE, TEXTURE_BASE2, TEXTURE_END };
	CTexture*						m_pTextureCom[TEXTURE_END] = { nullptr };
	TEXTURE							m_eJoyStickControl = { TEXTURE_BASE };
	_float							m_fJoyStickControlTime = { 0.f };
	_float4x4						m_UIMatrix[TEXTURE_END];

	CVIBuffer_Rect*					m_pVIBufferCom = { nullptr };
	CShader*						m_pShaderCom = { nullptr };
	class CKirbyDump_Cut2_State*	m_pState = { nullptr };

	_float							m_fAlpha = { 0.f };
	QTETYPE							m_eQTEType = { QTE_END };
	_float							m_fRedRatio = { 0.f };

	_int							m_iQTE_State = { 0 };
	_float							m_fBaseScale = { 5.f };
	_float							m_fPlateScale = { 1.f };

	_bool							m_bClick = { false };
	_float							m_fClickRestoreTime = { 0.f };

private:
	_float4x4				m_ViewMatrix, m_ProjMatrix;


public:
	static CQTE* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void				Free() override;
};

END