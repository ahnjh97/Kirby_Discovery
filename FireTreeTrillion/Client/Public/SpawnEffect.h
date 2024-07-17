#pragma once

#include "Client_Defines.h"
#include "PhysXObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CSpawnEffect final : public CPhysXObject
{
public:
	enum TEXTURETYPE { TYPE_DIFFUSE, TYPE_MASK, TYPE_BRUSH, TYPE_END };

	struct SPAWNEFFECT_DESC : public CGameObject::GAMEOBJECT_DESC {
		_bool	bTrail = { false };
		_float	fScale = { 0.f };
		_vector vPosition = {};
	};

private:
	CSpawnEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSpawnEffect(const CSpawnEffect& rhs);
	virtual ~CSpawnEffect() = default;

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

private:
	CShader*		m_pShaderCom = { nullptr };
	CTexture*		m_pTextureCom[TYPE_END] = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	_bool			m_bTrail = { false };
	_float			m_fTimeDelta = { 0.f };
	_float			m_fShaderTime = { 0.f };
	_float			m_fScaleTime = { 0.f };
	_float			m_fScale = { 0.f };
	_float			m_fAlpha = { 1.f };
	_vector			m_vPosition = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void Billboarding();

public:
	static CSpawnEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END