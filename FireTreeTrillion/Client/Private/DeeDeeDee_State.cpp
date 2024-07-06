#include "stdafx.h"
#include "DeeDeeDee_State.h"
#include "DeeDeeDee.h"
#include "Kirby.h"
#include "Camera_Main.h"
#include "MultiEffect.h"
#include "Ability.h"

#define DESC(Dst) DDDDesc->Dst

void Turn_Interpolate(CDeeDeeDee::DDDDESC* DDDDesc, CTransform* pTransformCom, _float fTimeDelta)
{
	if (DDDDesc->m_vMoveDir == DDDDesc->m_vTargetDir)
		return;

	///////// 보간 속도 조정임
	_float fInterpolate = fTimeDelta * 7.f;
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
		_float4x4 rotationMatrix;
		XMStoreFloat4x4(&rotationMatrix, XMMatrixIdentity());
		CUtils::Turn_OtherMatrix(rotationMatrix, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomInt(0, 1) == 1 ? 0.5f : -0.5f);
		DDDDesc->m_vMoveDir = XMVector3Transform(DDDDesc->m_vMoveDir, XMLoadFloat4x4(&rotationMatrix));
		DDDDesc->m_vMoveDir = XMVectorSetW(DDDDesc->m_vMoveDir, 0.0f);
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

static void Hammer_Slash(_float fTimeDelta, CTransform* pTransformCom, _bool bIsVertical = false)
{

	_float4 vMyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
	if (!bIsVertical)
	{
		CEffect::FX_DESC FXDesc{};
		FXDesc.vInitPos = { vMyPos.x, vMyPos.y + .3f, vMyPos.z };
		FXDesc.vInitPos.y += 2.7f;
		FXDesc.vInitPos.z -= .5f;

		FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };
		FXDesc.fStartDelay = 0.05f;

		FXDesc.vInitScale = { 4.f, 4.f, 4.f };

		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_hammer hit_hrzt"), &FXDesc)))
			return;
	}
	//찍기
	else
	{
		CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
		MultiFXDesc.vInitPos = { vMyPos.x, vMyPos.y + .3f, vMyPos.z };

		MultiFXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };
		MultiFXDesc.vInitScale = { 4.f, 4.f, 4.f };

		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_multi hammer hit A"), &MultiFXDesc)))
			return;

		MultiFXDesc = {};
		MultiFXDesc.vInitPos = _float3{ vMyPos.x, vMyPos.y + .3f, vMyPos.z } + _float3(pTransformCom->Get_State(CTransform::STATE_LOOK) * 8.f);
		/*MultiFXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };*/
		MultiFXDesc.vInitScale = {2.f, 2.f, 2.f};
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_DDD land smoke"), &MultiFXDesc)))
			return;
	}

}

static void Landing(_float fTimeDelta, CTransform* pTransformCom)
{
	_float3 vMyPos = (_float3)pTransformCom->Get_State(CTransform::STATE_POSITION);


	CMultiEffect::MULTI_FX_DESC MultiFXDesc{};
	MultiFXDesc.vInitPos = vMyPos;
	MultiFXDesc.vInitScale = { 4.f, 4.f, 4.f };
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_DDD land smoke"), &MultiFXDesc)))
		return;


	//CMultiEffect::MULTI_FX_DESC FXDesc{};
	//FXDesc.vInitPos = vMyPos + pTransformCom->Get_State(CTransform::STATE_LOOK);
	//FXDesc.vInitScale = { 4.f, 4.f, 4.f };

	//for (_uint i = 0; i < 3; ++i)
	//{
	//	FXDesc.fStartDelay = CUtils::Make_RandomFloat(0.f, .2f);
	//	_float vOffset = CUtils::Make_RandomFloat(-1.f, .5f);
	//	FXDesc.vInitScale += {vOffset, vOffset, vOffset};
	//	FXDesc.vInitRot = CUtils::Make_Degree_FromDir( (_float3)CUtils::Make_RandomAngle_Vector(80.f, _float4{ 0.f, 1.f, 0.f, 0.f }));
	//	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_explode lines"), &FXDesc)))
	//		return;
	//}

}

static void Sliding(_float fTimeDelta, CTransform* pTransformCom)
{
	static _float fBbongTime{ 0.f };
	fBbongTime += fTimeDelta;

	if (.2f < fBbongTime)
	{
		CMultiEffect::MULTI_FX_DESC FXDesc{};

		FXDesc.vInitPos = static_cast<_float3>(pTransformCom->Get_State(CTransform::STATE_POSITION) - pTransformCom->Get_State(CTransform::STATE_LOOK) * 2.5f + _float4{ 0.f, 1.7f, 0.f, 0.f });
		FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(-pTransformCom->Get_State(CTransform::STATE_LOOK)).y + CUtils::Make_RandomFloat(-20.f, 20.f), CUtils::Make_RandomFloat(-20.f, 20.f) };
		FXDesc.vInitScale = { 2.f ,2.f, 2.f };
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Fly End Smoke"), &FXDesc)))
			return;

		fBbongTime = 0.f;
	}
}

//static void Rock_Particle()
//{
//
//}

static void Shout(_float fTimeDelta, CTransform* pTransformCom)
{
	CEffect::FX_DESC FXDesc{};

	FXDesc.vInitPos = static_cast<_float3>(pTransformCom->Get_State(CTransform::STATE_POSITION) + pTransformCom->Get_State(CTransform::STATE_LOOK) * 2.f + _float4{ 0.f, 1.8f, 0.f, 0.f });
	FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(pTransformCom->Get_State(CTransform::STATE_LOOK)).y, 0.f };
	FXDesc.vInitScale = { 6.f, 6.f, 6.f };
	if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_ddd shout A"), &FXDesc)))
		return;
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

	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);
	Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
	DESC(m_isBattle) = true;

	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_TOWN)
		return;


	// 배틀 상태의 딜레이일 경우, 방향대로 보간하게 만든다.
	if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE)
	{
		Make_TargetDir(DDDDesc, vPos, vKirbyPos);
	}

	m_fIdleDelay += fTimeDelta;

	// 아이들 딜레이 1.5초
	if (m_fIdleDelay < 0.5f)
		return;

	// 패턴에 따라 [사이드 어택], [점프 공격], [슬라이딩 공격], [망치 공격] 을 하는데, 와들디한테는 슬라이딩공격만 안한다.
	if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_CENTER)
	{

		// 내가 원점에서 와들디들을 팰 목록들
		if ((vPos - DESC(m_vOriginPos)).Length() < 3.f)
		{
			// 와들디 들을 존나 팬다.
			switch (CUtils::Make_RandomInt(3, 3))
			{
			case 0:
				pDee->Change_State(CDeeDeeDee::STATE_SHOUTSTART, 60.f, false, true);
				return;
			case 1:
				pDee->Change_State(CDeeDeeDee::STATE_JUMPSTART, 60.f, true, true);
				return;
			case 2:
				pDee->Change_State(CDeeDeeDee::STATE_HAMMERSIDE, 60.f, false, true);
				return;
			case 3:
				pDee->Change_State(CDeeDeeDee::STATE_HAMMERATTACK, 60.f, false, true);
				return;
			}
		}
		else
		{
			// 원점에 없고, 센터로 가라는 명령을 받았다면 센터로 돌아간다.
			pDee->Change_State(CDeeDeeDee::STATE_WALK, 120.f, true, true);
			return;
		}
	}
	// 난 지금 전투 상태다. 점프 또는 달려서 조팬다.
	else if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE)
	{
		// 달릴때, 망치정빵, 망치 스윙, 슬라이딩, 괴성을 지를 수 있다.
		switch (CUtils::Make_RandomInt(0, 4))
		{
		case 0:
			pDee->Change_State(CDeeDeeDee::STATE_RUN, 40.f, true, true);
			return;
		case 1:
			pDee->Change_State(CDeeDeeDee::STATE_RUN, 40.f, true, true);
			return;
		case 2:
			pDee->Change_State(CDeeDeeDee::STATE_RUN, 40.f, true, true);
			return;
		case 3:
			pDee->Change_State(CDeeDeeDee::STATE_RUN, 40.f, true, true);
			return;
		case 4:
			pDee->Change_State(CDeeDeeDee::STATE_JUMPSTART, 60.f, true, true);
			return;
		}
		return;
	}

}

void CDeeDeeDee_Idle_State::OnStateExit()
{
	m_fIdleDelay = 0.f;
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


#pragma region RUN STATE

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


	if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE && pController->Compute_Wall(pTransformCom->Get_State(CTransform::STATE_LOOK), 0.f) < 3.5f)
	{
		pDee->Change_State(CDeeDeeDee::STATE_JUMPSTART, 60.f, true, true);
		return;
	}


	if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE)
	{
		// 배틀일 땐, 항상 뛸 것이다.
		DESC(m_isBattle) = true;
		pDee->Change_State(CDeeDeeDee::STATE_RUN, 40.f, true, true);

		// 커비를 향해 존나 뛴다.
		_float4 vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);
		_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		Make_TargetDir(DDDDesc, vPos, vKirbyPos);
		Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		Moving_Logic(DDDDesc, pTransformCom, pController, fTimeDelta, 50.f, 9.f);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

		// 커비랑 접근했다면, 어떤 패턴들이 발동한다.
		if ((vKirbyPos - vPos).Length() <= 10.f)
		{
			// 패턴발동! (랜덤)
			switch (CUtils::Make_RandomInt(0, 3))
			{
			case 0:
				pDee->Change_State(CDeeDeeDee::STATE_SHOUTSTART, 60.f, false, true);
				return;
			case 1:
				pDee->Change_State(CDeeDeeDee::STATE_HAMMERSIDE, 60.f, false, true);
				return;
			case 2:
				pDee->Change_State(CDeeDeeDee::STATE_HAMMERATTACK, 60.f, false, true);
				return;
			case 3:
				pDee->Change_State(CDeeDeeDee::STATE_SLIDINGSTART, 60.f, false, true);
				DESC(m_fJumpVelocity) = 35.f;
				return;
			}
			return;
		}

	}
	else if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_CENTER)
	{
		// 센터로 걸어가는 상황일 땐, 항상 걸을 것이다.
		if (pDee->Get_State() == CDeeDeeDee::STATE_RUN)
		{
			// 짜치게 돌아가면 안되니까 괴성지르기
			pDee->Change_State(CDeeDeeDee::STATE_SHOUTSTART, 60.f, false, true);
			return;
		}

		DESC(m_isBattle) = false;
		pDee->Change_State(CDeeDeeDee::STATE_WALK, 120.f, true, true);

		// 센터로 걸어간다.
		_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
		Make_TargetDir(DDDDesc, vPos, DESC(m_vOriginPos));
		Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		Moving_Logic(DDDDesc, pTransformCom, pController, fTimeDelta, 50.f, 6.5f);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

		// 다 걸어갔으면 와들 디 개 팰 준비.
		if ((vPos - DESC(m_vOriginPos)).Length() < 3.f)
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, true);
			return;
		}
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


#pragma region SHOUT STATE

CDeeDeeDee_Shout_State::CDeeDeeDee_Shout_State()
{
}

void CDeeDeeDee_Shout_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Shout_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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

	if (pDee->Get_State() == CDeeDeeDee::STATE_SHOUTSTART)
	{
		if (m_bTurnTrigger == true)
		{
			m_bSetTurn = DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE ? true : false;
			m_bTurnTrigger = false;
		}

		if (m_bSetTurn == true)
		{
			DESC(m_isBattle) = true;
			// 샤우팅을 시작하기 위해.
			Make_TargetDir(DDDDesc, vPos, vKirbyPos);
			Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
			pController->FreeFall(pTransformCom, fTimeDelta, 6.f);
		}
		else
			DESC(m_isBattle) = false;

		m_fShoutTime += fTimeDelta;

		if (m_bShoutTrigger == true && m_fShoutTime > 0.8f)
		{
			//한 틱 발동
			Shout(fTimeDelta, pTransformCom);

			m_pGameInstance->Setting_RadialBlur(vPos, 20.f, 60.f);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Make_Shake();
			m_bShoutTrigger = false;
		}

		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_SHOUT, 60.f, false, false);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_SHOUT)
	{
		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_SHOUTEND, 60.f, false, false);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_SHOUTEND)
	{
		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, false);
		}
	}
}

void CDeeDeeDee_Shout_State::OnStateExit()
{
	m_bShoutTrigger = true;
	m_fShoutTime = 0.f;

	m_bTurnTrigger = true;
}

CDeeDeeDee_Shout_State* CDeeDeeDee_Shout_State::Create()
{
	CDeeDeeDee_Shout_State* pInstance = new CDeeDeeDee_Shout_State();
	return pInstance;
}

void CDeeDeeDee_Shout_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region SILDE STATE

CDeeDeeDee_Slide_State::CDeeDeeDee_Slide_State()
{
}

void CDeeDeeDee_Slide_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Slide_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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

	DESC(m_isBattle) = true;

	if (pDee->Get_State() == CDeeDeeDee::STATE_SLIDINGSTART)
	{
		_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
		pController->Move_Dir(pTransformCom, vLook * fTimeDelta * 15.f, fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (pController->Is_Terrain())
		{
			m_pGameInstance->Setting_RadialBlur(vPos, 20.f, 100.f);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Make_Shake(2.f);
			pDee->Change_State(CDeeDeeDee::STATE_SLIDING, 60.f, false, false);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_SLIDING)
	{
		//슬라이딩 이펙트 발생
		if (pDee->IsAnimRatio() < .5f)
			Sliding(fTimeDelta, pTransformCom);

		_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
		pController->Move_Dir(pTransformCom, vLook * fTimeDelta * m_fSlidePower, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.0f, 1.f);
		m_fSlidePower = 15.f - (15.f * pDee->IsAnimRatio());
		if (m_fSlidePower < 0.f)
			m_fSlidePower = 0.f;

		if (pDee->IsAnimFinished())
		{
			vLook.y = 0.f;
			HRESULT hr;
			// 별 아이템 떨굼
			_uint iItemCnt = 2;
			for (_uint i = 0; i < iItemCnt; ++i)
			{
				CAbility::ABILITYITEM_DESC AbilityItemDesc = {};
				if (i < iItemCnt / 2)
					AbilityItemDesc.fRotateDir = 1.f;															// 별 회전 방향 오른쪽
				else
					AbilityItemDesc.fRotateDir = -1.f;															// 별 회전 방향 왼쪽
				if (0 == i)
					AbilityItemDesc.fAngle = -15.f;																// 별의 진행 방향의 각도
				else
					AbilityItemDesc.fAngle = 15.f;																// 별의 진행 방향의 각도
				AbilityItemDesc.vDir = XMVector3Normalize(vLook) * 3.f;					// 별의 진행 방향
				AbilityItemDesc.vPosition = pTransformCom->Get_State(CTransform::STATE_POSITION) + vLook * 3.f;		// 별의 생성 위치
				AbilityItemDesc.eAbilityType = ABILITY_DEFAULT;
				hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), g_strLayerItem, TEXT("Prototype_GameObject_Ability"), &AbilityItemDesc);
				CHECK_FAILED(hr);
			}
			pDee->Change_State(CDeeDeeDee::STATE_SLIDINGEND, 60.f, false, false);
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_SLIDINGEND)
	{
		pController->FreeFall(pTransformCom, fTimeDelta, 6.0f);


		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, false);
		}
	}

}

void CDeeDeeDee_Slide_State::OnStateExit()
{
	m_fSlidePower = 15.f;
}

CDeeDeeDee_Slide_State* CDeeDeeDee_Slide_State::Create()
{
	CDeeDeeDee_Slide_State* pInstance = new CDeeDeeDee_Slide_State();
	return pInstance;
}

void CDeeDeeDee_Slide_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region INITIALZE STATE

CDeeDeeDee_Initialize_State::CDeeDeeDee_Initialize_State()
{
}

void CDeeDeeDee_Initialize_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Initialize_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CDeeDeeDee* pDee = static_cast<CDeeDeeDee*>(pGameObject);
	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	if (pKirby == nullptr)
		return;
	CTransform* pKirbyTransform = pKirby->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CDeeDeeDee::DDDDESC* DDDDesc = pDee->Get_Info();

	_float4 vKirbyPos = pKirbyTransform->Get_State(CTransform::STATE_POSITION);
	_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);

	m_fDelayTime += fTimeDelta;

	if (pDee->Get_State() == CDeeDeeDee::STATE_COMMAND)
	{
		// 샤우팅을 시작하기 위해.
		Make_TargetDir(DDDDesc, vPos, vKirbyPos);
		Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

		if (pDee->IsAnimFinished() && m_fDelayTime > 0.1f)
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, true);
			return;
		}
	}
}

void CDeeDeeDee_Initialize_State::OnStateExit()
{
	m_fDelayTime = 0.f;
}

CDeeDeeDee_Initialize_State* CDeeDeeDee_Initialize_State::Create()
{
	CDeeDeeDee_Initialize_State* pInstance = new CDeeDeeDee_Initialize_State();
	return pInstance;
}

void CDeeDeeDee_Initialize_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region JUMP STATE

CDeeDeeDee_Jump_State::CDeeDeeDee_Jump_State()
{
}

void CDeeDeeDee_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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


	if (pDee->Get_State() == CDeeDeeDee::STATE_JUMPSTART)
	{
		DESC(m_isBattle) = false;

		if (m_bTargetTrigger == true)
		{
			DESC(m_bJumpTarget) = DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE ? true : false;
			DESC(m_vJumpTargetDir) = vKirbyPos - vPos;
			DESC(m_vJumpTargetDir).y = 0.f;
			m_bTargetTrigger = false;
		}

		if (DESC(m_bJumpTarget) == true)
		{
			Make_TargetDir(DDDDesc, vPos, vKirbyPos);
			Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		}

		m_fJumpDelayTime += fTimeDelta;

		if (m_fJumpDelayTime > 0.3f)
		{
			pDee->Change_State(CDeeDeeDee::STATE_JUMP, 60.f, false, false);
			DESC(m_fJumpVelocity) = 80.f;
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_JUMP)
	{
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);

		if (DESC(m_bJumpTarget) == true)
		{
			pController->Move_Dir(pTransformCom, DESC(m_vJumpTargetDir) * fTimeDelta * 0.8f, fTimeDelta);
		}

		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_JUMPEND, 60.f, false, false);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_JUMPEND)
	{
		if (DESC(m_bJumpTarget) == true)
		{
			pController->Move_Dir(pTransformCom, DESC(m_vJumpTargetDir) * fTimeDelta * 0.8f, fTimeDelta);
		}
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		if (pController->Is_Terrain())
		{
			pDee->Change_State(CDeeDeeDee::STATE_LANDING, 60.f, false, false);
			m_pGameInstance->Setting_RadialBlur(vPos, 20.f, 80.f);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Make_Shake(2.f);

			Landing(fTimeDelta, pTransformCom);

			return;
		}

	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_LANDING)
	{

		DESC(m_isBattle) = true;

		pController->FreeFall(pTransformCom, fTimeDelta, 6.0f);
		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, false);
			return;
		}
	}

}

void CDeeDeeDee_Jump_State::OnStateExit()
{
	m_bTargetTrigger = true;
	m_fJumpDelayTime = 0.f;
}

CDeeDeeDee_Jump_State* CDeeDeeDee_Jump_State::Create()
{
	CDeeDeeDee_Jump_State* pInstance = new CDeeDeeDee_Jump_State();
	return pInstance;
}

void CDeeDeeDee_Jump_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region SIDEATTACK STATE

CDeeDeeDee_SideAttack_State::CDeeDeeDee_SideAttack_State()
{
}

void CDeeDeeDee_SideAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_SideAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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

	DESC(m_isBattle) = true;

	if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERSIDE)
	{
		Make_TargetDir(DDDDesc, vPos, vKirbyPos);
		Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);


		if (pDee->IsAnimFinished())
		{
			if ((vKirbyPos - vPos).Length() < 8.f || DESC(m_ePattern) == CDeeDeeDee::PATTERN_CENTER)
			{
				pDee->Change_State(CDeeDeeDee::STATE_HAMMERSIDECHARGE, 60.f, false, false);
				return;
			}
			else
			{
				pDee->Change_State(CDeeDeeDee::STATE_HAMMERSIDEWALK, 60.f, true, false);
				return;
			}
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERSIDECHARGE)
	{
		Make_TargetDir(DDDDesc, vPos, vKirbyPos);
		Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);


		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_HAMMERSIDEATTACK, 60.f, false, false);

			Hammer_Slash(fTimeDelta, pTransformCom);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERSIDEWALK)
	{
		Make_TargetDir(DDDDesc, vPos, vKirbyPos);
		Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);
		Moving_Logic(DDDDesc, pTransformCom, pController, fTimeDelta, 40.f, 4.f);

		if ((vKirbyPos - vPos).Length() < 8.f)
		{
			pDee->Change_State(CDeeDeeDee::STATE_HAMMERSIDECHARGE, 60.f, false, false);
			return;
		}
		else if (DESC(m_ePattern) == CDeeDeeDee::PATTERN_CENTER)
		{
			pDee->Change_State(CDeeDeeDee::STATE_WALK, 60.f, true, true);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERSIDEATTACK)
	{
		pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, true, false);
			return;
		}

	}



}

void CDeeDeeDee_SideAttack_State::OnStateExit()
{
}

CDeeDeeDee_SideAttack_State* CDeeDeeDee_SideAttack_State::Create()
{
	CDeeDeeDee_SideAttack_State* pInstance = new CDeeDeeDee_SideAttack_State();
	return pInstance;
}

void CDeeDeeDee_SideAttack_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region HAMMERATTACK STATE

CDeeDeeDee_HammerAttack_State::CDeeDeeDee_HammerAttack_State()
{
}

void CDeeDeeDee_HammerAttack_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_HammerAttack_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
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

	DESC(m_isBattle) = true;

	if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERATTACK)
	{
		if (m_bTargetTrigger == true)
		{
			m_bTarget = DESC(m_ePattern) == CDeeDeeDee::PATTERN_BATTLE ? true : false;
			m_bTargetTrigger = false;
		}


		if (m_bTarget)
		{
			Make_TargetDir(DDDDesc, vPos, vKirbyPos);
			Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
			pController->FreeFall(pTransformCom, fTimeDelta, 6.f);
		}
		else
		{
			Turn_Interpolate(DDDDesc, pTransformCom, fTimeDelta);
			pController->FreeFall(pTransformCom, fTimeDelta, 6.f);
		}


		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_HAMMERATTACKCHARGE, 60.f, false, false);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERATTACKCHARGE)
	{

		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_HAMMERATTACKHIT, 60.f, false, false);
			m_pGameInstance->Setting_RadialBlur(vPos, 20.f, 80.f);
			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Make_Shake(1.5f);

			Hammer_Slash(fTimeDelta, pTransformCom, true);

			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_HAMMERATTACKHIT)
	{



		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_WAIT, 60.f, false, true);
			return;
		}
	}
}

void CDeeDeeDee_HammerAttack_State::OnStateExit()
{
	m_bTargetTrigger = true;
}

CDeeDeeDee_HammerAttack_State* CDeeDeeDee_HammerAttack_State::Create()
{
	CDeeDeeDee_HammerAttack_State* pInstance = new CDeeDeeDee_HammerAttack_State();
	return pInstance;
}

void CDeeDeeDee_HammerAttack_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region DEATH STATE

CDeeDeeDee_Death_State::CDeeDeeDee_Death_State()
{
}

void CDeeDeeDee_Death_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDeeDeeDee_Death_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CDeeDeeDee* pDee = static_cast<CDeeDeeDee*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CDeeDeeDee::DDDDESC* DDDDesc = pDee->Get_Info();


	pController->FreeFall(pTransformCom, fTimeDelta);
	DESC(m_isBattle) = false;

	// 죽는 모션이다.

	if (pDee->Get_State() == CDeeDeeDee::STATE_DEATH)
	{


		if (pDee->IsAnimFinished())
		{
			pDee->Change_State(CDeeDeeDee::STATE_DEATHWAIT, 60.f, true, false);
			return;
		}
	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_DEATHWAIT)
	{



	}
	else if (pDee->Get_State() == CDeeDeeDee::STATE_DEATHWAITEND)
	{

	}

}

void CDeeDeeDee_Death_State::OnStateExit()
{
}

CDeeDeeDee_Death_State* CDeeDeeDee_Death_State::Create()
{
	CDeeDeeDee_Death_State* pInstance = new CDeeDeeDee_Death_State();
	return pInstance;
}

void CDeeDeeDee_Death_State::Free()
{
	__super::Free();
}

#pragma endregion
