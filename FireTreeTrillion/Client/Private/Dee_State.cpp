#include "stdafx.h"
#include "Dee_State.h"
#include "HungryDee.h"

#include "Kirby.h"

#pragma region DEE STATE
/// <summary>
/// 모든 이들의 기본이 되는 Dee_State
/// </summary>

CDee_State::CDee_State()
{
}

void CDee_State::Free()
{
	__super::Free();
}

void CDee_State::Setup_BaseInfo(BASE_INFO& _baseInfo, CGameObject* pGameObject)
{
	_baseInfo.pDee = static_cast<CWaddleDee*>(pGameObject);
	_baseInfo.pTransformCom = pGameObject->Get_TransformCom();
	_baseInfo.pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	_baseInfo.pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	_baseInfo.pKirbyTransformCom = _baseInfo.pKirby->Get_TransformCom();

	_baseInfo.vMyPos = (_float3)_baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
	_baseInfo.vKirbyPos = (_float3)_baseInfo.pKirbyTransformCom->Get_State(CTransform::STATE_POSITION);

	_baseInfo.fDistance = (_baseInfo.vMyPos - _baseInfo.vKirbyPos).Length();

}
void CDee_State::System_Tick(_float fTimeDelta)
{
	m_fDuration += fTimeDelta;
}

#pragma endregion

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CDee_Idle_State::CDee_Idle_State()
{
}

void CDee_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);


}

void CDee_Idle_State::OnStateExit()
{
}

CDee_Idle_State* CDee_Idle_State::Create()
{
	CDee_Idle_State* pInstance = new CDee_Idle_State();
	return pInstance;
}

void CDee_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region SIT STATE
CDee_Sit_State::CDee_Sit_State()
{
}

void CDee_Sit_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Sit_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);

}

void CDee_Sit_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Sit_State* CDee_Sit_State::Create()
{
	CDee_Sit_State* pInstance = new CDee_Sit_State();
	return pInstance;
}

void CDee_Sit_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region WALK STATE
CDee_Walk_State::CDee_Walk_State()
{
}

void CDee_Walk_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Walk_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);
	System_Tick(fTimeDelta);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);


	_float3 vMyPos = baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float3 vDestPos = baseInfo.pDee->Make_DestPos();
	vDestPos.y = vMyPos.y;
	_float3 vMyLook = baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK);


	//목표 지점과의 거리 차이를 구하여 속도 정하기
	_float fSpeed = (vDestPos - vMyPos).Length();
	fSpeed = (fSpeed < 3.f) ? fSpeed * 3.f : 3.f;
	fSpeed = clamp(fSpeed, .1f, 3.f);


	//목표 방향을 향해 회전한다.
	_float3 vDir = (vDestPos - vMyPos);
	baseInfo.pTransformCom->Look_At_Interpolate(vDestPos, fTimeDelta);


	baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fSpeed, fTimeDelta);

	if ((vDestPos - vMyPos).Length() < 1.f)
	{
		pair<DEE_ANIM, _bool> ToDo = baseInfo.pDee->Make_WhatToDo();
		DEE_ANIM eNextState = ToDo.first;
		baseInfo.pDee->Change_State(eNextState, 60.f, ToDo.second, true);
	}

}

void CDee_Walk_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Walk_State* CDee_Walk_State::Create()
{
	CDee_Walk_State* pInstance = new CDee_Walk_State();
	return pInstance;
}

void CDee_Walk_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region RUN STATE
CDee_Run_State::CDee_Run_State()
{
}

void CDee_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);
	System_Tick(fTimeDelta);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);


	_float3 vMyPos = baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float3 vDestPos = baseInfo.pDee->Make_DestPos();
	vDestPos.y = vMyPos.y;


	//목표 지점과의 거리 차이를 구하여 속도 정하기
	_float fSpeed = (vDestPos - vMyPos).Length();
	fSpeed = (fSpeed < 2.f) ? fSpeed * 2.5f : 5.f;
	fSpeed = clamp(fSpeed, .1f, 5.f);

	//목표 방향을 향해 회전한, 이동한다.
	_float3 vDir = (vDestPos - vMyPos);
	baseInfo.pTransformCom->Look_At_Interpolate(vDestPos, fTimeDelta);
	baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fSpeed, fTimeDelta);

	//목표 지점에 도달하면 뭐 할 지 정한다!!
	if ((vDestPos - vMyPos).Length() < 1.f)
	{
		pair<DEE_ANIM, _bool> ToDo = baseInfo.pDee->Make_WhatToDo();
		DEE_ANIM eNextState = ToDo.first;
		baseInfo.pDee->Change_State(eNextState, 60.f, ToDo.second, true);
	}

}

void CDee_Run_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Run_State* CDee_Run_State::Create()
{
	CDee_Run_State* pInstance = new CDee_Run_State();
	return pInstance;
}

void CDee_Run_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region EMOTION STATE
CDee_Emotion_State::CDee_Emotion_State()
{
}

void CDee_Emotion_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
	m_fInteractActionTime = 4.f;
}

void CDee_Emotion_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);


	if (baseInfo.pDee->Get_State() == DEEANIM_ANGER)
	{
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_ANGER);
	}
	else if (baseInfo.pDee->Get_State() == DEEANIM_TALK2)
	{
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE);
	}

}

void CDee_Emotion_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Emotion_State* CDee_Emotion_State::Create()
{
	CDee_Emotion_State* pInstance = new CDee_Emotion_State();
	return pInstance;
}

void CDee_Emotion_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region HUNGRY STATE
//*********************************
//			 HUNGRY STATE
//*********************************
CDee_Hungry_State::CDee_Hungry_State()
{
}

void CDee_Hungry_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

	m_fStartMoveDelay = CUtils::Make_RandomFloat(0.f, .5f);

}

void CDee_Hungry_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);
	System_Tick(fTimeDelta);

	CHungryDee* pHungryDee = static_cast<CHungryDee*>(pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);

	//목표 지점과의 거리 차이를 구하여 속도 정하기
	_float fSpeed = (XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION))).Length();
	fSpeed = (fSpeed < 1.f) ? fSpeed * 3.f : 4.f;

	switch (baseInfo.pDee->Get_State())
	{

#pragma region 대기
		//기본 대기 상태
	case DEESHOPANIM_GUESTNORMAL:
	{
		baseInfo.pTransformCom->Look_At_Interpolate(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, 0.f, 1.f, 0.f }, fTimeDelta * 4.f);

		//내가 기다려야 할 위치를 넘어서면 이동
		if (.5f < _float3{ XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION)) }.Length())
		{
			m_fStartMoveDelay -= fTimeDelta;

			if (m_fStartMoveDelay < 0.f)
			{
				pHungryDee->Change_State((DEE_ANIM)DEESHOPANIM_WALK, 60.f, true, true);
			}
		}

		if (pHungryDee->Get_WaitingTime() < 20.f)
		{
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_SADNESS);
		}

		if (pHungryDee->Get_WaitingTime() < 15.f)
		{
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_ANGER);
			pHungryDee->Change_State((DEE_ANIM)DEESHOPANIM_GUESTANGER, 60.f, true, true);
		}
	}
	break;
	case DEESHOPANIM_GUESTANGER:
	{
		baseInfo.pTransformCom->Look_At_Interpolate(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, 0.f, 1.f, 0.f }, fTimeDelta * 4.f);

		//내가 기다려야 할 위치를 넘어서면 이동
		if (.5f < _float3{ XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION)) }.Length())
		{
			m_fStartMoveDelay -= fTimeDelta;

			if (m_fStartMoveDelay < 0.f)
			{
				pHungryDee->Change_State((DEE_ANIM)DEESHOPANIM_WALK, 60.f, true, true);
			}
		}

		if (15.f < pHungryDee->Get_WaitingTime())
		{
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_SADNESS);
			pHungryDee->Change_State((DEE_ANIM)DEESHOPANIM_GUESTNORMAL, 60.f, true, true);
		}

	}
	break;
	case DEESHOPANIM_ORDERNORMAL:
	{
		baseInfo.pTransformCom->Look_At_Interpolate(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION) + _float4{ 0.f, 0.f, 1.f, 0.f }, fTimeDelta * 4.f);

		if (pHungryDee->Get_WaitingTime() < 20.f)
		{
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_SADNESS);
		}

		if (pHungryDee->Get_WaitingTime() < 15.f)
		{
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_ANGER);
		}
	}
	break;

#pragma endregion

#pragma region 이동 처리
	//기본 워킹 상태
	case DEESHOPANIM_WALK:
	{
		_float3 vDir = pHungryDee->Get_DestWaitingPos() - baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
		vDir.Normalize();
		baseInfo.pTransformCom->Look_At_Interpolate(pHungryDee->Get_DestWaitingPos(), fTimeDelta * 2.f);


		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fSpeed, fTimeDelta);

		//다다르면 다시 웨이팅
		if (_float3{ XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION)) }.Length() < .5f)
		{
			pHungryDee->Change_State(pHungryDee->IsFrontWaiting() ? (DEE_ANIM)DEESHOPANIM_ORDERNORMAL : (DEE_ANIM)DEESHOPANIM_GUESTNORMAL, CUtils::Make_RandomFloat(50.f, 60.f), true, true);

			CUtils::Make_RandomInt(0, 2) == 2 ?
				baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE) :
				baseInfo.pDee->Set_DeeEyeState(DEEEYE_IDLE);
		}
	}
	break;
	case DEESHOPANIM_RUN:
	{
		if (0.f < m_fStartMoveDelay)
		{
			m_fStartMoveDelay -= fTimeDelta;
			if (m_fStartMoveDelay < 0.f)
				m_fStartMoveDelay = 0.f;
			break;
		}

		_float3 vDir = pHungryDee->Get_DestWaitingPos() - baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
		vDir.Normalize();
		baseInfo.pTransformCom->Look_At_Interpolate(pHungryDee->Get_DestWaitingPos(), fTimeDelta * 2.f);

		_float fDist = (XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION))).Length();
		fDist = (fDist < 1.f) ? fDist * 3.f : 4.f;

		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fSpeed * 2.f, fTimeDelta);

		//다다르면 다시 웨이팅
		if (_float3{ XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION)) }.Length() < .5f)
		{
			pHungryDee->Change_State(pHungryDee->IsFrontWaiting() ? (DEE_ANIM)DEESHOPANIM_ORDERNORMAL : (DEE_ANIM)DEESHOPANIM_GUESTNORMAL, CUtils::Make_RandomFloat(50.f, 60.f), true, true);

			CUtils::Make_RandomInt(0, 2) == 2 ?
				baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE) :
				baseInfo.pDee->Set_DeeEyeState(DEEEYE_IDLE);
		}
	}
	break;
#pragma endregion

#pragma region 맞춤
	case DEESHOPANIM_CLERKCORRECT:
	{
		if (baseInfo.pDee->IsAnimFinished())
		{
			//pHungryDee->Set_RenderPartObj(true);
			pHungryDee->Change_State((DEE_ANIM)DEESHOPANIM_CORRECTMOVE, 60.f, true, true);
		}
	}
	break;
#pragma endregion

#pragma region 맞춘 뒤 움직임
	case DEESHOPANIM_CORRECTMOVE:
	{

		if (.1f < m_fDuration)
			pHungryDee->Set_RenderPartObj(true);

		_float3 vDir = pHungryDee->Get_DestWaitingPos() - baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
		vDir.Normalize();
		baseInfo.pTransformCom->Look_At_Interpolate(pHungryDee->Get_DestWaitingPos(), fTimeDelta * 2.f);

		_float fDist = (XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION))).Length();
		fDist = (fDist < 1.f) ? fDist * 3.f : 4.f;

		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fDist, fTimeDelta);
	}
	break;
#pragma endregion

#pragma region 틀림
	case DEESHOPANIM_INCORRECT:
	{
		if (baseInfo.pDee->IsAnimFinished())
			pHungryDee->Change_State((DEE_ANIM)DEESHOPANIM_INCORRECTMOVE, 60.f, true, true);
	}
	break;
#pragma endregion

#pragma region 틀린 후 움직임
	case DEESHOPANIM_INCORRECTMOVE:
	{
		_float3 vDir = pHungryDee->Get_DestWaitingPos() - baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION);
		vDir.Normalize();
		baseInfo.pTransformCom->Look_At_Interpolate(pHungryDee->Get_DestWaitingPos(), fTimeDelta * 2.f);

		_float fDist = (XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION))).Length();
		fDist = (fDist < 1.f) ? fDist * 3.f : 4.f;

		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fDist, fTimeDelta);
	}
	break;
#pragma endregion

	default:
		break;
	}
}

void CDee_Hungry_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Hungry_State* CDee_Hungry_State::Create()
{
	CDee_Hungry_State* pInstance = new CDee_Hungry_State();
	return pInstance;
}

void CDee_Hungry_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region STUN STATE
//*********************************
//			 STUN STATE
//*********************************
CDee_FlyStun_State::CDee_FlyStun_State()
{
}

void CDee_FlyStun_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_FlyStun_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	_float3 vDamegeDir = baseInfo.pDee->Get_DamegeDir();

	if (baseInfo.pDee->Get_PhyXState() == PO_NORMAL)
	{
		// 이제 날아가는 것을 구현해보자.
		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, vDamegeDir * fTimeDelta * 3.f, fTimeDelta);

		if (baseInfo.pTransformCom->Get_State(CTransform::STATE_RIGHT) == _float4::Zero)
			baseInfo.pTransformCom->Set_Scaled({ 1.f, 1.f, 1.f });

		baseInfo.pTransformCom->Turn(_float4{ baseInfo.pTransformCom->Get_State(CTransform::STATE_RIGHT) }, fTimeDelta);

		// 점프되는 체공시간을 구현해보자.
		_float fDamageJumpPower = baseInfo.pDee->Get_DamageJumpPower();
		baseInfo.pController->Jump(baseInfo.pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 4.f;
		baseInfo.pDee->Set_DamageJumpPower(fDamageJumpPower);


		if (baseInfo.pController->Is_Terrain())
		{
			if (m_iBounceCnt != 0)
			{
				m_iBounceCnt--;

				//위로 한번 더 튕긴다.
				fDamageJumpPower *= -.6f;

				_float fDamageDirLength = vDamegeDir.Length() * .6f;
				_float3 vNewDamageDir = CUtils::Make_Random_Vector(fDamageDirLength);
				baseInfo.pDee->Set_DamageMoving(vNewDamageDir, fDamageJumpPower);
			}
			else
			{
				pair<DEE_ANIM, _bool> ToDo = baseInfo.pDee->Make_WhatToDo();
				baseInfo.pDee->Change_State(ToDo.first, 60.f, ToDo.second, true);
				m_iBounceCnt = 1;
			}
		}
	}
	// 날아가는 도중이다.  1초에 360도 회전하며, 30의 거리로 날아간다.
	else if (baseInfo.pDee->Get_PhyXState() == PO_FLYAWAY)
	{
		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, vDamegeDir * fTimeDelta * 30.f, fTimeDelta);
		baseInfo.pTransformCom->Turn(baseInfo.pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta, 360.f);

		if (1.f > baseInfo.pController->Compute_Wall(vDamegeDir))
		{
			baseInfo.pDee->Set_PhyXState(PO_FLYDEADAWAY);
			baseInfo.pDee->Set_DamageMoving(-1.f * vDamegeDir, 10.f);
		}
	}
	else if (baseInfo.pDee->Get_PhyXState() == PO_FLYDEADAWAY)
	{

		baseInfo.pController->Move_Dir(baseInfo.pTransformCom, vDamegeDir * fTimeDelta * 3.f, fTimeDelta);
		//baseInfo.pTransformCom->Turn(_float4{ baseInfo.pTransformCom->Get_State(CTransform::STATE_RIGHT) }, fTimeDelta);


		_float fDamageJumpPower = baseInfo.pDee->Get_DamageJumpPower();
		baseInfo.pController->Jump(baseInfo.pTransformCom, fDamageJumpPower, fTimeDelta);
		fDamageJumpPower -= GRAVITY * fTimeDelta * 4.f;
		baseInfo.pDee->Set_DamageJumpPower(fDamageJumpPower);


		if (baseInfo.pController->Is_Terrain())
		{
			if (m_iBounceCnt != 0)
			{
				m_iBounceCnt--;

				//위로 한번 더 튕긴다.
				fDamageJumpPower *= -.6f;

				_float fDamageDirLength = vDamegeDir.Length() * .6f;
				_float3 vNewDamageDir = CUtils::Make_Random_Vector(fDamageDirLength);
				baseInfo.pDee->Set_DamageMoving(vNewDamageDir, fDamageJumpPower);
			}
			else
			{
				pair<DEE_ANIM, _bool> ToDo = baseInfo.pDee->Make_WhatToDo();
				baseInfo.pDee->Change_State(ToDo.first, 60.f, ToDo.second, true);

				_float3 vLook = baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK);
				vLook.y = 0.f;
				vLook.Normalize();
				baseInfo.pTransformCom->Look_At_Axis(vLook);
				m_iBounceCnt = 1;
				baseInfo.pDee->Set_PhyXState(PO_NORMAL);
			}
		}
	}

}

void CDee_FlyStun_State::OnStateExit()
{
	m_iBounceCnt = 1;
}

CDee_FlyStun_State* CDee_FlyStun_State::Create()
{
	CDee_FlyStun_State* pInstance = new CDee_FlyStun_State();
	return pInstance;
}

void CDee_FlyStun_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region NPC STATE
CDee_NPC_State::CDee_NPC_State()
{
}

void CDee_NPC_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_NPC_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);

	//가까이 있으면 반가워해준다~
	//용건 있을 때 웃는 걸로 변경해야해
	if (baseInfo.fDistance < 10.f && !baseInfo.pDee->GetHiToKirby())
	{
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE);
		baseInfo.pDee->SetHiToKirby(true);
		baseInfo.pDee->Change_State(DEEANIM_CLERKWAVEHAND, 60.f, false, true);
	}


	//참 가까이 접근했고, 버튼 누르면 대화 시작
	//이것도 용건 있을 때 웃는 걸로 변경해야해
	if (baseInfo.pDee->IsCloseToKirby() && m_pGameInstance->Get_KeyState(DIK_C, KEY_DOWN))
	{
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE);
		baseInfo.pDee->Change_State(DEEANIM_CLERKTALK, 60.f, false, true);
	}

	if (baseInfo.pDee->IsAnimFinished())
	{
		//인사하는 거였으면 눈 바꾸고 다시 idle로 돌아가
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_IDLE);
		baseInfo.pDee->Change_State(DEEANIM_LOOKAROUND, 60.f, true, true);
	}
}

void CDee_NPC_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_NPC_State* CDee_NPC_State::Create()
{
	CDee_NPC_State* pInstance = new CDee_NPC_State();
	return pInstance;
}

void CDee_NPC_State::Free()
{
	__super::Free();
}

#pragma endregion

#pragma region SLEEP STATE
CDee_Sleep_State::CDee_Sleep_State()
{
}

void CDee_Sleep_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Sleep_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);
	baseInfo.pDee->Set_DeeEyeState(DEEEYE_CLOSE);

}

void CDee_Sleep_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Sleep_State* CDee_Sleep_State::Create()
{
	CDee_Sleep_State* pInstance = new CDee_Sleep_State();
	return pInstance;
}

void CDee_Sleep_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region INTERACT STATE

CDee_Interact_State::CDee_Interact_State()
{
}

void CDee_Interact_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Interact_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);
	System_Tick(fTimeDelta);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);

	//상호작용 스테이트마다 조금식 다르게 하자~
	switch (baseInfo.pDee->Get_State())
	{
	case DEEANIM_MOVEFALL:
	{
		if (m_fDuration < .5f)
		{
			_float fSpeed = 15.f * (.5f - m_fDuration);
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE);
			baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fSpeed, fTimeDelta);
		}

		if (abs(m_fDuration - .6f) < fTimeDelta * 2.f)
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_CLOSE);

		if (abs(m_fDuration - 1.f) < fTimeDelta * 2.f)
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_SADNESS);
	}
	break;
	case DEEANIM_WATERING:
	{
		if (abs(m_fDuration - 1.f) < fTimeDelta * 2.f)
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_SMILE);
	}
	break;
	case DEEANIM_CHOOSE_START:
	{
		if (baseInfo.pDee->IsAnimFinished())
		{
			//인사하는 거였으면 눈 바꾸고 다시 idle로 돌아가
			baseInfo.pDee->Set_DeeEyeState(DEEEYE_IDLE);
			baseInfo.pDee->Change_State(DEEANIM_CHOOSE_WAIT, 60.f, false, true);
			return;
		}
	}
	break;
	default:
		break;
	}


	if (baseInfo.pDee->IsAnimFinished())
	{
		//인사하는 거였으면 눈 바꾸고 다시 idle로 돌아가
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_IDLE);
		baseInfo.pDee->Change_State(DEEANIM_WALK, 60.f, true, true);
	}

}

void CDee_Interact_State::OnStateExit()
{
	m_fDuration = 0.f;
}

CDee_Interact_State* CDee_Interact_State::Create()
{
	CDee_Interact_State* pInstance = new CDee_Interact_State();
	return pInstance;
}

void CDee_Interact_State::Free()
{
	__super::Free();
}

#pragma endregion
