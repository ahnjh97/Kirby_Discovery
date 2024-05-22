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
	enum SELECTED {SELECTED_SINGLE_FX, SELECTED_PARTICLE_FX, SELECTED_MULTI_FX, SELECTED_END};

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
	virtual _int Tick(_float _fTimeDelta) override;
	virtual void Late_Tick(_float _fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void	Render_IMGUI() override;

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

	string m_curFXName = { "NONE" };
	_bool m_bLooping = { false };

	//수명
	//_float m_fDuration = { 1.f };
	_float m_fLifeTime[2] = { 0.f, 1.f };


	//파티클용 세팅 변수
	_float m_fRange[3] = { 1.f, 1.f, 1.f };

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




/*플레이 바 변수*/

	_bool m_bOpenKFPopup = { false };
	_bool m_bLoopingBar = { false };
	_bool m_bPlayingBar = { false };

	_float m_fTotalPlayDuration = { 1.f };
	_float m_fCurPlayDuration = { 0.f };


	void Render_FXHierarchy();

	void Render_FXProperty(_float _fTimeDelta);

	void Render_FXPlayBar(_float _fTimeDelta);
	void MakeBar_SingleFXProperty(_float _fTimeDelta);
	void MakeBar_ParticleFXProperty(_float _fTimeDelta);
	void MakeBar_MultiFXProperty(_float _fTimeDelta);
	
	void Render_MultiFXHierarchy();

	//이펙트용 컴포넌트 준비 작업
	HRESULT	Ready_FXPrototypeVector();
	//컴포넌트 맵에서 검색 문자열과 동일한 친구들을 찾아 벡터에 저장한다.
	void Ready_Ingredient(wstring wstrSearchTag, vector<char*>* vecCombo, CComponent_Manager::PROTOTYPES* comMap);

	HRESULT Add_Components();

public:
	static CFXToolDirector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END