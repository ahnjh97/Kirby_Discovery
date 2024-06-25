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

#pragma region MOVE STATE
//*********************************
//			 WALK STATE
//*********************************
CDee_Move_State::CDee_Move_State()
{
}

void CDee_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);
	baseInfo.pTransformCom->Turn({ 0.f, 1.f, 0.f, 1.f }, fTimeDelta * .3f);

	//_float fDist = (XZVec(pHungryDee->Get_DestWaitingPos()) - XZVec(baseInfo.pTransformCom->Get_State(CTransform::STATE_POSITION))).Length();
	//fDist = (fDist < 1.f) ? fDist * 3.f : 4.f;


	_float fSpeed =
		baseInfo.pDee->Get_State() == DEEANIM_WALK ? 3.f : 1.f;

	baseInfo.pController->Move_Dir(baseInfo.pTransformCom, baseInfo.pTransformCom->Get_State(CTransform::STATE_LOOK) * fTimeDelta * fSpeed, fTimeDelta);
}

void CDee_Move_State::OnStateExit()
{
}

CDee_Move_State* CDee_Move_State::Create()
{
	CDee_Move_State* pInstance = new CDee_Move_State();
	return pInstance;
}

void CDee_Move_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region EMOTION STATE
//*********************************
//			 EMOTION STATE
//*********************************
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


	if (baseInfo.pDee->IsAnimFinished())
	{
		//인사하는 거였으면 눈 바꾸고 다시 idle로 돌아가
		baseInfo.pDee->Set_DeeEyeState(DEEEYE_IDLE);
		baseInfo.pDee->Change_State(DEEANIM_LOOKAROUND, 60.f, true, true);
	}

}

void CDee_Emotion_State::OnStateExit()
{
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
CDee_Stun_State::CDee_Stun_State()
{
}

void CDee_Stun_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CDee_Stun_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	BASE_INFO baseInfo{};
	Setup_BaseInfo(baseInfo, pGameObject);

	baseInfo.pController->FreeFall(baseInfo.pTransformCom, fTimeDelta);
}

void CDee_Stun_State::OnStateExit()
{
}

CDee_Stun_State* CDee_Stun_State::Create()
{
	CDee_Stun_State* pInstance = new CDee_Stun_State();
	return pInstance;
}

void CDee_Stun_State::Free()
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
}

void CDee_Sleep_State::OnStateExit()
{
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
