#include "stdafx.h"
#include "CKirbyDump_State.h"
#include "Kirby_State_Function.h"
#include "FinaleKirby.h"

void Turn_Interpolate(CFinaleKirby::FINALEKIRBY_INFODESC* Kirbydesc, CTransform* pTransformCom, _float fTimeDelta)
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


}

void CKirbyDump_Jump_State::OnStateExit()
{
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
			pKirby->Change_State(CFinaleKirby::STATE_INHALESTART, 60.f, false, true, CFinaleKirby::BODY_VACUUM);
			CMultiEffect::MULTI_FX_DESC FXDesc{};
			FXDesc.vInitPos = { 0.f, .6f, .4f };
			FXDesc.pSocketMatrix = pTransformCom->Get_WorldFloat4x4_Ptr();
			if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Vacuum_v3"), &FXDesc)))
				return;
			return;
		}
    }
	else if (pKirby->Get_State() == CFinaleKirby::STATE_INHALESTART)
	{

		if (pKirby->isAnimFinish())
			pKirby->Change_State(CFinaleKirby::STATE_INHALE, 50.f, false, true, CFinaleKirby::BODY_VACUUM);
	}
	else if (pKirby->Get_State() == CFinaleKirby::STATE_INHALE)
	{
		DESC(m_eEyeState) = CFinaleKirby::EYE_CLOSE;

		if (pKirby->isAnimFinish())
			pKirby->Change_State(CFinaleKirby::STATE_SUPERINHALESTART, 60.f, false, true, CFinaleKirby::BODY_VACUUM);
	}
	else if (pKirby->Get_State() == CFinaleKirby::STATE_SUPERINHALESTART)
	{
		DESC(m_eEyeState) = CFinaleKirby::EYE_CLOSE;

		if (pKirby->isAnimFinish())
			pKirby->Change_State(CFinaleKirby::STATE_VACUUM, 50.f, true, true, CFinaleKirby::BODY_VACUUM);

	}
	else if (pKirby->Get_State() == CFinaleKirby::STATE_VACUUM)
	{
		DESC(m_eEyeState) = CFinaleKirby::EYE_CLOSE;


		if (DESC(m_bVacuumComplete) == true)
		{
			pKirby->Change_State(CFinaleKirby::DUMPSTATE_CUTDEMOKIRBY, 50.f, false, false, CFinaleKirby::BODY_DUMPVACUUM, CFinaleKirby::OFFSET_DUMPVACUUM);
		}
	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPSTATE_CUTDEMOKIRBY)
	{
		if (pKirby->isAnimFinish())
			pKirby->Change_State(CFinaleKirby::DUMPTSTATE_CUT, 50.f, false, false, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
	}
	else if (pKirby->Get_State() == CFinaleKirby::DUMPTSTATE_CUT)
	{
		if (pKirby->isAnimFinish())
			pKirby->Change_State(CFinaleKirby::DUMPSTATE_IDLING, 50.f, true, true, CFinaleKirby::BODY_DUMPDEFAULT, CFinaleKirby::OFFSET_DUMP);
	}
}

void CKirbyDump_Cut_State::OnStateExit()
{
	m_fRunTime = 0.f;
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
