#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CEffect;
END

BEGIN(Client)
using namespace ImGui;

class CFXToolDirector final :  public CGameObject
{
private:
	CFXToolDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFXToolDirector(const CFXToolDirector& rhs);
	virtual ~CFXToolDirector() = default;

public:
	//이펙트 생성
	void Make_Effect(SINGLE_FX_DATA& EffectData);
	void Make_Effect(MULTI_FX_DATA& EffectData);

	//이펙트 찾기
	CEffect* Find_Effect(string strEffectName);

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual _int Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void	Render_IMGUI() override;

private:

	//이펙트 추가 시 버퍼
	_int m_iAddingFXBufferIdx = { 0 };
	vector<char*> m_FXBufferList =
	{
	"Rect"
	};

	//이펙트 추가 시 텍스쳐
	_int m_iAddingFXTexIdx = { 0 };
	vector <char*> m_FXTexList =
	{
	};

	//이펙트 추가 시 마스크 텍스쳐
	_int m_iAddingFXMaskTexIdx = { 0 };
	vector <char*> m_FXMaskTexList =
	{
	};


/*단일 이펙트*/

	//존재하는 단일 이펙트 모음
	vector<CEffect*> m_SingleFXs;
	_int m_iSelectedSingleFXIdx = { -1 };

	string m_curFXName = { "NONE" };
	_bool m_bLooping = { false };

	//수명
	_float m_fDuration[2] = { 0.f, 1.f };
	_float m_fLifeTime[2] = { 0.f, 1.f };


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

	//_uint m_eEditEasing = { EASE_LINEAR };
	
	//현재 키프레임의 easing 플래그
	_int m_iCurEasing = { 0 };
	vector<char*> m_Easing =
	{
		"EASE_LINEAR", "EASE_IN", "EASE_IN_FAST",
		"EASE_OUT", "EASE_OUT_FAST", "EASE_INOUT_CUBIC", "EASE_INOUT_SINE"
	};


/*복합 이펙트*/

	//존재하는 복합 이펙트 모음
	vector<CEffect*> m_MultiFXs;
	_int m_iSelectedMultiFXIdx = { -1 };
	//_int m_iEffectIdxInComposition = { -1 };

/*플레이 바 변수*/

	_bool m_bOpenKFPopup = { false };
	_bool m_bLoopingBar = { false };
	_bool m_bPlayingBar = { false };

	_float m_fTotalPlayDuration = { 1.f };
	_float m_fCurPlayDuration = { 0.f };


	void Render_FXHierarchy();


	HRESULT Add_Components();

public:
	static CFXToolDirector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END