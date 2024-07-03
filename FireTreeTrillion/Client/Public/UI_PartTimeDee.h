#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

const _int g_iTextureCnt = 6;

BEGIN(Client)
class CUI_PartTimeDee : public CUIObject
{
public:
	enum TYPE { ORDER, THINKING, UI_END };

private:
	CUI_PartTimeDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_PartTimeDee(const CUI_PartTimeDee& rhs);
	virtual ~CUI_PartTimeDee() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;


#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif
	void						Change_Dialog(enum class PARTTIME_ITEM eItem);
	void						Make_RandomImg();
	void						Update_Pos(_float3 _vPosition);

	void						Set_Type(TYPE _eType) { m_eDialogTheme = _eType; }
	void						Set_Mask(_float _fMask) { m_fMask = _fMask; }
	
private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources();
	void						Setup_PosSizeColor(_int iTextureNum);

private:
	array<CTexture*, g_iTextureCnt>	m_arrTexures;
	array<_float2,	 g_iTextureCnt>	m_arrSize;
	array<_float2,	 g_iTextureCnt>	m_arrPosition;
	array<_bool,	 4>				m_arrFoodRender;

	// -------------- for IMGUI
	array<_float2,	 g_iTextureCnt>	m_arrOriginalSize;
	array<_float,	 g_iTextureCnt>	m_arrSizeRatio;
	array<_float3,	 g_iTextureCnt>	m_arrColor;

	// 음식 문제 다채롭게
	CTexture*		 m_pTexMask = { nullptr }; // 마스킹을 위한 텍스쳐
	_float			 m_fMask = _float();
	_float3			 m_vFoodColor = _float3(0.45f, 0.45f, 0.45f);
	_bool			 m_bRandomColor = false;
	_bool			 m_bRandomMask = false;
	
	_float2			 m_SizeDialog2D = _float2(406.f * 0.5f, 378.f * 0.5f);
	_float2			 m_SizeFood2D   = _float2(292.f * 0.5f, 292.f * 0.5f);
	_float4			 m_vFinPos = _float4();

	TYPE			 m_eDialogTheme = ORDER;

public:
	static CUI_PartTimeDee*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};


END