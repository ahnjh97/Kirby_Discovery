#include "stdafx.h"
#include "CKirbyDump_State.h"
#include "Kirby_State_Function.h"
#include "FinaleKirby.h"
#include "FinalePartical_Maker.h"
#include "FinaleCut_ControlCenter.h"

void Turn_Interpolate(CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta, _float fInterpolateSpeed = 12.f)
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

		if (fAngleDegrees < 3.0f)
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
}
void Deceleration(CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
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
void Moving_Logic(CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, CCharacterController* pController, _float fTimeDelta)
{
	Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
	if (Kirbydesc->m_fMoveSpeed > 8.3f)
		Kirbydesc->m_fMoveSpeed = 8.3f;

	// 타겟기준
	_vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
}
_float4 Make_TargetDir(CFinaleKirby::DIR _eDir, CGameObject* pCamera)
{

	CTransform* pCameraTransform = pCamera->Get_TransformCom();
	_float4 vCamRight = pCameraTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	_float4 vCamLook = XMVector3Cross(vCamRight, XMVectorSet(0.f, 1.f, 0.f, 1.f));

	if (_eDir == CFinaleKirby::DIR_RIGHT)
		return vCamRight;
	else if (_eDir == CFinaleKirby::DIR_LEFT)
		return -1.f * vCamRight;
	else if (_eDir == CFinaleKirby::DIR_FRONT)
		return vCamLook;
	else if (_eDir == CFinaleKirby::DIR_BACK)
		return -1.f * vCamLook;

	else if (_eDir == CFinaleKirby::DIR_LB)
		return XMVector4Normalize((-1.f * vCamRight) + (-1.f * vCamLook));
	else if (_eDir == CFinaleKirby::DIR_RB)
		return XMVector4Normalize(vCamRight + (-1.f * vCamLook));
	else if (_eDir == CFinaleKirby::DIR_LF)
		return XMVector4Normalize((-1.f * vCamRight) + (vCamLook));
	else if (_eDir == CFinaleKirby::DIR_RF)
		return XMVector4Normalize(vCamRight + vCamLook);

	return _float4(0.f, 0.f, 0.f, 0.f);
}
_bool JoyStick_controller(CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc, CGameObject* pCamera)
{
	if (GAMEINSTANCE Get_DIKeyState(DIK_UP, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_LF, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_RF, pCamera);
		else
			DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_FRONT, pCamera);

		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_DOWN, KEY_PRESS))
	{
		if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_LB, pCamera);
		else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
			DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_RB, pCamera);
		else
			DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_BACK, pCamera);

		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_LEFT, KEY_PRESS))
	{
		DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_LEFT, pCamera);
		return true;
	}
	else if (GAMEINSTANCE Get_DIKeyState(DIK_RIGHT, KEY_PRESS))
	{
		DESC(m_vTargetDir) = Make_TargetDir(CFinaleKirby::DIR_RIGHT, pCamera);
		return true;
	}

	return false;
}
void Turn_InterPolate_OtherVector(_float4& vDstDir, _float4& vSrcDir, CTransform* pTransformCom, _float fTimeDelta, _float fInterpolateSpeed = 12.f)
{
	if (vSrcDir == vDstDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * fInterpolateSpeed;
	_vector vTargetDir = vDstDir;
	_vector vMoveDir = vSrcDir;
	_vector vTargetDirXZ = XMVectorSet(XMVectorGetX(vTargetDir), 0.0f, XMVectorGetZ(vTargetDir), 0.0f);
	_vector vMoveDirXZ = XMVectorSet(XMVectorGetX(vMoveDir), 0.0f, XMVectorGetZ(vMoveDir), 0.0f);
	vTargetDirXZ = XMVector3Normalize(vTargetDirXZ);
	vMoveDirXZ = XMVector3Normalize(vMoveDirXZ);
	_float fcosTheta = XMVectorGetX(XMVector4Dot(vTargetDirXZ, vMoveDirXZ));

	if (fcosTheta < -0.9995f || fcosTheta > 0.9995f)
	{
		return;
	}
	else
	{
		_float ftheta = acos(fcosTheta);
		_float fAngleDegrees = XMConvertToDegrees(ftheta);

		if (fAngleDegrees < 0.5f)
		{
			vSrcDir = vDstDir;
		}
		else
		{
			_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
			_float fAlpha = sin((1 - fInterpolate) * ftheta) / fsinTheta;
			_float fBeta = sin(fInterpolate * ftheta) / fsinTheta;
			_float4 vResult = vMoveDirXZ * fAlpha + vTargetDirXZ * fBeta;
			vSrcDir = XMVector4Normalize(vResult);
			vSrcDir = XMVector3Normalize(vSrcDir);

		}
	}
}
void ToCut_Reset_Kirby(CTransform* pTransformCom, CCharacterController* pController)
{
	pController->Set_Position(pTransformCom, _float4(2550.f, 239.f, -136.f, 1.f));
	_float4 NewLook = _float4(1.f, 0.f, 0.f, 0.f);
	_float4 NewUp = _float4(0.f, 1.f, 0.f, 0.f);
	_float4 NewRight = XMVector3Cross(NewLook, NewUp);

	pTransformCom->Set_State(CTransform::STATE_LOOK, NewLook);
	pTransformCom->Set_State(CTransform::STATE_UP, NewUp);
	pTransformCom->Set_State(CTransform::STATE_RIGHT, NewRight);

	CFinaleCut_ControlCenter* pCenter = static_cast<CFinaleCut_ControlCenter*>(GAMEINSTANCE Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	pCenter->Set_CutScene(1);
}

#pragma region 차량 운전 상태


CKirbyDump_Run_State::CKirbyDump_Run_State()
{
}

void CKirbyDump_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyDump_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

	m_fParticalDelay += fTimeDelta;

	if (pTransformCom->Get_State(CTransform::STATE_POSITION).x > 1550.f)
	{
		pKirby->Change_State(CFinaleKirby::DUMPCUTSTATE_CUT1, 50.f, false, false, CFinaleKirby::BODY_DUMPCUT, CFinaleKirby::OFFSET_DUMPCUT);
		ToCut_Reset_Kirby(pTransformCom, pController);
		pKirby->Start_CutScene();
		return;
	}


	if (m_fParticalDelay > 0.2f)
	{
		CFinalePartical_Maker* pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinalePartical_Maker")));
		_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
		pMaker->Make_Partical(3, vPos + (vLook * 3.f), 3.f, 0.2f, 0.1f, XMVector3Normalize(_float4(0.f, 1.f, 0.f, 0.f) + vLook), 120.f, CUtils::Make_RandomFloat(40.f, 80.f));
		m_fParticalDelay = 0.f;
	}


	if ((pController->Compute_Height() < 1.5f || pController->RayCastToDynamicActor(_float4(0.f, -1.f, 0.f, 0.f)) < 1.5f) == false)
	{
		DESC(m_fJumpVelocity) = 0.f;
		pKirby->Change_State(CFinaleKirby::DUMPSTATE_JUMP, 60.f, true, false, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
		return;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS) == true)
	{
		Kirbydesc->m_vTargetDir = _float4(1.f, 0.f, 0.3f, 0.f);
		Kirbydesc->m_vTargetDir.Normalize();
		_float4 vMoveTarget = _float4(1.f, 0.f, 0.6f, 0.f);
		vMoveTarget.Normalize();
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vHandleDir, pTransformCom, fTimeDelta, 3.f);
		Turn_InterPolate_OtherVector(vMoveTarget, Kirbydesc->m_vMoveDir, pTransformCom, fTimeDelta, 1.f);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS) == true)
	{
		Kirbydesc->m_vTargetDir = _float4(1.f, 0.f, -0.3f, 0.f);
		Kirbydesc->m_vTargetDir.Normalize();
		_float4 vMoveTarget = _float4(1.f, 0.f, -0.6f, 0.f);
		vMoveTarget.Normalize();
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vHandleDir, pTransformCom, fTimeDelta, 3.f);
		Turn_InterPolate_OtherVector(vMoveTarget, Kirbydesc->m_vMoveDir, pTransformCom, fTimeDelta, 1.f);
	}
	else
	{
		Kirbydesc->m_vTargetDir = _float4(1.f, 0.f, 0.f, 0.f);
		Kirbydesc->m_vTargetDir.Normalize();
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vHandleDir, pTransformCom, fTimeDelta, 3.f);
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vMoveDir, pTransformCom, fTimeDelta, 2.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_SPACE, KEY_DOWN))
	{
		m_bBreak = !m_bBreak;
	}

	if (m_bBreak == true)
		return;


	Kirbydesc->m_fMoveSpeed += fTimeDelta * 10.f;

	if (DESC(m_bBooster) == true)
	{
		if (Kirbydesc->m_fMoveSpeed > 36.f)
			Kirbydesc->m_fMoveSpeed = 36.f;
	}
	else
	{
		if (Kirbydesc->m_fMoveSpeed > 20.f)
			Kirbydesc->m_fMoveSpeed = 20.f;
	}

	// 타겟기준
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


	if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
	{
		DESC(m_fJumpVelocity) = 20.f;
		pKirby->Change_State(CFinaleKirby::DUMPSTATE_JUMP, 60.f, true, false, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
		return;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBooster) == false)
	{
		//부슽 이펙트
		ComeOn_Dash_For_Dump(pTransformCom);
		pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
		DESC(m_bBooster) = true;
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
		pCamera->Make_Shake(0.6f, 2.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
	{
		DESC(m_fBoosterTime) = 5.f;
	}
	else
	{
		if (DESC(m_fBoosterTime) > 0.f)
			DESC(m_fBoosterTime) -= fTimeDelta;

		if (DESC(m_fBoosterTime) < 0.f)
			DESC(m_fBoosterTime) = 0.f;
	}

	if (DESC(m_fBoosterTime) <= 0.f)
	{
		DESC(m_bBooster) = false;
		pKirby->Delete_Effect("Come On Dash");
	}
	//트럭방구
	else
	{
		static _float fBoostTime = 0.f;
		fBoostTime += fTimeDelta;
		if (.05f < fBoostTime)
		{
			CEffect::FX_DESC fxDesc{};
			fxDesc.vInitRot = CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK));

			_float3 vCenterPos = (_float3)pTransformCom->Get_State(CTransform::STATE_POSITION);
			fxDesc.vInitPos = vCenterPos;
			fxDesc.vInitPos += (_float3)pTransformCom->Get_State(CTransform::STATE_RIGHT) * (2.3f + CUtils::Make_RandomFloat(-.2f, .2f));
			fxDesc.vInitPos += (_float3)pTransformCom->Get_State(CTransform::STATE_UP) * 7.2f;

			_float fScale = CUtils::Make_RandomFloat( 1.5f, 3.f);
			fxDesc.vInitScale = { fScale, fScale, fScale };
			pKirby->Add_Effect("dump dash smoke", fxDesc);

			fxDesc.vInitPos = vCenterPos;
			fxDesc.vInitPos -= (_float3)pTransformCom->Get_State(CTransform::STATE_RIGHT) * (2.3f + CUtils::Make_RandomFloat(-.2f, .2f));
			fxDesc.vInitPos += (_float3)pTransformCom->Get_State(CTransform::STATE_UP) * 7.2f;

			fScale = CUtils::Make_RandomFloat( 1.5f, 3.f);
			fxDesc.vInitScale = { fScale, fScale, fScale };
			pKirby->Add_Effect("dump dash smoke", fxDesc);
			fBoostTime = 0.f;
		}
	}
}

void CKirbyDump_Run_State::OnStateExit()
{
}

CKirbyDump_Run_State* CKirbyDump_Run_State::Create()
{
    CKirbyDump_Run_State* pInstance = new CKirbyDump_Run_State();
    return pInstance;
}

void CKirbyDump_Run_State::Free()
{
    __super::Free();
}

#pragma endregion


#pragma region 차량 점프 상태

CKirbyDump_Jump_State::CKirbyDump_Jump_State()
{
}

void CKirbyDump_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

}

void CKirbyDump_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


	if (pTransformCom->Get_State(CTransform::STATE_POSITION).x > 1550.f)
	{
		pKirby->Change_State(CFinaleKirby::DUMPCUTSTATE_CUT1, 50.f, false, false, CFinaleKirby::BODY_DUMPCUT, CFinaleKirby::OFFSET_DUMPCUT);
		ToCut_Reset_Kirby(pTransformCom, pController);
		pKirby->Start_CutScene();
		return;
	}


	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN) && DESC(m_bBooster) == false)
	{
		//부슽 이펙트
		ComeOn_Dash_For_Dump(pTransformCom);
		pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
		DESC(m_bBooster) = true;
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
		pCamera->Make_Shake(1.f, 2.f);
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
	{
		DESC(m_fBoosterTime) = 5.f;
	}
	else
	{
		if (DESC(m_fBoosterTime) > 0.f)
			DESC(m_fBoosterTime) -= fTimeDelta;
		if (DESC(m_fBoosterTime) < 0.f)
			DESC(m_fBoosterTime) = 0.f;
	}

	if (DESC(m_fBoosterTime) <= 0.f)
	{
		DESC(m_bBooster) = false;
		pKirby->Delete_Effect("Come On Dash");
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_PRESS) == true)
	{
		Kirbydesc->m_vTargetDir = _float4(1.f, 0.f, 0.3f, 0.f);
		Kirbydesc->m_vTargetDir.Normalize();
		_float4 vMoveTarget = _float4(1.f, 0.f, 0.6f, 0.f);
		vMoveTarget.Normalize();
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vHandleDir, pTransformCom, fTimeDelta, 3.f);
		Turn_InterPolate_OtherVector(vMoveTarget, Kirbydesc->m_vMoveDir, pTransformCom, fTimeDelta, 1.f);
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_PRESS) == true)
	{
		Kirbydesc->m_vTargetDir = _float4(1.f, 0.f, -0.3f, 0.f);
		Kirbydesc->m_vTargetDir.Normalize();
		_float4 vMoveTarget = _float4(1.f, 0.f, -0.6f, 0.f);
		vMoveTarget.Normalize();
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vHandleDir, pTransformCom, fTimeDelta, 3.f);
		Turn_InterPolate_OtherVector(vMoveTarget, Kirbydesc->m_vMoveDir, pTransformCom, fTimeDelta, 1.f);
	}
	else
	{
		Kirbydesc->m_vTargetDir = _float4(1.f, 0.f, 0.f, 0.f);
		Kirbydesc->m_vTargetDir.Normalize();
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vHandleDir, pTransformCom, fTimeDelta, 3.f);
		Turn_InterPolate_OtherVector(Kirbydesc->m_vTargetDir, Kirbydesc->m_vMoveDir, pTransformCom, fTimeDelta, 2.f);
	}

	Kirbydesc->m_fMoveSpeed += fTimeDelta * 10.f;
	if (DESC(m_bBooster) == true)
	{
		if (Kirbydesc->m_fMoveSpeed > 36.f)
			Kirbydesc->m_fMoveSpeed = 36.f;
	}
	else
	{
		if (Kirbydesc->m_fMoveSpeed > 20.f)
			Kirbydesc->m_fMoveSpeed = 20.f;
	}

	// 타겟기준
	_vector vMoveDelta = Kirbydesc->m_vMoveDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
	pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);


	if (pKirby->Get_State() == CFinaleKirby::DUMPSTATE_JUMP)
	{
		m_fFallTime += fTimeDelta;

		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CFinaleKirby::DUMPSTATE_LANDING, 60.f, false, false, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			if (m_fFallTime < 1.5f)
				pCamera->Make_Shake(1.5f);
			else
			{
				if (m_fFallTime > 3.f)
					pCamera->Make_Shake(3.f, 2.f);
				else
					pCamera->Make_Shake(m_fFallTime, 2.f);
			}

			CFinalePartical_Maker* pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinalePartical_Maker")));
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			pMaker->Make_Partical(20, vPos + (vLook * 3.f), 3.f, 0.2f, 0.1f, XMVector3Normalize(_float4(0.f, 1.f, 0.f, 0.f) + vLook * 2.f), 120.f, CUtils::Make_RandomFloat(40.f, 80.f));
			return;
		}
	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPSTATE_LANDING)
	{
		pController->FreeFall(pTransformCom, fTimeDelta);

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CFinaleKirby::DUMPSTATE_IDLING, 60.f, true, false, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
			return;
		}
	}


}

void CKirbyDump_Jump_State::OnStateExit()
{
	m_fFallTime = 0.f;
}

CKirbyDump_Jump_State* CKirbyDump_Jump_State::Create()
{
    CKirbyDump_Jump_State* pInstance = new CKirbyDump_Jump_State();
    return pInstance;
}

void CKirbyDump_Jump_State::Free()
{
    __super::Free();
}

#pragma endregion



#pragma region 차량 컷씬 상태

CKirbyDump_Cut_State::CKirbyDump_Cut_State()
{
}

void CKirbyDump_Cut_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

}

void CKirbyDump_Cut_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

    if (pKirby->Get_State() == CFinaleKirby::STATE_IDLE)
    {
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
		pKirby->Change_State(CFinaleKirby::STATE_RUN, 120.f, true, true, CFinaleKirby::BODY_DEFAULT);
		return;
    }
    else if (pKirby->Get_State() == CFinaleKirby::STATE_RUN)
    {
		m_fRunTime += fTimeDelta;

        Bbong_FX(fTimeDelta, pTransformCom);

		DESC(m_vTargetDir) = _float4(1.f, 0.f, 0.f, 0.f);
		Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
        pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (m_fRunTime > 1.35f)
		{
			pKirby->Change_State(CFinaleKirby::STATE_INHALESTART, 60.f, false, false, CFinaleKirby::BODY_VACUUM);
			CMultiEffect::MULTI_FX_DESC FXDesc{};
			FXDesc.vInitPos = { 0.f, .6f, .4f };
			FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Vacuum_v3"), &FXDesc)))
				return;
			pKirby->Add_Effect(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));

			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(0.3f, 100.f);

			return;
		}
    }
	else if (pKirby->Get_State() == CFinaleKirby::STATE_INHALESTART)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		//pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CFinaleKirby::STATE_INHALE, 50.f, false, false, CFinaleKirby::BODY_VACUUM);
		}
	}
	else if (pKirby->Get_State() == CFinaleKirby::STATE_INHALE)
	{
		DESC(m_eEyeState) = CFinaleKirby::EYE_CLOSE;
		//pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));


		if (pKirby->isAnimFinish())
			pKirby->Change_State(CFinaleKirby::STATE_SUPERINHALESTART, 60.f, false, false, CFinaleKirby::BODY_VACUUM);
	}
	else if (pKirby->Get_State() == CFinaleKirby::STATE_SUPERINHALESTART)
	{
		DESC(m_eEyeState) = CFinaleKirby::EYE_CLOSE;
		//pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));


		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CFinaleKirby::STATE_VACUUM, 50.f, true, false, CFinaleKirby::BODY_VACUUM);
		}

	}
	else if (pKirby->Get_State() == CFinaleKirby::STATE_VACUUM)
	{
		DESC(m_eEyeState) = CFinaleKirby::EYE_CLOSE;
		//pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));

		if (DESC(m_bVacuumComplete) == true)
		{
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake(0.3f, 0.05f);
			pKirby->Change_State(CFinaleKirby::DUMPSTATE_CUTDEMOKIRBY, 50.f, false, false, CFinaleKirby::BODY_DUMPVACUUM, CFinaleKirby::OFFSET_DUMPVACUUM);
			pKirby->Delete_AllEffect();
			return;
		}
	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPSTATE_CUTDEMOKIRBY)
	{
		Kirbydesc->m_fMoveSpeed = 0.f;
		m_fRunTime += fTimeDelta;
		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
		if (m_fRunTime > 2.f)
		{
			DESC(m_eEyeState) = CFinaleKirby::EYE_IDLE;
			pKirby->Change_State(CFinaleKirby::DUMPTSTATE_CUT, 50.f, false, false, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			pController->Set_Position(pTransformCom, vPos + _float4(-4.f, 0.f, 0.f, 0.f));
		}
	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPTSTATE_CUT)
	{
		m_fRunTime += fTimeDelta;
		if (m_bShakeTrigger == true && m_fRunTime > 1.55f)
		{
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
			pCamera->Make_Shake();

			CFinalePartical_Maker* pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinalePartical_Maker")));
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
			_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
			pMaker->Make_Partical(50, vPos, 3.f, 0.1f, 0.05f, _float4(0.f, 1.f, 0.f, 0.f), 120.f, CUtils::Make_RandomFloat(30.f, 70.f));

			m_bShakeTrigger = false;

			//랜딩 스모크
			_float3 vMyPos = (_float3)pTransformCom->Get_State(CTransform::STATE_POSITION);

			CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
			MultiFXDesc.vInitPos = vMyPos;
			MultiFXDesc.vInitScale = { 4.f, 4.f, 4.f };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_dump smoke"), &MultiFXDesc)))
				return;
		}

		if (m_fRunTime > 1.7f && m_fRunTime < 1.8f)
			DESC(m_eEyeState) = CFinaleKirby::EYE_BLINK;
		else if (m_fRunTime > 1.9f && m_fRunTime < 2.f)
			DESC(m_eEyeState) = CFinaleKirby::EYE_BLINK;
		else
			DESC(m_eEyeState) = CFinaleKirby::EYE_IDLE;


		pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset), -1.f);

		if (pKirby->isAnimFinish())
		{
			DESC(m_vHandleDir) = DESC(m_vMoveDir);
			pKirby->Change_State(CFinaleKirby::DUMPSTATE_IDLING, 50.f, true, true, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
		}
	}
}

void CKirbyDump_Cut_State::OnStateExit()
{
	m_fRunTime = 0.f;
	m_bShakeTrigger = true;
}

CKirbyDump_Cut_State* CKirbyDump_Cut_State::Create()
{
    CKirbyDump_Cut_State* pInstance = new CKirbyDump_Cut_State();
    return pInstance;
}

void CKirbyDump_Cut_State::Free()
{
    __super::Free();
}

#pragma endregion



#pragma region 차량 컷씬 상태 2

CKirbyDump_Cut2_State::CKirbyDump_Cut2_State()
{
}

void CKirbyDump_Cut2_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyDump_Cut2_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinaleKirby* pKirby = static_cast<CFinaleKirby*>(pGameObject);
	CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CCamera_Main* pCamera = static_cast<CCamera_Main*>(GAMEINSTANCE Get_CurCameraPtr());
	CFinaleCut_ControlCenter* pCenter = static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));

	_int iAnimIndex = pCenter->Get_CutScene();

	if (iAnimIndex == 1)
		pKirby->Change_State(CFinaleKirby::DUMPCUTSTATE_CUT1, 50.f, false, false, CFinaleKirby::BODY_DUMPCUT, CFinaleKirby::OFFSET_DUMPCUT);
	else if (iAnimIndex == 2)
		pKirby->Change_State(CFinaleKirby::DUMPCUTSTATE_CUT2, 50.f, false, false, CFinaleKirby::BODY_DUMPCUT, CFinaleKirby::OFFSET_DUMPCUT);
	else if (iAnimIndex == 6)
		pKirby->Change_State(CFinaleKirby::DUMPCUTSTATE_CUT6, 50.f, false, false, CFinaleKirby::BODY_DUMPCUT, CFinaleKirby::OFFSET_DUMPCUT);


	// 컷씬 진입소. 점프 점프
	if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT1)
	{


		if (pKirby->isAnimFinish())
		{
			pCenter->Set_CutScene(2);
		}
	}
	// 멀리서 부릉부릉
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT2)
	{


		if (pKirby->isAnimFinish())
		{
			pCenter->Set_CutScene(3);
		}
	}
	// 큐티 모션 후 점프
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT6)
	{

	}
	//
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT7)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT8)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT9)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT10)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT11)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT12)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT13)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT14)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT15)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT16)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT17)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT18)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT19)
	{

	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPCUTSTATE_CUT20)
	{

	}
}

void CKirbyDump_Cut2_State::OnStateExit()
{
	m_fTime = 0.f;
}

CKirbyDump_Cut2_State* CKirbyDump_Cut2_State::Create()
{
	CKirbyDump_Cut2_State* pInstance = new CKirbyDump_Cut2_State();
	return pInstance;
}

void CKirbyDump_Cut2_State::Free()
{
	__super::Free();
}

#pragma endregion
