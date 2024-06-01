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
		m_fFirstTimerRatio += fTimeDelta;
		if (m_fFirstTimerRatio > 1.f)
		{
			m_fFirstTimerRatio = 1.f;
			m_bFirstTimerRestore = false;
		}
	}


	if (m_bSecondTimerRestore == true)
	{
		m_fSecondTimerRatio += fTimeDelta;
		if (m_fSecondTimerRatio > 1.f)
		{
			m_fSecondTimerRatio = 1.f;
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
