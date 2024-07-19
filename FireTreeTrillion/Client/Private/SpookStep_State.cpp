#include "stdafx.h"
#include "SpookStep_State.h"
#include "SpookStep.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CSpookStep_Idle_State::CSpookStep_Idle_State()
{
}

void CSpookStep_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fGravityOffset = 0.f;
}

void CSpookStep_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSpookStep* pSpookStep = static_cast<CSpookStep*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	pController->FreeFall(pTransformCom, fTimeDelta, m_fGravityOffset);
	m_fGravityOffset += fTimeDelta;

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

	if (pSpookStep->IsAnimFinished() && pController->Is_Terrain())
		pSpookStep->Change_State(CSpookStep::SPOOKSTEP_MOVE, 90.f, true, true);
}

void CSpookStep_Idle_State::OnStateExit()
{
}

CSpookStep_Idle_State* CSpookStep_Idle_State::Create()
{
	CSpookStep_Idle_State* pInstance = new CSpookStep_Idle_State();
	return pInstance;
}

void CSpookStep_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region MOVE STATE
//*********************************
//			 MOVE STATE
//*********************************
CSpookStep_Move_State::CSpookStep_Move_State()
{
}

void CSpookStep_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CSpookStep_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSpookStep* pCappyBody = static_cast<CSpookStep*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();
	
	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 3.f);

	pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * 3.f, fTimeDelta);
}

void CSpookStep_Move_State::OnStateExit()
{
}

CSpookStep_Move_State* CSpookStep_Move_State::Create()
{
	CSpookStep_Move_State* pInstance = new CSpookStep_Move_State();
	return pInstance;
}

void CSpookStep_Move_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CSpookStep_Damage_State::CSpookStep_Damage_State()
{
}

void CSpookStep_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);
}

void CSpookStep_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSpookStep* pSpookStep = static_cast<CSpookStep*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pSpookStep->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pSpookStep->Get_DamegeDir();
		if (vDamegeDir != XMVectorZero())
			pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pSpookStep->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pSpookStep->Set_DamageJumpPower(fDamageJumpPower);


		if (true == pSpookStep->IsAnimFinished() || pController->Is_Terrain())
		{
			if (pSpookStep->Get_Hp() <= 0.f)
				pSpookStep->Set_Dead();
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pSpookStep->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pSpookStep->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pSpookStep->Set_PhyXState(PO_FLYDEADAWAY);
			pSpookStep->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pSpookStep->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pSpookStep->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pSpookStep->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pSpookStep->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pSpookStep->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
			pSpookStep->Set_Dead();

	}
}

void CSpookStep_Damage_State::OnStateExit()
{
}

CSpookStep_Damage_State* CSpookStep_Damage_State::Create()
{
	CSpookStep_Damage_State* pInstance = new CSpookStep_Damage_State();
	return pInstance;
}

void CSpookStep_Damage_State::Free()
{
	__super::Free();
}
#pragma endregion
