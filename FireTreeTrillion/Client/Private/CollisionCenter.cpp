#include "stdafx.h"
#include "CollisionCenter.h"
#include "GameInstance.h"
#include "Kirby.h"
#include "ItemObject.h"
#include "Camera_Main.h"

#include "HUD_StarPoint.h"

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
	m_eColliderType[HITBOX_PLYAER][MONSTER] = CONTENT_ATTACK;
	m_eColliderType[HITBOX_MONSTER][PLAYER]  = CONTENT_ATTACK;

	// 레디얼 기름칠
	GAMEINSTANCE Setting_RadialBlur(5.f, 300.f);
}

void CCollisionCenter::Collision_Tick(_float fTimeDelta)
{
	GAMEINSTANCE Get_CollisionObjects(m_WaitingList);

	// 게임 흐름에 있어서 필요한 슬로우 모션을 충돌에 따라 이곳에서 관리한다.
	Timer_System(fTimeDelta);

	// 게임 흐름에 있어서 필요한 사다리 등 충돌에 따라 Kirby가 작동하도록 이곳에서 관리한다.
	Ladder_Collider();

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

void CCollisionCenter::Add_Ladder(CLadder* pLadder)
{
	m_Ladders.emplace_back(pLadder);
	Safe_AddRef(pLadder);
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
			if (Kirby_Dodge_SlowMotionSystem(pPlayer) == true)
				return;

			// 몬스터의 상태가 노말일 때만, 서로 넉백이 발생하며, 데미지가 발생한다.
			Player_Monster_Knock_back(pPlayer, pMonster);
			Compute_Damage(pPlayer, pMonster);
		}

		// 흡수 등 로직이 있다. 건들지 마 시 오 ( 관리자 : 윤영우 )
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

			Camera_Shaking(1.2f);
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

	// HITBOX x 캐릭터
	else if (eType == CONTENT_ATTACK)
	{
		pSrcObject->Collision_Hitbox(pDstObject);
		pDstObject->Collision_Hitbox(pSrcObject);
	}

}

void CCollisionCenter::Ladder_Collider()
{
	if (m_Ladders.empty() == true)
		return;

	CKirby* pKirby = static_cast<CKirby*>(GAMEINSTANCE Get_GameObject(*GAMEINSTANCE Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	_vector vKirbyPos = pKirby->Get_TransformCom()->Get_State_Vector(CTransform::STATE_POSITION);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	_bool bCollide = { false };

	for (auto& pLadder : m_Ladders)
	{
		// 충돌이 될 때까지 계속 검사한다.
		if (bCollide == false)
		{
			bCollide = pLadder->Is_Collide(vKirbyPos);

			// 검사를 했는데, 충돌이 됐다고 했을 때
			if (bCollide == true)
			{
				Kirbydesc->m_bCanLadder = true;
				Kirbydesc->m_vLadderPoint = pLadder->Get_LadderPoint();
				Kirbydesc->m_vLadderLook = pLadder->Get_TransformCom()->Get_State_Float4(CTransform::STATE_LOOK);
				Kirbydesc->m_vLadderOriginalPos = pLadder->Get_LadderOriginalPos();
				// 커비에게 탈 수 있다는 정보와, 해당 포인팅 좌표를 준다.
			}
		}

		// 항상 릴리즈 해준다.
		Safe_Release(pLadder);
	}

	// 아무것도 충돌이 안 된 상태였다면, BlockLadder를 초기화한다.
	if (bCollide == false)
		Kirbydesc->m_bBlockLadder = false;


	m_Ladders.clear();
}

void CCollisionCenter::Camera_Shaking(_float fPower, _float fTime, _float2 vDir)
{
	CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
	pCamera->Make_Shake(fPower, fTime, vDir);
}

void CCollisionCenter::Camera_Zooming(_float fZoom)
{
	CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
	pCamera->Zoom(fZoom);
}

_bool CCollisionCenter::Kirby_Dodge_SlowMotionSystem(CPhysXObject* pPlayer)
{
	if (m_bCheckTimer == true)
		return true;


	CKirby* pKirby = static_cast<CKirby*>(pPlayer);
	
	if (pKirby->Get_State() == CKirby::STATE_DODGEBACK1 ||
		pKirby->Get_State() == CKirby::STATE_DODGEFRONT1 ||
		pKirby->Get_State() == CKirby::STATE_DODGELEFT1 ||
		pKirby->Get_State() == CKirby::STATE_DODGERIGHT1 ||
		pKirby->Get_State() == CKirby::STATE_DODGESTART)
	{
		_vector vKirbyPos = pKirby->Get_TransformCom()->Get_State_Vector(CTransform::STATE_POSITION);

		Camera_Zooming(-5.f);
		GAMEINSTANCE Set_FirstTimerRatio(0.5f);
		GAMEINSTANCE Set_SecondTimerRatio(0.2f);
		GAMEINSTANCE Setting_RadialBlur(vKirbyPos, 30.f, 10.f);
		GAMEINSTANCE Set_BlackBackGround(true);
		m_bCheckTimer = true;
		return true;
	}


	return false;
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
	CKirby* pKirby = static_cast<CKirby*>(pPlayer);
	CCharacter* pCMonster = static_cast<CCharacter*>(pMonster);

	// 무적이 아닐 경우
	if (pKirby->isOverPower() == false)
	{
		_float fMonsterAttack = pCMonster->Get_Attack();
		pKirby->Minus_Hp(fMonsterAttack);
		Camera_Shaking(1.2f);

		// fMonsterAttack는 몬스터의 공격력으로, 커비에게 데미지를 주는 곳. 카메라 쉐이킹 추가 완료
		// 여기에 Damage를 입히는 함수를 작동시키면 됨 (SJ)
	}

	_float fPlayerAttack = pKirby->Get_Attack();
	pCMonster->Minus_Hp(fPlayerAttack);

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

	// fItemPoint는 코인이 오르는 포인트임 저게 올라야할 "코인점수"임
	// 만약, int 형으로 올라야한다면 형변환 꼭 해주셔!!!
	

	// 코인을 증가시키는 함수를 넣으면 됨 (SJ)
}

void CCollisionCenter::Compute_SuperPower(CPhysXObject* pPlayer, CPhysXObject* pItem)
{
	CItemObject* pIItem = static_cast<CItemObject*>(pItem);
	_float fItemPoint = (_float)pIItem->Get_ItemPoint();


	// 무적시간을 커비에게 넣어주면 됨.
}

void CCollisionCenter::Timer_System(_float fTimeDelta)
{
	if (m_bCheckTimer == true)
	{
		m_fTimeDeltaResetTime += fTimeDelta;
		Camera_Zooming((m_fTimeDeltaResetTime * 1.25f) - 5.f);

		if (m_fTimeDeltaResetTime > 0.5f)
		{
			GAMEINSTANCE Restore_FirstTimer();
			GAMEINSTANCE Set_BlackBackGround(false);
		}
		
		if (m_fTimeDeltaResetTime > 4.f)
		{
			GAMEINSTANCE Restore_SecondTimer();

			Camera_Zooming(0.f);
			m_bCheckTimer = false;
			m_fTimeDeltaResetTime = 0.f;
		}

	}
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

	for (auto& pLadder : m_Ladders)
		Safe_Release(pLadder);
	m_Ladders.clear();

	__super::Free();

}
