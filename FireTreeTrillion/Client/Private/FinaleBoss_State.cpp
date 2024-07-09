#include "stdafx.h"
#include "FinaleBoss_State.h"
#include "FinaleBoss.h"
#include "FinaleKirby.h"

#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CFinaleBoss_Idle_State::CFinaleBoss_Idle_State()
{
}

void CFinaleBoss_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CFinaleBoss_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CFinaleBoss* pFinaleBoss = static_cast<CFinaleBoss*>(pGameObject);
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CCharacterController* pController = static_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));

	CFinaleKirby* pFinaleKirby = static_cast<CFinaleKirby*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player")));
	CTransform* pKirbyTransformCom = pFinaleKirby->Get_TransformCom();

	_vector vPos = pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vKirbyPos = pKirbyTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	// 플레이어와 몬스터의 거리 계산
	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSubtract(vPos, vKirbyPos)));
	_float fMaxDist = 400.f;

	if(0.f < vPos.m128_f32[1])
		vPos.m128_f32[1] = vKirbyPos.m128_f32[1] + 60.f;

	vPos.m128_f32[2] = vKirbyPos.m128_f32[2];
	if (400.f > fDistance)
	{
		vPos += XMVector3Normalize(XMVectorSet(1.f, 0.f, 0.f, 0.f)) * fTimeDelta * 50.f;
		pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	}

	//else if (fDistance <= fMaxDist)
	//{
	//	// 대상 객체에 가까워지는 방향으로 이동
	//	vPos += vKirbyPos + XMVectorSet(1.f, 0.f, 0.f, 0.f) * (fDistance - fMaxDist);
	//	pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	//}

}

void CFinaleBoss_Idle_State::OnStateExit()
{
}

CFinaleBoss_Idle_State* CFinaleBoss_Idle_State::Create()
{
	CFinaleBoss_Idle_State* pInstance = new CFinaleBoss_Idle_State();
	return pInstance;
}

void CFinaleBoss_Idle_State::Free()
{
	__super::Free();
}
#pragma endregion