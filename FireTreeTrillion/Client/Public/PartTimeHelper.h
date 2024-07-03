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
	enum TYPE { GAMEOVER, OVER, ETC, TYPE_END };
private:
	CPartTimeHelper();
	virtual ~ CPartTimeHelper() = default;

public: 
	void				Register_FirstDee(class CHungryDee* pDee);
	void				Register_PartTimerKirby(class CPartTimerKirby* pKirby);
	void				Register_UI(class CUI_PartTime* pUI);
	void				Register_PartTimeResult(class CUI_PartTimeResult* pUI);
	void				Register_Camera(class CCamera* pCamera);
	//void				NotifyObserver();

	// 문제 냅니다.
	void				Make_RandomItem();
	// 문제 잘 풀었는지 검사합니다.
	_bool				Check_Item(PARTTIME_ITEM eITEM);
	// 현 문제의 답안지를 공개합니다.
	PARTTIME_ITEM		Get_PartTimeItem() const { return m_eFood; }

	// 게임 흐름에 따른 이벤트들을 관할합니다.
	_bool				Handle_LunchTime();
	_bool				Handle_GameOver();
	void				Handle_UI(TYPE eContent);

private:
	class CCamera*				m_pCamera			 = nullptr;
	class CHungryDee*			m_pHungryDee		 = nullptr;
	class CPartTimerKirby*		m_pPartTimerKirby	 = nullptr;
	class CUI_PartTime*			m_pUI_PartTime		 = nullptr;
	class CUI_PartTimeResult*	m_pUI_PartTimeResult = nullptr;

	PARTTIME_ITEM				m_eFood	= PARTTIME_ITEM::CAKE;

public:
	virtual void Free() override;

};

END