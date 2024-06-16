
#include "EventCallBack.h"
#include "GameInstance.h"
#include "GameObject.h"
#include "Model.h"

void CEventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    if (*CGameInstance::Get_Instance()->Get_CurrentLevelID() == 6) return;
    for (PxU32 i = 0; i < count; i++) 
    {
        for (auto& controllerActor : m_vecControllers) // 컨트롤러들을 따로 받아놓아 데이터를 읽어온다.
        {
            PxActor* TriggerActor = pairs[i].triggerActor;
            if (pairs[i].otherActor == nullptr) continue;
            if (controllerActor.first == nullptr || controllerActor.second == nullptr) continue;
            
            if (controllerActor.first == pairs[i].otherActor)
            {
                // Trigger (HITBOX 혹은 ITEM)
                CComponent*  pComTrigger = static_cast<CComponent*>(TriggerActor->userData);
                if (pComTrigger == nullptr) continue;
                CGameObject* pTriggerObj = pComTrigger->Get_Object();
                if (pTriggerObj == nullptr) continue;

                // Controller
                PxController* pController = controllerActor.second;
                if (pController == nullptr) continue;
                CComponent*  pComponent = static_cast<CComponent*>(pController->getUserData());
                if (pComponent == nullptr) continue;
                CGameObject* pContrObj = pComponent->Get_Object();
                if (pContrObj == nullptr) continue;
                
                // Trigger가 그룹설정이 Passing 그룹 쪽으로 되어있다면 콜리젼 센터로 이양.
                if (pTriggerObj->Get_CollisionType() >= PASSING_GROUP)
                    CGameInstance::Get_Instance()->Add_CollisionObjects(pTriggerObj, pContrObj);
            }
        }

        // for Map's Trigger
        if (nullptr == m_pPlayerActor)
            continue;

        if (pairs[i].otherActor ==  m_pPlayerActor && IsActorInTriggerList(pairs[i].triggerActor))
        {
            m_pPlayerActor->userData;
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
                }
            }
        }
        else if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND) // For Map Deco Anim
        {
            if (pairs[i].otherActor == m_pPlayerActor && IsMapDecoAnimTrigger(pairs[i].triggerActor))
            {
                auto iter = m_TriggerToMapDecoAnimMap.find(pairs[i].triggerActor);
                if (iter != m_TriggerToMapDecoAnimMap.end())
                {
                    CModel* pMapDeco = get<0>(iter->second);
                    pMapDeco->Set_Animation(get<1>(iter->second), get<2>(iter->second), false, true);
                }
            }
        }
    }
}

void CEventCallBack::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
    for (PxU32 i = 0; i < nbPairs; i++)
    {
        const PxContactPair& cp = pairs[i];

        // 충돌 이벤트 플래그 확인
        if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) 
        {
            // 첫 번째 객체가 StaticRigid이고 두 번째 객체가 DynamicRigid인 경우
            if (pairHeader.actors[0]->is<PxRigidStatic>() && pairHeader.actors[1]->is<PxRigidDynamic>()) {
                printf("StaticRigid collided with DynamicRigid\n");
            }
            // 첫 번째 객체가 DynamicRigid이고 두 번째 객체가 StaticRigid인 경우
            else if (pairHeader.actors[0]->is<PxRigidDynamic>() && pairHeader.actors[1]->is<PxRigidStatic>()) {
                printf("DynamicRigid collided with StaticRigid\n");
            }
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
