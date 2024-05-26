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

    for (auto pair : m_Triggers) {
        if (pair.first == pRigidActor) {
            m_pCamSetIndexFunc(pair.second);
            return true;
        }
    }

    return _bool();
}
