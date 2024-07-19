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

	m_fDeadMaxTime = CUtils::Make_RandomFloat(0.35f, 0.7f);

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
		pController->Move_Dir(pTransformCom, vDamegeDir * fTimeDelta * 20.f, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = pKabu->Get_DamageJumpPower();
		pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
		pKabu->Set_DamageJumpPower(fDamageJumpPower);

		if (true == pKabu->IsAnimFinished())
		{
			if(true == pKabu->Get_RealDead())
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
			pKabu->Set_Dead();
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

		if (m_fDeadTime > m_fDeadMaxTime || pController->Is_Terrain())
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

	pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * 10.f);

	if (true == pKabu->IsAnimFinished())
	{
		if (pKabu->Get_Hp() <= 0.f)
		{
			pKabu->Set_Dead();

			HRESULT hr;
			CMonster::MONSTER_DESC KabuDesc = {};
			_float4x4 WorldMatrix = pTransformCom->Get_WorldMatrix();
			WorldMatrix.m[3][0] = pKabu->Get_Position().x;
			WorldMatrix.m[3][1] = pKabu->Get_Position().y;
			WorldMatrix.m[3][2] = pKabu->Get_Position().z;
			WorldMatrix.m[3][3] = pKabu->Get_Position().w;
			KabuDesc.matWorld = WorldMatrix;
			KabuDesc.wstrModelName = TEXT("Kabu");
			KabuDesc.iShaderVars = 6;
			KabuDesc.fRimWidth = 0.2f;
			KabuDesc.eMonState = pKabu->Get_MonState();
			KabuDesc.vecRallyPoints = pKabu->Get_RallyPoint();
			KabuDesc.bRealDead = true;
			KabuDesc.fAngle = pKabu->Get_Angle();
			hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Kabu"), &KabuDesc);
			CHECK_FAILED(hr);
		}
	}
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