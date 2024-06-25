#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_PartTime : public CUIObject
{
	enum UI_NAME { TIME_BAR_BG, UI_END };
private:
	CUI_PartTime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_PartTime(const CUI_PartTime& rhs);
	virtual ~CUI_PartTime() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif

private:
	HRESULT						Add_Components();
	HRESULT						Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom);
	
	HRESULT						Bind_ShaderResources();
	void						Move_Position(_int iTextureNum);

	HRESULT						Bind_VIBuffer();
	void						Compute_Timer(_float fTimeDelta);


private:
	_bool						m_IsMovingUP = { TRUE };
	_bool						m_IsKirbyExistence = { FALSE };

	// 마스킹을 위한 텍스쳐
	array<CTexture*, 6>			m_arrTexures;
	CTexture*					m_pTexMask = { nullptr };

	// 지영이거 시작
	_float2			m_size2D, m_position2D, m_Initial2D, m_Dest2D, m_WindowSize2D, m_progress2D;
	_float2			m_SizeBar2D = _float2(1024.f * 1.3f, 128.f * 1.3f);
	_float2			m_SizeTimeBarBlank2D = _float2(1600 * .65f, 61.f * .65f);


public:
	static CUI_PartTime*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};


END