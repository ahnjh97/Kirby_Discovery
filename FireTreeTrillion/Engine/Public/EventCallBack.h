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
    void Register_Trigger(PxActor* pTriggerActor) { m_Triggers.emplace_back(pTriggerActor); }

public:
    virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) override;

    virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override {}

    virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {}

    virtual void onWake(PxActor** actors, PxU32 count) override { }

    virtual void onSleep(PxActor** actors, PxU32 count) override { }
    virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {}

private:
    _bool IsActorInTriggerList(PxActor* pRigidActor);

private:
    PxActor* m_pPlayerActor = { nullptr };
    list<PxActor*> m_Triggers;
};

END