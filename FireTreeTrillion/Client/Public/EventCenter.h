#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

class CEventCenter : public CBase
{
	DECLARE_SINGLETON(CEventCenter)


public:
	struct LISTENER
	{
		_uint iPriority;
		function<void(CGameObject*)> func;
		CGameObject* pObj;
	};

private:
	CEventCenter() {};
	virtual ~CEventCenter() = default;

public:
	void Initialize();

	//원하는 이벤트가 발생했을 때 함수를 호출한다. 이벤트 이넘 + 내 주소 + 호출할 함수 바인딩 + 우선순위(기본 1)
	HRESULT Subscribe(KIRBY_EVENT eEvent, CGameObject* pObj, function<void(CGameObject*)> func, _uint iPriority = 1);

	//이벤트를 발생시킨다. 호출자의 주소를 넣어주세요(없으면 말고)
	HRESULT Notify(KIRBY_EVENT eEvent, CGameObject* pObj = nullptr);

	//특정 이벤트에 구독된 이력만 삭제한다.
	HRESULT Unsubscribe(KIRBY_EVENT eEvent, CGameObject* pObj);
	//모든 이벤트의 구독 이력을 삭제한다.
	HRESULT Unsubscribe(CGameObject* pObj);


private:
	struct Ascending
	{
		_bool operator()(const LISTENER& A, const LISTENER& B)
		{
			return A.iPriority > B.iPriority;
		}
	};

	priority_queue<LISTENER, vector<LISTENER>, Ascending>* m_pListeners = { nullptr };

	virtual void Free() override;
};

