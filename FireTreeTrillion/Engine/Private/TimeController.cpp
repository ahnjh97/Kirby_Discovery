#include "TimeController.h"

CTimeController::CTimeController()
{
}

HRESULT CTimeController::Initialize()
{
    return S_OK;
}

void CTimeController::Update_TimeController(_float fTimeDelta)
{
	m_fTimeDelta = fTimeDelta;

	if (m_bFirstTimerRestore == true)
	{
		m_fFirstTimerRatio += fTimeDelta * m_fFirstTimerRestoreMaxTime;
		if (m_fFirstTimerRatio > 1.f)
		{
			m_fFirstTimerRatio = 1.f;
			m_fFirstTimerRestoreMaxTime = 1.f;
			m_bFirstTimerRestore = false;
		}
	}


	if (m_bSecondTimerRestore == true)
	{
		m_fSecondTimerRatio += fTimeDelta * m_fSecondTimerRestoreMaxTime;
		if (m_fSecondTimerRatio > 1.f)
		{
			m_fSecondTimerRatio = 1.f;
			m_fSecondTimerRestoreMaxTime = 1.f;
			m_bSecondTimerRestore = false;
		}
	}
}

CTimeController* CTimeController::Create()
{
	CTimeController* pInstance = new CTimeController();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CTimeController"));

		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTimeController::Free()
{
}
