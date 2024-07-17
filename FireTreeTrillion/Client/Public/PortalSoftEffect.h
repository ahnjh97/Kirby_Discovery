#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CPortalSoftEffect :
    public CGameObject
{
private:
	CPortalSoftEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPortalSoftEffect(const CPortalSoftEffect& rhs);
	virtual ~CPortalSoftEffect() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize(void* pArg) override;
	virtual _int		Tick(_float fTimeDelta) override;
	virtual void		Late_Tick(_float fTimeDelta) override;
	virtual HRESULT		Render() override;
#ifdef _DEBUG
	virtual void		Render_IMGUI()					override;
#endif


private:
	CTexture*			m_pTextureCom = { nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };

private:
	HRESULT				Add_Components();
	HRESULT				Bind_ShaderResources();

public:
	static CPortalSoftEffect*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

};

END