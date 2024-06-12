#pragma once
#include "GameObject.h"
#include "VIBuffer_Rect.h"

BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
protected:
	enum UI_TYPE { UI_TEXTURE, UI_FONT, UI_NONE };
	enum UI_STATE { UI_LAYER, UI_GROUP, UI_END };
	enum FONT_TYPE { FONT_KIRBYFORM, FONT_STARPOINT, FONT_SCRIPT, FONT_NONE };

	enum UI_PROJ { PROJ_ORTHO, PROJ_PERSPEC, PROJ_NONE };
	enum GROUP_TYPE { GROUP_ALL, GROUP_SELECT, GROUP_NONE };

protected:
	enum UI_ANIMSTATE { ANIM_LOOP, ANIM_ONCE, ANIM_PAUSE, ANIM_END };
	enum ANIM_TYPE { ANIM_SCALE, ANIM_TRANS, ANIM_ROTATE, ANIM_NONE };
	
	enum SHADER_PS //셰이더 옵션
	{
		PS_DEFAULT, PS_ALPHABLEND,
		//PS_WHITETOBLACK, PS_WHITETOBLACKALPHA,
		//PS_WHITETOCYAN, PS_WHITETORED, PS_WHITETOMINT, PS_WHITETOPINK, PS_BLACKALPHA
	};

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		UI_TYPE		eUIType = { UI_NONE };
		UI_PROJ		eUIProj = { PROJ_NONE };
		wstring		wstrUITag = { TEXT("") };

		_float3		vCenter, vSize, vPos, vDegree = { 0.f, 0.f, 0.f };
		_int		iTexIndex = { 0 };
		
		FONT_TYPE	eFontType = { FONT_NONE };
		wstring		wstrText = { TEXT("") };
		_float3		vColorRGB = { 1.f, 1.f, 1.f };
		_float		fAlpha = { 1.f };
	}UIOBJ_DESC;

	typedef struct : public CUIObject::UIOBJ_DESC
	{
		//재생 상태
		UI_ANIMSTATE	eUIAnimState = { ANIM_END }; //재생 모드
		_float			fAnimFPS = { 0.f }; //초당 속도
		
		//애니메이션 타이밍
		_float			fPreFrame, fCurFrame = { 0.f }; //이전, 현재 프레임
		_float			fStartFrame, fEndFrame = { 0.f }; //첫, 끝 프레임
		_float			fFrameAcc = { 0.f }; //누적 시간

		//키프레임 정보
		ANIM_TYPE		eUIAnimType = { ANIM_NONE }; //변환 타입
		_float3			vScale, vTrans, vRotate = { 0.f, 0.f, 0.f };
		_float			fDuration = { 0.f }; //총 길이

		//애니메이션 상태
		string			strAnimTag = { "" }; 
	}UIANIM_DESC;

#pragma region Getter/Setter

public:

	UIOBJ_DESC		Get_UIObj_Desc() const { return m_UIObjDesc; }
	void			Set_UIObj_Desc(UIOBJ_DESC _UIDesc) { m_UIObjDesc = _UIDesc; }

	_uint			Get_TexIndex() { return m_iTexIndex; }
	void			Set_TexIndex(_uint _iTexIndex) { m_iTexIndex = _iTexIndex; }

	void			Set_LayerUITag(wstring _wstrUITag) { m_UIObjDesc.wstrUITag = _wstrUITag; }

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

	ID3D11RenderTargetView*		m_pRTV = { nullptr };
	ID3D11Texture2D*			m_pTexture2D = { nullptr };

	UIOBJ_DESC					m_UIObjDesc{};
	UI_TYPE						m_eUIType = { UI_NONE };
	UI_PROJ						m_eUIProj = { PROJ_NONE };
	UIANIM_DESC					m_UIAnimDesc{};
	
	_uint						m_iTexIndex = { 0 };
	_float4x4					m_ViewMatrix, m_ProjMatrix;
	_float4						m_vColorRGBA = { 0.f, 0.f, 0.f, 1.f };

	//Shader 원시데이터용
	_float3						m_vColorRGB = { 1.f, 1.f, 1.f };
	_float						m_fAlpha = { 1.f };

	_bool						m_bIsRender = false;

	vector<CUIObject*>			m_LayerUIs;
	vector <vector<CUIObject*>>	m_GroupUIs;

	vector<CUIObject*>			m_HUDs;
	
public:
	virtual CGameObject* Clone(_uint iLevelIndex, void* pArg) { return nullptr; }
	virtual void		 Free() override;

};

END

