#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CShader;
class CTexture;
END

class CFog_Instance final : public CGameObject
{
private:
	CFog_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFog_Instance(const CFog_Instance& rhs);
	virtual ~CFog_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float	m_fAlpha = { 0.f };
	_uint	m_iRandomFog = { 0 };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();
	//void	Effect_Billboard(_float fTimeDelta);

public:
	static CFog_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};
