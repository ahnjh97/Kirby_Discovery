#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "Dialog.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CUI_MessageWindow : public CUIObject
{
public:
	struct MESSAGE_DESC
	{
		wstring wstrNPC = { TEXT("") };
		wstring wstrFontTag = { TEXT("") };
		_float2	vFontPos = { 0.f, 0.f };
		_float4	vFontRGBA = { 0.f, 0.f, 0.f, 0.f };

		_float2 vFontSize = { 0.f, 0.f }; //원본 사이즈
		_float2 vFontScale = { 0.f, 0.f }; //원본대비 키울 스케일 비율
		_float  fRadian = { XMConvertToRadians(0.f) };

		_float fDisplayTime = { 0.f }; //출력 시간
		_float fElapsedyTime = { 0.f }; //경과 시간

		vector<wstring> vecMsg;

		//struct TITLE_DESC
		//{
			wstring wstrTitleTag = { TEXT("") };
			wstring wstrTitleText = { TEXT("") };
			_float2	vTitlePos = { 0.f, 0.f };
			_float4	vTitleRGBA = { 0.f, 0.f, 0.f, 0.f };

			_float2 vTitleSize = { 0.f, 0.f }; //원본 사이즈
			_float2 vTitleScale = { 0.f, 0.f }; //원본대비 키울 스케일 비율
		//};
		//TITLE_DESC tTitleDesc{};

		//struct HIGHLIGHT_DESC
		//{
			wstring wstrHighlightText = { TEXT("") };
			_float4	vHighlightRGBA = { 0.f, 0.f, 0.f, 0.f };
			_float2 vHighlightScale = { 0.f, 0.f };
			vector<wstring> vecHighlight;
		//};
		//HIGHLIGHT_DESC tHighlightDesc{};
	};

public:
	enum TEX_MESSAGEWINDOW { TEXMW_BASE, TEXMW_BTNBASE, TEXMW_NONE };
	enum TEX_MWTYPE { TYPE_ELFILIN, TYPE_DEFAULT, TYPE_BOSS, TYPE_NONE };
	
	enum MESSAGEWINDOW_STATE { WINDOW_IDLE, WINDOW_HIDE, WINDOW_SHOW, WINDOW_NONE };
	enum MWFONT_TYPE { MWFONT_TITLE, MWFONT_TEXT, MWFONT_NONE };

private:
	CUI_MessageWindow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_MessageWindow(const CUI_MessageWindow& rhs);
	virtual ~CUI_MessageWindow() = default;

#pragma region GETTER/SETTER
public:
	MESSAGEWINDOW_STATE Get_MWindowState() { return m_eCurState; }
	void Reset_MessageIndex(CGameObject* pObj) { m_iCurMessageIndex = 0; } 
#pragma endregion

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;
	void						Show_DialogMessage();

#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif

private:
	HRESULT						Add_Transform(void* _pArg);
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

	HRESULT						Display_Message(_float _fTimeDelta);
	HRESULT						Render_Message();
	void						Split_Message();
	_float2						Repose_Fonts(_float2 fontPos, wstring wstrHighlightMsg);

	// MessageWindow와 관련있는 이벤트를 처리하기 위한 함수들입니다.
	void						OnEvent();
	void						Start_Message(CGameObject* pObj = nullptr);
	void						Event_Tick(_float fTimeDelta);
	void						Ready_FadeOut();

public:
	static CUI_MessageWindow*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	CTransform*					m_pTransCom[TEXMW_NONE] = { nullptr };
	CTexture*					m_pTexCom[TEXMW_NONE] = { nullptr };
	CTexture*					m_pTexClaw = { nullptr };

	class CUI_BtnIcon*			m_pUIBtn = { nullptr };
	
	MESSAGEWINDOW_STATE			m_eCurState = { WINDOW_NONE };
	MESSAGE_DESC				m_tMessageDesc{};
	vector <tuple<wstring, wstring, wstring>> m_vecSplitMsg;

	// BTN
	_float						m_fHideTime = { 0.f };
	_float3						m_vBaseScale = { 0.f, 0.f, 1.f };
	_float3						m_vBtnScale = { 0.f, 0.f, 1.f };

	// FONT
	_float						m_fElapsedTime = { 0.f }; //경과 시간
	_float						m_fDisplayTime = { 0.f }; //출력 시간
	_uint						m_iCurMessageIndex = { 0 };
	_uint						m_iCurCharIndex = { 0 };
	_uint						m_iCurCharIndexHightlight = { 0 };
	_uint						m_iCurCharIdxPostHightlight = { 0 };

	_bool						m_bSignalHightlight = false;
	_bool						m_bSignalPostHightlight = false;

	_bool						m_bEventCall = false;

	_bool						m_bNextDialog1Notified = { false };
	_bool						m_bNextDialog2Notified = { false };
	_bool						m_bLastDialogNotified = { false };
};
END