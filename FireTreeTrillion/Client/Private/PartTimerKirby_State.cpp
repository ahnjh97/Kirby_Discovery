#include "stdafx.h"
#include "PartTimerKirby_State.h"
#include "PartTimerKirby.h"
#include "PartTimeHelper.h"


#pragma region IDLE STATE
//*********************************
//			 IDLE STATE
//*********************************
CPartTimerKirby_Idle_State::CPartTimerKirby_Idle_State()
{
}

void CPartTimerKirby_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CPartTimerKirby_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pAlbaKirby = static_cast<CPartTimerKirby*>(pGameObject);
	if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_DOWN))
		pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_MOVEL, 50.f, false, true);
	else if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_DOWN))
		pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_MOVER, 50.f, false, true);
	else if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
	{
		// 아이템 위치에 따라 PARTTIME_ITEM를 다르게
		PARTTIME_ITEM curItem = Get_CurrentFood(pAlbaKirby->Get_TransformCom()->Get_State_Float4(CTransform::STATE_POSITION));
		if (CPartTimeHelper::Get_Instance()->Check_Item(curItem))
		{
			pAlbaKirby->Render_Food(true, curItem);
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_CORRECT, 50.f, false, true);
		}
		else
		{
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_INCORRECTSTART, 50.f, false, true);
		}
	}
}

void CPartTimerKirby_Idle_State::OnStateExit()
{
}

PARTTIME_ITEM CPartTimerKirby_Idle_State::Get_CurrentFood(_float4 vPos)
{
	if (vPos.x <= 19.5f && vPos.x > 18.5f)
		return PARTTIME_ITEM::CAKE;
	else if(vPos.x <= 18.5f && vPos.x > 17.5f)
		return PARTTIME_ITEM::TOMATO;
	else if (vPos.x <= 17.5f && vPos.x > 16.5f)
		return PARTTIME_ITEM::DRINK;
	else if (vPos.x <= 16.5f && vPos.x > 15.5f)
		return PARTTIME_ITEM::BURGER;

	return PARTTIME_ITEM::ITEM_END;
}

CPartTimerKirby_Idle_State* CPartTimerKirby_Idle_State::Create()
{
	CPartTimerKirby_Idle_State* pInstance = new CPartTimerKirby_Idle_State();
	return pInstance;
}

void CPartTimerKirby_Idle_State::Free()
{
	__super::Free();
}

#pragma endregion


#pragma region MOVE STATE
//*********************************
//			 MOVE STATE
//*********************************
CPartTimerKirby_Move_State::CPartTimerKirby_Move_State()
{
}

void CPartTimerKirby_Move_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_uDir = CPartTimerKirby::FOODSHOP_MOVEL == _iAnimIndex ? LEFT : RIGHT;
}

void CPartTimerKirby_Move_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pAlbaKirby = static_cast<CPartTimerKirby*>(pGameObject);
	CTransform* pTransform = pAlbaKirby->Get_TransformCom();

	// 커비를 앞에서 보는 기준이라, 관념과 반대로 이동 방향 셋업
	if (m_uDir == LEFT) pTransform->Go_Right(fTimeDelta);
	else pTransform->Go_Left(fTimeDelta);

	if (pAlbaKirby->IsAnimFinished())
		pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_SELECT, 50.f, true, true);
}

void CPartTimerKirby_Move_State::OnStateExit()
{
}

CPartTimerKirby_Move_State* CPartTimerKirby_Move_State::Create()
{
	CPartTimerKirby_Move_State* pInstance = new CPartTimerKirby_Move_State();
	return pInstance;
}

void CPartTimerKirby_Move_State::Free()
{
	__super::Free();

}

#pragma endregion

#pragma region GRAB STATE
//*********************************
//			GRAB STATE
//*********************************
CPartTimerKirby_Grab_State::CPartTimerKirby_Grab_State()
{
}

void CPartTimerKirby_Grab_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
	m_fSpeed = 2.f;
}

void CPartTimerKirby_Grab_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pAlbaKirby = static_cast<CPartTimerKirby*>(pGameObject);
	CTransform* pTransform = pAlbaKirby->Get_TransformCom();
	if (pAlbaKirby->IsAnimFinished())
	{
		if (pAlbaKirby->Get_State() == CPartTimerKirby::HANDOVERSHORT || pAlbaKirby->Get_State() == CPartTimerKirby::HANDOVERSHORTL|| pAlbaKirby->Get_State() == CPartTimerKirby::FOODSHOP_INCORRECT)
		{
			if (pAlbaKirby->Get_State() == CPartTimerKirby::FOODSHOP_INCORRECT)
			{
				_float4 vPos = pTransform->Get_State(CTransform::STATE_POSITION);
				pTransform->Set_State(CTransform::STATE_POSITION, _float4(vPos.x, vPos.y, 29.3f, 1.f));
			}
			pAlbaKirby->Render_Food(false, PARTTIME_ITEM::ITEM_END);
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_SELECT, 50.f, true, true);
		}
		else if (pAlbaKirby->Get_State() == CPartTimerKirby::FOODSHOP_INCORRECTSTART)
		{
			//+)QZR : 음식 날라가게
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_INCORRECT, 2.f, false, true);
		}
		else if(CPartTimeHelper::Get_Instance()->Get_PartTimeItem() == PARTTIME_ITEM::CAKE || CPartTimeHelper::Get_Instance()->Get_PartTimeItem() == PARTTIME_ITEM::TOMATO)
			pAlbaKirby->Change_State(CPartTimerKirby::HANDOVERSHORT, 70.f, false, true);
		else if(CPartTimeHelper::Get_Instance()->Get_PartTimeItem() == PARTTIME_ITEM::DRINK || CPartTimeHelper::Get_Instance()->Get_PartTimeItem() == PARTTIME_ITEM::BURGER)
			pAlbaKirby->Change_State(CPartTimerKirby::HANDOVERSHORTL, 70.f, false, true);
	}
	else
	{
		if (pAlbaKirby->Get_State() == CPartTimerKirby::FOODSHOP_INCORRECT)
		{
			if (m_fSpeed < 0.f) m_fSpeed = 0.f;
			pTransform->Go_Straight(fTimeDelta * m_fSpeed);
			m_fSpeed -= fTimeDelta * 5.f;
		}
	}
}
 
void CPartTimerKirby_Grab_State::OnStateExit()
{
	CPartTimeHelper::Get_Instance()->Make_RandomItem();
}

CPartTimerKirby_Grab_State* CPartTimerKirby_Grab_State::Create()
{
	CPartTimerKirby_Grab_State* pInstance = new CPartTimerKirby_Grab_State();
	return pInstance;
}

void CPartTimerKirby_Grab_State::Free()
{
	__super::Free();
}

#pragma endregion

