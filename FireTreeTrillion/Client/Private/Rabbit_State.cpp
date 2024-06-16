#include "stdafx.h"
#include "Rabbit_State.h"
#include "Rabbit.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CRabbit_Idle_State::CRabbit_Idle_State()
{
}

void CRabbit_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CRabbit_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CRabbit* pRabbit = static_cast<CRabbit*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 몬스터, 플레이어 위치
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vPos.m128_f32[1] = 0.f;
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vKirbyPos.m128_f32[1] = 0.f;

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if(CRabbit::RS_TARGET == pRabbit->Get_RabbitState())
	{
		// 플레이어와 몬스터의 거리 계산
		_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

		_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
		vLook.m128_f32[1] = 0.f;

		// 몬스터와 플레이어 사이의 각도 계산
		_float fAngle = XMVectorGetX(XMVector3AngleBetweenVectors(XMVector3Normalize(vLook), XMVector3Normalize(XMVectorSubtract(vKirbyPos, vPos))));

		// 몬스터가 타겟을 찾았을 때
		if (true == pRabbit->Get_Find())
		{
			// 타겟이 일정 범위 안에 있으면 계속 쫒아감
			if (15.f > fDistance)
			{
				// 플레이어를 향해 회전
				pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

				if (true == pRabbit->IsAnimFinished())
				{
					pRabbit->Set_TimeDelta(0.f);

					// 일정 거리를 넘어가면 최소 사정거리 점프
					if (7.f < fDistance)
					{
						//_vector vEndPos = XMVector3Normalize(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
						pRabbit->Compute_Parabola(pTransformCom->Get_State_Vector(CTransform::STATE_POSITION) + (XMVector3Normalize(XMVectorSubtract(vKirbyPos, vPos)) * 5.f));
					}
					// 플레이어를 향해 점프
					else if (7.f >= fDistance)
						pRabbit->Compute_Parabola(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION));

					pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
					pRabbit->Change_State(CRabbit::RABBIT_JUMPSTART, 100.f, false, true);
				}
			}
		}
		// 일정 거리 안으로 플레이어가 들어오면 상태 전환
		if (10.f > fDistance)
		{
			if (false == pRabbit->Get_Find())
			{
				if (135.f > XMConvertToDegrees(fAngle))
				{
					pRabbit->Set_Find(true);
					pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
					pRabbit->Change_State(CRabbit::RABBIT_FIND, 40.f, false, true);
				}
			}
		}
		// 타겟이 일정 범위를 넘어갔을 때 타겟을 못찾음
		else if (15.f < fDistance)
		{
			pRabbit->Set_Find(false);

			// 여러 상태의 IDLE로 전환
			if (true == pRabbit->IsAnimFinished())
			{
				if (rand() % 3 == 1 || rand() % 3 == 2)
					pRabbit->Change_State(CRabbit::RABBIT_WAIT, 40.f, false, true);
				else
					pRabbit->Change_State(CRabbit::RABBIT_LOOKAROUND, 40.f, false, true);
			}
		}
		else
		{
			// 여러 상태의 IDLE로 전환
			if (true == pRabbit->IsAnimFinished())
			{
				if (rand() % 3 == 1 || rand() % 3 == 2)
					pRabbit->Change_State(CRabbit::RABBIT_WAIT, 40.f, false, true);
				else
					pRabbit->Change_State(CRabbit::RABBIT_LOOKAROUND, 40.f, false, true);
			}
		}
	}
	else if (CRabbit::RS_NONTARGET == pRabbit->Get_RabbitState())
	{
		// 플레이어를 향해 회전
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

		if (true == pRabbit->IsAnimFinished())
		{
			pRabbit->Set_TimeDelta(0.f);

			pRabbit->Compute_Parabola(pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));

			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
			pRabbit->Change_State(CRabbit::RABBIT_JUMPSTART, 100.f, false, true);
		}
	}

	// eye state
	if(CRabbit::RABBIT_WAIT == pRabbit->Get_State())
	{
		if (0.45f < pRabbit->Get_AnimRatio() && 0.5f > pRabbit->Get_AnimRatio())
			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_HAPPY);
		else
			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
	}
	else if (CRabbit::RABBIT_LOOKAROUND == pRabbit->Get_State())
	{
		if (0.59f < pRabbit->Get_AnimRatio() && 0.6f > pRabbit->Get_AnimRatio())
			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_HAPPY);
		else if (0.63f < pRabbit->Get_AnimRatio() && 0.64f > pRabbit->Get_AnimRatio())
			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_HAPPY);
		else
			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
	}
}

void CRabbit_Idle_State::OnStateExit()  
{
}

CRabbit_Idle_State* CRabbit_Idle_State::Create()
{
	CRabbit_Idle_State* pInstance = new CRabbit_Idle_State();
	return pInstance;
}

void CRabbit_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region FIND STATE
//*********************************
//			 FIND STATE
//*********************************
CRabbit_Find_State::CRabbit_Find_State()
{
}

void CRabbit_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CRabbit_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CRabbit* pRabbit = static_cast<CRabbit*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

	if (true == pRabbit->IsAnimFinished())
	{
		pRabbit->Set_TimeDelta(0.f);
		pRabbit->Compute_Parabola(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
		pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
		pRabbit->Change_State(CRabbit::RABBIT_JUMPSTART, 100.f, false, true);
	}
}

void CRabbit_Find_State::OnStateExit()
{
}

CRabbit_Find_State* CRabbit_Find_State::Create()
{
	CRabbit_Find_State* pInstance = new CRabbit_Find_State();
	return pInstance;
}

void CRabbit_Find_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region JUMP STATE
//*********************************
//			 JUMP STATE
//*********************************
CRabbit_Jump_State::CRabbit_Jump_State()
{
}

void CRabbit_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CRabbit_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CRabbit* pRabbit = static_cast<CRabbit*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	pTransformCom->Look_At_Axis(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));

	if (true == pRabbit->IsAnimFinished())
	{
		switch (pRabbit->Get_State())
		{
		case CRabbit::RABBIT_JUMPSTART:
			pRabbit->Change_State(CRabbit::RABBIT_JUMP, 85.f, false, true);
			break;
		case CRabbit::RABBIT_JUMP:
			pRabbit->Change_State(CRabbit::RABBIT_JUMPEND, 45.f, false, true);
			break;
		case CRabbit::RABBIT_JUMPEND:
			pRabbit->Change_State(CRabbit::RABBIT_JUMPFALL, 45.f, false, true);
			break;
		default:
			pRabbit->Change_State(CRabbit::RABBIT_WAIT, 45.f, false, true);
			break;
		}
	}

	_float fJumpTimeDelta = pRabbit->Get_TimeDelta();
	fJumpTimeDelta += fTimeDelta;
	// 매 Tick 점프 중인 위치 벡터 받아오기
	_vector vGoPos = pRabbit->JumpAttak(fJumpTimeDelta);

	_bool bJump = { false };

	// 점프 위치 벡터를 physx에 던지기
	bJump = pController->Jump_Parabola(pTransformCom, vGoPos, fTimeDelta);

	pRabbit->Set_TimeDelta(fJumpTimeDelta);

	pRabbit->Get_State();

	if(CRabbit::RABBIT_JUMPFALL == pRabbit->Get_State())
	{
		if (!bJump)
		{
			// 자유 낙하
			pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
			pRabbit->Change_State(CRabbit::RABBIT_JUMPLANDING, 45.f, false, true);
		}
	}
}

void CRabbit_Jump_State::OnStateExit()
{
}

CRabbit_Jump_State* CRabbit_Jump_State::Create()
{
	CRabbit_Jump_State* pInstance = new CRabbit_Jump_State();
	return pInstance;
}

void CRabbit_Jump_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region JUMPLANDING STATE
//*********************************
//			 JUMPLANDING STATE
//*********************************
CRabbit_JumpLanding_State::CRabbit_JumpLanding_State()
{
}

void CRabbit_JumpLanding_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CRabbit_JumpLanding_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CRabbit* pRabbit = static_cast<CRabbit*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (pRabbit->IsAnimFinished())
	{
		pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
		pRabbit->Change_State(CRabbit::RABBIT_WAIT, 100.f, false, true);
	}
}

void CRabbit_JumpLanding_State::OnStateExit()
{
}

CRabbit_JumpLanding_State* CRabbit_JumpLanding_State::Create()
{
	CRabbit_JumpLanding_State* pInstance = new CRabbit_JumpLanding_State();
	return pInstance;
}

void CRabbit_JumpLanding_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CRabbit_Damage_State::CRabbit_Damage_State()
{
}

void CRabbit_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();
}

void CRabbit_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CRabbit* pRabbit = static_cast<CRabbit*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));


	if (pRabbit->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pRabbit->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pRabbit->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pRabbit->Set_DamageJumpPower(fDamageJumpPower);

		if (true == pRabbit->IsAnimFinished() || pController->Is_Terrain())
		{
			pRabbit->Set_RabbitEye(CRabbit::RABBITEYE_IDLE);
			pRabbit->Change_State(CRabbit::RABBIT_WAIT, 45.f, false, true);
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pRabbit->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pRabbit->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;
		if (m_fFlyTime > 2.f)
		{
			pRabbit->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pRabbit->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pRabbit->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pRabbit->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pRabbit->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > 0.7f)
			pRabbit->Set_Dead();
		
	}

}

void CRabbit_Damage_State::OnStateExit()
{
}

CRabbit_Damage_State* CRabbit_Damage_State::Create()
{
	CRabbit_Damage_State* pInstance = new CRabbit_Damage_State();
	return pInstance;
}

void CRabbit_Damage_State::Free()
{
	__super::Free();
}

#pragma endregion