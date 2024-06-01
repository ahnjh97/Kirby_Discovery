#pragma once
#include "GameObject.h"
#include "VIBuffer_Rect.h"

//BEGIN(Client)
//class CHUD;
//END

BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
protected:
	enum SHADER_PS
	{
		PS_DEFAULT, PS_ALPHABLEND,
		//PS_WHITETOBLACK, PS_WHITETOBLACKALPHA,
		//PS_WHITETOCYAN, PS_WHITETORED, PS_WHITETOMINT, PS_WHITETOPINK, PS_BLACKALPHA
	};

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		wstring		wstrUITag = { TEXT("") };
		_float2		vSize, vCenter, vPos = { 0.f, 0.f };
		_float		fRaito = { 0.f };
	}UIOBJ_DESC;

protected:
	CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject(const CUIObject& rhs);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

public:
	UIOBJ_DESC		Get_UIObj_Desc() const { return m_UIObjDesc; }
	void			Set_UIObj_Desc(UIOBJ_DESC _UIDesc) { m_UIObjDesc = _UIDesc; }

	_float2			Get_pos2D() const { return m_position2D; }
	constexpr _bool	Get_IsRender() const noexcept { return m_bIsRender; }
	void			Set_IsRender(_bool _isRender) { m_bIsRender = _isRender; }

protected:
	vector<CUIObject*>	m_vecUIObj;
	UIOBJ_DESC			m_UIObjDesc{};

	// 2D UI 처리용
	_float2				m_size2D, m_position2D, m_WindowSize2D;
	_float2				m_Ratio2D;
	_float4x4			m_ViewMatrix, m_ProjMatrix;

	// 상황에 따른 Render 처리해주는 bool값
	_bool				m_bIsRender = false;

	CShader*			m_pShaderCom = { nullptr };
	CVIBuffer_Rect*		m_pVIBufferCom = { nullptr };
	CTexture*			m_pTextureCom = { nullptr };

public:
	virtual CGameObject* Clone(_uint iLevelIndex, void* pArg) { return nullptr; }
	virtual void		 Free() override;

};

END

