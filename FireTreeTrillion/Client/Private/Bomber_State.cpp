#include "stdafx.h"
#include "Bomber_State.h"
#include "Bomber.h"
#include "Kirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CBomber_Idle_State::CBomber_Idle_State()
{
}

void CBomber_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CBomber_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBomber* pBomber = static_cast<CBomber*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();

	CKirby* pKirby = static_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vPos.m128_f32[1] = 0.f;
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	vKirbyPos.m128_f32[1] = 0.f;

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));

	// 일정 거리 안으로 플레이어가 들어오면 상태 전환
	if (7.f > fDistance)
	{
		pBomber->Change_State(CBomber::BOMBER_WALK, 30.f, true, true);
	}
}

void CBomber_Idle_State::OnStateExit()
{
}

CBomber_Idle_State* CBomber_Idle_State::Create()
{
	CBomber_Idle_State* pInstance = new CBomber_Idle_State();
	return pInstance;
}

void CBomber_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region WALK STATE
//*********************************
//			 WALK STATE
//*********************************
CBomber_Walk_State::CBomber_Walk_State()
{
}

void CBomber_Walk_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);

	m_fTimeDelta = 0.f;
}

void CBomber_Walk_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBomber* pBomber = static_cast<CBomber*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (5.f > pController->Compute_Height())
	{
		m_fTimeDelta += fTimeDelta;
		if(3.f < m_fTimeDelta)
			pBomber->Change_State(CBomber::BOMBER_EXPLOSION, 30.f, false, true);
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta, fTimeDelta);
	}
	else
		pBomber->Change_State(CBomber::BOMBER_CLIFFFALLSTART, 30.f, false, true);
}

void CBomber_Walk_State::OnStateExit()
{
}

CBomber_Walk_State* CBomber_Walk_State::Create()
{
	CBomber_Walk_State* pInstance = new CBomber_Walk_State();
	return pInstance;
}

void CBomber_Walk_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region SUICIDE STATE
//*********************************
//			 SUICIDE STATE
//*********************************
CBomber_Suicide_State::CBomber_Suicide_State()
{
}

void CBomber_Suicide_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CBomber_Suicide_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBomber* pBomber = static_cast<CBomber*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	if (CBomber::BOMBER_CLIFFFALLSTART == pBomber->Get_State())

	{
		pController->Move_Dir(pTransformCom, XMVector3Normalize(pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)) * fTimeDelta * 1.5f, fTimeDelta);
	}
	else if(CBomber::BOMBER_CLIFFFALL != pBomber->Get_State())
		pController->FreeFall(pTransformCom, fTimeDelta);

	if(CBomber::BOMBER_CLIFFFALLEND == pBomber->Get_State())
	if (2.f > pController->Compute_Height())
		pBomber->Set_Dead();


	if (true == pBomber->IsAnimFinished())
	{
		switch (pBomber->Get_State())
		{
		case CBomber::BOMBER_CLIFFFALLSTART:
			pBomber->Change_State(CBomber::BOMBER_CLIFFFALL, 40.f, false, true);
			break;
		case CBomber::BOMBER_CLIFFFALL:
			pBomber->Change_State(CBomber::BOMBER_CLIFFFALLEND, 40.f, true, true);
			break;
		}
	}
}

void CBomber_Suicide_State::OnStateExit()
{
}

CBomber_Suicide_State* CBomber_Suicide_State::Create()
{
	CBomber_Suicide_State* pInstance = new CBomber_Suicide_State();
	return pInstance;
}

void CBomber_Suicide_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region EXPLOSION STATE
//*********************************
//			 EXPLOSION STATE
//*********************************
CBomber_Explosion_State::CBomber_Explosion_State()
{
}

void CBomber_Explosion_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CBomber_Explosion_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CBomber* pBomber = static_cast<CBomber*>(pGameObject);

	if (pBomber->IsAnimFinished())
		pBomber->Set_Dead();
}

void CBomber_Explosion_State::OnStateExit()
{
}

CBomber_Explosion_State* CBomber_Explosion_State::Create()
{
	CBomber_Explosion_State* pInstance = new CBomber_Explosion_State();
	return pInstance;
}

void CBomber_Explosion_State::Free()
{
	__super::Free();
}
#pragma endregion
