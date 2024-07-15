#include "stdafx.h"
#include "CollisionCenter.h"
#include "GameInstance.h"
#include "Trigger.h"
#include "ItemObject.h"
#include "Camera_Main.h"
#include "Kirby.h"
#include "Monster.h"
#include "AnimDeco.h"
#include "DeeDeeDee.h"
#include "Simba.h"
#include "FinalBoss.h"
#include "HUD_StarPoint.h"
#include "Kirby_State_Function.h"
#include "HitBox.h"
#include "FinaleKirby.h"

#define GAMEINSTANCE CGameInstance::Get_Instance()->

IMPLEMENT_SINGLETON(CCollisionCenter)

void CCollisionCenter::Initialize()
{


	// 레디얼 기름칠
	GAMEINSTANCE Setting_RadialBlur(5.f, 300.f);
}

void CCollisionCenter::Collision_Tick(_float fTimeDelta)
{
	LEVEL eLevel = (LEVEL)*GAMEINSTANCE Get_CurrentLevelID();

	// 게임 흐름에 있어서 필요한 슬로우 모션을 충돌에 따라 이곳에서 관리한다.
	Dodge_Timer_System(fTimeDelta);

	// 게임 흐름에 있어서 필요한 슬로우 모션을 충돌에 따라 이곳에서 관리한다.
	Hit_Timer_System(fTimeDelta);

	// 게임 흐름에 있어서 필요한 사다리 등 충돌에 따라 Kirby가 작동하도록 이곳에서 관리한다.
	if (eLevel >= LEVEL_INTRO && eLevel <= LEVEL_FINALBOSS)
	{
		Ladder_Collider();
		// 몸통 박치기 및 몸끼리으로 상호작용하는 콜리전 검사 진행. (보스 제외)
		Body_To_Body_Collision();
		// 히트박스 또는 불릿(투사체) 관련 상호작용 콜리전 검사 진행. (보스 제외)
		Hitbox_Collision();
	}

	// 디디디와 싸우는 특수한 충돌로직들 모아두었습니다.
	if (eLevel == LEVEL_DEEDEEDEE)
		DeeDeeDee_Battle();

	// 파이널 보스와 싸우는 특수한 충돌로직들 모아두었습니다.
	if (eLevel == LEVEL_FINALBOSS)
		FinalStage_Battle();

	if(eLevel == LEVEL_SIMBA)
		Simba_Battle();

	if (eLevel == LEVEL_FINALE)
		RealFinaleStage_Battle();

	for (auto& ObjectVector : m_GameObjects)
	{
		for (auto& pObject : ObjectVector)
			Safe_Release(pObject);
		ObjectVector.clear();
	}
}

void CCollisionCenter::Add_Ladder(CLadder* pLadder)
{
	if (pLadder == nullptr)
		return;

	m_Ladders.emplace_back(pLadder);
	Safe_AddRef(pLadder);
}

void CCollisionCenter::Add_Collision(COLLISION_TYPE eCollType, CGameObject* pGameObject)
{
	if (eCollType >= COLLISION_END)
		return;

	m_GameObjects[eCollType].emplace_back(pGameObject);
	Safe_AddRef(pGameObject);
}

void CCollisionCenter::Collision_Collider(vector<CGameObject*> Dsts, vector<CGameObject*> Srcs, CCollisionCenter* pthis, void(*func)(CHitBox*, CHitBox*, CCollisionCenter*))
{
	if (Dsts.empty() || Srcs.empty())
		return;

	for (auto& Dst : Dsts)
	{
		CHitBox* pHitDst = static_cast<CHitBox*>(Dst);
		for (auto& Src : Srcs)
		{
			CHitBox* pHitSrc = static_cast<CHitBox*>(Src);

			if (pHitDst == nullptr || pHitSrc == nullptr)
				return;

			if (pHitDst->Get_Owner() == pHitSrc->Get_Owner())
				continue;

			if (Intersect(pHitDst, pHitSrc) == true) // 몸통과 충돌하였을 때
			{
				func(pHitDst, pHitSrc, pthis);
			}
		}
	}
}

// 열지 말고... 수정도 하지 마시오... - 영우
_bool CCollisionCenter::Intersect(CHitBox* Dst, CHitBox* Src)
{
	COLLISION_DESC* pDstDesc = Dst->Get_CollisionDesc();
	COLLISION_DESC* pSrcDesc = Src->Get_CollisionDesc();

	CTransform* pDstTransform = Dst->Get_TransformCom();
	CTransform* pSrcTransform = Src->Get_TransformCom();

	if (pDstDesc->eHitbox == COLLIDER_CYLINDER)
	{
		// Dst 원기둥, Src 원기둥일 경우
		if (pSrcDesc->eHitbox == COLLIDER_CYLINDER)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vXZDistance = (vDstXZ - vSrcXZ).Length();

			// 각자의 Radius 값을 합산했는데, 실제 거리가 클 경우 false (충돌 안 함)
			if (vXZDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;
			


			_float vDstMaxY = vDstPos.y + (pDstDesc->fHeight * 0.5f);
			_float vDstMinY = vDstPos.y - (pDstDesc->fHeight * 0.5f);

			_float vSrcMaxY = vSrcPos.y + (pSrcDesc->fHeight * 0.5f);
			_float vSrcMinY = vSrcPos.y - (pSrcDesc->fHeight * 0.5f);

			// Y축에서 겹치는지 확인한다.
			if (vDstMinY <= vSrcMaxY && vDstMaxY >= vSrcMinY)
				return true;

			// 그 외에는 겹치지 않는다.
			return false;
		}
		// Dst 원기둥, Src 구 일 경우
		else if (pSrcDesc->eHitbox == COLLIDER_SPHERE)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vXZDistance = (vDstXZ - vSrcXZ).Length();

			// 각자의 Radius 값을 합산했는데, 실제 거리가 클 경우 false (충돌 안 함)
			if (vXZDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;

			// Y축에서 겹치는지 확인
			_float vDstMaxY = vDstPos.y + (pDstDesc->fHeight * 0.5f);
			_float vDstMinY = vDstPos.y - (pDstDesc->fHeight * 0.5f);

			_float vSrcMaxY = vSrcPos.y + pSrcDesc->fRadius;
			_float vSrcMinY = vSrcPos.y - pSrcDesc->fRadius;

			if (vDstMinY <= vSrcMaxY && vDstMaxY >= vSrcMinY) {
				return true;
			}

			// 겹치지 않는 경우
			return false;
		}
		// Dst 원기둥, Src 원뿔
		else if (pSrcDesc->eHitbox == COLLIDER_FRUSTUM)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vDistance = (vDstPos - vSrcPos).Length();

			// 위에서 봤을 때, 서로의 범위보다 멀다면, 무조건 충돌은 아니다.
			if (vDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;

			CTransform* pConeTransform = Src->Get_TransformCom();
			_float4 vConeLookDir = pConeTransform->Get_State(CTransform::STATE_LOOK);
			vConeLookDir.Normalize();
			_float2 vDir = XMVector2Normalize(vDstXZ - vSrcXZ);
			_float2 vConeDir = XMVector2Normalize(_float2(vConeLookDir.x, vConeLookDir.z));

			// 중점간의 각도를 구하였다.
			_float fAngle = ToDegree(acos(vConeDir.Dot(vDir)));

			// 만약, 실제 앵글의 범위가 내가 설정한 각도보다 작을 경우
			if (fAngle < pSrcDesc->fAngle * 0.5f)
				return true;

			// 그 외는 전부 나가리
			return false;
		}
		else if (pSrcDesc->eHitbox == COLLIDER_TUBE)
		{

		}

	}
	else if (pDstDesc->eHitbox == COLLIDER_SPHERE)
	{
		// Dst 구, Src 원기둥
		if (pSrcDesc->eHitbox == COLLIDER_CYLINDER)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vXZDistance = (vDstXZ - vSrcXZ).Length();

			// 각자의 Radius 값을 합산했는데, 실제 거리가 클 경우 false (충돌 안 함)
			if (vXZDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;

			// Y축에서 겹치는지 확인
			_float vSrcMaxY = vSrcPos.y + (pSrcDesc->fHeight * 0.5f);
			_float vSrcMinY = vSrcPos.y - (pSrcDesc->fHeight * 0.5f);

			_float vDstMaxY = vDstPos.y + pDstDesc->fRadius;
			_float vDstMinY = vDstPos.y - pDstDesc->fRadius;

			if (vSrcMinY <= vDstMaxY && vSrcMaxY >= vDstMinY) {
				return true;
			}
			// 겹치지 않는 경우
			return false;

		}
		else if (pSrcDesc->eHitbox == COLLIDER_SPHERE)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float fDistance = (vDstPos - vSrcPos).Length();

			// 실제 거리가 둘의 반지름보다 작다면 충돌한것이다.
			if (fDistance <= pDstDesc->fRadius + pSrcDesc->fRadius)
				return true;

			return false;
		}
		else if (pSrcDesc->eHitbox == COLLIDER_FRUSTUM)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vDistance = (vDstPos - vSrcPos).Length();

			// 위에서 봤을 때, 서로의 범위보다 멀다면, 무조건 충돌은 아니다.
			if (vDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;

			CTransform* pConeTransform = Src->Get_TransformCom();
			_float4 vConeLookDir = pConeTransform->Get_State(CTransform::STATE_LOOK);
			vConeLookDir.Normalize();
			_float2 vDir = XMVector2Normalize(vDstXZ - vSrcXZ);
			_float2 vConeDir = XMVector2Normalize(_float2(vConeLookDir.x, vConeLookDir.z));

			// 중점간의 각도를 구하였다.
			_float fAngle = ToDegree(acos(vConeDir.Dot(vDir)));

			// 만약, 실제 앵글의 범위가 내가 설정한 각도보다 작을 경우
			if (fAngle < pSrcDesc->fAngle * 0.5f)
				return true;

			// 그 외는 전부 나가리
			return false;

		}
		else if (pSrcDesc->eHitbox == COLLIDER_TUBE)
		{

		}
	}
	else if (pDstDesc->eHitbox == COLLIDER_FRUSTUM)
	{
		// Dst가 절두체(부채모양) , Src가 원통
		if (pSrcDesc->eHitbox == COLLIDER_CYLINDER)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vDistance = (vDstPos - vSrcPos).Length();

			// 위에서 봤을 때, 서로의 범위보다 멀다면, 무조건 충돌은 아니다.
			if (vDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;

			CTransform* pConeTransform = Dst->Get_TransformCom();
			_float4 vConeLookDir = pConeTransform->Get_State(CTransform::STATE_LOOK);
			vConeLookDir.Normalize();
			_float2 vDir = XMVector2Normalize(vSrcXZ - vDstXZ);
			_float2 vConeDir = XMVector2Normalize(_float2(vConeLookDir.x, vConeLookDir.z));

			// 중점간의 각도를 구하였다.
			_float fAngle = ToDegree(acos(vConeDir.Dot(vDir)));

			// 만약, 실제 앵글의 범위가 내가 설정한 각도보다 작을 경우
			if (fAngle < pDstDesc->fAngle * 0.5f)
				return true;

			// 그 외는 전부 나가리
			return false;

		}
		// Dst가 절두체(부채모양) , Src가 구
		else if (pSrcDesc->eHitbox == COLLIDER_SPHERE)
		{
			_float4 vDstPos = pDstTransform->Get_State(CTransform::STATE_POSITION);
			_float4 vSrcPos = pSrcTransform->Get_State(CTransform::STATE_POSITION);

			_float2 vDstXZ = { vDstPos.x, vDstPos.z };
			_float2 vSrcXZ = { vSrcPos.x, vSrcPos.z };

			// 위로 보았을 때, 서로의 거리를 구한다.
			_float vDistance = (vDstPos - vSrcPos).Length();

			// 위에서 봤을 때, 서로의 범위보다 멀다면, 무조건 충돌은 아니다.
			if (vDistance > pDstDesc->fRadius + pSrcDesc->fRadius)
				return false;

			CTransform* pConeTransform = Dst->Get_TransformCom();
			_float4 vConeLookDir = pConeTransform->Get_State(CTransform::STATE_LOOK);
			vConeLookDir.Normalize();
			_float2 vDir = XMVector2Normalize(vSrcXZ - vDstXZ);
			_float2 vConeDir = XMVector2Normalize(_float2(vConeLookDir.x, vConeLookDir.z));

			// 중점간의 각도를 구하였다.
			_float fAngle = ToDegree(acos(vConeDir.Dot(vDir)));

			// 만약, 실제 앵글의 범위가 내가 설정한 각도보다 작을 경우
			if (fAngle < pDstDesc->fAngle * 0.5f)
				return true;

			// 그 외는 전부 나가리
			return false;
		}
		else if (pSrcDesc->eHitbox == COLLIDER_FRUSTUM)
		{

		}
		else if (pSrcDesc->eHitbox == COLLIDER_TUBE)
		{

		}
	}
	else if (pDstDesc->eHitbox == COLLIDER_TUBE)
	{
		if (pSrcDesc->eHitbox == COLLIDER_CYLINDER)
		{

		}
		else if (pSrcDesc->eHitbox == COLLIDER_SPHERE)
		{

		}
		else if (pSrcDesc->eHitbox == COLLIDER_FRUSTUM)
		{

		}
		else if (pSrcDesc->eHitbox == COLLIDER_TUBE)
		{

		}
	}


	return false;
}

void CCollisionCenter::Ladder_Collider()
{
	if (m_Ladders.empty() == true)
		return;

	CKirby* pKirby = static_cast<CKirby*>(GAMEINSTANCE Get_GameObject(*GAMEINSTANCE Get_CurrentLevelID(), TEXT("Layer_Player"), 0));

	if (nullptr == pKirby)
		return;

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

void CCollisionCenter::DeeDeeDee_Battle()
{

	// 플레이어와 와들 디의 몸 충돌. 흡수 할때만 작동할 것이다.
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[BATTLEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pPlayer = static_cast<CKirby*>(Dst);
			CPhysXObject* pDee = static_cast<CMonster*>(Src);

			// 디가 흡수 상태일 경우에만 작동하라.
			if (pDee->Get_PhyXState() != PO_VACUUMING)
				return;

			pPlayer->Collision(CONTENT_BODY, pDee);
		});

	// 플레이어 공격에 대한 처리.
	Collision_Collider(m_GameObjects[HITBOX_PLYAER], m_GameObjects[BOSS_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CDeeDeeDee* pMonster = static_cast<CDeeDeeDee*>(Src);

			// 데미지 공식과 이펙트, 쉐이킹, 히트스탑 등 시스템적인 요소들이 잔뜩 들어가있다.
			pthis->Damage_And_Effect_For_Monster(pKirby, pMonster, 1.2f);
			DstHit->Set_Alive(false);
		});

	// 플레이어 공격에 대한 처리
	Collision_Collider(m_GameObjects[PLAYERBULLET], m_GameObjects[BOSS_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CDeeDeeDee* pMonster = static_cast<CDeeDeeDee*>(Src);


			_float fAttack = pKirby->Get_Attack();
			pMonster->Minus_Hp(fAttack);
			pthis->Camera_Shaking(1.2f);
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
			Dst->Set_Dead();
		});

	// 플레이어 공격에 대한 처리
	Collision_Collider(m_GameObjects[OBJECT], m_GameObjects[BOSS_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pObject = static_cast<CPhysXObject*>(Dst);
			CMonster* pMonster = static_cast<CMonster*>(Src);

			if (pObject->Get_PhyXState() != PO_FLYAWAY)
				return;

			_float fAttack = pObject->Get_Attack();
			pMonster->Minus_Hp(fAttack);
			pthis->Camera_Shaking(1.2f);
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
			pObject->Set_PhyXState(PO_FLYDEADAWAY);
		});


	// 플레이어가 공격당하는 로직
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[HITBOX_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CDeeDeeDee* pMonster = static_cast<CDeeDeeDee*>(Src);


			// 커비가 혹시 닷지를 하였는가? 만약 닷지를 했다면 충돌이 발생하지않는다.
			if (pthis->Kirby_Dodge_SlowMotionSystem(pKirby) == true)
			{
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
				return;
			}

			// 플레이어와 보스 양쪽에 넉백을 만든다.
			pthis->Player_Monster_Knock_back(pKirby, pMonster);
			pthis->Compute_HitBoxDamage(pKirby, pMonster);
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);

			// 별도의 충돌로직이 발생할 것이다.
			pKirby->Collision(CONTENT_ATTACK, pMonster);
		});


	Collision_Collider(m_GameObjects[BATTLEDEE], m_GameObjects[HITBOX_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pNpc = static_cast<CPhysXObject*>(Dst);
			CDeeDeeDee* pMonster = static_cast<CDeeDeeDee*>(Src);

			if (pNpc->Get_PhyXState() != PO_NORMAL)
				return;

			_float4 vNpcPos = pNpc->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
			_float4 vDeeDeeDeePos = pMonster->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

			_float3 vKnockDir = XMVector3Normalize(vNpcPos - vDeeDeeDeePos);

			pthis->Knock_back(pNpc, vKnockDir * 5.f, CUtils::Make_RandomFloat(15.f, 28.f));
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);

			pNpc->Collision(CONTENT_ATTACK, pMonster);

		});


	Collision_Collider(m_GameObjects[BATTLEDEE], m_GameObjects[BOSS_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pNpc = static_cast<CPhysXObject*>(Dst);
			CDeeDeeDee* pMonster = static_cast<CDeeDeeDee*>(Src);

			if (pNpc->Get_PhyXState() != PO_FLYAWAY)
				return;

			_float4 vNpcPos = pNpc->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
			_float4 vDeeDeeDeePos = pMonster->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

			_float3 vKnockDir = XMVector3Normalize(vNpcPos - vDeeDeeDeePos);

			pthis->Knock_back(pNpc, vKnockDir * 5.f, 10.f);
			pthis->Camera_Shaking(1.2f);
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
			pNpc->Set_PhyXState(PO_FLYDEADAWAY);
			pMonster->Minus_Hp(10.f);
		});

		Collision_Collider(m_GameObjects[BATTLEDEE], m_GameObjects[HITBOX_DEEDEEDEE], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pNpc = static_cast<CPhysXObject*>(Dst);
			CDeeDeeDee* pMonster = static_cast<CDeeDeeDee*>(Src);

			if (pNpc->Get_PhyXState() != PO_NORMAL)
				return;

			_float4 vNpcPos = pNpc->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
			_float4 vDeeDeeDeePos = pMonster->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

			_float3 vKnockDir = XMVector3Normalize(vNpcPos - vDeeDeeDeePos);

			pthis->Knock_back(pNpc, vKnockDir * 5.f, CUtils::Make_RandomFloat(15.f, 28.f));
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);

			pNpc->Collision(CONTENT_ATTACK, pMonster);

		});


}

void CCollisionCenter::Simba_Battle()
{
	// 플레이어 공격에 대한 처리.
	Collision_Collider(m_GameObjects[HITBOX_PLYAER], m_GameObjects[BOSS_SIMBA], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pMonster = static_cast<CPhysXObject*>(Src);

			// 데미지 공식과 이펙트, 쉐이킹, 히트스탑 등 시스템적인 요소들이 잔뜩 들어가있다.
			pthis->Damage_And_Effect_For_Monster(pKirby, pMonster, 1.2f);
			pMonster->Collision(CONTENT_DAMAGE, nullptr);
			DstHit->Set_Alive(false);
		});

	// 플레이어 공격에 대한 처리
	Collision_Collider(m_GameObjects[OBJECT], m_GameObjects[BOSS_SIMBA], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pObject = static_cast<CPhysXObject*>(Dst);
			CMonster* pMonster = static_cast<CMonster*>(Src);

			if (pObject->Get_AbilityType() != PO_FLYAWAY)
				return;

			_float fAttack = pObject->Get_Attack();
			pMonster->Minus_Hp(fAttack);
			pthis->Camera_Shaking(1.2f);
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
			pObject->Set_PhyXState(PO_FLYDEADAWAY);
		});

}

void CCollisionCenter::FinalStage_Battle()
{
	// 플레이어 공격에 대한 처리.
	Collision_Collider(m_GameObjects[HITBOX_PLYAER], m_GameObjects[BOSS_FINALBOSS], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pMonster = static_cast<CPhysXObject*>(Src);

			// 데미지 공식과 이펙트, 쉐이킹, 히트스탑 등 시스템적인 요소들이 잔뜩 들어가있다.
			pthis->Damage_And_Effect_For_Monster(pKirby, pMonster, 1.2f);
			DstHit->Set_Alive(false);
		});


	// 플레이어 공격에 대한 처리
	Collision_Collider(m_GameObjects[OBJECT], m_GameObjects[BOSS_FINALBOSS], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pObject = static_cast<CPhysXObject*>(Dst);
			CMonster* pMonster = static_cast<CMonster*>(Src);

			if (pObject->Get_AbilityType() != PO_FLYAWAY)
				return;

			_float fAttack = pObject->Get_Attack();
			pMonster->Minus_Hp(fAttack);
			pthis->Camera_Shaking(1.2f);
			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
			pObject->Set_PhyXState(PO_FLYDEADAWAY);
		});


}

void CCollisionCenter::RealFinaleStage_Battle()
{
	Collision_Collider(m_GameObjects[FINALE_PLAYER], m_GameObjects[FINALE_BAUM], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(Dst);
			CPhysXObject* pBaum = static_cast<CPhysXObject*>(Src);

			pKirby->Collision(CONTENT_BODY, pBaum);
			pKirby->Set_HitStop(0.2f);
			pthis->Camera_Shaking(1.5f, 1.f);
			pBaum->Collision(CONTENT_BODY, pKirby);

			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
		});

	Collision_Collider(m_GameObjects[FINALE_PLAYER], m_GameObjects[FINALE_BREAKABLEBLOCK], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(Dst);
			CPhysXObject* pBreakable = static_cast<CPhysXObject*>(Src);

			pBreakable->Collision(CONTENT_BODY, pKirby);

		});


}

void CCollisionCenter::Body_To_Body_Collision()
{
	// 깔끔하게 완료되었음 : 플레이어 X 몬스터
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[MONSTER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CMonster* pMonster = static_cast<CMonster*>(Src);

			// 무적상태인가?
			if (pMonster->Get_MonsterOverPower() == true)
				return;
			// 플레이어가 공격중인가? (공격중이라면 몸박치기로 데미지가 닳을 시, 근거리 공격이 너무 고통스럽다.)
			if (pKirby->Is_Attacking() == true)
				return;

			if (pMonster->Get_PhyXState() == PO_PRESSED)
				return;

			// 몬스터가 일반적인 상황일때만, 서로 데미지가 계산된다.
			if (pMonster->Get_PhyXState() == PO_NORMAL)
			{
				if (pKirby->Get_KirbyInfo()->m_bBooster == true)
				{
					pMonster->Set_PhyXState(PO_PRESSED);

					pKirby->Set_HitStop();
					pthis->Camera_Shaking(1.2f);
					return;
				}

				// 커비가 혹시 닷지를 하였는가? 만약 닷지를 했다면 충돌이 발생하지않는다.
				if (pthis->Kirby_Dodge_SlowMotionSystem(pKirby) == true)
					return;

				// 몬스터의 상태가 평범하며, 닷지를 안 했을 때, 서로 넉백과 데미지가 발생한다.
				pthis->Player_Monster_Knock_back(pKirby, pMonster);
				pthis->Compute_Damage(pKirby, pMonster);
				// 몬스터에게 부여되는 무적시간이다. (기본적으로 히트박스는 0.1초의 딜레이를 갖지만 몸통박치기는 2초의 딜레이가 필요할것이다)
				pMonster->Set_Damage_Delay(1.f);
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
			}

			// 몸끼리 충돌했을 때, 세부적인건 각 객체에서 정의된다.
			// 커비는 흡수했을때도 몸 충돌, 들고 있을때도 몸 충돌이다. 즉, 내부에서 예외처리가 잘 되어있는 것이다.
			pKirby->Collision(CONTENT_BODY, pMonster);
			pMonster->Collision(CONTENT_BODY, pKirby);
		});

	// 깔끔하게 완료되었음 : 몬스터 X 몬스터 (커비가 날릴때의 충돌)
	Collision_Collider(m_GameObjects[MONSTER], m_GameObjects[MONSTER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CMonster* pDstMonster = static_cast<CMonster*>(Dst);
			CMonster* pSrcMonster = static_cast<CMonster*>(Src);

			// 몬스터 둘 중 하나라도 날아가는 놈이였다면???
			if (pDstMonster->Get_PhyXState() == PO_FLYAWAY ||
				pSrcMonster->Get_PhyXState() == PO_FLYAWAY)
			{
				// 세부적인건 해당 CONTENT에서 한다.
				pDstMonster->Collision(CONTENT_VACUUMOBJECT, pSrcMonster);
				pSrcMonster->Collision(CONTENT_VACUUMOBJECT, pDstMonster);

				// 각자의 상태를 PO_FLYDEADAWAY 로 바꿔줌과 동시에 죽는 방향과 힘을 정해준다.
				pthis->Fly_DeadAway(pDstMonster, pSrcMonster);
				// 카메라 쉐이킹을 해준다.
				pthis->Camera_Shaking(1.2f);

				// 0.1초의 콜라이더 딜레이를 넣어준다.
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
			}

		});

	// 깔끔하게 완료되었음 : 오브젝트 X 몬스터 (커비가 날릴때의 충돌)
	Collision_Collider(m_GameObjects[OBJECT], m_GameObjects[MONSTER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pDstObject = static_cast<CMonster*>(Dst);
			CMonster* pSrcMonster = static_cast<CMonster*>(Src);

			// 반드시, 오브젝트가 날아가는 상황이여야 한다.
			if (pDstObject->Get_PhyXState() == PO_FLYAWAY)
			{
				// 세부적인건 해당 CONTENT에서 한다.
				pDstObject->Collision(CONTENT_VACUUMOBJECT, pSrcMonster);
				pSrcMonster->Collision(CONTENT_VACUUMOBJECT, pDstObject);

				// 각자의 상태를 PO_FLYDEADAWAY 로 바꿔줌과 동시에 죽는 방향과 힘을 정해준다.
				pthis->Fly_DeadAway(pDstObject, pSrcMonster);
				// 카메라 쉐이킹을 해준다.
				pthis->Camera_Shaking(1.2f);

				// 0.1초의 콜라이더 딜레이를 넣어준다.
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
			}
		});

	// 깔끔하게 완료되었음 : 플레이어 X 아이템
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[ITEM], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CItemObject* pItem = static_cast<CItemObject*>(Src);

			// 아이템이 이미 충돌이 완료된 상태라면?
			if (pItem->Get_ItemCollisionComplete() == true)
				return;

			// 이곳에서 ItemCollisionComplete 라는 불 변수가 작동할 것이다. 세부적인거 포함해서 각자 아이템에서 구현중.
			pItem->Collision(CONTENT_ITEM, pKirby);

			// 아이템 타입에 따라, 플레이어와 상호작용한다.
			switch (pItem->Get_ItemType())
			{
			case CItemObject::ITEM_FOOD:
				pthis->Compute_Heal(pKirby, pItem);
				break;
			case CItemObject::ITEM_COIN:
				pthis->Compute_Coin(pKirby, pItem);
				break;
			case CItemObject::ITEM_SUPERPOWER:
				pthis->Compute_SuperPower(pKirby, pItem);
				break;
			}
		});

	// 깔끔하게 완료되었음 : 플레이어 X 오브젝트류 (발로 차기 및 흡수 로직)
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[OBJECT], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pObject = static_cast<CPhysXObject*>(Src);

			// 돌덩이에게 물리적인 힘을 주는 것이 구현되어있을 것이다.
			// 또는 내가 흡수했을때도 먹는 로직이 되어있을듯 함.
			pObject->Collision(CONTENT_KICK, pKirby);
			pKirby->Collision(CONTENT_KICK, pObject);
			// 0.1초의 충돌 딜레이를 주기위함.
			SrcHit->Set_Alive(false);
		});

	// 깔끔하게 완료되었음 : 플레이어 X 어빌리티아이템
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[ABILITYITEM], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pAbility = static_cast<CPhysXObject*>(Src);

			if (pAbility->Get_PhyXState() != PO_VACUUMING)
				return;

			// 커비는 이 친구와 충돌하면 바로 능력을 먹을듯 하다.
			pKirby->Collision(CONTENT_BODY, pAbility);
		});

	// 깔끔하게 완료되었음 : 플레이어 X 형 변환
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[DEFORMOBJECT], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pDeform = static_cast<CPhysXObject*>(Src);

			pKirby->Collision(CONTENT_DEFORM, pDeform);
			pKirby->Get_KirbyInfo()->m_bBlockOtherVacuum = true;
		});

	// 디벨롭중
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[NPC], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pNPC = static_cast<CPhysXObject*>(Src);
			pNPC->Collision(CONTENT_INTERACT, pKirby);

		});

	// 깔끔하게 완료되었음 : 플레이어 X 오브젝트류 (발로 차기 및 흡수 로직)
	Collision_Collider(m_GameObjects[OBJECT], m_GameObjects[OBJECT], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pDst = static_cast<CPhysXObject*>(Dst);
			CPhysXObject* pSrc = static_cast<CPhysXObject*>(Src);

			if (pDst->Get_PhyXState() == PO_FLYAWAY ||
				pSrc->Get_PhyXState() == PO_FLYAWAY)
			{
				// 각자의 상태를 PO_FLYDEADAWAY 로 바꿔줌과 동시에 죽는 방향과 힘을 정해준다.
				pthis->Fly_DeadAway(pDst, pSrc);
				// 카메라 쉐이킹을 해준다.
				pthis->Camera_Shaking(1.2f);

				// 0.1초의 콜라이더 딜레이를 넣어준다.
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
			}
		});
}

void CCollisionCenter::Hitbox_Collision()
{
	// 완료.
	Collision_Collider(m_GameObjects[HITBOX_PLYAER], m_GameObjects[MONSTER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pMonster = static_cast<CPhysXObject*>(Src);

			// 혹시 몬스터가 무적상태인가?
			if (static_cast<CMonster*>(pMonster)->Get_MonsterOverPower() == true)
				return;

			// 넉백방향을 정해주기 위한 과정이다.
			CTransform* pPlayerTransform = pKirby->Get_TransformCom();
			_vector vPos = pPlayerTransform->Get_State_Vector(CTransform::STATE_POSITION);
			CTransform* pMonsterTransform = pMonster->Get_TransformCom();
			_vector vMonsterPos = pMonsterTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_float4 vDistance = vMonsterPos - vPos;
			vDistance.y = 0.f;
			vDistance.Normalize();
			_vector vPlayerKnockbackDir = vDistance;


			// 만약, 작은 넉백이였을 경우
			if (pthis->Small_KnockBack(pKirby->Get_State()))
			{
				// 넉백의 2차원 방향 (y축없는) 과 뜨는 힘을 정해준다.
				pthis->Knock_back(pMonster, vPlayerKnockbackDir * 1.3f, 5.f);
			}
			// 만약, 적당한 넉백이였을 경우
			else if (pthis->Normal_KnockBack(pKirby->Get_State()))
			{
				// 넉백의 2차원 방향 (y축없는) 과 뜨는 힘을 정해준다.
				pthis->Knock_back(pMonster, vPlayerKnockbackDir * 1.5f, 8.f);
			}
			// 위로 뜨는 공격이였을 경우.
			else if (pthis->Up_KnockBack(pKirby->Get_State()))
			{
				// 넉백의 2차원 방향 (y축없는) 과 뜨는 힘을 정해준다.
				pthis->Knock_back(pMonster, vPlayerKnockbackDir * 0.5f, 13.f);
			}
			// 아예 날아가는 공격이였을 경우.
			else if (pthis->FlyAway_KnockBack(pKirby->Get_State()))
			{
				// 넉백의 2차원 방향 (y축없는) 과 뜨는 힘을 정해준다.
				pthis->Knock_back(pMonster, vPlayerKnockbackDir * 4.f, 20.f);
			}

			// 몬스터의 CONTENT_ATTACK타입의 Collision함수를 발동시킨다. 정말 세부적인건 이쪽에서 처리된다.
			pMonster->Collision(CONTENT_ATTACK, pKirby);

			// 데미지 공식과 이펙트, 쉐이킹, 히트스탑 등 시스템적인 요소들이 잔뜩 들어가있다.
			pthis->Damage_And_Effect_For_Monster(pKirby, pMonster);
			DstHit->Set_Alive(false);
		});

	// 완료.
	Collision_Collider(m_GameObjects[HITBOX_PLYAER], m_GameObjects[OBJECT], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CPhysXObject* pObject = static_cast<CPhysXObject*>(Src);


			// 몬스터의 CONTENT_ATTACK타입의 Collision함수를 발동시킨다. 정말 세부적인건 이쪽에서 처리된다.
			pObject->Collision(CONTENT_ATTACK, pKirby);
			DstHit->Set_Alive(false);
		});

	// 완료.
	Collision_Collider(m_GameObjects[PLAYERBULLET], m_GameObjects[OBJECT], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			// 폭발물
			CPhysXObject* pDst = static_cast<CPhysXObject*>(Src);
			// 오브젝트
			CPhysXObject* pObject = static_cast<CPhysXObject*>(Src);
			pObject->Collision(CONTENT_ATTACK, pDst);
			Dst->Set_Dead();
		});

	// 완료.
	Collision_Collider(m_GameObjects[HITBOX_MONSTER], m_GameObjects[PLAYER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;


			CKirby* pKirby = static_cast<CKirby*>(Src);
			CMonster* pMonster = static_cast<CMonster*>(Dst);

			// 커비가 혹시 닷지를 하였는가? 만약 닷지를 했다면 충돌이 발생하지않는다.
			if (pthis->Kirby_Dodge_SlowMotionSystem(pKirby) == true)
			{
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
				return;
			}

			if (pKirby->isOverPower() == false)
			{
				CTransform* pMonsterTransformCom = pMonster->Get_TransformCom();
				/*if (LEVEL_SIMBA == *GAMEINSTANCE Get_CurrentLevelID())
					pMonsterTransformCom = DstHit->Get_TransformCom();*/
				_vector vMonsterPos = pMonsterTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();
				_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
				_float4 vDistance = vKirbyPos - vMonsterPos;
				vDistance.y = 0.f;
				vDistance.Normalize();
				_vector vKnockbackDir = vDistance;
				pthis->Knock_back(pKirby, vKnockbackDir * 1.5f, 7.f);
				pthis->Compute_HitBoxDamage(pKirby, pMonster);
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
				pKirby->Collision(CONTENT_ATTACK, pMonster);
			}
		});

	// 완료.
	Collision_Collider(m_GameObjects[PLAYERBULLET], m_GameObjects[PLAYERBULLET], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			pthis->Camera_Shaking(1.2f);
			Dst->Set_Dead();
			Src->Set_Dead();
		});

	// 완료.
	Collision_Collider(m_GameObjects[PLAYERBULLET], m_GameObjects[MONSTER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pDst = static_cast<CPhysXObject*>(Dst);
			CMonster* pMonster = static_cast<CMonster*>(Src);

			// 넉백방향을 정해주기 위한 과정이다.
			CTransform* pBulletTransform = Dst->Get_TransformCom();
			_vector vBulletPos = pBulletTransform->Get_State_Vector(CTransform::STATE_POSITION);
			CTransform* pMonsterTransform = pMonster->Get_TransformCom();
			_vector vMonsterPos = pMonsterTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_float4 vDistance = vMonsterPos - vBulletPos;
			vDistance.y = 0.f;
			vDistance.Normalize();
			_vector vKnockbackDir = vDistance;

			// 넉백의 2차원 방향 (y축없는) 과 뜨는 힘을 정해준다.
			pthis->Knock_back(pMonster, vKnockbackDir * 1.3f, 5.f);
			// 몬스터의 CONTENT_ATTACK타입의 Collision함수를 발동시킨다. 정말 세부적인건 이쪽에서 처리된다.
			pMonster->Collision(CONTENT_ATTACK, pDst);

			_float fAttack = pDst->Get_Attack();
			pMonster->Minus_Hp(fAttack);

			DstHit->Set_Alive(false);
			SrcHit->Set_Alive(false);
			pthis->Camera_Shaking(1.2f);
			Dst->Set_Dead();
		});

	// 완료.
	Collision_Collider(m_GameObjects[MONSTERBULLET], m_GameObjects[PLAYER], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pMonsterBullet = static_cast<CPhysXObject*>(Dst);
			CKirby* pKirby = static_cast<CKirby*>(Src);

			// 커비가 혹시 닷지를 하였는가? 만약 닷지를 했다면 충돌이 발생하지않는다.
			if (pthis->Kirby_Dodge_SlowMotionSystem(pKirby) == true)
			{
				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
				return;
			}

			if (pMonsterBullet->Get_PhyXState() == PO_NORMAL)
			{
				// 불릿이 PO_NORMAL (평범한 상태) 였을땐, 커비가 넉백되며 다친다. 또한, 폭탄은 터진다.
				pthis->Player_Monster_Knock_back(pKirby, pMonsterBullet);
				_float fAttack = pMonsterBullet->Get_Attack();
				pKirby->Minus_Hp(fAttack);

				if (pMonsterBullet->Get_NonDead() == false)
					Dst->Set_Dead();

				DstHit->Set_Alive(false);
				SrcHit->Set_Alive(false);
			}

			// 커비의 충돌로직은 항상 발동한다. 만약, 폭탄이 PO_VACUUM일땐 서로 충돌은 나지만, 먹을 수 있는 상황이라면
			// 별도의 충돌로직이 발생할 것이다.
			pKirby->Collision(CONTENT_ATTACK, pMonsterBullet);
		});



	// 풀 등과 플레이어
	Collision_Collider(m_GameObjects[HITBOX_PLYAER], m_GameObjects[ANIMDECO], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CKirby* pKirby = static_cast<CKirby*>(Dst);
			CAnimDeco* pAnimDeco = static_cast<CAnimDeco*>(Src);
			if (true == pAnimDeco->IsHidden())
				return;

			pAnimDeco->HideModel();
		});

	// PhysX의 트리거 외에 객체호출 등 작은 단위의 트리거용
	Collision_Collider(m_GameObjects[PLAYER], m_GameObjects[TRIGGER_FOR_NOT_PHYSX], this,
		[](CHitBox* DstHit, CHitBox* SrcHit, CCollisionCenter* pthis)
		{
			CGameObject* Dst = DstHit->Get_Owner();
			CGameObject* Src = SrcHit->Get_Owner();
			if (Dst == nullptr || Src == nullptr || Dst->Get_Dead() || Src->Get_Dead())
				return;

			CPhysXObject* pPlayer = static_cast<CPhysXObject*>(Dst);
			CPhysXObject* pMonster = static_cast<CDeeDeeDee*>(Src);

			//if (pNpc->Get_PhyXState() != PO_NORMAL)
			//	return;
			//_float4 vNpcPos = pNpc->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
			//_float4 vDeeDeeDeePos = pMonster->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
			//_float3 vKnockDir = XMVector3Normalize(vNpcPos - vDeeDeeDeePos);
			//pthis->Knock_back(pNpc, vKnockDir * 5.f, CUtils::Make_RandomFloat(15.f, 28.f));
			//DstHit->Set_Alive(false);
			//SrcHit->Set_Alive(false);

			pMonster->Collision(CONTENT_TRIGGER, pPlayer);
		});
}

_bool CCollisionCenter::Small_KnockBack(_uint uKirbyState)
{
	return uKirbyState == CKirby::SWORDSTATE_SIDESLASH ||
		uKirbyState == CKirby::SWORDSTATE_MULITSWORDATTACK ||
		uKirbyState == CKirby::SWORDSTATE_GIGANTSPINSLASH ||
		uKirbyState == CKirby::SWORDSTATE_SUPERSPINSLASHLOOP ||
		uKirbyState == CKirby::HAMMERSTATE_HAMMERATTACKHITTOY;
}

_bool CCollisionCenter::Normal_KnockBack(_uint uKirbyState)
{
	return uKirbyState == CKirby::SWORDSTATE_DECISIVESLASH || uKirbyState == CKirby::SWORDSTATE_SWORDSPIN ||
		uKirbyState == CKirby::HAMMERSTATE_HAMMERATTACKFINALTOY || uKirbyState == CKirby::HAMMERSTATE_ONIGOROSIHAMMERFIRST ||
		uKirbyState == CKirby::HAMMERSTATE_WHEELHAMMER;
}

_bool CCollisionCenter::Up_KnockBack(_uint uKirbyState)
{
	return uKirbyState == CKirby::SWORDSTATE_UPWARDSLASH;
}

_bool CCollisionCenter::FlyAway_KnockBack(_uint uKirbyState)
{
	return uKirbyState == CKirby::HAMMERSTATE_ONIGOROSIHAMMEREND ||
		uKirbyState == CKirby::CRASHSTATE_ATTACK ||
		uKirbyState == CKirby::CRASHSTATE_BIGATTACKFIRE;
}

void CCollisionCenter::HitStop_Rogic(CKirby* pKirby, _float fStopTime)
{
	pKirby->Set_HitStop(fStopTime);
}

void CCollisionCenter::Damage_And_Effect_For_Monster(CKirby* pKirby, CPhysXObject* pMonster, _float fEffectOffSet)
{
	CCharacter* pCMonster = static_cast<CCharacter*>(pMonster);
	_float fAttack = { 0.f };
	_uint uKirbyState = pKirby->Get_State();

	CTransform* pMonsterTransform = pCMonster->Get_TransformCom();
	CTransform* pKirbyTransform = pKirby->Get_TransformCom();
	_float4 vMonsterPos = pMonsterTransform->Get_State(CTransform::STATE_POSITION);
	_float4 vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);

	_float4 vEffectLook = XMVector3Normalize(vKirbyPos - vMonsterPos);
	_float4 vEffectRandomPos = vMonsterPos + (vEffectLook * fEffectOffSet) + (_float4)CUtils::Make_Random_Vector(0.5f);

	switch (uKirbyState)
	{
	// SWORD 연속기 1타
	case CKirby::SWORDSTATE_SIDESLASH:
	{
		fAttack = 5.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking(0.7f, 0.5f);
		SwordHit(pMonsterTransform);
	}
	break;
	// SWORD 연속기 2타
	case CKirby::SWORDSTATE_MULITSWORDATTACK:
	{
		fAttack = 5.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking(0.7f, 0.5f);
		SwordHit(pMonsterTransform);
	}
	break;
	// SWORD 연속기 3타
	case CKirby::SWORDSTATE_DECISIVESLASH:
	{
		fAttack = 10.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking();
		SwordHit_Big(pMonsterTransform);

	}
	break;
	// 덜 차징 회전베기
	case CKirby::SWORDSTATE_GIGANTSPINSLASH:
	{
		fAttack = 5.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking(0.7f, 0.5f);
		SwordHit(pMonsterTransform);

	}
	break;
	// 풀 차징 회전베기
	case CKirby::SWORDSTATE_SUPERSPINSLASHLOOP:
	{
		fAttack = 5.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking(0.7f, 0.5f);
		SwordHit(pMonsterTransform);

	}
	break;
	// 위로 올려베기 (대쉬기 중 점프 키)
	case CKirby::SWORDSTATE_UPWARDSLASH:
	{
		fAttack = 10.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking();
		SwordHit(pMonsterTransform);
	}
	// 공중제비 도는 공격
	case CKirby::SWORDSTATE_SWORDSPIN:
	{
		fAttack = 5.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking();
		SwordHit(pMonsterTransform);
	}
	break;
	// 해머 5타 통 애님 (막타)
	case CKirby::HAMMERSTATE_HAMMERATTACKFINALTOY:
	{
		fAttack = 10.f;
		HitStop_Rogic(pKirby, 0.2f);
	}
	break;
	// 해머 평타
	case CKirby::HAMMERSTATE_HAMMERATTACKHITTOY:
	{
		fAttack = 5.f;
		HitStop_Rogic(pKirby, 0.06f);
	}
	break;
	// 해머 풀 차징 공격
	case CKirby::HAMMERSTATE_ONIGOROSIHAMMEREND:
	{
		fAttack = 50.f;
		Hit_TimeStop(0.01f, 0.5f);
		Camera_Shaking(2.f);
		CGameInstance::Get_Instance()->Setting_RadialBlur(vKirbyPos, 20.f, 100.f);
	}
	break;
	// 해머 덜 차징 공격
	case CKirby::HAMMERSTATE_ONIGOROSIHAMMERFIRST:
	{
		fAttack = 20.f;
		Hit_TimeStop(0.01f, 0.3f);
		Camera_Shaking(1.f);
		CGameInstance::Get_Instance()->Setting_RadialBlur(vKirbyPos, 20.f, 150.f);
	}
	break;
	// 해머 공중 회전 공격
	case CKirby::HAMMERSTATE_WHEELHAMMER:
	{
		fAttack = 7.f;
		HitStop_Rogic(pKirby);
		Camera_Shaking();
	}
	break;
	// 해머 공중 회전 공격
	case CKirby::CRASHSTATE_ATTACK:
	{
		fAttack = 20.f;
		//HitStop_Rogic(pKirby);
		Camera_Shaking(0.5f);
	}
	break;
	// 해머 공중 회전 공격
	case CKirby::CRASHSTATE_BIGATTACK:
	{
		fAttack = 20.f;
		//HitStop_Rogic(pKirby);
		Camera_Shaking(0.5f);
	}
	break;

	default:
		fAttack = 5.f;
		break;
	}

	pCMonster->Minus_Hp(fAttack);
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

	if (static_cast<CCharacter*>(pPlayer)->Get_State() != CKirby::STATE_SLIDE)
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
	pObject->Set_DamageMoving(vKnockbackDir, fPower);
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

void CCollisionCenter::Compute_HitBoxDamage(CPhysXObject* pPlayer, CPhysXObject* pMonster)
{
	CKirby* pKirby = static_cast<CKirby*>(pPlayer);

	// 무적이 아닐 경우
	if (pKirby->isOverPower() == false)
	{
		_float fMonsterAttack = pMonster->Get_Attack();
		pKirby->Minus_Hp(fMonsterAttack);
		Camera_Shaking(1.2f);
	}
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
	CKirby* pCPlayer = static_cast<CKirby*>(pPlayer);
	CItemObject* pIItem = static_cast<CItemObject*>(pItem);

	_float fItemPoint = (_float)pIItem->Get_ItemPoint();
	pCPlayer->Plus_Coin((_uint)fItemPoint);
}

void CCollisionCenter::Compute_SuperPower(CPhysXObject* pPlayer, CPhysXObject* pItem)
{
	CItemObject* pIItem = static_cast<CItemObject*>(pItem);
	_float fItemPoint = (_float)pIItem->Get_ItemPoint();


	// 무적시간을 커비에게 넣어주면 됨.
}

void CCollisionCenter::Dodge_Timer_System(_float fTimeDelta)
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

void CCollisionCenter::Hit_TimeStop(_float fTimeRatio, _float fTime)
{
	m_bHitTimeStop = true;
	m_fHitTimeDeltaMaxResetTime = fTime;
	GAMEINSTANCE Set_FirstTimerRatio(fTimeRatio);
	GAMEINSTANCE Set_SecondTimerRatio(fTimeRatio);
}

void CCollisionCenter::Hit_Timer_System(_float fTimeDelta)
{
	if (m_bHitTimeStop == true)
	{
		m_fHitTimeDeltaResetTime += fTimeDelta;

		if (m_fHitTimeDeltaResetTime > m_fHitTimeDeltaMaxResetTime)
		{
			GAMEINSTANCE Restore_FirstTimer(3.f);
			GAMEINSTANCE Restore_SecondTimer(3.f);
			m_fHitTimeDeltaResetTime = 0.f;
			m_bHitTimeStop = false;
		}
	}
}

void CCollisionCenter::Free()
{
	for (auto& pLadder : m_Ladders)
		Safe_Release(pLadder);
	m_Ladders.clear();



	for (auto& ObjectVector : m_GameObjects)
	{
		for (auto& pObject : ObjectVector)
			Safe_Release(pObject);
		ObjectVector.clear();
	}



	__super::Free();

}
