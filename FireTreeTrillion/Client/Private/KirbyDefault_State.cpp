#include "stdafx.h"
#include "KirbyDefault_State.h"

#include "Kirby.h"
#include "Utils.h"


void Turn_Interpolate(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta)
{
	if (Kirbydesc->m_vMoveDir == Kirbydesc->m_vTargetDir)
		return;
	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 12.f;
	_vector vTargetDir = Kirbydesc->m_vTargetDir;
	_vector vMoveDir = Kirbydesc->m_vMoveDir;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);

	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));
	if (fcosTheta < -0.9995f || fcosTheta > 0.9995f)
	{
		// 180도로 NaN 방지 랜덤으로 -1, 1도 틀어줌
		_float4x4 rotationMatrix;
		XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
		CUtils::Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomInt(0, 1) == 1 ? 1.f : -1.f);
		Kirbydesc->m_vMoveDir = XMVector3Transform(Kirbydesc->m_vMoveDir, XMLoadFloat4x4(&rotationMatrix));
		Kirbydesc->m_vMoveDir = XMVectorSetW(Kirbydesc->m_vMoveDir, 0.0f);
	}
	else
	{
		_float ftheta = acos(fcosTheta);
		_float fAngleDegrees = XMConvertToDegrees(ftheta);

		if (fAngleDegrees < 10.0f)
		{
			Kirbydesc->m_vMoveDir = Kirbydesc->m_vTargetDir;
		}
		else
		{
			_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
			_float fAlpha = sin((1 - fInterpolate) * ftheta) / fsinTheta;
			_float fBeta = sin(fInterpolate * ftheta) / fsinTheta;
			_float4 vResult = vMoveDirXZ * fAlpha + vTargetDirXZ * fBeta;
			Kirbydesc->m_vMoveDir = XMVector4Normalize(vResult);
			Kirbydesc->m_vMoveDir = XMVector3Normalize(Kirbydesc->m_vMoveDir);

		}
	}
	///////////

}



#pragma region IDLE STATE

CKirbyDefault_Idle_State::CKirbyDefault_Idle_State()
{
}

void CKirbyDefault_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);


}

void CKirbyDefault_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	// 0.1초간 풀 감속 (최대 속도 8이라 가정)
	if (Kirbydesc->m_fMoveSpeed > 0.f)
		Kirbydesc->m_fMoveSpeed -= 80.f * fTimeDelta;
	if (Kirbydesc->m_fMoveSpeed < 0.f)
		Kirbydesc->m_fMoveSpeed = 0.f;

	// Z 회전 복구 (최대 회전 각도 10도)
	Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 4.f;


	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	//Kirbydesc->m_vMoveDir = XMVector3Normalize(Kirbydesc->m_vMoveDir);
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	//pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vMoveDelta);
	pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

void CKirbyDefault_Idle_State::OnStateExit()
{


}

CKirbyDefault_Idle_State* CKirbyDefault_Idle_State::Create()
{
	CKirbyDefault_Idle_State* pInstance = new CKirbyDefault_Idle_State();
	return pInstance;
}

void CKirbyDefault_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion




#pragma region RUN STATE

CKirbyDefault_Run_State::CKirbyDefault_Run_State()
{
}

void CKirbyDefault_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CKirbyDefault_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));


	Kirbydesc->m_fMoveSpeed += fTimeDelta * 100.f;

	if (Kirbydesc->m_fMoveSpeed > 8.f)
		Kirbydesc->m_fMoveSpeed = 8.f;

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	// 타겟기준
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	//pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vMoveDelta);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


	// 각도 (얼마나 벌어졌느냐)
	_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_float cosAngle = XMVectorGetX(XMVector4Dot(Kirbydesc->m_vMoveDir, Kirbydesc->m_vTargetDir));
	_float angle = acosf(cosAngle);

	_float angleInDegrees = XMConvertToDegrees(angle);

	// 외적으로 방향탐색
	_vector crossProduct = XMVector3Cross(Kirbydesc->m_vMoveDir, Kirbydesc->m_vTargetDir);
	_float direction = XMVectorGetY(crossProduct);

	// 회전 각도 설정
	if (direction < 0)
	{
		if (angleInDegrees > 5.f)
			Kirbydesc->m_fZAngle += fTimeDelta * 40.f;
		else
		{
			Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 5.f;
		}
	}
	else
	{
		if (angleInDegrees > 5.f)
			Kirbydesc->m_fZAngle -= fTimeDelta * 40.f;
		else
		{
			Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 5.f;
		}
	}

	if (Kirbydesc->m_fZAngle > 20.f)
		Kirbydesc->m_fZAngle = 20.f;
	if (Kirbydesc->m_fZAngle < -20.f)
		Kirbydesc->m_fZAngle = -20.f;
	pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
	
}
void CKirbyDefault_Run_State::OnStateExit()
{

}

CKirbyDefault_Run_State* CKirbyDefault_Run_State::Create()
{
	CKirbyDefault_Run_State* pInstance = new CKirbyDefault_Run_State();
	return pInstance;
}

void CKirbyDefault_Run_State::Free()
{
	__super::Free();
}

#pragma endregion