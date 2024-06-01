#pragma once
#include "Base.h"
#include <PxPhysicsAPI.h>
#include <functional>
#include <iostream>

using namespace physx;

BEGIN(Engine)

class CEventCallBack : public PxSimulationEventCallback
{
public:
    void Register_Player(PxActor* pPlayerActor) { m_pPlayerActor = pPlayerActor; }
    void Register_Trigger(PxActor* pTriggerActor, _int iType, _int iIndex) { m_Triggers.emplace_back(pTriggerActor, iType, iIndex); }
    void Emplace_TriggerFunc(_int iType, function<void(_int)> func) { m_TriggerFuncs.emplace(iType, func); }
    void Emplace_ExitFunc(_int iType, function<void(void)> func) { m_ExitFuncs.emplace(iType, func); }

public:
    virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;

    virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override {}

    virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {}

    virtual void onWake(PxActor** actors, PxU32 count) override { }

    virtual void onSleep(PxActor** actors, PxU32 count) override { }
    virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {}

public:
    void Clear_EventCallBack();

private:
    _bool IsActorInTriggerList(PxActor* pRigidActor);

private:
    _int m_iTriggerType = {};
    _int m_iTriggerIndex = {};

private:
    PxActor* m_pPlayerActor = { nullptr };
    list<tuple<PxActor*, _int, _int>> m_Triggers; // 
    map<_int, function<void(_int)>> m_TriggerFuncs;
    map<_int, function<void(void)>>  m_ExitFuncs;
};

END