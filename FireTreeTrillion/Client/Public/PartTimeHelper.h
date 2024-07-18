#pragma once
#include "Client_Defines.h"

BEGIN(Client)

enum class PARTTIME_ITEM { CAKE, TOMATO, DRINK, BURGER, ITEM_END };

/// PartTimeHelper가 하는 일
///1. 아이템 관할하는 곳입니다.
///	- 와들디가 요구하는 아이템을 생성하는 곳입니다.
///	- 와들디 UI에 랜덤생성된 아이템을 넘깁니다.
///2. 게임 성적 o, x 개수 카운팅
///	- 와들디가 요구하는 아이템에 따른 커비가 선택한 아이템을 체크하는 로직이 있습니다.
///	- 맞을 경우, 해당 아이템 값(enum)에 따라 PartTimeFood 클래스 모델을 교체
class CPartTimeHelper final : public CBase
{
	DECLARE_SINGLETON(CPartTimeHelper)

public:
	enum TYPE { GAMEOVER, OVER, START, TYPE_END };
private:
	CPartTimeHelper();
	virtual ~CPartTimeHelper() = default;

public: 
	void				Register_FirstDee(class CHungryDee* pDee);
	void				Register_PartTimerKirby(class CPartTimerKirby* pKirby);
	void				Register_UI(class CUI_PartTime* pUI);
	void				Register_PartTimeResult(class CUI_PartTimeResult* pUI);
	void				Register_Camera(class CCamera_Main* pCamera);
	void				NotifyObserver();

	//헝그리 디의 총 줄선 수를 체크, 관리
	_int				Get_CurDeeWaitingNum() { return m_iDeeWaitingNum; }
	void				Add_WaitingNum() { m_iDeeWaitingNum++; }

	// 이펙트
	void				Set_EffectPos(_float3 vPos) { m_vEffectPos = vPos; }
	_float3				Get_EffectPos() const { return m_vEffectPos; }

	void				Set_TimeAttack(_bool bTimeAttack) { m_bTimeAttack = bTimeAttack; }
	_bool				Get_TimeAttack() const { return m_bTimeAttack; }

	// 문제 냅니다.
	void				Make_RandomItem();
	// 문제 잘 풀었는지 검사합니다.
	_bool				Check_Item(PARTTIME_ITEM eITEM);
	// 현 문제의 답안지를 공개합니다.
	PARTTIME_ITEM		Get_PartTimeItem() const { return m_eFood; }

	// 게임 흐름에 따른 이벤트들을 관할합니다.
	_bool				Handle_GameStart();
	_bool				Handle_GameOver();
	void				Handle_UI(TYPE eContent);

private:
	class CCamera_Main*			m_pCamera			 = nullptr;
	class CHungryDee*			m_pHungryDee		 = nullptr;
	class CPartTimerKirby*		m_pPartTimerKirby	 = nullptr;
	class CUI_PartTime*			m_pUI_PartTime		 = nullptr;
	class CUI_PartTimeResult*	m_pUI_PartTimeResult = nullptr;

	PARTTIME_ITEM				m_eFood	= PARTTIME_ITEM::CAKE;
	_float						m_fScore = 0.f;
	_int						m_iDeeWaitingNum = { 0 };
	_float3						m_vEffectPos = _float3();
	_bool						m_bTimeAttack = _bool();

public:
	virtual void Free() override;

};

END