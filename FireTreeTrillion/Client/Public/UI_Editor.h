#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CUI_Editor : public CUIObject
{
private:
	CUI_Editor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Editor(const CUI_Editor& rhs);
	virtual ~CUI_Editor() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual void	Render_IMGUI()								override;

private:
	HRESULT			Add_Components();
	HRESULT			Bind_ShaderResources();

private:
	CShader*			m_pShaderCom = { nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };
	CTexture*			m_pTextureCom = { nullptr };

private:
	_float				m_fX, m_fY, m_fSizeX, m_fSizeY;
	_float4x4			m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;

	_float				m_fRatio = 0.f;


public:
	static CUI_Editor*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};


END