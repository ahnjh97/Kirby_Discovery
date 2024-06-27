
#include "EventCallBack.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Model.h"

void CEventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; i++) 
    {
        // for Map's Trigger
        if (nullptr == m_pPlayerActor)
            continue;

        if (pairs[i].otherActor ==  m_pPlayerActor && IsActorInTriggerList(pairs[i].triggerActor))
        {
            if (m_iTriggerType == 1 || m_iTriggerType == 3) // 3 : LevelChanger
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
                }
            }
        }
        else if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND) 
        {
            // For Map Deco Anim
            if (pairs[i].otherActor == m_pPlayerActor && IsMapDecoAnimTrigger(pairs[i].triggerActor))
            {
                auto iter = m_TriggerToMapDecoAnimMap.find(pairs[i].triggerActor);
                if (iter != m_TriggerToMapDecoAnimMap.end())
                {
                    CModel* pMapDeco = get<0>(iter->second);
                    pMapDeco->Set_Animation(get<1>(iter->second), get<2>(iter->second), false, true);
                }
            }

            //// For Level Trigger
            //if (pairs[i].otherActor == m_pPlayerActor && IsLevelTrigger(pairs[i].triggerActor))
            //{
            //    auto iter = m_umapTrigger.find(pairs[i].triggerActor);
            //    if (iter != m_umapTrigger.end())
            //    {
            //        CModel* pMapDeco = get<0>(iter->second);
            //        pMapDeco->Set_Animation(get<1>(iter->second), get<2>(iter->second), false, true);
            //    }

            //    auto functionIter = m_TriggerFuncs.find(m_iTriggerType);
            //    if (functionIter != m_TriggerFuncs.end())
            //        functionIter->second(m_iTriggerIndex);
            //}
        }
    }
}

void CEventCallBack::Clear_EventCallBack()
{
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
