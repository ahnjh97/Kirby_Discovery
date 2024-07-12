#pragma once

#include "Base.h"

BEGIN(Engine)

// 클라로 들어온 fTimeDelta를 비율별로 쪼개어 섬세하게 컨트롤 할 수 있게 하는 클래스.
class CTimeController final : public CBase
{
private:
	CTimeController();
	virtual ~CTimeController() = default;

public:
	HRESULT Initialize();
	void	Update_TimeController(_float fTimeDelta);

	_float Get_OriginalTimer() {
		return m_fTimeDelta;
	}

	_float	Get_FirstTimer() {
		return m_fTimeDelta * m_fFirstTimerRatio;
	}
	_float	Get_SecondTimer() {
		return m_fTimeDelta * m_fSecondTimerRatio;
	}

	void	Set_FirstTimerRatio(_float fRatio) {
		m_fFirstTimerRatio = fRatio;
	}
	void	Set_SecondTimerRatio(_float fRatio) {
		m_fSecondTimerRatio = fRatio;
	}

	void	Restore_FirstTimer(_float fRestoreMaxTime = 1.f) {
		m_bFirstTimerRestore = true;
		m_fFirstTimerRestoreMaxTime = fRestoreMaxTime;
	}
	void	Restore_SecondTimer(_float fRestoreMaxTime = 1.f) {
		m_bSecondTimerRestore = true;
		m_fSecondTimerRestoreMaxTime = fRestoreMaxTime;
	}

private:
	_float	m_fTimeDelta = { 0.f };

	_float	m_fFirstTimerRatio = { 1.f };
	_float	m_fSecondTimerRatio = { 1.f };

	_bool	m_bFirstTimerRestore = { false };
	_bool	m_bSecondTimerRestore = { false };

	_float	m_fFirstTimerRestoreMaxTime = { 1.f };
	_float	m_fSecondTimerRestoreMaxTime = { 1.f };

public:
	static CTimeController* Create();
	virtual void Free() override;
};

END