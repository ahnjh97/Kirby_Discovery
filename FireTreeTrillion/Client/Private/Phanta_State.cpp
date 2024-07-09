#include "stdafx.h"
#include "Phanta_State.h"
#include "Phanta.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CPhanta_Idle_State::CPhanta_Idle_State()
{
}

void CPhanta_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CPhanta_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPhanta* pPhanta = static_cast<CPhanta*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

	if(pPhanta->IsAnimFinished())
		pPhanta->Change_State(CPhanta::PHANTA_ATTACK, 50.f, true, true);
}

void CPhanta_Idle_State::OnStateExit()
{
}

CPhanta_Idle_State* CPhanta_Idle_State::Create()
{
	CPhanta_Idle_State* pInstance = new CPhanta_Idle_State();
	return pInstance;
}

void CPhanta_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region MOVE STATE
//*********************************
//			 MOVE STATE
//*********************************
CPhanta_Move_State::CPhanta_Move_State()
{
}

void CPhanta_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fTimeDelta = 0.f;
	m_fSpeed = 4.f;
}

void CPhanta_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPhanta* pPhanta = static_cast<CPhanta*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 몬스터, 플레이어 위치
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	pController->FreeFall(pTransformCom, fTimeDelta, 0.01f);

	m_fTimeDelta += fTimeDelta;

	if (4.f > m_fTimeDelta)
	{
		// 플레이어를 향해 바라본다
		if (3.f > m_fTimeDelta)
			pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);
		//else
		//{
		//	if (0.f < m_fSpeed)
		//		m_fSpeed -= fTimeDelta * 2.f;
		//	else
		//		m_fSpeed = 0.f;
		//}
		pController->Move_Dir(pTransformCom, XMVector3Normalize(vLook) * fTimeDelta * m_fSpeed, fTimeDelta);
	}
	else
		pPhanta->Change_State(CPhanta::PHANTA_BRAKE, 50.f, false, true);
}

void CPhanta_Move_State::OnStateExit()
{
}

CPhanta_Move_State* CPhanta_Move_State::Create()
{
	CPhanta_Move_State* pInstance = new CPhanta_Move_State();
	return pInstance;
}

void CPhanta_Move_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CPhanta_Damage_State::CPhanta_Damage_State()
{
}

void CPhanta_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);
}

void CPhanta_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPhanta* pPhanta = static_cast<CPhanta*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pPhanta->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pPhanta->Get_DamegeDir();
		if (vDamegeDir != XMVectorZero())
			pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pPhanta->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pPhanta->Set_DamageJumpPower(fDamageJumpPower);


		if (true == pPhanta->IsAnimFinished() || pController->Is_Terrain())
		{
			if (pPhanta->Get_Hp() <= 0.f)
				pPhanta->Set_Dead();
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pPhanta->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pPhanta->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pPhanta->Set_PhyXState(PO_FLYDEADAWAY);
			pPhanta->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pPhanta->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pPhanta->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pPhanta->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pPhanta->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pPhanta->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
			pPhanta->Set_Dead();

	}
}

void CPhanta_Damage_State::OnStateExit()
{
}

CPhanta_Damage_State* CPhanta_Damage_State::Create()
{
	CPhanta_Damage_State* pInstance = new CPhanta_Damage_State();
	return pInstance;
}

void CPhanta_Damage_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region BRAKE STATE
//*********************************
//			 BRAKE STATE
//*********************************
CPhanta_Brake_State::CPhanta_Brake_State()
{
}

void CPhanta_Brake_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fSpeed = 4.f;
}

void CPhanta_Brake_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPhanta* pPhanta = static_cast<CPhanta*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	if (0.f < m_fSpeed)
		m_fSpeed -= fTimeDelta * 8.f;
	else
	{
		if(pPhanta->IsAnimFinished())
			pPhanta->Change_State(CPhanta::PHANTA_FLYINGFIND, 50.f, false, true);
	}

	pController->Move_Dir(pTransformCom, XMVector3Normalize(vLook) * fTimeDelta * m_fSpeed, fTimeDelta);
}

void CPhanta_Brake_State::OnStateExit()
{
}

CPhanta_Brake_State* CPhanta_Brake_State::Create()
{
	CPhanta_Brake_State* pInstance = new CPhanta_Brake_State();
	return pInstance;
}

void CPhanta_Brake_State::Free()
{
	__super::Free();
}
#pragma endregion
