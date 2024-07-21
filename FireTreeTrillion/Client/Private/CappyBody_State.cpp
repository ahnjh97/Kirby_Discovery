#include "stdafx.h"
#include "CappyBody_State.h"
#include "CappyBody.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CCappyBody_Idle_State::CCappyBody_Idle_State()
{
}

void CCappyBody_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);


}

void CCappyBody_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CCappyBody* pCappyBody = static_cast<CCappyBody*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();
	
	pCappyBody->Set_Render(true);

	// 몬스터, 플레이어 위치
	_vector vPosistion = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vPosistion.m128_f32[1] = 0.f;
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vKirbyPos.m128_f32[1] = 0.f;

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPosistion, vKirbyPos)));

	if (8.f > fDistance)
		pCappyBody->Change_State(CCappyBody::CAPPYBODY_KASAUP1, 60.f, false, true);

	if (CCappyBody::CAPPYBODY_HIDINGWAITA == pCappyBody->Get_State())
	{
		if (true == pCappyBody->IsAnimFinished())
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_HIDINGJUMPA, 60.f, false, true);
	}
	else if(CCappyBody::CAPPYBODY_HIDINGJUMPA == pCappyBody->Get_State())
	{
		if (true == pCappyBody->IsAnimFinished())
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_HIDINGWAITB, 60.f, false, true);
	}
	else if (CCappyBody::CAPPYBODY_HIDINGWAITB == pCappyBody->Get_State())
	{
		if (true == pCappyBody->IsAnimFinished())
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_HIDINGJUMPB, 60.f, false, true);
	}
	else if (CCappyBody::CAPPYBODY_HIDINGJUMPB == pCappyBody->Get_State())
	{
		if (true == pCappyBody->IsAnimFinished())
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_HIDINGWAITA, 60.f, false, true);
	}
}

void CCappyBody_Idle_State::OnStateExit()
{
}

CCappyBody_Idle_State* CCappyBody_Idle_State::Create()
{
	CCappyBody_Idle_State* pInstance = new CCappyBody_Idle_State();
	return pInstance;
}

void CCappyBody_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region FIND STATE
//*********************************
//			 FIND STATE
//*********************************
CCappyBody_Find_State::CCappyBody_Find_State()
{
}

void CCappyBody_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CCappyBody_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CCappyBody* pCappyBody = static_cast<CCappyBody*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	pCappyBody->Set_Render(false);

	if (true == pCappyBody->IsAnimFinished())
	{
		_float4 vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

		//m_fAngle += m_fTimeDelta * 80.f;

		for(_uint i = 0; i < 3; ++i)
		{
			_float4 vRotatePos = {};
			_float fAngle = CUtils::Make_RandomFloat(10.f, 90.f);
			vRotatePos.x = vPos.x + (0.3f * sin(XMConvertToRadians((_float)i * 120.f + fAngle)));
			vRotatePos.y = vPos.y;
			vRotatePos.z = vPos.z - (0.3f * cos(XMConvertToRadians((_float)i * 120.f + fAngle)));

			CMultiEffect::MULTI_FX_DESC FXDesc{};
			FXDesc.vInitPos = { vRotatePos.x, vRotatePos.y + 1.f, vRotatePos.z };
			FXDesc.vInitScale = { 1.1f, 1.1f, 1.1f };

			_float3 vDir = vPos - vRotatePos;
			vDir.Normalize();
			_float3 vLook = { 0.f, 0.f, 1.f };

			_float fAngleLook = atan2f(vLook.z, vLook.x);
			_float fAngleDiff = fAngleLook - atan2f(vDir.z, vDir.x);
			fAngleDiff = ToDegree(fAngleDiff);

			_float3 vAngle = { 10.f, fAngleDiff, 0.f };
			FXDesc.vInitRot = vAngle;

			if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SmokeMix"), &FXDesc)))
				return;
		}

		pCappyBody->Change_State(CCappyBody::CAPPYBODY_WAIT, 60.f, true, true);
	}

	// 플레이어를 향해 바라본다
	//pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 2.f);
}

void CCappyBody_Find_State::OnStateExit()
{
}

CCappyBody_Find_State* CCappyBody_Find_State::Create()
{
	CCappyBody_Find_State* pInstance = new CCappyBody_Find_State();
	return pInstance;
}

void CCappyBody_Find_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region RUN STATE
//*********************************
//			 RUN STATE
//*********************************
CCappyBody_Run_State::CCappyBody_Run_State()
{
}

void CCappyBody_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CCappyBody_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CCappyBody* pCappyBody = static_cast<CCappyBody*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	pCappyBody->Set_Render(true);

	// 몬스터, 플레이어 위치
	_vector vPosition = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vPosition.m128_f32[1] = 0.f;
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vKirbyPos.m128_f32[1] = 0.f;

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPosition, vKirbyPos)));

	if (15.f < fDistance)
		pCappyBody->Change_State(CCappyBody::CAPPYBODY_HIDINGWAITA, 60.f, false, true);

	// 커비를 바라보며 쫒아감
	_vector		vLook = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION) - vPosition;

	m_fTimeDelta += fTimeDelta;

	if (2.f > m_fTimeDelta)
	{
		if (0.5f <= XMVector3Length(vLook).m128_f32[0])
			vPosition += XMVector3Normalize(vLook) * fTimeDelta * 2.f;

		// 플레이어를 향해 바라본다
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 4.f);

		pController->Move(pTransformCom, vPosition, fTimeDelta);
	}
	else
	{
		m_fTimeDelta = 0.f;

		for(_uint i = 0; i < 6; ++i)
		{
			_float4 vRotatePos = {};
			_float4 vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

			_float fAngle = CUtils::Make_RandomFloat(10.f, 90.f);
			vRotatePos.x = vPos.x + (0.4f * sin(XMConvertToRadians((_float)i * 60.f + fAngle)));
			vRotatePos.y = vPos.y;
			vRotatePos.z = vPos.z - (0.4f * cos(XMConvertToRadians((_float)i * 60.f + fAngle)));

			CEffect::FX_DESC FXDesc{};
			FXDesc.vInitPos = { vRotatePos.x, vRotatePos.y + 0.25f, vRotatePos.z };
			//FXDesc.vInitRot = { 0.f, CUtils::Make_RandomFloat(0.f, 90.f), 0.f };
			FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
			//FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

			pCappyBody->Add_Effect("BBongBBongE", FXDesc, false);
		}

		_uint iRand = rand() % 3;
		if(iRand == 0)
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_KASAUP1, 45.f, false, true);
		else if(iRand == 1)
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_KASAUP2, 45.f, false, true);
		else if (iRand == 2)
			pCappyBody->Change_State(CCappyBody::CAPPYBODY_KASAUP3, 65.f, false, true);
	}
}

void CCappyBody_Run_State::OnStateExit()
{
}

CCappyBody_Run_State* CCappyBody_Run_State::Create()
{
	CCappyBody_Run_State* pInstance = new CCappyBody_Run_State();
	return pInstance;
}

void CCappyBody_Run_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region HATLOSE STATE
//*********************************
//			 HATLOSE STATE
//*********************************
CCappyBody_HatLose_State::CCappyBody_HatLose_State()
{
}

void CCappyBody_HatLose_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CCappyBody_HatLose_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CCappyBody* pCappyBody = static_cast<CCappyBody*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	pCappyBody->Set_Render(false);
}

void CCappyBody_HatLose_State::OnStateExit()
{
}

CCappyBody_HatLose_State* CCappyBody_HatLose_State::Create()
{
	CCappyBody_HatLose_State* pInstance = new CCappyBody_HatLose_State();
	return pInstance;
}

void CCappyBody_HatLose_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region DAMAGE STATE
//*********************************
//			 DAMAGE STATE
//*********************************
CCappyBody_Damage_State::CCappyBody_Damage_State()
{
}

void CCappyBody_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);
	m_fEffectTime = 0.f;
}

void CCappyBody_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CCappyBody* pCappy = static_cast<CCappyBody*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pCappy->Get_PhyXState() == PO_NORMAL)
	{
		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pCappy->Get_DamegeDir();
		if (vDamegeDir != XMVectorZero())
			pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 6.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pCappy->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pCappy->Set_DamageJumpPower(fDamageJumpPower);

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

			pCappy->Add_Effect("Flying", FXDesc, false);
		}

		if (pController->Is_Terrain())
		{
			if (pCappy->Get_Hp() <= 0.f)
				pCappy->Set_Dead();
			else
				pCappy->Change_State(CCappyBody::CAPPYBODY_WAIT, 40.f, true, true);
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (pCappy->Get_PhyXState() == PO_FLYAWAY)
	{
		_float3 vDamegeDir = pCappy->Get_DamegeDir();
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		pTransformCom->Turn(pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);
		m_fFlyTime += fTimeDelta;

		if (1.f > pController->Compute_Wall(vDamegeDir))
		{
			pCappy->Set_PhyXState(PO_FLYDEADAWAY);
			pCappy->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}

		if (m_fFlyTime > 2.f)
		{
			pCappy->Set_Dead();
		}

	}
	// 죽는 도중이다.	 (날아가다 터질예정임)
	else if (pCappy->Get_PhyXState() == PO_FLYDEADAWAY)
	{
		m_fDeadTime += fTimeDelta;

		// 일단 그 방향으로 바라보게만 한다.
		_float3 vDamegeDir = pCappy->Get_DamegeDir();
		pTransformCom->Look_At_Axis(-vDamegeDir);

		// 이제 날아가는 것을 구현해보자.
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 10.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pCappy->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;

		pCappy->Set_DamageJumpPower(fDamageJumpPower);

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
			pCappy->Set_Dead();

	}
}

void CCappyBody_Damage_State::OnStateExit()
{
}

CCappyBody_Damage_State* CCappyBody_Damage_State::Create()
{
	CCappyBody_Damage_State* pInstance = new CCappyBody_Damage_State();
	return pInstance;
}

void CCappyBody_Damage_State::Free()
{
	__super::Free();
}
#pragma endregion