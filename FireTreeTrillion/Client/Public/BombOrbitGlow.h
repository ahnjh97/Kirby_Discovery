#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
class CTexture;
END

BEGIN(Client)

// 미리 15개를 생성하여, 폭탄일때만 활성화한다.
// 모든 연산은 커비가 관리하고, Orbit은 경로 위치에서 
class CBombOrbitGlow final :
    public CGameObject
{
private:
	CBombOrbitGlow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBombOrbitGlow(const CBombOrbitGlow& rhs);
	virtual ~CBombOrbitGlow() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void	Update_GlowPosition(_fvector vPos);

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	void Billboard(_float fTimeDelta);

public:
	static CBombOrbitGlow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

END