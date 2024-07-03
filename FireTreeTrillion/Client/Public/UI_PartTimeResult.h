#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

#define TEXTURECNT	8

BEGIN(Client)
class CUI_PartTimeResult : public CUIObject
{
private:
	CUI_PartTimeResult(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_PartTimeResult(const CUI_PartTimeResult& rhs);
	virtual ~CUI_PartTimeResult() = default;

public:
	virtual HRESULT					Initialize_Prototype()						override;
	virtual HRESULT					Initialize(void* pArg)						override;
	virtual _int					Tick(_float fTimeDelta)						override;
	virtual void					Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT					Render()									override;
#ifdef _DEBUG
	virtual void					Render_IMGUI()								override;
#endif

	void							Initialize_TexturePos();

private:
	HRESULT							Add_Components();
	HRESULT							Bind_ShaderResources();

private:
	array<CTexture*, TEXTURECNT>	m_arrTexures;

	array<_float2,	 TEXTURECNT>	m_arrPosition;
	array<_float2,	 TEXTURECNT>	m_arrSize;	

	// -------------- for IMGUI
	array<_float2,   TEXTURECNT>	m_arrOriginalSize;
	array<_float,    TEXTURECNT>	m_arrSizeRatio;


	_float2							m_SizeScoreBar2D = _float2(438.f, 156.f);
	_float2							m_SizeScoreResult2D = _float2(720.f, 102.f);
	_float2							m_SizeDigits2D = _float2(50.f, 60.f);

	CTexture*						m_pTexMask = { nullptr }; // 마스킹을 위한 텍스쳐

public:
	static CUI_PartTimeResult*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;
	virtual void					Free() override;
};


END