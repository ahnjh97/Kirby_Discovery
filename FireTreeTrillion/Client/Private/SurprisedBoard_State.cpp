#include "stdafx.h"
#include "SurprisedBoard_State.h"
#include "SurprisedBoard.h"

#pragma region WAIT STATE
//*********************************
//			WAIT STATE
//*********************************
CSurprisedBoard_Wait_State::CSurprisedBoard_Wait_State()
{
}

void CSurprisedBoard_Wait_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
	m_uDir = CSurprisedBoard::WAIT_L == _iAnimIndex ? LEFT : RIGHT;
}

void CSurprisedBoard_Wait_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CTransform* pTransformCom = pGameObject->Get_TransformCom();
	CSurprisedBoard* pSurprisedBoard = static_cast<CSurprisedBoard*>(pGameObject);
	if (pSurprisedBoard->IsAnimFinished())
	{
		if(m_uDir == LEFT)
			pSurprisedBoard->Change_State(CSurprisedBoard::PREPOP_OUT_L, 50.f, false, true);
		else
			pSurprisedBoard->Change_State(CSurprisedBoard::PREPOP_OUT_R, 50.f, false, true);
	}
}

void CSurprisedBoard_Wait_State::OnStateExit()
{
}

CSurprisedBoard_Wait_State* CSurprisedBoard_Wait_State::Create()
{
	CSurprisedBoard_Wait_State* pInstance = new CSurprisedBoard_Wait_State();
	return pInstance;
}
void CSurprisedBoard_Wait_State::Free()
{
	__super::Free();
}
#pragma endregion

#pragma region POPOUT STATE
//*********************************
//			POPOUT STATE
//*********************************
CSurprisedBoard_Popout_State::CSurprisedBoard_Popout_State()
{
}

void CSurprisedBoard_Popout_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CSurprisedBoard_Popout_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSurprisedBoard* pSurprisedBoard = static_cast<CSurprisedBoard*>(pGameObject);
	//CTransform* pTransform = pSurprisedBoard->Get_TransformCom();
	//_float fSecondTimeDelta = m_pGameInstance->Get_SecondTimer();
	if (pSurprisedBoard->IsAnimFinished())
	{
		switch (pSurprisedBoard->Get_State())
		{
		case CSurprisedBoard::ARM_MOVE_L:
			pSurprisedBoard->Change_State(CSurprisedBoard::RETURN_L, 50.f, false, true);
			break;
		case CSurprisedBoard::ARM_MOVE_R:
			pSurprisedBoard->Change_State(CSurprisedBoard::RETURN_R, 50.f, false, true);
			break;
		case CSurprisedBoard::POP_OUT_L:
			pSurprisedBoard->Change_State(CSurprisedBoard::ARM_MOVE_L, 50.f, false, true);
		break;
		case CSurprisedBoard::POP_OUT_R:
			pSurprisedBoard->Change_State(CSurprisedBoard::ARM_MOVE_R, 50.f, false, true);
			break;
		case CSurprisedBoard::PREPOP_OUT_L:
			pSurprisedBoard->Change_State(CSurprisedBoard::POP_OUT_L, 50.f, false, true);
			break;
		case CSurprisedBoard::PREPOP_OUT_R:
			pSurprisedBoard->Change_State(CSurprisedBoard::POP_OUT_R, 50.f, false, true);
			break;
		}
	}
	else
	{
		switch (pSurprisedBoard->Get_State())
		{
		case CSurprisedBoard::POP_OUT_L:
			pSurprisedBoard->Go_Right_Rigid();
			break;
		case CSurprisedBoard::POP_OUT_R:
			pSurprisedBoard->Go_Left_Rigid();
			break;
		}
	}
}

void CSurprisedBoard_Popout_State::OnStateExit()
{
}

CSurprisedBoard_Popout_State* CSurprisedBoard_Popout_State::Create()
{
	CSurprisedBoard_Popout_State* pInstance = new CSurprisedBoard_Popout_State();
	return pInstance;
}

void CSurprisedBoard_Popout_State::Free()
{
	__super::Free();
}
#pragma endregion


#pragma region RETURN STATE
//*********************************
//			RETURN STATE
//*********************************
CSurprisedBoard_Return_State::CSurprisedBoard_Return_State()
{
}

void CSurprisedBoard_Return_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
	__super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CSurprisedBoard_Return_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSurprisedBoard* pSurprisedBoard = static_cast<CSurprisedBoard*>(pGameObject);
	CTransform* pTransform = pSurprisedBoard->Get_TransformCom();
	_float fSecondTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (pSurprisedBoard->IsAnimFinished())
	{
		switch (pSurprisedBoard->Get_State())
		{
		case CSurprisedBoard::RETURN_L:
			pSurprisedBoard->Change_State(CSurprisedBoard::WAIT_L, 50.f, true, true);
			pSurprisedBoard->Set_Activated(FALSE);
			break;
		case CSurprisedBoard::RETURN_R:
			pSurprisedBoard->Change_State(CSurprisedBoard::WAIT_R, 50.f, true, true);
			pSurprisedBoard->Set_Activated(FALSE);
			break;
		}
	}
	else
	{
		switch (pSurprisedBoard->Get_State())
		{
		case CSurprisedBoard::RETURN_L:
			pSurprisedBoard->Go_Left_Rigid();
			break;
		case CSurprisedBoard::RETURN_R:
			pSurprisedBoard->Go_Right_Rigid();
			break;
		}
	}
}

void CSurprisedBoard_Return_State::OnStateExit()
{
}

CSurprisedBoard_Return_State* CSurprisedBoard_Return_State::Create()
{
	CSurprisedBoard_Return_State* pInstance = new CSurprisedBoard_Return_State();
	return pInstance;
}

void CSurprisedBoard_Return_State::Free()
{
	__super::Free();
}
#pragma endregion

