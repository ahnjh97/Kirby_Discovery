#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
END

BEGIN(Client)

class CBulbFlare final : public CGameObject
{
private:
	CBulbFlare(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBulbFlare(const CBulbFlare& rhs);
	virtual ~CBulbFlare() = default;

public:
	virtual HRESULT Initialize_Prototype()				override;
	virtual HRESULT Initialize(void* pArg)				override;
	virtual _int	Tick(_float fTimeDelta)				override;
	virtual void	Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT Render()							override;

	void	Set_Position(_float4 vPos, _bool bStart)
	{
		m_bStart = bStart;
		vPos.y += 1.5f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	}

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float	m_fAlpha = { 0.f };
	_float	m_fBigAlpha = { 0.f };
	_float	m_fTurn = { 0.f };
	_float	m_fScale = { 0.f };
	_float	m_fBigScale = { 0.f };
	_float	m_fTime = { 0.f };

	_bool	m_bStart = { false };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void	Effect_Billboard(_float fTimeDelta);

public:
	static CBulbFlare* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)				override;
	virtual void Free()									override;
};

END