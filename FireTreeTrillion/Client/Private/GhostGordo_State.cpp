#include "stdafx.h"
#include "GhostGordo_State.h"
#include "GhostGordo.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CGhostGordo_Idle_State::CGhostGordo_Idle_State()
{
}

void CGhostGordo_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CGhostGordo_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CGhostGordo* pGhostGordo = static_cast<CGhostGordo*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (15.f > fDistance)
	{
		if(/*true == pGhostGordo->Get_Light()*/true == pKirby->Get_KirbyInfo()->m_bLightOn)
			pGhostGordo->Change_State(CGhostGordo::GORDO_EYEOPENSTART, 40.f, false, true);
	}

	if (CGhostGordo::GORDO_APPEAR == pGhostGordo->Get_State())
	{
		if (pGhostGordo->IsAnimFinished())
			pGhostGordo->Change_State(CGhostGordo::GORDO_EYECLOSEWAIT, 40.f, true, true);
	}
	else if (CGhostGordo::GORDO_EYEOPENSTART == pGhostGordo->Get_State())
	{
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 8.f);

		if(pGhostGordo->IsAnimFinished())
			pGhostGordo->Change_State(CGhostGordo::GORDO_EYEOPENWAIT, 40.f, true, true);
	}
}

void CGhostGordo_Idle_State::OnStateExit()
{
}

CGhostGordo_Idle_State* CGhostGordo_Idle_State::Create()
{
	CGhostGordo_Idle_State* pInstance = new CGhostGordo_Idle_State();
	return pInstance;
}

void CGhostGordo_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region MOVE STATE
//*********************************
//			 MOVE STATE
//*********************************
CGhostGordo_Move_State::CGhostGordo_Move_State()
{
}

void CGhostGordo_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fSpeed = 2.5f;
	m_fTimeDelta = 0.f;
}

void CGhostGordo_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CGhostGordo* pGhostGordo = static_cast<CGhostGordo*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vOriginPos = pGhostGordo->Get_Position();

	// 플레이어를 향해 바라본다
	if(true == pGhostGordo->Get_R2B())
	{
		if(/*true == pGhostGordo->Get_Light()*/true == pKirby->Get_KirbyInfo()->m_bLightOn)
		{
			pGhostGordo->Set_R2B(false);
			pGhostGordo->Change_State(CGhostGordo::GORDO_EYEOPENSTART, 40.f, false, true);
		}

		// 플레이어와 몬스터의 거리 계산
		_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vOriginPos)));

		// 일정 거리 안으로 플레이어가 들어오면 상태 전환
		if (1.5f > fDistance)
		{
			pTransformCom->Look_At_Rotate_Dir(pGhostGordo->Get_Look(), fTimeDelta * 8.f);

			m_fTimeDelta += fTimeDelta;
			if(0.5f < m_fTimeDelta)
				pGhostGordo->Change_State(CGhostGordo::GORDO_EYECLOSEWAIT, 40.f, true, true);
		}
		else
		{
			pTransformCom->Look_At_Rotate(pGhostGordo->Get_Position(), fTimeDelta * 8.f);

			vPos += XMVector3Normalize(vOriginPos - vPos) * fTimeDelta * m_fSpeed;
			//vPos.m128_f32[1] += 0.5f;
			pController->Move(pTransformCom, vPos, fTimeDelta);
		}
	}
	else
	{
		pTransformCom->Look_At_Rotate(pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION), fTimeDelta * 8.f);

		if (/*false == pGhostGordo->Get_Light()*/false == pKirby->Get_KirbyInfo()->m_bLightOn)
			pGhostGordo->Change_State(CGhostGordo::GORDO_LOOK, 40.f, false, true);
	
		_vector vLook = pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
		pController->Move_Dir(pTransformCom, XMVector3Normalize(vLook) * fTimeDelta * m_fSpeed, fTimeDelta);
	}
}

void CGhostGordo_Move_State::OnStateExit()
{
}

CGhostGordo_Move_State* CGhostGordo_Move_State::Create()
{
	CGhostGordo_Move_State* pInstance = new CGhostGordo_Move_State();
	return pInstance;
}

void CGhostGordo_Move_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region LOOK STATE
//*********************************
//			 LOOK STATE
//*********************************
CGhostGordo_Look_State::CGhostGordo_Look_State()
{
}

void CGhostGordo_Look_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CGhostGordo_Look_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CGhostGordo* pGhostGordo = static_cast<CGhostGordo*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if(pGhostGordo->IsAnimFinished())
	{
		pGhostGordo->Set_R2B(true);
		pGhostGordo->Change_State(CGhostGordo::GORDO_EYEOPENWAIT, 40.f, true, true);
	}
}

void CGhostGordo_Look_State::OnStateExit()
{
}

CGhostGordo_Look_State* CGhostGordo_Look_State::Create()
{
	CGhostGordo_Look_State* pInstance = new CGhostGordo_Look_State();
	return pInstance;
}

void CGhostGordo_Look_State::Free()
{
	__super::Free();
}
#pragma endregion

