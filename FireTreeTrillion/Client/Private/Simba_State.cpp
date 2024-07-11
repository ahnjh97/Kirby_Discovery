#include "stdafx.h"
#include "Simba_State.h"
#include "Simba.h"
#include "Kirby.h"

// *********************** APPEAR1 ***********************
void CSimba_Appear1::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	if (true == pSimba->IsAnimFinished())
	{
		switch (pSimba->Get_State())
		{
		case CSimba::SIMBA_DEMOAPPEAR1CUT2:
			break;
		}
		pSimba->Change_State(CSimba::SIMBA_DEMOAPPEAR1CUT2, 50.f, false, true);
	}
}

// *********************** APPEAR2 ***********************
void CSimba_Appear2::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	CSimba* pSimba = static_cast<CSimba*>(pGameObject);
	if (true == pSimba->IsAnimFinished())
	{
		switch (pSimba->Get_State())
		{
		case CSimba::SIMBA_DEMOAPPEAR2CUT1:
			pSimba->Change_State(CSimba::SIMBA_DEMOAPPEAR2CUT2, 50.f, false, true);
			break;
		case CSimba::SIMBA_DEMOAPPEAR2CUT2:
			m_fTime += fTimeDelta;
			if (m_fTime > 1.5f)
				pSimba->Change_State(CSimba::SIMBA_WALK, 50.f, true, false);
			break;
		}
	}
}

// *********************** WALK ***********************
void CSimba_Walk::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
	_float fDis = m_pGameInstance->Compute_Distance(m_pGameInstance->Get_GameObject(LEVEL_SIMBA, TEXT("Layer_Player")), pGameObject);
	if (fDis < 5.f)
	{

	}
}