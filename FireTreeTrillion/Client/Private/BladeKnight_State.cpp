#include "stdafx.h"
#include "BladeKnight_State.h"
#include "BladeKnight.h"
#include "Kirby.h"


#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CBladeKnight_Idle_State::CBladeKnight_Idle_State()
{
}

void CBladeKnight_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBladeKnight_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
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

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	if(6.f > fDistance)
		pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_FIND, 50.f, false, true);
}

void CBladeKnight_Idle_State::OnStateExit()
{
}

CBladeKnight_Idle_State* CBladeKnight_Idle_State::Create()
{
	CBladeKnight_Idle_State* pInstance = new CBladeKnight_Idle_State();
	return pInstance;
}

void CBladeKnight_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region MOVE STATE
//*********************************
//			 MOVE STATE
//*********************************
CBladeKnight_Move_State::CBladeKnight_Move_State()
{
}

void CBladeKnight_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fSpeed = 3.5f;
}

void CBladeKnight_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
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

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리를 벗어나면 IDLE 상태
	if (15.f < fDistance)
		pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_WAIT, 55.f, true, true);
	// 일정 거리 안으로 들어오면 공격
	else if(6.f > fDistance)
	{
		if(rand() % 2 == 0)
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_ATTACKSTART, 55.f, false, true);
		else
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_TORNADOATTACKCHARGE, 55.f, false, true);
	}

	pTransformCom->Look_At(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
	pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * m_fSpeed * fTimeDelta, fTimeDelta);
}

void CBladeKnight_Move_State::OnStateExit()
{
}

CBladeKnight_Move_State* CBladeKnight_Move_State::Create()
{
	CBladeKnight_Move_State* pInstance = new CBladeKnight_Move_State();
	return pInstance;
}

void CBladeKnight_Move_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region FIND STATE
//*********************************
//			 FIND STATE
//*********************************
CBladeKnight_Find_State::CBladeKnight_Find_State()
{
}

void CBladeKnight_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fTimeDelta = 0.f;
}

void CBladeKnight_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 누적 시간
	m_fTimeDelta += fTimeDelta;

	// 몬스터, 플레이어 위치
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vPos.m128_f32[1] = 0.f;
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vKirbyPos.m128_f32[1] = 0.f;

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	if (1.f < m_fTimeDelta)
	{
		if (4.f < fDistance)
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_MOVE, 55.f, true, true);
	}

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

	if(CBladeKnight::BLADEKNIGHT_FINDWAIT == pBladeKnight->Get_State())
	{
		if(1.f < m_fTimeDelta)
		{
			if(rand() % 2 == 0)
				pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_ATTACKSTART, 55.f, false, true);
			else
				pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_TORNADOATTACKCHARGE, 55.f, false, true);
		}
	}
	else
	{
		if(true == pBladeKnight->IsAnimFinished())
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_MOVE, 55.f, true, true);
	}
}

void CBladeKnight_Find_State::OnStateExit()
{
}

CBladeKnight_Find_State* CBladeKnight_Find_State::Create()
{
	CBladeKnight_Find_State* pInstance = new CBladeKnight_Find_State();
	return pInstance;
}

void CBladeKnight_Find_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region ATTACK STATE
//*********************************
//			 ATTACK STATE
//*********************************
CBladeKnight_Attack_State::CBladeKnight_Attack_State()
{
}

void CBladeKnight_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fX = 0.f;
	bStop = false;
	m_fSpeed = 14.f;
}

void CBladeKnight_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if(CBladeKnight::BLADEKNIGHT_ATTACKSTART == pBladeKnight->Get_State() && true == pBladeKnight->IsAnimFinished())
	{
		if(rand() % 2 == 0)
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_ATTACK, 55.f, false, true);
		else
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_DOUBLEATTACK, 55.f, false, true);
	}

	// SmoothStep 보간
	if (0.45f > m_fX)
		m_fX += fTimeDelta;
	else
	{
		bStop = true;
		m_fX = 0.f;
	}

	_float fTime = clamp(m_fX - 0.f, 0.f, 0.45f);

	if (CBladeKnight::BLADEKNIGHT_ATTACK == pBladeKnight->Get_State() || CBladeKnight::BLADEKNIGHT_DOUBLEATTACK == pBladeKnight->Get_State())
	{
		if (CBladeKnight::BLADEKNIGHT_DOUBLEATTACK == pBladeKnight->Get_State())
		{
			_float fAnimRatio = pBladeKnight->Get_AnimRatio();

			if (0.3f < fAnimRatio)
				m_fSpeed = 0.f;
		}
		if (false == bStop)
			pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * m_fSpeed * (fTime * fTime * (3.f - 2.f * fTime) * 2.f), fTimeDelta);

		if(pBladeKnight->IsAnimFinished())
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_RETREAT, 55.f, false, true);
	}
}

void CBladeKnight_Attack_State::OnStateExit()
{
}

CBladeKnight_Attack_State* CBladeKnight_Attack_State::Create()
{
	CBladeKnight_Attack_State* pInstance = new CBladeKnight_Attack_State();
	return pInstance;
}

void CBladeKnight_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region RETREAT STATE
//*********************************
//			 RETREAT STATE
//*********************************
CBladeKnight_Retreat_State::CBladeKnight_Retreat_State()
{
}

void CBladeKnight_Retreat_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fSpeed = 3.f;
	m_bRecoverSpeed = false;
}

void CBladeKnight_Retreat_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_float fAnimRatio = pBladeKnight->Get_AnimRatio();

	// 백스텝 움직임 제어
	if (0.25f > fAnimRatio)
		m_fSpeed -= fTimeDelta * 3.f;
	else if (0.25f <= fAnimRatio && 0.35f >= fAnimRatio)
	{
		if (false == m_bRecoverSpeed)
		{
			m_fSpeed = 3.f;
			m_bRecoverSpeed = true;
		}
	}
	else if (0.35f < fAnimRatio && 0.6f > fAnimRatio)
		m_fSpeed -= fTimeDelta * 3.f;
	else
		m_fSpeed = 0.f;

	if (0.f > m_fSpeed)
		m_fSpeed = 0.f;

	pController->Move_Dir(pTransformCom, -pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * m_fSpeed * fTimeDelta, fTimeDelta);

	if(true == pBladeKnight->IsAnimFinished())
		pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_FINDWAIT, 55.f, false, true);
}

void CBladeKnight_Retreat_State::OnStateExit()
{
}

CBladeKnight_Retreat_State* CBladeKnight_Retreat_State::Create()
{
	CBladeKnight_Retreat_State* pInstance = new CBladeKnight_Retreat_State();
	return pInstance;
}

void CBladeKnight_Retreat_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region TORNADOATTACK STATE
//*********************************
//			 TORNADOATTACK STATE
//*********************************
CBladeKnight_TornadoAttack_State::CBladeKnight_TornadoAttack_State()
{
}

void CBladeKnight_TornadoAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBladeKnight_TornadoAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (CBladeKnight::BLADEKNIGHT_TORNADOATTACKCHARGE == pBladeKnight->Get_State() && true == pBladeKnight->IsAnimFinished())
		pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_TORNADOATTACK, 55.f, false, true);

	if (CBladeKnight::BLADEKNIGHT_TORNADOATTACK == pBladeKnight->Get_State())
	{
		if (pBladeKnight->IsAnimFinished())
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_FINDWAIT, 55.f, false, true);
	}
}

void CBladeKnight_TornadoAttack_State::OnStateExit()
{
}

CBladeKnight_TornadoAttack_State* CBladeKnight_TornadoAttack_State::Create()
{
	CBladeKnight_TornadoAttack_State* pInstance = new CBladeKnight_TornadoAttack_State();
	return pInstance;
}

void CBladeKnight_TornadoAttack_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CBladeKnight_Damage_State::CBladeKnight_Damage_State()
{
}

void CBladeKnight_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CBladeKnight_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBladeKnight* pBladeKnight = static_cast<CBladeKnight*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pBladeKnight->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pBladeKnight->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pBladeKnight->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pBladeKnight->Set_DamageJumpPower(fDamageJumpPower);


		if (true == pBladeKnight->IsAnimFinished() || pController->Is_Terrain())
		{
			pBladeKnight->Change_State(CBladeKnight::BLADEKNIGHT_WAIT, 50.f, true, true);
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pBladeKnight->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pBladeKnight->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;
		if (m_fFlyTime > 2.f)
		{
			pBladeKnight->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pBladeKnight->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pBladeKnight->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pBladeKnight->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pBladeKnight->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > 0.7f)
			pBladeKnight->Set_Dead();
	}

}

void CBladeKnight_Damage_State::OnStateExit()
{
}

CBladeKnight_Damage_State* CBladeKnight_Damage_State::Create()
{
	CBladeKnight_Damage_State* pInstance = new CBladeKnight_Damage_State();
	return pInstance;
}

void CBladeKnight_Damage_State::Free()
{
	__super::Free();
}

#pragma endregion