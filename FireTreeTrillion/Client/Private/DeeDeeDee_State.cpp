#include "stdafx.h"
#include "DeeDeeDee_State.h"
#include "DeeDeeDee.h"
#include "Kirby.h"

void Turn_Interpolate(CDeeDeeDee::DDDDESC* DDDDesc, CTransform* pTransformCom, _float fTimeDelta)
{
	if (DDDDesc->m_vMoveDir == DDDDesc->m_vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 12.f;
	_vector vTargetDir = DDDDesc->m_vTargetDir;
	_vector vMoveDir = DDDDesc->m_vMoveDir;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);

	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));

	if (fcosTheta < -0.96f)
	{
		DDDDesc->m_fMoveSpeed *= 0.3f;
	}

	if (fcosTheta < -0.9995f || fcosTheta > 0.9995f)
	{
		// 180도로 NaN 방지 랜덤으로 -1, 1도 틀어줌
		/*_float4x4 rotationMatrix;
		XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
		CUtils::Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomInt(0, 1) == 1 ? 1.f : -1.f);
		DDDDesc->m_vMoveDir = XMVector3Transform(DDDDesc->m_vMoveDir, XMLoadFloat4x4(&rotationMatrix));
		DDDDesc->m_vMoveDir = XMVectorSetW(DDDDesc->m_vMoveDir, 0.0f);*/

		return;
	}
	else
	{
		_float ftheta = acos(fcosTheta);
		_float fAngleDegrees = XMConvertToDegrees(ftheta);

		if (fAngleDegrees < 5.0f)
		{
			DDDDesc->m_vMoveDir = DDDDesc->m_vTargetDir;
		}
		else
		{
			_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
			_float fAlpha = sin((1 - fInterpolate) * ftheta) / fsinTheta;
			_float fBeta = sin(fInterpolate * ftheta) / fsinTheta;
			_float4 vResult = vMoveDirXZ * fAlpha + vTargetDirXZ * fBeta;
			DDDDesc->m_vMoveDir = XMVector4Normalize(vResult);
			DDDDesc->m_vMoveDir = XMVector3Normalize(DDDDesc->m_vMoveDir);

		}
	}
	///////////
}

void Moving_Logic(CDeeDeeDee::DDDDESC* DDDDesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta, _float fAcc, _float fMaxSpeed)
{
	DDDDesc->m_fMoveSpeed += fTimeDelta * fAcc;
	if (DDDDesc->m_fMoveSpeed > fMaxSpeed)
		DDDDesc->m_fMoveSpeed = fMaxSpeed;

	// 타겟기준
	_vector vMoveDelta = DDDDesc->m_vTargetDir * fTimeDelta * DDDDesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

void Make_TargetDir(CDeeDeeDee::DDDDESC* DDDDesc, const _float4& vMyPos, const _float4& vKirbyPos)
{
	_float4 vDir = vKirbyPos - vMyPos;
	vDir.y = 0.f;
	vDir.Normalize();
	DDDDesc->m_vTargetDir = vDir;
}
#pragma region IDLE STATE

CDeeDeeDee_Idle_State::CDeeDeeDee_Idle_State()
{
}

void CDeeDeeDee_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return;
	CTransform* pKirbyTransform = pKirby->Get_TransformCom();
	CDeeDeeDee* pDee = static_cast<CDeeDeeDee*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CDeeDeeDee::DDDDESC* DDDDesc = pDee->Get_Info();
	_float4 vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);

	Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);


	DDDDesc->m_isBattle = true;

	if ((vKirbyPos - vPos).Length() > 12.f)
	{
		pDee->Change_State(CDeeDeeDee::STATE_RUN, 60.f, true, true);
		return;
	}
}

void CDeeDeeDee_Idle_State::OnStateExit()
{

}

CDeeDeeDee_Idle_State* CDeeDeeDee_Idle_State::Create()
{
	CDeeDeeDee_Idle_State* pInstance = new CDeeDeeDee_Idle_State();
	return pInstance;
}

void CDeeDeeDee_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion



#pragma region IDLE STATE

CDeeDeeDee_Run_State::CDeeDeeDee_Run_State()
{
}

void CDeeDeeDee_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return;
	CTransform* pKirbyTransform = pKirby->Get_TransformCom();
	CDeeDeeDee* pDee = static_cast<CDeeDeeDee*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CDeeDeeDee::DDDDESC* DDDDesc = pDee->Get_Info();

	_float4 vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
	Make_TargetDir(DDDDesc, vPos, vKirbyPos);
	Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
	Moving_Logic(DDDDesc, pTransformCom, pController, fTimeDelta, 50.f, 4.f);
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	DDDDesc->m_isBattle = false;

	if ((vKirbyPos - vPos).Length() <= 12.f)
	{
		pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, true);
		return;
	}

}

void CDeeDeeDee_Run_State::OnStateExit()
{
}

CDeeDeeDee_Run_State* CDeeDeeDee_Run_State::Create()
{
	CDeeDeeDee_Run_State* pInstance = new CDeeDeeDee_Run_State();
	return pInstance;
}

void CDeeDeeDee_Run_State::Free()
{
	__super::Free();
}

#pragma endregion
