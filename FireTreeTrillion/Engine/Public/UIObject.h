#pragma once
#include "GameObject.h"
#include "VIBuffer_Rect.h"

BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
protected:
	enum UI_TYPE { UI_TEXTURE, UI_FONT, UI_NONE };
	enum UI_STATE { UI_LAYER, UI_GROUP, UI_END };
	enum GROUP_TYPE { GROUP_ALL, GROUP_SELECT, GROUP_NONE };

	enum SHADER_PS
	{
		PS_DEFAULT, PS_ALPHABLEND,
		//PS_WHITETOBLACK, PS_WHITETOBLACKALPHA,
		//PS_WHITETOCYAN, PS_WHITETORED, PS_WHITETOMINT, PS_WHITETOPINK, PS_BLACKALPHA
	};

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		UI_TYPE		eUIType = { UI_NONE };
		wstring		wstrUITag = { TEXT("") };

		_float3		vCenter, vSize, vPos = { 0.f, 0.f, 0.f };
		_float		fDegree = { 0.f };
		_int		iTexIndex = { 0 };
		
		wstring		wstrText = { TEXT("") };
		_float4		vColorRGBA = { 0.f, 0.f, 0.f, 0.f };
	}UIOBJ_DESC;

#pragma region Getter/Setter

public:
	UIOBJ_DESC		Get_UIObj_Desc() const { return m_UIObjDesc; }
	void			Set_UIObj_Desc(UIOBJ_DESC _UIDesc) { m_UIObjDesc = _UIDesc; }

	_uint			Get_TexIndex() { return m_iTexIndex; }
	void			Set_TexIndex(_uint _iTexIndex) { m_iTexIndex = _iTexIndex; }

	constexpr _bool	Get_IsRender() const noexcept { return m_bIsRender; }
	void			Set_IsRender(_bool _isRender) { m_bIsRender = _isRender; }

#pragma endregion

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

protected:
	CShader*					m_pShaderCom = { nullptr };
	CVIBuffer_Rect*				m_pVIBufferCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	//ID3D11Texture2D* m_;
	ID3D11RenderTargetView*		m_pRTV = { nullptr };

	UIOBJ_DESC					m_UIObjDesc{};
	UI_TYPE						m_eUIType = { UI_NONE };
	
	_uint						m_iTexIndex = { 0 };
	_float4x4					m_ViewMatrix, m_ProjMatrix;
	_float4						m_vColorRGBA = { 0.f, 0.f, 0.f, 1.f };

	_bool						m_bIsRender = false;

	vector<CUIObject*>			m_LayerUIs;
	vector <vector<CUIObject*>>	m_GroupUIs;
	
public:
	virtual CGameObject* Clone(_uint iLevelIndex, void* pArg) { return nullptr; }
	virtual void		 Free() override;

};

END

