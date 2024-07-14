#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CQTE_Effect final : public CGameObject
{
public:
	struct QTEEFFECTDESC : public GAMEOBJECT_DESC
	{
		_float3 vColor = { 0.f, 0.f, 0.f };
		_float3 vOffSet = { 0.f, 0.f, 0.f };
	};


private:
	CQTE_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Effect(const CQTE_Effect& rhs);
	virtual ~CQTE_Effect() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

private:
	HRESULT						Add_Components();

private:
	_float						m_fX, m_fY, m_fSizeX, m_fSizeY;

	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Rect*				m_pVIBufferCom = { nullptr };
	CShader*					m_pShaderCom = { nullptr };
	_float						m_fAlpha = { 1.f };
	_float3						m_vColor = { 0.f, 0.f, 0.f };
	_float3						m_vOffSet = { 0.f, 0.f, 0.f };

	_float						m_fScale = { 1.f };

	void						QTE_Animation(_float fTimeDelta);
	void						Update_Pos(_float fTimeDelta);

private:
	_float4x4				m_ViewMatrix, m_ProjMatrix;

public:
	static CQTE_Effect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void				Free() override;

};

END