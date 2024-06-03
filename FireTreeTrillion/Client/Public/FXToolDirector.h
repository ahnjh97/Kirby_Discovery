#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CEffect;
END

BEGIN(Client)
#ifdef _DEBUG
using namespace ImGui;
#endif

class CFXToolDirector final :  public CGameObject
{
#ifdef _MY_DEBUG
	friend class CMultiEffect;
#endif

	enum SELECTED {SELECTED_SINGLE_FX, SELECTED_PARTICLE_FX, SELECTED_MULTI_FX, SELECTED_END};

private:
	CFXToolDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFXToolDirector(const CFXToolDirector& rhs);
	virtual ~CFXToolDirector() = default;

public:
	//이펙트 생성
	void Make_Effect(SINGLE_FX_DATA& _FXData);
	void Make_Effect(MULTI_FX_DATA& _FXData);
	void Make_Effect(PARTICLE_DATA& _FXData);

	//이펙트 찾기
	CEffect* Find_Effect(string strEffectName);
	
	HRESULT Save_AllEffect();
	HRESULT Save_Effect(CEffect* pEffect, const wstring& strFileName);
	HRESULT Save_Particle(CEffect* pEffect, const wstring& strFileName);
	HRESULT Save_MultiEffect(CEffect* pEffect, const wstring& strFileName);

	HRESULT Load_AllEffect();
	HRESULT Load_Effect(path FilePath, _Out_ SINGLE_FX_DATA* pData);
	HRESULT Load_Effect(path FilePath, _Out_ PARTICLE_DATA* pData);
	HRESULT Load_Effect(path FilePath, _Out_ MULTI_FX_DATA* pData);

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float _fTimeDelta) override;
	virtual void Late_Tick(_float _fTimeDelta) override;
	virtual HRESULT Render() override;
#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

private:

	//이펙트 추가 시 버퍼
	_int m_iAddingFXBufferIdx = { 0 };
	vector<char*> m_FXBufferList;

	//이펙트 추가 시 텍스쳐
	_int m_iAddingFXTexIdx = { 0 };
	vector <char*> m_FXTexList;

	//이펙트 추가 시 마스크 텍스쳐
	_int m_iAddingFXMaskTexIdx = { 0 };
	vector <char*> m_FXMaskTexList;

	_int m_iAddingInstanceNum = { 10 };

/*이펙트*/

	//존재하는 이펙트 모음
	vector<CEffect*> m_FXs;
	_int m_iSelectedFXIdx = { -1 };

/*복합 이펙트*/

	//존재하는 복합 이펙트 모음
	vector<CEffect*> m_MultiFXs;
	_int m_iSelectedMultiFXIdx = { -1 };

	SELECTED m_eSelected = { SELECTED_END };


	//이펙트 편집 (기본)
	string m_curFXName = { "NONE" };
	_bool m_bLooping = { false };

	_int m_iCurFXPassIdx = { 0 };
	_int m_iCurFXTexIdx = { 0 };
	_int m_iCurFXMaskTexIdx = { 0 };
	_int m_iCurRenderGroup = { 2 };

	//수명
	//_float m_fDuration = { 1.f };
	_float m_fLifetime[2] = { 0.f, 1.f };


	//파티클용 세팅 변수
	_float m_vCenter[3] = { 1.f, 1.f, 1.f };
	_float m_vRange[3] = { 1.f, 1.f, 1.f };

	_float m_vRotation[3] = { 1.f, 1.f, 1.f };
	_float m_vRotationRandomOffset[3] = { 1.f, 1.f, 1.f };
	_float m_vScale[3] = { 1.f, 1.f, 1.f };
	_float m_vScaleRandomOffset[3] = { 1.f, 1.f, 1.f };
	_float m_vDir[3] = { 1.f, 1.f, 1.f };
	_float m_vDirRandomOffset[3] = { 1.f, 1.f, 1.f };
	_float m_vColor[3] = { 1.f, 1.f, 1.f };
	_float m_vColorRandomOffset[3] = { 1.f, 1.f, 1.f };

	_float m_vPivot[3] = { 1.f, 1.f, 1.f };


	// 이펙트 키프레임 편집(단일)

	//크자이
	_float m_vEditPos[3] = { 0.f, 0.f, 0.f };
	_float m_vEditRot[3] = { 0.f, 0.f, 0.f };
	_float m_vEditScale[3] = { 1.f, 1.f, 1.f };

	//색상
	_float m_vEditRColor[3] = { 1.f, 0.f, 0.f };
	_float m_vEditGColor[3] = { 0.f, 1.f, 0.f };
	_float m_vEditBColor[3] = { 0.f, 0.f, 1.f };

	_float m_fEditAlpha = { 1.f };
	_float m_fEditMaskThreshold = { 1.f };


	//**** 값을 복사하기 위해 저장하는 임시 변수들 ****//
	
	//임시로 복사 정보를 저장한다.
	SINGLE_FX_DATA m_TempCopySingleFXData = {};
	MULTI_FX_DATA m_TempCopyMultiFXData = {};
	PARTICLE_DATA m_TempCopyInstanceData = {};


	//현재 키프레임의 easing 플래그
	_int m_iCurEasing = { 0 };
	vector<char*> m_Easing =
	{
		"EASE_LINEAR", "EASE_IN", "EASE_IN_FAST",
		"EASE_OUT", "EASE_OUT_FAST", "EASE_INOUT", "EASE_INOUT_FAST"
	};



/*플레이 바 변수*/

	_bool m_bOpenKFPopup = { false };
	_bool m_bLoopingBar = { false };
	_bool m_bPlayingBar = { false };

	_float m_fTotalPlayDuration = { 1.f };
	_float m_fCurPlayDuration = { 0.f };

	//추가하는 키프레임
	_float	m_vKFPopupValue[3] = { 0.f, 0.f, 0.f };
	_int	m_eKFPopupEasing = { -1 };
	//_bool	m_bMakeKFPopupToFront = { false };
	_bool m_bOpenKeyframeEditor = { false };

	KF_PROPERTY m_eSelectedProperty = { KF_END };
	_int		m_iSelectedKFIdx = { -1 };

	void Render_FXHierarchy();

	void Render_FXProperty();

	void Render_FXPlayBar(_float _fTimeDelta);
	void MakeBar_SingleFXProperty(_float _fTimeDelta, _float _fWidth);
	void MakeBar_ParticleFXProperty(_float _fTimeDelta, _float _fWidth);
	void MakeBar_MultiFXProperty(_float _fTimeDelta, _float _fWidth);
	
	void Make_KeyframeList(_float _fWidth, _float _fInitialYPos, CEffect* _pCurFX, KF_PROPERTY _eRenderProperty);


	void Render_MultiFXHierarchy();
	void Render_AxisLines();

	//이펙트용 컴포넌트 준비 작업
	HRESULT	Ready_FXPrototypeVector();
	//컴포넌트 맵에서 검색 문자열과 동일한 친구들을 찾아 벡터에 저장한다.
	void Ready_Ingredient(wstring wstrSearchTag, vector<char*>* vecCombo, CComponent_Manager::PROTOTYPES* comMap);

	HRESULT Add_Components();


#ifdef _DEBUG
	inline void SetupImGuiStyle(bool bStyleDark_, float alpha_)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.FrameRounding = 8.f;
		style.GrabRounding = 8.f;
		style.TabRounding = 8.f;


		style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		//style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		//style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		//style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		//style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
		//style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		//style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		//style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

		if (bStyleDark_)
		{
			for (int i = 0; i <= ImGuiCol_COUNT; i++)
			{
				ImVec4& col = style.Colors[i];
				float H, S, V;
				ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

				if (S < 0.1f)
				{
					V = 1.0f - V;
				}
				ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
				if (col.w < 1.00f)
				{
					col.w *= alpha_;
				}
			}
		}
		else
		{
			for (int i = 0; i <= ImGuiCol_COUNT; i++)
			{
				ImVec4& col = style.Colors[i];
				if (col.w < 1.00f)
				{
					col.x *= alpha_;
					col.y *= alpha_;
					col.z *= alpha_;
					col.w *= alpha_;
				}
			}
		}
	}

#endif

public:
	static CFXToolDirector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END