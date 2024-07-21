#pragma once
#include "Client_Defines.h"
#include "PhysXObject.h"
   
BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
 
class CLightBugEffect final : public CPhysXObject
{
private:
	CLightBugEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLightBugEffect(const CLightBugEffect& rhs);
	virtual ~CLightBugEffect() = default;

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
	CTexture*		m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	_float			m_fTimeDelta = { 0.f };
	_float			m_fShaderTime = { 0.f };
	_float			m_fAlpha = { 1.f };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

	void Billboarding();

public:
	static CLightBugEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END