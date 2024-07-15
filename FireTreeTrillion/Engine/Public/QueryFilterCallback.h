#pragma once
#include "Base.h"
#include <PxPhysicsAPI.h>

using namespace physx;
BEGIN(Engine)

class CQueryFilterCallback : public PxQueryFilterCallback
{
public:
    CQueryFilterCallback(PxRigidActor* selfActor) : m_pMyActor(selfActor) {}

        virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override {
            // 자기 자신의 액터를 필터링하여 제외
            if (actor == m_pMyActor) 
                return PxQueryHitType::eNONE;
            return PxQueryHitType::eBLOCK;
        }

        virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) override {
            return PxQueryHitType::eBLOCK;
        }

private:
    PxRigidActor* m_pMyActor = { nullptr };
};

END