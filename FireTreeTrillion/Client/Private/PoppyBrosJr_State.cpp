#include "stdafx.h"
#include "PoppyBrosJr_State.h"
#include "PoppyBrosJr.h"
#include "Kirby.h"
#include "PoppyBomb.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CPoppyBrosJr_Idle_State::CPoppyBrosJr_Idle_State()
{
}

void CPoppyBrosJr_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffset);

	m_fTimeDelta = 0.f;
}

void CPoppyBrosJr_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPoppyBrosJr* pPoppyJr = static_cast<CPoppyBrosJr*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if(CPoppyBrosJr::PS_TARGET == pPoppyJr->Get_PoppyState())
	{
		// 플레이어와 몬스터의 거리 계산
		_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

		// 플레이어를 향해 바라본다
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);

		m_fTimeDelta += fTimeDelta;

		if (3.f < m_fTimeDelta)
		{
			// 일정 거리 안으로 플레이어가 들어오면 상태 전환
			if (10.f > fDistance)
				pPoppyJr->Change_State(CPoppyBrosJr::POPPY_THROW, 50.f, false, true);

			m_fTimeDelta = 0.f;
		}
	}
	else if (CPoppyBrosJr::PS_NONTARGET == pPoppyJr->Get_PoppyState())
	{
		m_fTimeDelta += fTimeDelta;

		if (3.f < m_fTimeDelta)
		{
			pPoppyJr->Change_State(CPoppyBrosJr::POPPY_THROW, 50.f, false, true);

			m_fTimeDelta = 0.f;
		}
	}
}

void CPoppyBrosJr_Idle_State::OnStateExit()
{
}

CPoppyBrosJr_Idle_State* CPoppyBrosJr_Idle_State::Create()
{
	CPoppyBrosJr_Idle_State* pInstance = new CPoppyBrosJr_Idle_State();
	return pInstance;
}

void CPoppyBrosJr_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region ATTACK STATE
//*********************************
//			 ATTACK STATE
//*********************************
CPoppyBrosJr_Attack_State::CPoppyBrosJr_Attack_State()
{
}

void CPoppyBrosJr_Attack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffset);

	m_bBomb = true;
}

void CPoppyBrosJr_Attack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPoppyBrosJr* pPoppyJr = static_cast<CPoppyBrosJr*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPoppyPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vPoppyLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (CPoppyBrosJr::PS_TARGET == pPoppyJr->Get_PoppyState())
	{
		if (true == m_bBomb)
		{
			m_bBomb = false;

			HRESULT hr = S_OK;

			CPoppyBomb::POPPYBOMB_DESC PoppyBombDesc = {};
			_float4 vPos = pPoppyJr->Compute_BoneWorldMatrix();
			PoppyBombDesc.vPosition = vPos;
			PoppyBombDesc.vLook = vPoppyLook;
			PoppyBombDesc.vTargetPosition = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			PoppyBombDesc.pGameObject = pPoppyJr;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Bomb"), TEXT("Prototype_GameObject_PoppyBomb"), &PoppyBombDesc);
			CHECK_FAILED(hr);
		}
	}
	else if (CPoppyBrosJr::PS_NONTARGET == pPoppyJr->Get_PoppyState())
	{
		if (true == m_bBomb)
		{
			m_bBomb = false;

			HRESULT hr = S_OK;

			CPoppyBomb::POPPYBOMB_DESC PoppyBombDesc = {};
			_float4 vPos = pPoppyJr->Compute_BoneWorldMatrix();
			PoppyBombDesc.vPosition = vPos;
			PoppyBombDesc.vLook = vPoppyLook;
			PoppyBombDesc.vTargetPosition = vPoppyPos + XMVector3Normalize(vPoppyLook) * 7.f;
			PoppyBombDesc.pGameObject = pPoppyJr;
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Bomb"), TEXT("Prototype_GameObject_PoppyBomb"), &PoppyBombDesc);
			CHECK_FAILED(hr);
		}
	}

	if (true == pPoppyJr->IsAnimFinished())
		pPoppyJr->Change_State(CPoppyBrosJr::POPPY_ENEMYWAIT2, 50.f, true, true);
}

void CPoppyBrosJr_Attack_State::OnStateExit()
{
}

CPoppyBrosJr_Attack_State* CPoppyBrosJr_Attack_State::Create()
{
	CPoppyBrosJr_Attack_State* pInstance = new CPoppyBrosJr_Attack_State();
	return pInstance;
}

void CPoppyBrosJr_Attack_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CPoppyBrosJr_Damage_State::CPoppyBrosJr_Damage_State()
{
}

void CPoppyBrosJr_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);
}

void CPoppyBrosJr_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPoppyBrosJr* pPoppyJr = static_cast<CPoppyBrosJr*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pPoppyJr->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pPoppyJr->Get_DamegeDir();
		if (vDamegeDir != XMVectorZero())
			pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pPoppyJr->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pPoppyJr->Set_DamageJumpPower(fDamageJumpPower);


		if (true == pPoppyJr->IsAnimFinished() || pController->Is_Terrain())
		{
			if (pPoppyJr->Get_Hp() <= 0.f)
				pPoppyJr->Set_Dead();
			else
			{
				pPoppyJr->Change_State(CPoppyBrosJr::POPPY_ENEMYWAIT2, 50.f, true, true);
			}
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pPoppyJr->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pPoppyJr->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pPoppyJr->Set_PhyXState(PO_FLYDEADAWAY);
			pPoppyJr->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pPoppyJr->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pPoppyJr->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pPoppyJr->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pPoppyJr->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pPoppyJr->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
			pPoppyJr->Set_Dead();
	}
}

void CPoppyBrosJr_Damage_State::OnStateExit()
{
}

CPoppyBrosJr_Damage_State* CPoppyBrosJr_Damage_State::Create()
{
	CPoppyBrosJr_Damage_State* pInstance = new CPoppyBrosJr_Damage_State();
	return pInstance;
}

void CPoppyBrosJr_Damage_State::Free()
{
	__super::Free();
}

#pragma endregion