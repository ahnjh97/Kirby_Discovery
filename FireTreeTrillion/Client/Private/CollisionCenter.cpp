#include "stdafx.h"
#include "CollisionCenter.h"
#include "GameInstance.h"
#include "Kirby.h"
#include "ItemObject.h"

#define GAMEINSTANCE CGameInstance::Get_Instance()->

IMPLEMENT_SINGLETON(CCollisionCenter)

void CCollisionCenter::Initialize()
{
	// CONTENT 등록을 초기화한다.
	for (_int i = 0; i < COLLISION_END; ++i)
	{
		for (_int j = 0; j < COLLISION_END; ++j)
		{
			m_eColliderType[i][j] = CONTENT_END;
		}
	}

	// CONTENT 등록을 한다.

	// For CONTENT_BODY
	m_eColliderType[PLAYER][MONSTER] = CONTENT_BODY;

	// For CONTENT_VACUUMOBJECT
	m_eColliderType[MONSTER][MONSTER] = CONTENT_VACUUMOBJECT;

	// For CONTENT_ITEM
	m_eColliderType[PLAYER][ITEM] = CONTENT_ITEM;

	// For CONTENT_ATTACK
	m_eColliderType[HITBOX][MONSTER] = CONTENT_ATTACK;

}

void CCollisionCenter::Collision_Tick(_float fTimeDelta)
{
	GAMEINSTANCE Get_CollisionObjects(m_WaitingList);

	if (m_WaitingList.empty() == true)
		return;


	// 받아온 두개의 포인터를 사용하여 해당 타입에 맞는 콜라이더를 발동시킨다.
	for (auto& Pair : m_WaitingList)
	{
		CPhysXObject* pSrc = static_cast<CPhysXObject*>(Pair.first);
		CPhysXObject* pDst = static_cast<CPhysXObject*>(Pair.second);

		CONTENT_TYPE eType = Find_ColliderType(pSrc, pDst);

		// 등록되지않았다면 컨티뉴 한다.
		if (eType == CONTENT_END)
		{
			Safe_Release(pSrc);
			Safe_Release(pDst);
			continue;
		}

		Collision_Collider(eType, pSrc, pDst);
		Safe_Release(pSrc);
		Safe_Release(pDst);
	}

	m_WaitingList.clear();
}

CCollisionCenter::CONTENT_TYPE CCollisionCenter::Find_ColliderType(CPhysXObject* pSrc, CPhysXObject* pDst)
{
	COLLISION_TYPE eSrcCollisionType = static_cast<COLLISION_TYPE>(pSrc->Get_CollisionType());
	COLLISION_TYPE eDstCollisionType = static_cast<COLLISION_TYPE>(pDst->Get_CollisionType());

	// 둘 사이에 지정된 CONTENTTYPE을 받는다.
	CONTENT_TYPE eContentType = m_eColliderType[eSrcCollisionType][eDstCollisionType];

	// 받았는데, CONTENT_END (등록되지 않았다면) 일 경우 바꿔주고 등록되어 있었다면 그대로 리턴한다.
	eContentType = eContentType == CONTENT_END ?
		m_eColliderType[eDstCollisionType][eSrcCollisionType] : eContentType;

	return eContentType;
}

CPhysXObject* CCollisionCenter::Find_TypePtr(COLLISION_TYPE eType, CPhysXObject* pSrc, CPhysXObject* pDst)
{
	if (pSrc->Get_CollisionType() == eType)
		return pSrc;
	else if (pDst->Get_CollisionType() == eType)
		return pDst;
	
	// if not found
	return nullptr;
}

void CCollisionCenter::Collision_Collider(CONTENT_TYPE eType, CPhysXObject* pSrc, CPhysXObject* pDst)
{
	CPhysXObject* pSrcObject = static_cast<CPhysXObject*>(pSrc);
	CPhysXObject* pDstObject = static_cast<CPhysXObject*>(pDst);


	// 캐릭터 X 몬스터 몸박
	if (eType == CONTENT_BODY)
	{
		CPhysXObject* pPlayer = Find_TypePtr(PLAYER, pSrcObject, pDstObject);
		if (pPlayer == nullptr)
			return;
		CPhysXObject* pMonster = Find_TypePtr(MONSTER, pSrcObject, pDstObject);
		if (pMonster == nullptr)
			return;

		if (pMonster->Get_PhyXState() == PO_NORMAL)
		{
			// 몬스터의 상태가 노말일 때만, 서로 넉백이 발생하며, 데미지가 발생한다.
			Player_Monster_Knock_back(pPlayer, pMonster);
			Compute_Damage(pPlayer, pMonster);
		}

		pPlayer->Collision(CONTENT_BODY, pMonster);
		pMonster->Collision(CONTENT_BODY, pMonster);


	}

	// 커비가 뱉은 물체에 맞는 충돌처리
	else if (eType == CONTENT_VACUUMOBJECT)
	{
		// 둘 중 하나가 날아가게끔 한다.
		if (pSrcObject->Get_PhyXState() == PO_FLYAWAY ||
			pDstObject->Get_PhyXState() == PO_FLYAWAY)
		{
			pSrcObject->Collision(CONTENT_VACUUMOBJECT, pDstObject);
			pDstObject->Collision(CONTENT_VACUUMOBJECT, pSrcObject);
			Fly_DeadAway(pSrcObject, pDstObject);
		}
	}

	else if (eType == CONTENT_ITEM)
	{
		CPhysXObject* pPlayer = Find_TypePtr(PLAYER, pSrcObject, pDstObject);
		if (pPlayer == nullptr)
			return;
		CPhysXObject* pItem = Find_TypePtr(ITEM, pSrcObject, pDstObject);
		if (pItem == nullptr)
			return;

		CItemObject* pIItem = static_cast<CItemObject*>(pItem);

		// 이미 충돌이 완료된 상태라면.
		if (pIItem->Get_ItemCollisionComplete() == true)
			return;

		pPlayer->Collision(CONTENT_ITEM, pItem);
		pItem->Collision(CONTENT_ITEM, pPlayer);

		switch(pIItem->Get_ItemType())
		{
		case CItemObject::ITEM_FOOD:
			Compute_Heal(pPlayer, pItem);
			break;
		case CItemObject::ITEM_COIN:
			Compute_Coin(pPlayer, pItem);
			break;
		case CItemObject::ITEM_SUPERPOWER:
			Compute_SuperPower(pPlayer, pItem);
			break;
		}
	}

	// 커비 HITBOX x 몬스터
	else if (eType == CONTENT_ATTACK)
	{
		pSrcObject->Collision_Overlap(pDstObject);
		pDstObject->Collision_Overlap(pSrcObject);
	}

}

void CCollisionCenter::Camera_Shaking(_float fShakePower)
{


}

void CCollisionCenter::Player_Monster_Knock_back(CPhysXObject* pPlayer, CPhysXObject* pMonster)
{
	CTransform* pPlayerTransform = pPlayer->Get_TransformCom();
	_vector vPos = pPlayerTransform->Get_State_Vector(CTransform::STATE_POSITION);
	CTransform* pMonsterTransform = pMonster->Get_TransformCom();
	_vector vMonsterPos = pMonsterTransform->Get_State_Vector(CTransform::STATE_POSITION);

	_vector vPlayerKnockbackDir = vPos - vMonsterPos;
	vPlayerKnockbackDir.m128_f32[1] = 0.f;
	vPlayerKnockbackDir = XMVector3Normalize(vPlayerKnockbackDir);

	_vector vMonsterKnockbackDir = -1.f * vPlayerKnockbackDir;

	pPlayer->Set_DamageMoving(vPlayerKnockbackDir, 5.f);

	if (static_cast<CCharacter*>(pPlayer)->Get_State() == CKirby::STATE_SLIDE)
		pMonster->Set_DamageMoving(vMonsterKnockbackDir, 10.f);
	else
		pMonster->Set_DamageMoving(vMonsterKnockbackDir, 5.f);
}

void CCollisionCenter::Fly_DeadAway(CPhysXObject* pSrc, CPhysXObject* pDst)
{
	pSrc->Set_PhyXState(PO_FLYDEADAWAY);
	pDst->Set_PhyXState(PO_FLYDEADAWAY);

	CTransform* pSrcTransformCom = pSrc->Get_TransformCom();
	CTransform* pDstTransformCom = pDst->Get_TransformCom();

	_vector vSrcPos = pSrcTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vDstPos = pDstTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	_vector vSrcDeadDir = vSrcPos - vDstPos;
	vSrcDeadDir.m128_f32[1] = 0.f;
	vSrcDeadDir = XMVector3Normalize(vSrcDeadDir);
	_vector vDstDeadDir = -1.f * vSrcDeadDir;

	pSrc->Set_DamageMoving(vSrcDeadDir * CUtils::Make_RandomFloat(0.5f, 1.f), CUtils::Make_RandomFloat(12.f, 20.f));
	pDst->Set_DamageMoving(vDstDeadDir * CUtils::Make_RandomFloat(0.5f, 1.f), CUtils::Make_RandomFloat(12.f, 20.f));
}

void CCollisionCenter::Knock_back(CPhysXObject* pObject, _float3 vKnockbackDir, _float fPower)
{
}

void CCollisionCenter::Compute_Damage(CPhysXObject* pPlayer, CPhysXObject* pMonster)
{
	CCharacter* pCPlayer = static_cast<CCharacter*>(pPlayer);
	CCharacter* pCMonster = static_cast<CCharacter*>(pMonster);

	_float fMonsterAttack = pCMonster->Get_Attack();
	pCPlayer->Minus_Hp(fMonsterAttack);
	_float fPlayerAttack = pCPlayer->Get_Attack();
	pCMonster->Minus_Hp(fMonsterAttack);
}

void CCollisionCenter::Compute_Heal(CPhysXObject* pPlayer, CPhysXObject* pItem)
{
	CCharacter* pCPlayer = static_cast<CCharacter*>(pPlayer);
	CItemObject* pIItem = static_cast<CItemObject*>(pItem);

	_float fItemPoint = (_float)pIItem->Get_ItemPoint();
	pCPlayer->Plus_Hp(fItemPoint);
}

void CCollisionCenter::Compute_Coin(CPhysXObject* pPlayer, CPhysXObject* pItem)
{
	CItemObject* pIItem = static_cast<CItemObject*>(pItem);
	_float fItemPoint = (_float)pIItem->Get_ItemPoint();

	// 코인을 증가시키는 함수를 넣으면 됨.
}

void CCollisionCenter::Compute_SuperPower(CPhysXObject* pPlayer, CPhysXObject* pItem)
{
	CItemObject* pIItem = static_cast<CItemObject*>(pItem);
	_float fItemPoint = (_float)pIItem->Get_ItemPoint();


	// 무적시간을 커비에게 넣어주면 됨.
}

void CCollisionCenter::Free()
{
	for (auto& pObject : m_WaitingList)
	{
		CGameObject* pSrc = pObject.first;
		Safe_Release(pSrc);
		pSrc = pObject.second;
		Safe_Release(pSrc);
	}
	m_WaitingList.clear();

	__super::Free();

}
