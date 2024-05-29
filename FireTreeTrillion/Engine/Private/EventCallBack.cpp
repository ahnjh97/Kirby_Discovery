#include "EventCallBack.h"
#include "GameInstance.h"

void CEventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; i++) {
        // 트리거 조건을 확인하고 인덱스를 설정
        if (nullptr != m_pPlayerActor)
        {
            if (pairs[i].otherActor ==  m_pPlayerActor && IsActorInTriggerList(pairs[i].triggerActor)) {
                //MSG_BOX(TEXT("trigger"));
            }
        }
    }
}

_bool CEventCallBack::IsActorInTriggerList(PxActor* pRigidActor)
{
    if (m_Triggers.empty())
        return false;

    for (auto& tuple : m_Triggers) {
        if (get<0>(tuple) == pRigidActor) {
            auto iter = m_TriggerFunctions.find(get<1>(tuple));

            wstring wstrDebug = TEXT("TYPE: ") + to_wstring(get<1>(tuple)) + TEXT(", INDEX: ") + to_wstring(get<2>(tuple));
            //MSG_BOX(wstrDebug.c_str());

            if (iter != m_TriggerFunctions.end()) {
                iter->second(get<2>(tuple));
                return true;
            }
        }
    }

    return _bool();
}
