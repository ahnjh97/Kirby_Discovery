#include "stdafx.h"
#include "Awoofy_State.h"
#include "Awoofy.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CAwoofy_Idle_State::CAwoofy_Idle_State()
{
}

void CAwoofy_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CAwoofy_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (CAwoofy::MON_SLEEP == pAwoofy->Get_MonState())
	{
		pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_SLEEP);
		return;
	}

	else if (CAwoofy::MON_WAIT == pAwoofy->Get_MonState())
	{
		// 플레이어와 몬스터의 거리 계산
		_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));
		if (LEVEL_SIMBA == *m_pGameInstance->Get_CurrentLevelID())
			fDistance = 0.f;

		// 일정 거리 안으로 플레이어가 들어오면 상태 전환
		if (10.f > fDistance)
		{
			pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
			pAwoofy->Change_State(CAwoofy::AWOOFY_FIND, 40.f, false, true);
		}

		// 여러 상태의 IDLE로 전환
		if (true == pAwoofy->IsAnimFinished())
		{
			if (rand() % 5 == 0)
				pAwoofy->Change_State(CAwoofy::AWOOFY_GROOMING, 45.f, false, true);
			else if (rand() % 5 == 1)
				pAwoofy->Change_State(CAwoofy::AWOOFY_LOOKAROUND, 40.f, false, true);
			else
				pAwoofy->Change_State(CAwoofy::AWOOFY_WAIT, 40.f, false, true);
		}

		m_fTimeDelta += fTimeDelta;

		// Awoofy의 역동적인 eye state
		if (CAwoofy::AWOOFY_WAIT == pAwoofy->Get_State() || CAwoofy::AWOOFY_LOOKAROUND == pAwoofy->Get_State())
		{
			if (2.5f < m_fTimeDelta)
			{
				pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_SLEEP);

				if (2.65f < m_fTimeDelta)
					m_fTimeDelta = 0.f;
			}
			else
				pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
		}
		else if (CAwoofy::AWOOFY_GROOMING == pAwoofy->Get_State())
		{
			m_fTimeDelta = 0.f;

			if (0.45f < pAwoofy->Get_AnimRatio() && 0.75f > pAwoofy->Get_AnimRatio())
				pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_HAPPY);
			else if (0.8f > pAwoofy->Get_AnimRatio())
				pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
			else if (0.8f <= pAwoofy->Get_AnimRatio())
				pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_ANGER);
		}
	}

	//if (m_pGameInstance->Get_DIKeyState(DIK_SPACE, KEY_DOWN))
	//	pAwoofy->Change_State(CAwoofy::AWOOFY_DAMAGE, 40.f, false, true);
}

void CAwoofy_Idle_State::OnStateExit()
{
}

CAwoofy_Idle_State* CAwoofy_Idle_State::Create()
{
	CAwoofy_Idle_State* pInstance = new CAwoofy_Idle_State();
	return pInstance;
}

void CAwoofy_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region WALK STATE
//*********************************
//			 WALK STATE
//*********************************
CAwoofy_Walk_State::CAwoofy_Walk_State()
{
}

void CAwoofy_Walk_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fDistance = 3.f;
}

void CAwoofy_Walk_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	m_fAngle += fTimeDelta * 50.f;
	m_fDistance = 3.f;
	m_vOriginPos = pAwoofy->Get_Pos();

	m_vRotatePos.x = m_vOriginPos.x + (m_fDistance * sin(XMConvertToRadians(m_fAngle)));
	m_vRotatePos.y = m_vOriginPos.y;
	m_vRotatePos.z = m_vOriginPos.z - (m_fDistance * cos(XMConvertToRadians(m_fAngle)));

	pController->Move(pTransformCom, m_vRotatePos, fTimeDelta);

	m_vRally = m_vRotatePos - m_vBeforePos;
	pTransformCom->Look_At_Axis(m_vRally);
	m_vBeforePos = m_vRotatePos;

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (6.f > fDistance)
		pAwoofy->Change_State(CAwoofy::AWOOFY_FIND, 40.f, false, true);
}

void CAwoofy_Walk_State::OnStateExit()
{
}

CAwoofy_Walk_State* CAwoofy_Walk_State::Create()
{
	CAwoofy_Walk_State* pInstance = new CAwoofy_Walk_State();
	return pInstance;
}

void CAwoofy_Walk_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region RUN STATE
//*******************************************************
//						 RUN STATE
//*******************************************************
CAwoofy_Run_State::CAwoofy_Run_State()
{
}

void CAwoofy_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CAwoofy_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	m_fTimeDelta += fTimeDelta;

	// n초 동안 돌진
	if (3.f > m_fTimeDelta)
	{
		// 플레이어와 몬스터사이의 각도 계산
		_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
		vLook.m128_f32[1] = 0.f;
		_vector vTargetLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		vTargetLook.m128_f32[1] = 0.f;

		// 벡터가 0 벡터가 아닌지 확인
		if (XMVector3Equal(vLook, XMVectorZero()) || XMVector3Equal(vTargetLook, XMVectorZero()))
			return;

		_float fAngle = acos(XMVectorGetX(XMVector3Dot(XMVector3Normalize(vLook), XMVector3Normalize(vTargetLook))));

		// n초 동안 일정 각도 이하만큼만 플레이어를 향해 회전
		if (1.f > m_fTimeDelta)
		{
			_float fcosTheta = XMVectorGetX(XMVector4Dot(vLook, vTargetLook));

			if (6.f > XMConvertToDegrees(fAngle))
			{
				m_vAxisY = XMVector3Cross(XMVector3Normalize(vLook), XMVector3Normalize(vTargetLook));

				m_fAngle = fAngle;

				pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);
				pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 6.f, fTimeDelta);
			}
			else
			{
				pTransformCom->Look_At_Angle(vLook, m_vAxisY, m_fAngle * fTimeDelta * 6.f);
				pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 6.f, fTimeDelta);
			}
		}
		// 일정 시간이 지나면 고정된 각도로 회전
		else
		{
			pTransformCom->Look_At_Angle(vLook, m_vAxisY, m_fAngle * fTimeDelta * 6.f);
			pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 6.f, fTimeDelta);
		}
	}
	else
	{
		m_fTimeDelta = 0.f;
		pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
		pAwoofy->Change_State(CAwoofy::AWOOFY_BRAKE, 40.f, false, true);
	}
}

void CAwoofy_Run_State::OnStateExit()
{
}

CAwoofy_Run_State* CAwoofy_Run_State::Create()
{
	CAwoofy_Run_State* pInstance = new CAwoofy_Run_State();
	return pInstance;
}

void CAwoofy_Run_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region FIND STATE
//*******************************************************
//						 FIND STATE
//*******************************************************
CAwoofy_Find_State::CAwoofy_Find_State()
{
}

void CAwoofy_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CAwoofy_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

	if (true == pAwoofy->IsAnimFinished())
	{
		// Awoofy 눈 상태
		pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_ANGER);
		pAwoofy->Set_MonState(CAwoofy::MON_WAIT);
		pAwoofy->Change_State(CAwoofy::AWOOFY_RUN, 40.f, true, true);
	}
}

void CAwoofy_Find_State::OnStateExit()
{
}

CAwoofy_Find_State* CAwoofy_Find_State::Create()
{
	CAwoofy_Find_State* pInstance = new CAwoofy_Find_State();
	return pInstance;
}

void CAwoofy_Find_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region LOOKAROUNDAFTERBRAKE STATE
//*******************************************************
//						 LOOKAROUNDAFTERBRAKE STATE
//*******************************************************
CAwoofy_LookAroundAfterBrake_State::CAwoofy_LookAroundAfterBrake_State()
{
}

void CAwoofy_LookAroundAfterBrake_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CAwoofy_LookAroundAfterBrake_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (true == pAwoofy->IsAnimFinished())
	{
		// Awoofy 눈 상태
		pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
		pAwoofy->Change_State(CAwoofy::AWOOFY_WAIT, 40.f, false, true);
	}
}

void CAwoofy_LookAroundAfterBrake_State::OnStateExit()
{
}

CAwoofy_LookAroundAfterBrake_State* CAwoofy_LookAroundAfterBrake_State::Create()
{
	CAwoofy_LookAroundAfterBrake_State* pInstance = new CAwoofy_LookAroundAfterBrake_State();
	return pInstance;
}

void CAwoofy_LookAroundAfterBrake_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region BRAKE STATE
//*******************************************************
//						 BRAKE STATE
//*******************************************************
CAwoofy_Brake_State::CAwoofy_Brake_State()
{
}

void CAwoofy_Brake_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CAwoofy_Brake_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 브레이크 : 제곱 감속
	_float fDeceleration = m_fSpeed * m_fSpeed;

	if (0.f < m_fSpeed)
		m_fSpeed -= fTimeDelta * 0.3f;
	else
		m_fSpeed = 1.f;

	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (true == pAwoofy->IsAnimFinished())
	{
		// Awoofy 눈 상태
		pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
		pAwoofy->Change_State(CAwoofy::AWOOFY_LOOKAROUNDAFTERBRAKE, 45.f, false, true);
	}
	else
		pController->Move_Dir(pTransformCom, (pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 6.f) * fDeceleration, fTimeDelta);
}

void CAwoofy_Brake_State::OnStateExit()
{
}

CAwoofy_Brake_State* CAwoofy_Brake_State::Create()
{
	CAwoofy_Brake_State* pInstance = new CAwoofy_Brake_State();
	return pInstance;
}

void CAwoofy_Brake_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE
//*******************************************************
//						 DAMAGE STATE
//*******************************************************
CAwoofy_Damage_State::CAwoofy_Damage_State()
{
}

void CAwoofy_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	m_vKirbyLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);
	m_fEffectTime = 0.f;

}

void CAwoofy_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CAwoofy* pAwoofy = static_cast<CAwoofy*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pAwoofy->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pAwoofy->Get_DamegeDir();
		if (vDamegeDir != XMVectorZero())
			pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pAwoofy->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pAwoofy->Set_DamageJumpPower(fDamageJumpPower);

		m_fEffectTime += fTimeDelta;
		if (0.1f < m_fEffectTime)
		{
			m_fEffectTime = 0.f;
			_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			CEffect::FX_DESC FXDesc{};

			vPos.m128_f32[1] += 0.5f;
			FXDesc.vInitPos = vPos;
			FXDesc.vInitRot = { CUtils::Make_RandomFloat(0.f, 90.f), 0.f, 0.f };
			FXDesc.vInitScale = { 2.f, 2.f, 2.f };
			//FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

			pAwoofy->Add_Effect("Flying", FXDesc, false);
		}


		if (true == pAwoofy->IsAnimFinished() || pController->Is_Terrain())
		{
			if (pAwoofy->Get_Hp() <= 0.f)
				pAwoofy->Set_Dead();
			else
			{
				// Awoofy 눈 상태
				pAwoofy->Set_AwoofyEye(CAwoofy::AWOOFYEYE_IDLE);
				pAwoofy->Set_MonState(CAwoofy::MON_WAIT);
				pAwoofy->Change_State(CAwoofy::AWOOFY_WAIT, 40.f, false, true);
			}
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pAwoofy->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pAwoofy->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
			CGameObject* pObj = pKirby->FindStarBox(pController->Get_MostRecentActor());
			if (nullptr != pObj) {
				return;
			}
			pObj = pKirby->FindBox(pController->Get_MostRecentActor());
			if (nullptr != pObj) {
				return;
			}

			pAwoofy->Set_PhyXState(PO_FLYDEADAWAY);
			pAwoofy->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pAwoofy->Set_Dead();
		}
	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pAwoofy->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pAwoofy->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pAwoofy->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pAwoofy->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
			pAwoofy->Set_Dead();

	}
}

void CAwoofy_Damage_State::OnStateExit()
{
}

CAwoofy_Damage_State* CAwoofy_Damage_State::Create()
{
	CAwoofy_Damage_State* pInstance = new CAwoofy_Damage_State();
	return pInstance;
}

void CAwoofy_Damage_State::Free()
{
	__super::Free();
}

#pragma endregion
