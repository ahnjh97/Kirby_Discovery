#include "stdafx.h"
#include "Buffahorn_State.h"
#include "Buffahorn.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CBuffahorn_Idle_State::CBuffahorn_Idle_State()
{
}

void CBuffahorn_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBuffahorn_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (15.f > fDistance)
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_FIND, 50.f, false, true);

	if (0.5f < pBuffahorn->Get_AnimRatio() && 0.55f > pBuffahorn->Get_AnimRatio())
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_SLEEP);
	else
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
}

void CBuffahorn_Idle_State::OnStateExit()
{
}

CBuffahorn_Idle_State* CBuffahorn_Idle_State::Create()
{
	CBuffahorn_Idle_State* pInstance = new CBuffahorn_Idle_State();
	return pInstance;
}

void CBuffahorn_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region FIND STATE
//*********************************
//			 FIND STATE
//*********************************
CBuffahorn_Find_State::CBuffahorn_Find_State()
{
}

void CBuffahorn_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBuffahorn_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.5f);

	if (true == pBuffahorn->IsAnimFinished())
	{
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_WAIT, 50.f, true, true);
	}
}

void CBuffahorn_Find_State::OnStateExit()
{
}

CBuffahorn_Find_State* CBuffahorn_Find_State::Create()
{
	CBuffahorn_Find_State* pInstance = new CBuffahorn_Find_State();
	return pInstance;
}

void CBuffahorn_Find_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region WAIT STATE
//*********************************
//			 WAIT STATE
//*********************************
CBuffahorn_Wait_State::CBuffahorn_Wait_State()
{
}

void CBuffahorn_Wait_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBuffahorn_Wait_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	pTransformCom->Look_At(vKirbyPos);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (13.f > fDistance)
	{
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RUNSTART, 50.f, false, true);
	}
	else if(15.f < fDistance)
	{
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_CHARGEWAIT, 50.f, true, true);
	}

	if (0.5f < pBuffahorn->Get_AnimRatio() && 0.55f > pBuffahorn->Get_AnimRatio())
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_SLEEP);
	else
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
}

void CBuffahorn_Wait_State::OnStateExit()
{
}

CBuffahorn_Wait_State* CBuffahorn_Wait_State::Create()
{
	CBuffahorn_Wait_State* pInstance = new CBuffahorn_Wait_State();
	return pInstance;
}

void CBuffahorn_Wait_State::Free()
{
	__super::Free();
}


#pragma region RUN STATE
//*********************************
//			 RUN STATE
//*********************************
CBuffahorn_Run_State::CBuffahorn_Run_State()
{
}

void CBuffahorn_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fTimeDelta = 0.f;
}

void CBuffahorn_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pBuffahorn->IsAnimFinished())
	{
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RUN, 50.f, true, true);
	}

	if (CBuffahorn::BUFFAHORN_RUN == pBuffahorn->Get_State())
	{
		m_fTimeDelta += fTimeDelta;

		if (2.f > m_fTimeDelta)
			pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 10.f, fTimeDelta);
		else
		{
			pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_BRAKE, 50.f, false, true);
		}
	}

	_float fHeight = pController->Compute_Height(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));

	if(2.f < fHeight)
	{
		pBuffahorn->Set_JumpTime(15.f);
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_JUMP, 80.f, false, true);
	}

	if (1.5f > pController->Compute_Wall(XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK))))
	{
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_BOUNCETOTURN, 50.f, false, true);
	}
}

void CBuffahorn_Run_State::OnStateExit()
{
}

CBuffahorn_Run_State* CBuffahorn_Run_State::Create()
{
	CBuffahorn_Run_State* pInstance = new CBuffahorn_Run_State();
	return pInstance;
}

void CBuffahorn_Run_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region BRAKE STATE
//*********************************
//			 BRAKE STATE
//*********************************
CBuffahorn_Brake_State::CBuffahorn_Brake_State()
{
}

void CBuffahorn_Brake_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fSpeed = 1.f;
}

void CBuffahorn_Brake_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 브레이크 : 제곱 감속
	_float fDeceleration = m_fSpeed * m_fSpeed;

	if (0.f < m_fSpeed)
		m_fSpeed -= fTimeDelta * 0.4f;

	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (true == pBuffahorn->IsAnimFinished())
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_BRAKEEND, 45.f, false, true);
	else
		pController->Move_Dir(pTransformCom, (pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 10.f) * fDeceleration, fTimeDelta);

	if (CBuffahorn::BUFFAHORN_BRAKEEND == pBuffahorn->Get_State())
	{
		if(true == pBuffahorn->IsAnimFinished())
		{
			pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_CHARGEWAIT, 50.f, true, true);
		}
	}
}

void CBuffahorn_Brake_State::OnStateExit()
{
}

CBuffahorn_Brake_State* CBuffahorn_Brake_State::Create()
{
	CBuffahorn_Brake_State* pInstance = new CBuffahorn_Brake_State();
	return pInstance;
}

void CBuffahorn_Brake_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region JUMP STATE
//*********************************
//			 JUMP STATE
//*********************************
CBuffahorn_Jump_State::CBuffahorn_Jump_State()
{
}

void CBuffahorn_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBuffahorn_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 6.f, fTimeDelta);

	_float fJumpVelocity = pBuffahorn->Get_JumpTime();
	fJumpVelocity -= GRAVITY * fTimeDelta * 3.f;
	pController->Jump(pTransformCom, fJumpVelocity, fTimeDelta);
	pBuffahorn->Set_JumpTime(fJumpVelocity);

	if (true == pBuffahorn->IsAnimFinished() && true == pController->Is_Terrain())
	{
		pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RETURNJUMPEND, 100.f, false, true);
	}

	if (CBuffahorn::BUFFAHORN_RETURNJUMPEND == pBuffahorn->Get_State())
	{
		if (true == pBuffahorn->IsAnimFinished())
		{
			pBuffahorn->Set_BuffahornEye(CBuffahorn::BUFFAHORNEYE_IDLE);
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RUN, 50.f, true, true);
		}
	}
}

void CBuffahorn_Jump_State::OnStateExit()
{
}

CBuffahorn_Jump_State* CBuffahorn_Jump_State::Create()
{
	CBuffahorn_Jump_State* pInstance = new CBuffahorn_Jump_State();
	return pInstance;
}

void CBuffahorn_Jump_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CBuffahorn_Damage_State::CBuffahorn_Damage_State()
{
}

void CBuffahorn_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffSet);
}

void CBuffahorn_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pBuffahorn->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pBuffahorn->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		//pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 14.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pBuffahorn->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pBuffahorn->Set_DamageJumpPower(fDamageJumpPower);


		if (true == pBuffahorn->IsAnimFinished())
		{
			if (pBuffahorn->Get_Hp() <= 0.f)
				pBuffahorn->Set_Dead();
			else
			{
				pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_CHARGEWAIT, 50.f, true, true);
			}
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pBuffahorn->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pBuffahorn->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pBuffahorn->Set_PhyXState(PO_FLYDEADAWAY);
			pBuffahorn->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pBuffahorn->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pBuffahorn->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pBuffahorn->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pBuffahorn->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pBuffahorn->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > 0.7f)
			pBuffahorn->Set_Dead();
	}
}

void CBuffahorn_Damage_State::OnStateExit()
{
}

CBuffahorn_Damage_State* CBuffahorn_Damage_State::Create()
{
	CBuffahorn_Damage_State* pInstance = new CBuffahorn_Damage_State();
	return pInstance;
}

void CBuffahorn_Damage_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CBuffahorn_BounceToTurn_State::CBuffahorn_BounceToTurn_State()
{
}

void CBuffahorn_BounceToTurn_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffSet);
}

void CBuffahorn_BounceToTurn_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (CBuffahorn::BUFFAHORN_BOUNCETOTURN == pBuffahorn->Get_State())
	{
		if (true == pBuffahorn->IsAnimFinished())
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_CHARGEWAIT, 50.f, true, true);
	}
}

void CBuffahorn_BounceToTurn_State::OnStateExit()
{
}

CBuffahorn_BounceToTurn_State* CBuffahorn_BounceToTurn_State::Create()
{
	CBuffahorn_BounceToTurn_State* pInstance = new CBuffahorn_BounceToTurn_State();
	return pInstance;
}

void CBuffahorn_BounceToTurn_State::Free()
{
	__super::Free();
}
#pragma endregion
