#include "stdafx.h"
#include "Kabu_State.h"
#include "Kabu.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CKabu_Idle_State::CKabu_Idle_State()
{
}

void CKabu_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CKabu_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
}

void CKabu_Idle_State::OnStateExit()
{
}

CKabu_Idle_State* CKabu_Idle_State::Create()
{
	CKabu_Idle_State* pInstance = new CKabu_Idle_State();
	return pInstance;
}

void CKabu_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CKabu_Damage_State::CKabu_Damage_State()
{
}

void CKabu_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CKabu_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKabu* pKabu = static_cast<CKabu*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pKabu->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pKabu->Get_DamegeDir();
		if (vDamegeDir != XMVectorZero())
			pTransformCom->Look_At_Axis(pKabu->Get_Look());

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pKabu->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pKabu->Set_DamageJumpPower(fDamageJumpPower);

		if (true == pKabu->IsAnimFinished())
		{
			if (pKabu->Get_Hp() <= 0.f)
				pKabu->Set_Dead();
			else
				pKabu->Change_State(CKabu::KABU_WARP1, 40.f, false, true);
		}

	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pKabu->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pKabu->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pKabu->Set_PhyXState(PO_FLYDEADAWAY);
			pKabu->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pKabu->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pKabu->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pKabu->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pKabu->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pKabu->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > 0.4f || pController->Is_Terrain())
			pKabu->Set_Dead();
	}
}

void CKabu_Damage_State::OnStateExit()
{
}

CKabu_Damage_State* CKabu_Damage_State::Create()
{
	CKabu_Damage_State* pInstance = new CKabu_Damage_State();
	return pInstance;
}

void CKabu_Damage_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region WARP STATE
//*********************************
//			 WARP STATE
//*********************************
CKabu_Warp_State::CKabu_Warp_State()
{
}

void CKabu_Warp_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CKabu_Warp_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKabu* pKabu = static_cast<CKabu*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * 9.f);

	if (true == pKabu->IsAnimFinished())
		pKabu->Change_State(CKabu::KABU_WAIT, 50.f, false, true);
}

void CKabu_Warp_State::OnStateExit()
{
}

CKabu_Warp_State* CKabu_Warp_State::Create()
{
	CKabu_Warp_State* pInstance = new CKabu_Warp_State();
	return pInstance;
}

void CKabu_Warp_State::Free()
{
	__super::Free();
}

#pragma endregion