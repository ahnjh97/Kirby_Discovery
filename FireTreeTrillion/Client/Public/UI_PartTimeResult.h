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

	void							Render_Digits();
	void							Render_TotalScore();
	void							Render_Font();

	void							Initialize_TexturePos();
	void							Set_Score(_float _fScore) { m_fScore = _fScore; }

	// 10 11    타임 숫자
	// 12 13 14 스코어 숫자
private:
	HRESULT							Add_Components();
	_int							Change_ScoreTextures(_int iNum);
	HRESULT							Bind_ShaderResources();

private:
	array<CTexture*, TEXTURECNT>	m_arrTexures;
	CTexture*						m_pTexMask = { nullptr }; // 마스킹을 위한 텍스쳐

	array<_float2,	 TEXTURECNT>	m_arrPosition; 
	array<_float2,	 TEXTURECNT>	m_arrSize;	

	_float2							m_SizeScoreBar2D = _float2(438.f, 156.f);
	_float2							m_SizeScoreResult2D = _float2(720.f, 102.f);
	_float2							m_SizeDigits2D = _float2(72.f, 72.f);
	// Score-Digits
	_float							m_fScore = _float();
	array<_int, 3>					m_arrScoreDigits;

	_float							m_fTimeDelta = _float();
	_float							m_fMoveRatio = 1.f;
	_float2							m_fMovePosition2D = _float2();
	_float							m_fSizeRatio = 0.f;
	_float2							m_fSize2D = _float2();

	_bool							m_bRenderTotalScore = false;

	// -------------- for IMGUI
	array<_float2,   TEXTURECNT>	m_arrOriginalSize;
	array<_float2,   TEXTURECNT>	m_arrSizeRatio;
	_float2							m_posTemp = _float2();

public:
	static CUI_PartTimeResult*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*			Clone(void* pArg) override;
	virtual void					Free() override;

};


END
