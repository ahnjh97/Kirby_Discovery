#include "EventCallBack.h"
#include "GameInstance.h"

void CEventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; i++) {
        if (nullptr == m_pPlayerActor)
            continue;
  
        if (pairs[i].otherActor ==  m_pPlayerActor && IsActorInTriggerList(pairs[i].triggerActor)) {
            if (m_iTriggerType == 1)
            {
                auto functionIter = m_TriggerFuncs.find(m_iTriggerType);
                if (functionIter != m_TriggerFuncs.end())
                    functionIter->second(m_iTriggerIndex);
            }
            else
            {
                if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND)
                {
                    auto functionIter = m_TriggerFuncs.find(m_iTriggerType);
                    if (functionIter != m_TriggerFuncs.end())
                        functionIter->second(m_iTriggerIndex);
                }
                else if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_LOST)
                {
                    auto exitFuncIter = m_ExitFuncs.find(m_iTriggerType);
                    if (exitFuncIter != m_ExitFuncs.end())
                        exitFuncIter->second();
                    //MSG_BOX(TEXT("Touch Lost"));
                }
            }
 
        }
    }
}

void CEventCallBack::Clear_EventCallBack()
{
    if (!m_Triggers.empty())
    {
        for (auto& tuple : m_Triggers)
        {
            if (nullptr != get<0>(tuple))
                get<0>(tuple)->release();
        }
    }
    m_Triggers.clear();
    m_TriggerFuncs.clear();
    m_ExitFuncs.clear();
}

_bool CEventCallBack::IsActorInTriggerList(PxActor* pRigidActor)
{
    if (m_Triggers.empty())
        return false;

    for (auto& tuple : m_Triggers) {
        if (get<0>(tuple) == pRigidActor) {
            m_iTriggerType = get<1>(tuple);
            m_iTriggerIndex = get<2>(tuple);
            return true;
        }
    }

    return _bool();
}
