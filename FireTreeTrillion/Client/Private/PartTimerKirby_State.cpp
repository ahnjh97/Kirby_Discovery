#include "stdafx.h"
#include "PartTimerKirby_State.h"
#include "PartTimerKirby.h"
#include "PartTimeHelper.h"
#include "PartTimeFood.h"

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
	pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_IDLE);
	pAlbaKirby->Set_EyeState(CPartTimerKirby::EYE_IDLE);

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
			_uint uItem = static_cast<_uint>(curItem);
			RenderOff_Food(uItem);
			pAlbaKirby->Render_Food(true, curItem);
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_CORRECT, 50.f, false, true);
		}
		else
		{
			CTransform* pTransform = pAlbaKirby->Get_TransformCom();
			pAlbaKirby->Set_PrePosition(pTransform->Get_State(CTransform::STATE_POSITION));
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
	else if (vPos.x <= 18.5f && vPos.x > 17.5f)
		return PARTTIME_ITEM::TOMATO;
	else if (vPos.x <= 17.5f && vPos.x > 16.5f)
		return PARTTIME_ITEM::DRINK;
	else if (vPos.x <= 16.5f && vPos.x > 15.5f)
		return PARTTIME_ITEM::BURGER;

	return PARTTIME_ITEM::ITEM_END;
}

void CPartTimerKirby_Idle_State::RenderOff_Food(_uint uFoodIdx)
{
	_uint uCurLevel = *CGameInstance::Get_Instance()->Get_CurrentLevelID();
	auto food = m_pGameInstance->Get_GameObject(uCurLevel, TEXT("Layer_Food"), uFoodIdx);
	//auto listFood = m_pGameInstance->Get_List(uCurLevel, TEXT("Layer_Food"));
	//for (auto& food : *listFood)
	//{
	CPartTimeFood* pFood = static_cast<CPartTimeFood*>(food);
	pFood->Set_Render(false);
	//}
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
	pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_IDLE);

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
	m_fSpeed = 5.f;
}

void CPartTimerKirby_Grab_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pAlbaKirby = static_cast<CPartTimerKirby*>(pGameObject);
	CTransform* pTransform = pAlbaKirby->Get_TransformCom();
	if (pAlbaKirby->IsAnimFinished())
	{
		switch (pAlbaKirby->Get_State())
		{
		case CPartTimerKirby::HANDOVERSHORT:
		case CPartTimerKirby::HANDOVERSHORTL:
		{
			RenderOn_AllFood();
		}
		case CPartTimerKirby::FOODSHOP_INCORRECT:
		{
			if (pAlbaKirby->Get_State() == CPartTimerKirby::FOODSHOP_INCORRECT)
			{
				_float4 vPrePos = pAlbaKirby->Get_PrePosition();
				pTransform->Set_State(CTransform::STATE_POSITION, _float4(vPrePos.x, vPrePos.y, vPrePos.z, 1.f));

				if (nullptr != m_pFood)
				{
					_float4 vOriginPos = m_pFood->Get_OriginPosition();
					m_pFood->Get_TransformCom()->Set_State(CTransform::STATE_POSITION, vOriginPos);
				}
			}
			pAlbaKirby->Render_Food(false, PARTTIME_ITEM::ITEM_END);
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_SELECT, 50.f, true, true);
			CPartTimeHelper::Get_Instance()->Make_RandomItem();
		}
		break;
		case CPartTimerKirby::FOODSHOP_INCORRECTSTART:
		{
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_INCORRECT, 2.f, false, true);
		}
		break;
		case CPartTimerKirby::FOODSHOP_CORRECT:
		{
			auto item = CPartTimeHelper::Get_Instance()->Get_PartTimeItem();
			switch (item)
			{
			case PARTTIME_ITEM::CAKE:
			case PARTTIME_ITEM::TOMATO:
			{
				pAlbaKirby->Change_State(CPartTimerKirby::HANDOVERSHORT, 80.f, false, true);
			}
			break;
			case PARTTIME_ITEM::DRINK:
			case PARTTIME_ITEM::BURGER:
			{
				pAlbaKirby->Change_State(CPartTimerKirby::HANDOVERSHORTL, 80.f, false, true);
			}
			break;
			}
		}
		break;
		}
	}
	else
	{
		switch (pAlbaKirby->Get_State())
		{
		case CPartTimerKirby::FOODSHOP_INCORRECT:
		{
			pAlbaKirby->Set_EyeState(CPartTimerKirby::EYE_BLINK);
			pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_ANGER);

			// 현재 내가 밀어버린 아이템을 가져온다.
			PARTTIME_ITEM curItem = Get_CurrentFood(pAlbaKirby->Get_PrePosition());//TransformCom()->Get_State_Float4(CTransform::STATE_POSITION));

			// 아이템들을 갖고있는 레이어를 돌면서 밀어버린 아이템 객체를 찾는다.
			_uint iLevel = *m_pGameInstance->Get_CurrentLevelID();
			auto listFood = m_pGameInstance->Get_List(iLevel, TEXT("Layer_Food"));
			for (auto& GameObj : *listFood)
			{
				CPartTimeFood* pFood = static_cast<CPartTimeFood*>(GameObj);
				if (pFood->Get_Item() == curItem) // 해당 푸드일 경우, 푸드 날려버리기
				{
					m_pFood = pFood;
					CTransform* pTransformFood = pFood->Get_TransformCom();
					_float4		vPos = pTransformFood->Get_State(CTransform::STATE_POSITION);
					_vector		vLook = pTransform->Get_State_Float4(CTransform::STATE_LOOK);

					vPos += XMVector3Normalize(vLook) * 8.f * fTimeDelta;
					pTransformFood->Set_State(CTransform::STATE_POSITION, vPos);
					//pTransformFood->Set_State(CTransform::STATE_POSITION, _float4(vPos.x , vPos.y, vPos.z + fTimeDelta * 12.f, 1.f));
				}
			}

			if (m_fSpeed < 0.f) m_fSpeed = 0.f;
			pTransform->Go_Straight(fTimeDelta * m_fSpeed);
			m_fSpeed -= fTimeDelta * 20.f;
		}
		break;
		case CPartTimerKirby::FOODSHOP_CORRECT:
		{
			pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_HAPPY);
		}
		break;
		case CPartTimerKirby::FOODSHOP_INCORRECTSTART:
		{
			pAlbaKirby->Set_EyeState(CPartTimerKirby::EYE_BLINK);
			pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_ANGER);
		}
		break;
		default:
		{
			pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_SMILE);
			pAlbaKirby->Set_EyeState(CPartTimerKirby::EYE_IDLE);
		}
		break;
		}
	}
}
 
void CPartTimerKirby_Grab_State::OnStateExit()
{
}

void CPartTimerKirby_Grab_State::RenderOn_AllFood()
{
	_uint uCurLevel = *CGameInstance::Get_Instance()->Get_CurrentLevelID();
	auto listFood = m_pGameInstance->Get_List(uCurLevel, TEXT("Layer_Food"));
	for (auto& food : *listFood)
	{
		CPartTimeFood* pFood = static_cast<CPartTimeFood*>(food);
		pFood->Set_Render(true);
	}
}

PARTTIME_ITEM CPartTimerKirby_Grab_State::Get_CurrentFood(_float4 vPos)
{
	if (vPos.x <= 19.5f && vPos.x > 18.5f)
		return PARTTIME_ITEM::CAKE;
	else if (vPos.x <= 18.5f && vPos.x > 17.5f)
		return PARTTIME_ITEM::TOMATO;
	else if (vPos.x <= 17.5f && vPos.x > 16.5f)
		return PARTTIME_ITEM::DRINK;
	else if (vPos.x <= 16.5f && vPos.x > 15.5f)
		return PARTTIME_ITEM::BURGER;

	return PARTTIME_ITEM::ITEM_END;
}

CPartTimerKirby_Grab_State* CPartTimerKirby_Grab_State::Create()
{
	CPartTimerKirby_Grab_State* pInstance = new CPartTimerKirby_Grab_State();
	return pInstance;
}

void CPartTimerKirby_Grab_State::Free()
{
	__super::Free();

	Safe_Release(m_pFood);
}

#pragma endregion


#pragma region WIN STATE
//*********************************
//			WIN STATE
//*********************************
CPartTimerKirby_Win_State::CPartTimerKirby_Win_State()
{
}

void CPartTimerKirby_Win_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint iOffset)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, iOffset);
}

void CPartTimerKirby_Win_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CPartTimerKirby* pAlbaKirby = static_cast<CPartTimerKirby*>(pGameObject);
	if (pAlbaKirby->Get_State() == CPartTimerKirby::FOODSHOP_RESULTWINSTART)
	{
		if (pAlbaKirby->IsAnimFinished())
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_RESULTWIN, 50.f, false, true);

		pAlbaKirby->Set_EyeState(CPartTimerKirby::EYE_CLOSE);
		pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_SMILE);
	}
	else
	{
		/*if (pAlbaKirby->IsAnimFinished())
			pAlbaKirby->Change_State(CPartTimerKirby::FOODSHOP_SELECT, 50.f, true, true);*/
		pAlbaKirby->Set_EyeState(CPartTimerKirby::EYE_IDLE);
		pAlbaKirby->Set_MouthState(CPartTimerKirby::MOUTH_HAPPY);
	}
}

void CPartTimerKirby_Win_State::OnStateExit()
{

}

CPartTimerKirby_Win_State* CPartTimerKirby_Win_State::Create()
{
	CPartTimerKirby_Win_State* pInstance = new CPartTimerKirby_Win_State();
	return pInstance;
}

void CPartTimerKirby_Win_State::Free()
{
	__super::Free();
}
#pragma endregion

