#pragma once
#include "Kirby.h"
#include "Utils.h"
#include "SingleEffect.h"
#include "MultiEffect.h"

#include "KirbyBomb.h"

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

// 방향키를 누르면 그쪽으로 2차원 원형 보간이 된다. (속도 조정가능)
static void Turn_Interpolate(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta, _float fInterpolateSpeed)
{
	if (Kirbydesc->m_vMoveDir == Kirbydesc->m_vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * fInterpolateSpeed;
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
	if (Kirbydesc->m_fMoveSpeed > 8.3f)
		Kirbydesc->m_fMoveSpeed = 8.3f;

	// 타겟기준
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

// 가드 중 감속
static void Guard_Deceleration(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	// 0.1초간 풀 감속 (최대 속도 8이라 가정)
	if (Kirbydesc->m_fMoveSpeed > 0.f)
		Kirbydesc->m_fMoveSpeed -= 18.f * fTimeDelta;
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

// 흡수 중 방향키를 누르면 그쪽으로 2차원 원형 보간이 된다
static void Inhale_Turn_Interpolate(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta)
{
	if (Kirbydesc->m_vMoveDir == Kirbydesc->m_vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 4.f;
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

// 단순 조이스틱 On? or Off?
static _bool JoyStick_On()
{
	if (GAMEINSTANCE Get_DIKeyState(DIK_UP, KEY_PRESS))
		return true;
	else if (GAMEINSTANCE Get_DIKeyState(DIK_DOWN, KEY_PRESS))
		return true;
	else if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
		return true;
	else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
		return true;

	return false;
}

// 순수한 방향을 얻는 함수
static _float4 JoyStick_controller_OtherDir(CGameObject* pCamera)
{
	if (GAMEINSTANCE Get_DIKeyState(DIK_UP, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			return Make_TargetDir(CKirby::DIR_LF, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			return Make_TargetDir(CKirby::DIR_RF, pCamera);
		else
			return Make_TargetDir(CKirby::DIR_FRONT, pCamera);

	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_DOWN, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			return Make_TargetDir(CKirby::DIR_LB, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			return Make_TargetDir(CKirby::DIR_RB, pCamera);
		else
			return Make_TargetDir(CKirby::DIR_BACK, pCamera);
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		return Make_TargetDir(CKirby::DIR_LEFT, pCamera);
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		return Make_TargetDir(CKirby::DIR_RIGHT, pCamera);
	}
	return _float4(0.f, 0.f, 0.f, 0.f);
}

// 조이스틱을 제어하고 있으면 true, 제어하고 있지 않으면 false. 근데 이 친구는 커비가 방향을 돌지 않는다.
static _bool JoyStick_controller_Attack(CKirby::KIRBY_INFODESC* Kirbydesc, CGameObject* pCamera)
{
	if (GAMEINSTANCE Get_DIKeyState(DIK_UP, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_LF, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_RF, pCamera);
		else
			DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_FRONT, pCamera);

		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_DOWN, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_LB, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_RB, pCamera);
		else
			DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_BACK, pCamera);

		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_LEFT, pCamera);
		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		DESC(m_vAttackDir) = Make_TargetDir(CKirby::DIR_RIGHT, pCamera);
		return true;
	}
	return false;
}

// 커비 몸체 기준에서, 내가 누른 방향이 어느 방향인지 배출하는 기능
static CKirby::DIR Kirby_Standard_Angle(CKirby::KIRBY_INFODESC* Kirbydesc)
{
	_float fCX = DESC(m_vMoveDir).x;
	_float fCZ = DESC(m_vMoveDir).z;
	_float fDX = DESC(m_vDodgeDir).x;
	_float fDZ = DESC(m_vDodgeDir).z;

	_float fAngle = (atan2f(fCX, fCZ) * 180.0f / XM_PI) - (atan2f(fDX, fDZ) * 180.0f / XM_PI);
	if (fAngle < 0.f) fAngle += 360.0f;

	if (fAngle >= 315.f || fAngle < 45.f) return CKirby::DIR_FRONT;
	else if (fAngle >= 45.f && fAngle < 135.f) return CKirby::DIR_LEFT;
	else if (fAngle >= 135.f && fAngle < 225.f) return CKirby::DIR_BACK;
	else if (fAngle >= 225.f && fAngle < 315.f) return CKirby::DIR_RIGHT;

	return CKirby::DIR_FRONT;
}

// 커비 몸체 기준에서, 내가 누른 방향이 어느 방향인지 배출하는 기능 (1. 기준축, 2. 비교군)
static CKirby::DIR Kirby_Standard_Angle(_float4 vAxisDir, _float4 vDiffDir)
{
	_float fCX = vAxisDir.x;
	_float fCZ = vAxisDir.z;
	_float fDX = vDiffDir.x;
	_float fDZ = vDiffDir.z;

	_float fAngle = (atan2f(fCX, fCZ) * 180.0f / XM_PI) - (atan2f(fDX, fDZ) * 180.0f / XM_PI);
	if (fAngle < 0.f) fAngle += 360.0f;

	if (fAngle >= 315.f || fAngle < 45.f) return CKirby::DIR_FRONT;
	else if (fAngle >= 45.f && fAngle < 135.f) return CKirby::DIR_LEFT;
	else if (fAngle >= 135.f && fAngle < 225.f) return CKirby::DIR_BACK;
	else if (fAngle >= 225.f && fAngle < 315.f) return CKirby::DIR_RIGHT;

	return CKirby::DIR_FRONT;
}

// 덤블링 하면서 그 방향으로 나가게 한다.
static void Dodge_Moving_Logic(CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	DESC(m_fMoveSpeed) += fTimeDelta * 100.f;
	if (DESC(m_fMoveSpeed) > 10.f)
		DESC(m_fMoveSpeed) = 10.f;

	// 타겟기준
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vMoveDelta = DESC(m_vDodgeDir) * fTimeDelta * DESC(m_fMoveSpeed);
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);

}

// 큰 회전인지 구한다.
static _bool Is_BigTurn(CKirby::KIRBY_INFODESC* Kirbydesc)
{
	if (Kirbydesc->m_vMoveDir == Kirbydesc->m_vTargetDir)
		return false;

	_vector vTargetDir = Kirbydesc->m_vTargetDir;
	_vector vMoveDir = Kirbydesc->m_vMoveDir;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);

	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));

	if (fcosTheta < -0.8f)
	{
		return true;
	}

	return false;
}

// 주변에서 가장 가까운 Object를 흡수한다. 몬스터 -> 아이템 -> 오브젝트 우선순위
static _bool Vacuum_Object(CKirby* pKirby, _float fTimeDelta)
{
	CTransform* pTransformCom = pKirby->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	_float fDistance = 9.f;
	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);


	// 0차 목표인 버섯의 갓을 탐색한다.
	if (nullptr != GAMEINSTANCE Get_List(*GAMEINSTANCE Get_CurrentLevelID(), TEXT("Layer_CappyHat")))
	{
		for (auto& pObject : *GAMEINSTANCE Get_List(*GAMEINSTANCE Get_CurrentLevelID(), TEXT("Layer_CappyHat")))
		{
			if (static_cast<CPhysXObject*>(pObject)->Get_PhyXState() != PO_NORMAL)
				continue;

			CTransform* pObjectTransform = pObject->Get_TransformCom();
			_vector vObjectPos = pObjectTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vObjectDir = vObjectPos - vPos;
			_float fObjectDistance = XMVectorGetX(XMVector3Length(vObjectDir));
			// 만약, 목표 오브젝트가 거리보다 멀었을 경우
			if (fObjectDistance > fDistance)
				continue;
			// 만약, 목표 오브젝트가 거리보다 가까웠을 경우
			else
			{
				_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
				// 내적 ( 30도 )
				_float fDot = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vObjectDir), vLook));
				// 각도 계산 (도 단위)
				_float fDegrees = XMConvertToDegrees(acosf(fDot));

				// 각도가 30도 이상이면 스킵한다.
				if (fDegrees > 60.f)
					continue;

				// 작은 흡입일때 진정코 흡수를 시작한다.
				if (pKirby->Get_State() == CKirby::STATE_INHALE ||
					pKirby->Get_State() == CKirby::STATE_INHALEFALL ||
					pKirby->Get_State() == CKirby::STATE_INHALELANDING ||
					pKirby->Get_State() == CKirby::STATE_INHALEWALK ||
					pKirby->Get_State() == CKirby::STATE_SUPERINHALEWALK ||
					pKirby->Get_State() == CKirby::STATE_SUPERINHALE ||
					pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART)
				{
					fDistance = fObjectDistance;
					DESC(m_pObject) = static_cast<CPhysXObject*>(pObject);
					DESC(m_vObjectScale) = pObjectTransform->Get_Scaled();
					DESC(m_fObjectDistance) = fObjectDistance;
				}
			}
		}
	}

	if (DESC(m_pObject) != nullptr)
	{
		// 참조하면서 애니메이션으로 끌고간다.
		Safe_AddRef(DESC(m_pObject));
		// 커비가 동일한 애니메이션으로 몬스터를 포착해서 꽤 긴 시간동안 서로 짝짝꿍하겠다는 것이다.
		//pKirby->Set_PhyXState(PO_VACUUMING);
		DESC(m_pObject)->Set_PhyXState(PO_VACUUMING);
		pKirby->Change_State(CKirby::STATE_VACUUM, 50.f, true, true, CKirby::BODY_VACUUM);
		return true;
	}


	// 1차로 우선순위인 몬스터들 순회를 돈다.
	if (nullptr != GAMEINSTANCE Get_List(*GAMEINSTANCE Get_CurrentLevelID(), g_strLayerMonster))
	{

		for (auto& pObject : *GAMEINSTANCE Get_List(*GAMEINSTANCE Get_CurrentLevelID(), g_strLayerMonster))
		{
			if (static_cast<CPhysXObject*>(pObject)->Get_PhyXState() != PO_NORMAL)
				continue;

			CTransform* pObjectTransform = pObject->Get_TransformCom();
			_vector vObjectPos = pObjectTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vObjectDir = vObjectPos - vPos;
			_float fObjectDistance = XMVectorGetX(XMVector3Length(vObjectDir));

			// 만약, 목표 오브젝트가 거리보다 멀었을 경우
			if (fObjectDistance > fDistance)
				continue;
			// 만약, 목표 오브젝트가 거리보다 가까웠을 경우
			else
			{
				_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
				// 내적 ( 30도 )
				_float fDot = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vObjectDir), vLook));
				// 각도 계산 (도 단위)
				_float fDegrees = XMConvertToDegrees(acosf(fDot));

				// 각도가 30도 이상이면 스킵한다.
				if (fDegrees > 60.f)
					continue;

				// 사이즈가 작을 경우
				if (static_cast<CPhysXObject*>(pObject)->Get_VacuumSize() == SIZE_SMALL)
				{
					// 작은 흡입일때 진정코 흡수를 시작한다.
					if (pKirby->Get_State() == CKirby::STATE_INHALE ||
						pKirby->Get_State() == CKirby::STATE_INHALEFALL ||
						pKirby->Get_State() == CKirby::STATE_INHALELANDING ||
						pKirby->Get_State() == CKirby::STATE_INHALEWALK ||
						pKirby->Get_State() == CKirby::STATE_SUPERINHALEWALK ||
						pKirby->Get_State() == CKirby::STATE_SUPERINHALE ||
						pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART)
					{
						fDistance = fObjectDistance;
						DESC(m_pObject) = static_cast<CPhysXObject*>(pObject);
						DESC(m_vObjectScale) = pObjectTransform->Get_Scaled();
						DESC(m_fObjectDistance) = fObjectDistance;
					}
				}
				// 사이즈가 클 경우
				else if (static_cast<CPhysXObject*>(pObject)->Get_VacuumSize() == SIZE_BIG)
				{
					// 작은 흡입일때 진정코 흡수를 시작한다.
					if (pKirby->Get_State() == CKirby::STATE_SUPERINHALEWALK ||
						pKirby->Get_State() == CKirby::STATE_SUPERINHALE ||
						pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART)
					{
						fDistance = fObjectDistance;
						DESC(m_pObject) = static_cast<CPhysXObject*>(pObject);
						DESC(m_vObjectScale) = pObjectTransform->Get_Scaled();
						DESC(m_fObjectDistance) = fObjectDistance;
					}
				}
			}
		}
	}

	if (DESC(m_pObject) != nullptr)
	{
		// 참조하면서 애니메이션으로 끌고간다.
		Safe_AddRef(DESC(m_pObject));
		// 커비가 동일한 애니메이션으로 몬스터를 포착해서 꽤 긴 시간동안 서로 짝짝꿍하겠다는 것이다.
		//pKirby->Set_PhyXState(PO_VACUUMING);
		DESC(m_pObject)->Set_PhyXState(PO_VACUUMING);

		if (DESC(m_pObject)->Get_VacuumSize() == SIZE_SMALL)
			pKirby->Change_State(CKirby::STATE_VACUUM, 50.f, true, true, CKirby::BODY_VACUUM);
		else if (DESC(m_pObject)->Get_VacuumSize() == SIZE_BIG)
			pKirby->Change_State(CKirby::STATE_VACUUMHUSTLELV2, 50.f, true, true, CKirby::BODY_VACUUM);

		return true;
	}


	// 이 이후로는 아이템, 돌멩이 등 진행하여야 한다.



	return false;
}

// 주변에서 가장 가까운 목표 지점을 반환한다. 몬스터만 구현함.
static _float4 Spit_Target_Object(CKirby* pKirby)
{
	CTransform* pTransformCom = pKirby->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	_float fDistance = 30.f;
	_float4 vTargetPos = { 0.f, 0.f, 0.f, 0.f };

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 1차로 우선순위인 몬스터들 순회를 돈다.
	if (nullptr != GAMEINSTANCE Get_List(*GAMEINSTANCE Get_CurrentLevelID(), g_strLayerMonster))
	{

		for (auto& pObject : *GAMEINSTANCE Get_List(*GAMEINSTANCE Get_CurrentLevelID(), g_strLayerMonster))
		{
			if (static_cast<CPhysXObject*>(pObject)->Get_PhyXState() != PO_NORMAL)
				continue;

			CTransform* pObjectTransform = pObject->Get_TransformCom();
			_vector vObjectPos = pObjectTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vObjectDir = vObjectPos - vPos;
			_float fObjectDistance = XMVectorGetX(XMVector3Length(vObjectDir));

			// 만약, 목표 오브젝트가 거리보다 멀었을 경우
			if (fObjectDistance > fDistance)
				continue;
			// 만약, 목표 오브젝트가 거리보다 가까웠을 경우
			else
			{
				_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
				// 내적 ( 30도 )
				_float fDot = XMVectorGetX(XMVector3Dot(XMVector3Normalize(vObjectDir), vLook));
				// 각도 계산 (도 단위)
				_float fDegrees = XMConvertToDegrees(acosf(fDot));

				if (fDegrees > 60.f)
					continue;

				// 최고기록 갱신
				fDistance = fObjectDistance;
				vTargetPos = vObjectPos;
			}
		}
	}



	return vTargetPos;
}

// 커비가 빌보드 한다.
static void Kirby_Billboard(CTransform* pTransformCom, CGameObject* pCamera)
{
	_float3   vScale = pTransformCom->Get_Scaled();
	_float4x4      CamMatrix;
	CTransform* pCamTransform = pCamera->Get_TransformCom();
	CamMatrix = pCamTransform->Get_WorldFloat4x4();

	_vector vLook, vRight, vUp;

	vRight = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_RIGHT);
	vLook = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_LOOK);
	vUp = CUtils::Get_State_Vector_Matrix(CamMatrix, CUtils::STATE_UP);

	vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 1.f), vLook);
	vLook = XMVector3Cross(vRight, vUp);
	pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);
	pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScale.y);
	pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
}

// 커비가 자신의 능력에 따라 상태로 변형된다!! (공용에서 비슷한 애들로 넘어갈 때만)
static void Kirby_AbilityType_Assist(CKirby* pKirby, CKirby::STATE eState)
{
	if (eState == CKirby::STATE_IDLE)
	{
		if (pKirby->Get_AbilityType() == ABILITY_SWORD)
			pKirby->Change_State(CKirby::SWORDSTATE_WAIT, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		else
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
	}
	else if (eState == CKirby::STATE_RUNSTART)
	{
		if (pKirby->Get_AbilityType() == ABILITY_SWORD)
			pKirby->Change_State(CKirby::SWORDSTATE_RUN, 120.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		else
			pKirby->Change_State(CKirby::STATE_RUNSTART, 120.f, true, true, CKirby::BODY_DEFAULT);
	}
	else if (eState == CKirby::STATE_GUARD)
	{
		if (pKirby->Get_AbilityType() == ABILITY_SWORD)
			pKirby->Change_State(CKirby::SWORDSTATE_GUARD, 60.f, true, true, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		else
			pKirby->Change_State(CKirby::STATE_GUARD, 60.f, true, true, CKirby::BODY_DEFAULT);
	}
	else if (eState == CKirby::STATE_SLIDESTART)
	{
		if (pKirby->Get_AbilityType() == ABILITY_SWORD)
			pKirby->Change_State(CKirby::SWORDSTATE_SWORDSLIDESTART, 60.f, false, false, CKirby::BODY_SWORDDEFAULT, CKirby::OFFSET_SWORD);
		else
			pKirby->Change_State(CKirby::STATE_SLIDESTART, 60.f, false, false, CKirby::BODY_DEFAULT);
	}
	else if (eState == CKirby::STATE_FLIGHT)
	{
		if (pKirby->Get_AbilityType() == ABILITY_SWORD)
			pKirby->Change_State(CKirby::SWORDSTATE_HAVESWORDWAITFLIGHT, 60.f, false, false, CKirby::BODY_SWORDBALLOON, CKirby::OFFSET_SWORD);
		else
			pKirby->Change_State(CKirby::STATE_FLIGHT, 60.f, false, false, CKirby::BODY_BALLOON);
	}

}

// 커비가 사다리로 넘어가느냐 마느냐의 운명을 결정짓는 함수이다.
static _bool Kirby_Ladder_Logic(CKirby* pKirby, CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom)
{
	// 사다리에 탑승 가능한 상태가 아닐경우
	if (DESC(m_bCanLadder) == false)
		return false;

	// 다시 사다리에 올라가는걸 Block하는 기능이 켜져 있다면, 바로 탑승 불가 처리한다.
	if (DESC(m_bBlockLadder) == true)
		return false;

	_float4 vToLadderDir = DESC(m_vLadderOriginalPos) - pTransformCom->Get_State(CTransform::STATE_POSITION);
	vToLadderDir.y = 0.f;
	vToLadderDir = XMVector3Normalize(vToLadderDir);

	_float fDegree = acos(DESC(m_vMoveDir).Dot(vToLadderDir));
	fDegree = ToDegree(fDegree);

	// 만약, 내가 보고있는 방향에 사다리가 있을 때, 강제로 타진다.
	if (fDegree < 70.f)
		return true;

	// 그 외의 상황일 경우 false
	return false;
}

// 내가 누른 키가 사다리와 내적 했을 때, 제대로 붙어지는지 여부를 알 수 있는 함수
static _bool Kirby_JoyStickLadder_Logic(CKirby* pKirby, CKirby::KIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CGameObject* pCamera)
{
	if (JoyStick_On())
	{
		_float4 vJoyStickDir = JoyStick_controller_OtherDir(pCamera);
		_float4 vToLadderDir = DESC(m_vLadderOriginalPos) - pTransformCom->Get_State(CTransform::STATE_POSITION);
		vToLadderDir.y = 0.f;
		vToLadderDir = XMVector3Normalize(vToLadderDir);

		_float fDegree = acos(vJoyStickDir.Dot(vToLadderDir));
		fDegree = ToDegree(fDegree);

		if (fDegree < 70.f)
			return true;

	}

	return false;
}


#pragma region Hyo Effect

static void LadderStart_FX(CTransform* pTransformCom)
{
	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

	MultiFXDesc.vInitPos = static_cast<_float3>(pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, .3f, 0.f, 0.f });
	MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(CGameInstance::Get_Instance()->Get_CamLook());
	MultiFXDesc.vInitScale = { 2.f, 2.f, 2.f };
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Ladder Star"), &MultiFXDesc)))
		return;
}

static void Bbong_FX(_float fTimeDelta, CTransform* pTransformCom)
{
	static _float fBbongTime{ 0.f };
	fBbongTime += fTimeDelta;
	if (.2f < fBbongTime)
	{
		CMultiEffect::MULTI_FX_DESC FXDesc{};
		_float4 vMyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		vMyPos += pTransformCom->Get_State(CTransform::STATE_LOOK) * .4f;

		FXDesc.vInitPos = { vMyPos.x, vMyPos.y + .3f, vMyPos.z };
		FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };
		FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };

		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
			return;

		fBbongTime = 0.f;
	}
}

static void SwordSlash_One(CTransform* pTransformCom)
{
	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

	MultiFXDesc.vInitPos = static_cast<_float3>(pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, .3f, 0.f, 0.f } + pTransformCom->Get_State(CTransform::STATE_LOOK) * 2.f);
	MultiFXDesc.vInitRot = _float3{ 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };
	MultiFXDesc.vInitScale = { 4.f, 4.f, 4.f };
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SwordTrail_One"), &MultiFXDesc)))
		return;
}

static void SwordSlash_Two(CTransform* pTransformCom)
{
	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

	MultiFXDesc.vInitPos = static_cast<_float3>(pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, .3f, 0.f, 0.f } + pTransformCom->Get_State(CTransform::STATE_LOOK) * 2.f);
	MultiFXDesc.vInitRot = _float3{ 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };
	MultiFXDesc.vInitScale = { 4.f, 4.f, 4.f };
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_SwordTrail_Multi"), &MultiFXDesc)))
		return;
}
static void SwordSpinCharge(CTransform* pTransformCom)
{
	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

	MultiFXDesc.vInitPos = static_cast<_float3>(pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, -.4f, 0.f, 0.f });
	MultiFXDesc.vInitRot = CUtils::Make_Degree_FromDir(CGameInstance::Get_Instance()->Get_CamLook());
	MultiFXDesc.vInitScale = { 4.f, 4.f, 4.f };
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Sword_Copy Bubble_One"), &MultiFXDesc)))
		return;
}
static void SwordSpinSlash_One(CTransform* pTransformCom)
{
	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};

	MultiFXDesc.vInitPos = _float3{ 0.f, .2f, 0.f};
	MultiFXDesc.vInitScale = { 5.f, 5.f, 5.f };
	MultiFXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Sword_Spin Attack A"), &MultiFXDesc)))
		return;
}
#pragma endregion 


static void Throw_Bomb(CKirby::KIRBY_INFODESC* Kirbydesc, _float4 vDir, _float fPower)
{
	DESC(m_bBombHold) = false;
	DESC(m_vBombThrowDir) = vDir;
	DESC(m_fBombPower) = fPower;
}