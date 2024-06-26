#include "stdafx.h"
#include "KirbyVacuum_State.h"
#include "Kirby_State_Function.h"

#pragma region SPIT STATE

CKirbyVacuum_Spit_State::CKirbyVacuum_Spit_State()
{
}

void CKirbyVacuum_Spit_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyVacuum_Spit_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// 뱉는 로직이다.
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	// 커비는 모델이 여러개기 때문에, 항상 Default가 Idle로 위치하도록 보정해주어야 한다.
	pKirby->DefaultIdle();
	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	// 뱉는 시간을 측정한다.
	m_fSpitTime += fTimeDelta;

	if (pKirby->Get_State() == CKirby::STATE_SPIT)
	{
		if (m_fSpitTime > 0.05f && m_bSpitTrigger == true)
		{
			// 날려 보낸다. 날려보내는 순간 나의 관할이 아니기 때문에 그냥 보내버린다.
			// 또한 보내기전에 마지막으로 Fly로 만들어준다. 또한 방향을 여기서 정해준다.
			// 이곳에선 상대가 컨트롤러든 뭐시기든 아무런 상관이 없다. 내가 정해준 방향을 사용하여 객체의 움직임 구현대로 나가는것이기 때문이다. 
			if (DESC(m_pObject) != nullptr)
			{
				CTransform* pObjectTransform = DESC(m_pObject)->Get_TransformCom();
				pObjectTransform->Set_Scaled(1.f, 1.f, 1.f);

				_float4 vTargetPos = Spit_Target_Object(pKirby);

				if (vTargetPos == _float4(0.f, 0.f, 0.f, 0.f))
				{
					DESC(m_pObject)->Set_DamageMoving(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 1.f);

					_vector vNewUp = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
					_vector vNewLook = XMVector3Cross(vNewUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));
					_vector vNewRight = XMVector3Cross(vNewUp, vNewLook);
					pObjectTransform->Set_State(CTransform::STATE_UP, vNewUp);
					pObjectTransform->Set_State(CTransform::STATE_RIGHT, vNewRight);
					pObjectTransform->Set_State(CTransform::STATE_LOOK, vNewLook);

				}
				else
				{
					_float4 vObjectPos = pObjectTransform->Get_State(CTransform::STATE_POSITION);
					_vector vObjectToTargetDir = XMVector3Normalize(vTargetPos - vObjectPos);
					DESC(m_pObject)->Set_DamageMoving(vObjectToTargetDir, 1.f);

					vObjectToTargetDir.m128_f32[1] = 0.f;
					DESC(m_vTargetDir) = XMVector3Normalize(vObjectToTargetDir);

					_vector vNewUp = vObjectToTargetDir;
					_vector vNewLook = XMVector3Cross(vNewUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));
					_vector vNewRight = XMVector3Cross(vNewUp, vNewLook);
					pObjectTransform->Set_State(CTransform::STATE_UP, vNewUp);
					pObjectTransform->Set_State(CTransform::STATE_RIGHT, vNewRight);
					pObjectTransform->Set_State(CTransform::STATE_LOOK, vNewLook);


				}

				DESC(m_pObject)->Set_PhyXState(PO_FLYAWAY);
				Safe_Release(DESC(m_pObject));
				DESC(m_pObject) = nullptr;
			}
			m_bSpitTrigger = false;
		}


		// 뱉는 모션의 마지막이다.
		if (pKirby->isAnimFinish())
		{
			// 이제 먹은 상태가 아니며, 표정이 전부 원래대로 들어온다.
			DESC(m_isEat) = false;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			DESC(m_eMouthState) = CKirby::MOUTH_IDLE;
			DESC(m_eTemporaryEatType) = ABILITY_END;

			// 애님 끝났는데 땅을 밟았을 경우
			if (pController->Is_Terrain() == true)
			{
				pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
				return;
			}
			// 애님 끝났는데 땅을 밟지않고 공중에 있다고 판단 될 경우
			else
			{
				pKirby->Change_State(CKirby::STATE_FALL, 50.f, false, true, CKirby::BODY_DEFAULT);
				return;
			}
		}
	}
	else if (pKirby->Get_State() == CKirby::STATE_SPITDEFORM)
	{
		_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);
		if (m_bSpitTrigger == true)
		{
			// 자동차 등을 뱉는다. 이쪽에서.
			_float4 vPos = pTransformCom->Get_State(CTransform::STATE_POSITION);

			pController->Set_Position(pTransformCom, vPos + _float4(0.f, 4.f, 0.f, 0.f));

			// Car Spit
			// 추후, 전 상태를 받고 있다가 뱉을 것 정해주면 될것같다.
			CGameObject::GAMEOBJECT_DESC ObjDesc{};
			ObjDesc.fSpeedPerSec = 5.f;
			ObjDesc.fRotationPerSec = ToRadian(90.f);
			_float4x4 InitMat = _float4x4::Identity;
			InitMat = pTransformCom->Get_WorldFloat4x4();
			CUtils::Set_State_Matrix(InitMat, CUtils::STATE_POSITION, vPos + _float4(0.f, 3.f, 0.f, 0.f));
			//InitMat.Translation((_float3)vPos + (_float3)(0.f, 3.f, 0.f));
			ObjDesc.matWorld = InitMat;
			if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Deform"), TEXT("Prototype_GameObject_Car"), &ObjDesc)))
				return;

			m_bSpitTrigger = false;
		}

		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
		pController->Move_Dir(pTransformCom, vLook * 10.f * fTimeDelta, fTimeDelta);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		if (pController->Is_Terrain() == true)
		{
			pKirby->Change_State(CKirby::STATE_LANDINGSMALL, 50.f, false, false, CKirby::BODY_DEFAULT);
			return;
		}
	}
}

void CKirbyVacuum_Spit_State::OnStateExit()
{
	m_bSpitTrigger = true;
	m_fSpitTime = 0.f;
}

CKirbyVacuum_Spit_State* CKirbyVacuum_Spit_State::Create()
{
	CKirbyVacuum_Spit_State* pInstance = new CKirbyVacuum_Spit_State();
	return pInstance;
}

void CKirbyVacuum_Spit_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region Vacuum STATE

CKirbyVacuum_Vacuum_State::CKirbyVacuum_Vacuum_State()
{
}

void CKirbyVacuum_Vacuum_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyVacuum_Vacuum_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
	pKirby->DefaultIdle();


	if (pKirby->Get_State() == CKirby::STATE_INHALE ||
			pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART ||
			pKirby->Get_State() == CKirby::STATE_SUPERINHALE ||
			pKirby->Get_State() == CKirby::STATE_INHALEFALL ||
			pKirby->Get_State() == CKirby::STATE_INHALELANDING)
	{
		// 흡수에 성공했을땐 return 으로 바로 빠져나간다.
		if (Vacuum_Object(pKirby, fTimeDelta) == true)
			return;
	}


	// Vacuum 시작. 이 애니메이션이 무조건 발생한다.
	if (pKirby->Get_State() == CKirby::STATE_INHALESTART)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
		{
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_INHALE, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// 빨아들이는 곳 X를 유지하여야 한다.
	else if (pKirby->Get_State() == CKirby::STATE_INHALE)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
		DESC(m_fVacuumTime) += fTimeDelta;
		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
		{
			if (DESC(m_fVacuumTime) > 1.2f)
			{
				pKirby->Change_State(CKirby::STATE_SUPERINHALESTART, 60.f, false, true, CKirby::BODY_VACUUM);
				return;
			}
		}
		// 최소 0.2초간은 흡입한다.
		else if (DESC(m_fVacuumTime) > 0.2f)
		{
			// X키에서 손을 떼었고, 0.2초가 지나서 상황이 종료된다.
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			return;
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			pKirby->Change_State(CKirby::STATE_INHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALESTART)
	{
		DESC(m_eEyeState) = CKirby::EYE_ANGER;
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		// 조이스틱 만졌을땐 바로 슈퍼로 넘어감.
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			pKirby->Change_State(CKirby::STATE_SUPERINHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}

		// 애님이 자연스럽게 끝났으면 슈퍼빨기
		if (pKirby->isAnimFinish())
		{
			DESC(m_eEyeState) = CKirby::EYE_CLOSE;
			pKirby->Change_State(CKirby::STATE_SUPERINHALE, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			return;
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALE)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			pKirby->Change_State(CKirby::STATE_SUPERINHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			return;
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	else if (pKirby->Get_State() == CKirby::STATE_INHALEEND)
	{
		Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (pKirby->isAnimFinish())
		{
			pKirby->Change_State(CKirby::STATE_IDLE, 60.f, true, true, CKirby::BODY_DEFAULT);
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			return;
		}

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
		pKirby->Delete_AllEffect();

	}
	else if (pKirby->Get_State() == CKirby::STATE_INHALEFALL)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		// 바닥에 닿았다면
		//pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
		pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
		DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);

		if (pController->Is_Terrain())
		{
			pKirby->Change_State(CKirby::STATE_INHALELANDING, 50.f, false, true, CKirby::BODY_VACUUM);
			return;
		}

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			return;
		}

	}
	else if (pKirby->Get_State() == CKirby::STATE_INHALELANDING)
	{
		if (JoyStick_controller(Kirbydesc, pCamera) == true)
		{
			Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
			Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
		}
		else
			Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);

		//뽀잉 끝나면
		if (pKirby->isAnimFinish())
		{
			if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS))
			{
				if (DESC(m_fVacuumTime) > 1.2f)
				{
					pKirby->Change_State(CKirby::STATE_SUPERINHALESTART, 60.f, false, true, CKirby::BODY_VACUUM);
					return;
				}
				else
				{
					pKirby->Change_State(CKirby::STATE_INHALE, 50.f, true, true, CKirby::BODY_VACUUM);
					return;
				}
			}
			else
			{
				DESC(m_fVacuumTime) = 0.f;
				DESC(m_eEyeState) = CKirby::EYE_IDLE;
				pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
				return;
			}
		}
	}
}

void CKirbyVacuum_Vacuum_State::OnStateExit()
{
}

CKirbyVacuum_Vacuum_State* CKirbyVacuum_Vacuum_State::Create()
{
	CKirbyVacuum_Vacuum_State* pInstance = new CKirbyVacuum_Vacuum_State();
	return pInstance;
}

void CKirbyVacuum_Vacuum_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region VacuumWalk STATE

CKirbyVacuum_VacuumWalk_State::CKirbyVacuum_VacuumWalk_State()
{
}

void CKirbyVacuum_VacuumWalk_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyVacuum_VacuumWalk_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
	pKirby->DefaultIdle();


	// 흡수에 성공했을땐 return 으로 바로 빠져나간다.
	if (Vacuum_Object(pKirby, fTimeDelta) == true)
		return;


	if (pController->Compute_Height() > 2.f)
	{
		pKirby->Change_State(CKirby::STATE_INHALEFALL, 50.f, true, true, CKirby::BODY_VACUUM);
		return;
	}

	// 빨아들이면서 걷기
	if (pKirby->Get_State() == CKirby::STATE_INHALEWALK)
	{
		DESC(m_fVacuumTime) += fTimeDelta;
		Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false && DESC(m_fVacuumTime) > 0.2f)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			return;
		}

		if (DESC(m_fVacuumTime) > 1.2f)
		{
			pKirby->Change_State(CKirby::STATE_SUPERINHALEWALK, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == false)
		{
			pKirby->Change_State(CKirby::STATE_INHALE, 50.f, true, true, CKirby::BODY_VACUUM);
			return;
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
	// 슈퍼 빨아들이면서 걷기
	else if (pKirby->Get_State() == CKirby::STATE_SUPERINHALEWALK)
	{
		DESC(m_eEyeState) = CKirby::EYE_CLOSE;

		Inhale_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
		Inhale_Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);

		if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false)
		{
			DESC(m_fVacuumTime) = 0.f;
			DESC(m_eEyeState) = CKirby::EYE_IDLE;
			pKirby->Change_State(CKirby::STATE_INHALEEND, 100.f, false, false, CKirby::BODY_VACUUM);
			return;
		}

		if (JoyStick_controller(Kirbydesc, pCamera) == false)
		{
			pKirby->Change_State(CKirby::STATE_SUPERINHALESTART, 60.f, false, true, CKirby::BODY_VACUUM);
			return;
		}
		pController->FreeFall(pTransformCom, fTimeDelta, Kirbydesc->m_fGravityOffset);
	}
}

void CKirbyVacuum_VacuumWalk_State::OnStateExit()
{
}

CKirbyVacuum_VacuumWalk_State* CKirbyVacuum_VacuumWalk_State::Create()
{
	CKirbyVacuum_VacuumWalk_State* pInstance = new CKirbyVacuum_VacuumWalk_State();
	return pInstance;
}

void CKirbyVacuum_VacuumWalk_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region Vacuuming STATE

CKirbyVacuum_Vacuuming_State::CKirbyVacuum_Vacuuming_State()
{

}

void CKirbyVacuum_Vacuuming_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyVacuum_Vacuuming_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	// 흡수하는 로직으로, 이곳에서 모든 흡수처리를 담당한다.
	// 강제적으로 상호작용할 기물을 나에게 끌어당기는 로직이고, 실제 Body충돌처리로 커비 스테이트가 바뀌고, 물체의 스테이트도 바뀐다.
	CKirby* pKirby = static_cast<CKirby*>(pGameObject);
	CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
	CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	DESC(m_eEyeState) = CKirby::EYE_CLOSE;
	CTransform* pObjectTransform = DESC(m_pObject)->Get_TransformCom();
	_vector vObjectPos = pObjectTransform->Get_State_Vector(CTransform::STATE_POSITION);
	_float4 vDistance = vPos - vObjectPos;
	_float vCurDistance = XMVectorGetX(XMVector3Length(vDistance));
	_float4 vObjectDir = XMVector3Normalize(vDistance);

	if (m_bKirbyTurnDirTrigger == true)
	{
		_float4 vLookDir = vObjectDir;
		vLookDir.y = 0.f;
		m_vLookDir = -1.f * vLookDir;
		DESC(m_vTargetDir) = m_vLookDir;
		m_bKirbyTurnDirTrigger = false;
	}

	Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

	// 머금기 흡수
	if (DESC(m_bisDeforming) == true)
	{
		_float4 vDeformLook = pObjectTransform->Get_State(CTransform::STATE_LOOK);
		_float4 vLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

		if (Turn_Deform(vDeformLook, vLook, pObjectTransform, fTimeDelta) == false)
		{
			pObjectTransform->Look_At_Axis(vDeformLook);
		}
		else
		{
			pObjectTransform->Look_At_Axis(vDeformLook);
			// 상대의 컨트롤러를 탐색한다.
			CCharacterController* pObjectController = static_cast<CCharacterController*>(DESC(m_pObject)->Get_Component(TEXT("Com_Controller")));

			pObjectController->Move_Dir(pObjectTransform, vObjectDir * pow(m_fDeformObjectSpeed, 3.f) * fTimeDelta, fTimeDelta);

			// 그 외, 스케일 변화를 준다.
			_float fScaleinverse = 1.f - ((DESC(m_fObjectDistance) - vCurDistance) / DESC(m_fObjectDistance) * 0.3f);
			_float3 vObjectScale = pObjectTransform->Get_Scaled();
			pObjectTransform->Set_Scaled(DESC(m_vObjectScale).x * fScaleinverse, DESC(m_vObjectScale).y * fScaleinverse, DESC(m_vObjectScale).z * fScaleinverse);

			// 물체가 나에게 오는 시간을 타임델타 기반으로 가산시켜준다.
			m_fDeformObjectSpeed += fTimeDelta * 20.f;
		}
	}
	// 일반 오브젝트 흡수
	else
	{
		// 상대의 컨트롤러를 탐색한다.
		CCharacterController* pObjectController = static_cast<CCharacterController*>(DESC(m_pObject)->Get_Component(TEXT("Com_Controller")));

		// 만약 이 과정에서, Controller 가 nullptr 일 경우 그것은 컨트롤러가 아닌것이다. 트랜스폼을 직접 움직여주어야 한다.
		if (pObjectController == nullptr)
		{
			_float4 vPos = pObjectTransform->Get_State(CTransform::STATE_POSITION);
			pObjectTransform->Set_State(CTransform::STATE_POSITION, vPos + vObjectDir * m_fVacuumObjectSpeed * fTimeDelta);
		}
		// 만약 이 과정에서, Controller 가 nullptr이 아닐 경우 그것은 컨트롤러가 맞고, Move_Dir로 나에게 당겨준다.
		else
		{
			pObjectController->Move_Dir(pObjectTransform, vObjectDir * m_fVacuumObjectSpeed * fTimeDelta, fTimeDelta);
		}

		// 그 외, 스케일 변화를 준다.
		_float fScaleinverse = 1.f - ((DESC(m_fObjectDistance) - vCurDistance) / DESC(m_fObjectDistance) * 0.3f);
		_float3 vObjectScale = pObjectTransform->Get_Scaled();
		pObjectTransform->Set_Scaled(DESC(m_vObjectScale).x * fScaleinverse, DESC(m_vObjectScale).y * fScaleinverse, DESC(m_vObjectScale).z * fScaleinverse);

		// 물체가 나에게 오는 시간을 타임델타 기반으로 가산시켜준다.
		m_fVacuumObjectSpeed += fTimeDelta * 150.f;
	}


}

void CKirbyVacuum_Vacuuming_State::OnStateExit()
{
	m_fVacuumObjectSpeed = 2.f;
	m_fDeformObjectSpeed = 0.f;
	m_bDeformVacuumStart = false;

	m_bKirbyTurnDirTrigger = true;
	m_vLookDir = { 0.f, 0.f, 0.f, 0.f };
}

_bool CKirbyVacuum_Vacuuming_State::Turn_Deform(_float4& InterpolateDir, const _float4& TargetDir, CTransform* pDeformTransformCom, _float fTimeDelta)
{
	if (InterpolateDir == TargetDir)
		return true;

	_float fInterpolate = fTimeDelta * 6.f;
	_vector vTargetDir = TargetDir;
	_vector vMoveDir = InterpolateDir;

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
		InterpolateDir = XMVector3Transform(vMoveDir, XMLoadFloat4x4(&rotationMatrix));
		InterpolateDir = XMVectorSetW(InterpolateDir, 0.0f);

		return true;
	}
	else
	{
		_float ftheta = acos(fcosTheta);
		_float fAngleDegrees = XMConvertToDegrees(ftheta);

		if (fAngleDegrees < 5.0f)
		{
			InterpolateDir = vTargetDir;
			return true;
		}
		else
		{
			_float fsinTheta = sqrt(1.0f - fcosTheta * fcosTheta);
			_float fAlpha = sin((1 - fInterpolate) * ftheta) / fsinTheta;
			_float fBeta = sin(fInterpolate * ftheta) / fsinTheta;
			_float4 vResult = vMoveDirXZ * fAlpha + vTargetDirXZ * fBeta;
			InterpolateDir = XMVector4Normalize(vResult);
			InterpolateDir = XMVector3Normalize(InterpolateDir);
			return false;
		}
	}

	return false;
}

CKirbyVacuum_Vacuuming_State* CKirbyVacuum_Vacuuming_State::Create()
{
	CKirbyVacuum_Vacuuming_State* pInstance = new CKirbyVacuum_Vacuuming_State();
	return pInstance;
}

void CKirbyVacuum_Vacuuming_State::Free()
{
	__super::Free();
}

#pragma endregion
