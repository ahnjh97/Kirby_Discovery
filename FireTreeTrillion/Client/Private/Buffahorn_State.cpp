#include "stdafx.h"
#include "Buffahorn_State.h"
#include "Buffahorn.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CBuffahorn_Idle_State::CBuffahorn_Idle_State()
{
}

void CBuffahorn_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CBuffahorn_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (15.f > fDistance)
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_FIND, 50.f, false, true);
}

void CBuffahorn_Idle_State::OnStateExit()
{
}

CBuffahorn_Idle_State* CBuffahorn_Idle_State::Create()
{
	CBuffahorn_Idle_State* pInstance = new CBuffahorn_Idle_State();
	return pInstance;
}

void CBuffahorn_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region FIND STATE
//*********************************
//			 FIND STATE
//*********************************
CBuffahorn_Find_State::CBuffahorn_Find_State()
{
}

void CBuffahorn_Find_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CBuffahorn_Find_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어를 향해 바라본다
	pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 1.5f);

	if (true == pBuffahorn->IsAnimFinished())
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_WAIT, 50.f, true, true);
}

void CBuffahorn_Find_State::OnStateExit()
{
}

CBuffahorn_Find_State* CBuffahorn_Find_State::Create()
{
	CBuffahorn_Find_State* pInstance = new CBuffahorn_Find_State();
	return pInstance;
}

void CBuffahorn_Find_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region WAIT STATE
//*********************************
//			 WAIT STATE
//*********************************
CBuffahorn_Wait_State::CBuffahorn_Wait_State()
{
}

void CBuffahorn_Wait_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);
}

void CBuffahorn_Wait_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), 0));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	pTransformCom->Look_At(vKirbyPos);

	// 자유 낙하
	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (13.f > fDistance)
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RUNSTART, 50.f, false, true);
	else if(15.f < fDistance)
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_CHARGEWAIT, 50.f, true, true);
}

void CBuffahorn_Wait_State::OnStateExit()
{
}

CBuffahorn_Wait_State* CBuffahorn_Wait_State::Create()
{
	CBuffahorn_Wait_State* pInstance = new CBuffahorn_Wait_State();
	return pInstance;
}

void CBuffahorn_Wait_State::Free()
{
	__super::Free();
}


#pragma region RUN STATE
//*********************************
//			 RUN STATE
//*********************************
CBuffahorn_Run_State::CBuffahorn_Run_State()
{
}

void CBuffahorn_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);

	m_fTimeDelta = 0.f;
}

void CBuffahorn_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (pBuffahorn->IsAnimFinished())
	{
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RUN, 50.f, true, true);
	}

	if (CBuffahorn::BUFFAHORN_RUN == pBuffahorn->Get_State())
	{
		m_fTimeDelta += fTimeDelta;

		if (2.f > m_fTimeDelta)
			pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 10.f, fTimeDelta);
		else
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_BRAKE, 50.f, false, true);
	}

	_float fHeight = pController->Compute_Height(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));

	if(2.f > fHeight)
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RETURNJUMPSTART, 50.f, false, true);
}

void CBuffahorn_Run_State::OnStateExit()
{
}

CBuffahorn_Run_State* CBuffahorn_Run_State::Create()
{
	CBuffahorn_Run_State* pInstance = new CBuffahorn_Run_State();
	return pInstance;
}

void CBuffahorn_Run_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region BRAKE STATE
//*********************************
//			 BRAKE STATE
//*********************************
CBuffahorn_Brake_State::CBuffahorn_Brake_State()
{
}

void CBuffahorn_Brake_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);

	m_fSpeed = 1.f;
}

void CBuffahorn_Brake_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	// 브레이크 : 제곱 감속
	_float fDeceleration = m_fSpeed * m_fSpeed;

	if (0.f < m_fSpeed)
		m_fSpeed -= fTimeDelta * 0.4f;

	pController->FreeFall(pTransformCom, fTimeDelta, 6.f);

	if (true == pBuffahorn->IsAnimFinished())
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_BRAKEEND, 45.f, false, true);
	else
		pController->Move_Dir(pTransformCom, (pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 10.f) * fDeceleration, fTimeDelta);

	if (CBuffahorn::BUFFAHORN_BRAKEEND == pBuffahorn->Get_State())
	{
		if(true == pBuffahorn->IsAnimFinished())
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_CHARGEWAIT, 45.f, true, true);
	}
}

void CBuffahorn_Brake_State::OnStateExit()
{
}

CBuffahorn_Brake_State* CBuffahorn_Brake_State::Create()
{
	CBuffahorn_Brake_State* pInstance = new CBuffahorn_Brake_State();
	return pInstance;
}

void CBuffahorn_Brake_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region JUMP STATE
//*********************************
//			 JUMP STATE
//*********************************
CBuffahorn_Jump_State::CBuffahorn_Jump_State()
{
}

void CBuffahorn_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation);

	m_fJumpVelocity = 5.f;
}

void CBuffahorn_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBuffahorn* pBuffahorn = static_cast<CBuffahorn*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (true == pBuffahorn->IsAnimFinished())
	{
		switch (pBuffahorn->Get_State())
		{
		case CBuffahorn::BUFFAHORN_RETURNJUMPSTART:
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RETURNJUMP, 45.f, false, true);
			break;
		case CBuffahorn::BUFFAHORN_RETURNJUMP:
			pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RETURNJUMPEND, 45.f, false, true);
			break;
		}
	}
	pController->Move_Dir(pTransformCom, pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * fTimeDelta * 10.f, fTimeDelta);

	m_fJumpVelocity -= GRAVITY * fTimeDelta * 6.f;
	pController->Jump(pTransformCom, m_fJumpVelocity, fTimeDelta);

	if ((true == pBuffahorn->IsAnimFinished() || pController->Is_Terrain()) && CBuffahorn::BUFFAHORN_RETURNJUMPEND == pBuffahorn->Get_State())
		pBuffahorn->Change_State(CBuffahorn::BUFFAHORN_RUN, 45.f, true, true);
}

void CBuffahorn_Jump_State::OnStateExit()
{
}

CBuffahorn_Jump_State* CBuffahorn_Jump_State::Create()
{
	CBuffahorn_Jump_State* pInstance = new CBuffahorn_Jump_State();
	return pInstance;
}

void CBuffahorn_Jump_State::Free()
{
	__super::Free();
}

#pragma endregion