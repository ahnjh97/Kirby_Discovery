#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
END

BEGIN(Client)

class CMoon final : public CGameObject
{
private:
	CMoon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMoon(const CMoon& rhs);
	virtual ~CMoon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	//_float	m_fAlpha = { 1.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void Effect_Billboard(_float fTimeDelta);

public:
	static CMoon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END