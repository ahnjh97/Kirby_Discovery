#pragma once
#include "Client_Defines.h"

BEGIN(Client)

enum PARTTIME_ITEM { CAKE, TOMATO, DRINK, BURGER, ITEM_END };

/// PartTimeHelper가 하는 일
///1. 아이템 관할하는 곳입니다.
///	- 와들디가 요구하는 아이템을 생성하는 곳입니다.
///	- 와들디 UI에 랜덤생성된 아이템을 넘깁니다.
///2. 게임 성적 o, x 개수 카운팅
///	- 와들디가 요구하는 아이템에 따른 커비가 선택한 아이템을 체크하는 로직이 있습니다.
///	- 맞을 경우, 해당 아이템 값(enum)에 따라 PartTimeFood 클래스 모델을 교체
///3. 타이머 재기
class CPartTimeHelper final : public CBase
{
	DECLARE_SINGLETON(CPartTimeHelper)
private:
	CPartTimeHelper();
	virtual ~CPartTimeHelper() = default;

public: 
	void	Make_RandomItem();
	_bool	Check_Item(PARTTIME_ITEM eITEM);

private:
	PARTTIME_ITEM	m_eFood = ITEM_END;
	_float			fTimeLimit = _float();

public:
	virtual void Free() override;

};

END