#pragma once
#include "Kirby.h"
#include "Utils.h"
#define DESC(state) Kirbydesc->state
#define GAMEINSTANCE CGameInstance::Get_Instance()->

// 방향키를 누르면 그쪽으로 2차원 원형 보간이 된다
static void Turn_Interpolate(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta)
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

	if (fcosTheta < -0.96f)
	{
		Kirbydesc->m_fMoveSpeed *= 0.3f;
	}

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

// 조이스틱의 방향이 꺾일 때, Dir방향으로 Z 회전하는 기능 (오토바이 무빙)
static void Turn_Z_Interpolate(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta)
{
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

// 가속을 주면서 앞으로 나가는 로직이다.
static void Moving_Logic(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
	if (Kirbydesc->m_fMoveSpeed > 10.f)
		Kirbydesc->m_fMoveSpeed = 10.f;

	// 타겟기준
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

// 감속을 주면서, Z회전도 죽이면서 속도 원상복구를 하려고 한다.
static void Deceleration(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	// 0.1초간 풀 감속 (최대 속도 8이라 가정)
	if (Kirbydesc->m_fMoveSpeed > 0.f)
		Kirbydesc->m_fMoveSpeed -= 120.f * fTimeDelta;
	if (Kirbydesc->m_fMoveSpeed < 0.f)
		Kirbydesc->m_fMoveSpeed = 0.f;

	// Z 회전 복구 (최대 회전 각도 10도)
	Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 4.f;

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

// 점프 중 감속
static void Jump_Deceleration(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	// 0.1초간 풀 감속 (최대 속도 8이라 가정)
	if (Kirbydesc->m_fMoveSpeed > 0.f)
		Kirbydesc->m_fMoveSpeed -= 10.f * fTimeDelta;
	if (Kirbydesc->m_fMoveSpeed < 0.f)
		Kirbydesc->m_fMoveSpeed = 0.f;

	// Z 회전 복구 (최대 회전 각도 10도)
	Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 4.f;

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

// 점프 중 방향키를 누르면 그쪽으로 2차원 원형 보간이 된다
static void Jump_Turn_Interpolate(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta)
{
	if (Kirbydesc->m_vMoveDir == Kirbydesc->m_vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 8.f;
	_vector vTargetDir = Kirbydesc->m_vTargetDir;
	_vector vMoveDir = Kirbydesc->m_vMoveDir;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);

	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));

	if (fcosTheta < -0.96f)
	{
		Kirbydesc->m_fMoveSpeed *= 0.3f;
	}

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

// 빨아들일 때의 움직임 로직
static void Inhale_Moving_Logic(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	Kirbydesc->m_fMoveSpeed += fTimeDelta * 10.f;
	if (Kirbydesc->m_fMoveSpeed > 3.f)
		Kirbydesc->m_fMoveSpeed = 3.f;

	// 타겟기준
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

// 날때의 로직
static void Fly_Moving_Logic(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	Kirbydesc->m_fMoveSpeed += fTimeDelta * 10.f;
	if (Kirbydesc->m_fMoveSpeed > 5.f)
		Kirbydesc->m_fMoveSpeed = 5.f;

	// 타겟기준
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

// 날때 감속
static void Fly_Deceleration(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	// 0.1초간 풀 감속 (최대 속도 8이라 가정)
	if (Kirbydesc->m_fMoveSpeed > 0.f)
		Kirbydesc->m_fMoveSpeed -= 5.f * fTimeDelta;
	if (Kirbydesc->m_fMoveSpeed < 0.f)
		Kirbydesc->m_fMoveSpeed = 0.f;

	// Z 회전 복구 (최대 회전 각도 10도)
	Kirbydesc->m_fZAngle -= Kirbydesc->m_fZAngle / 4.f;

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pTransformCom->Turn(Kirbydesc->m_vMoveDir, 1.f, Kirbydesc->m_fZAngle);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}

// 카메라 기준 방향을 만들어준다. (JoyStick_controler에서 사용)
static _float4 Make_TargetDir(CKirby::DIR _eDir, CGameObject* pCamera)
{

	CTransform* pCameraTransform = pCamera->Get_TransformCom();
	_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));

	if (_eDir == CKirby::DIR_RIGHT)
		return vCamRight;
	else if (_eDir == CKirby::DIR_LEFT)
		return -1.f * vCamRight;
	else if (_eDir == CKirby::DIR_FRONT)
		return vCamLook;
	else if (_eDir == CKirby::DIR_BACK)
		return -1.f * vCamLook;

	else if (_eDir == CKirby::DIR_LB)
		return XMVector4Normalize((-1.f * vCamRight) + (-1.f * vCamLook));
	else if (_eDir == CKirby::DIR_RB)
		return XMVector4Normalize(vCamRight + (-1.f * vCamLook));
	else if (_eDir == CKirby::DIR_LF)
		return XMVector4Normalize((-1.f * vCamRight) + (vCamLook));
	else if (_eDir == CKirby::DIR_RF)
		return XMVector4Normalize(vCamRight + vCamLook);

	return _float4(0.f, 0.f, 0.f, 0.f);
}

// 조이스틱을 제어하고 있으면 true, 제어하고 있지 않으면 false
static _bool JoyStick_controller(CKirby::KIRBY_INFODESC* Kirbydesc, CGameObject* pCamera)
{
	if (GAMEINSTANCE Get_DIKeyState(DIK_UP, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_LF, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_RF, pCamera);
		else
			DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_FRONT, pCamera);

		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_DOWN, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_LB, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_RB, pCamera);
		else
			DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_BACK, pCamera);

		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_LEFT, pCamera);
		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		DESC(m_vTargetDir) = Make_TargetDir(CKirby::DIR_RIGHT, pCamera);
		return true;
	}

	return false;
}