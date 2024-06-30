#include "stdafx.h"
#include "EventCenter.h"

IMPLEMENT_SINGLETON(CEventCenter)

void CEventCenter::Initialize()
{
	m_pListeners = new priority_queue<LISTENER, vector<LISTENER>, Ascending>[KEVENT_END];
}

HRESULT CEventCenter::Subscribe(KIRBY_EVENT eEvent, CGameObject* pObj, function<void(CGameObject*)> func, _uint iPriority)
{
	if (KEVENT_END <= eEvent)
		return E_FAIL;

	m_pListeners[eEvent].emplace(LISTENER{ iPriority, func, pObj });
	//Safe_AddRef(pObj);

	return S_OK;
}

HRESULT CEventCenter::Unsubscribe(KIRBY_EVENT eEvent, CGameObject* pObj)
{
	if (KEVENT_END <= eEvent)
		return E_FAIL;

	auto& Listeners = m_pListeners[eEvent];
	priority_queue<LISTENER, std::vector<LISTENER>, Ascending> newQueue;

	while (!Listeners.empty())
	{
		LISTENER curListener = Listeners.top();
		Listeners.pop();
		if (pObj != curListener.pObj)
			newQueue.push(curListener);
	}

	m_pListeners[eEvent] = move(newQueue);

	return S_OK;
}

HRESULT CEventCenter::Unsubscribe(CGameObject* pObj)
{
	for (size_t i = (KIRBY_EVENT)0; i < KEVENT_END; ++i)
	{
		auto& Listeners = m_pListeners[i];
		priority_queue<LISTENER, std::vector<LISTENER>, Ascending> newQueue;

		while (!Listeners.empty())
		{
			LISTENER curListener = Listeners.top();
			Listeners.pop();


			if (pObj != curListener.pObj)
				newQueue.push(curListener);
		}

		m_pListeners[i] = move(newQueue);
	}


	return S_OK;
}

HRESULT CEventCenter::Notify(KIRBY_EVENT _eEvent, CGameObject* _pNotifier)
{
	if (KEVENT_END <= _eEvent)
		return E_FAIL;

	//현재 이벤트의 리스너들 복사
	priority_queue<LISTENER, vector<LISTENER>, Ascending> Listeners = m_pListeners[_eEvent];

	////하나씩 꺼내 먹는다.
	while (!Listeners.empty())
	{
		LISTENER curListener = Listeners.top();
		if(curListener.pObj != nullptr)
		//Listeners에 저장되어 있던 func 호출
		Listeners.top().func(_pNotifier);
		Listeners.pop();
	}

	return S_OK;
}


void CEventCenter::Free()
{

	for (size_t i = (KIRBY_EVENT)0; i < KEVENT_END; ++i)
	{
		if (m_pListeners[i].empty())
			continue;

		while (!m_pListeners[i].empty())
		{
			LISTENER curListener = m_pListeners[i].top();
			//Safe_Release(curListener.pObj);
			m_pListeners[i].pop();
		}
	}

	Safe_Delete_Array(m_pListeners);

	__super::Free();
}
