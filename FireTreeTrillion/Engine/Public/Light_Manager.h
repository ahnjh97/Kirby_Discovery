#pragma once

#include "Base.h"

BEGIN(Engine)

class CLight_Manager final : public CBase
{
private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);

public:
	HRESULT Initialize();
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, _bool bForTool);
	void	IMGUI_Tick();

	void	Blink_Light(_float fTimeDelta, _uint iLightNum, _float fRandomSpeed = 1.f);
	void	Set_LightState(_uint uState) { m_eState = static_cast<STATE>(uState); }
	void	Set_CurLightRange(_uint iLightNum, _float fRange);


	// 방금 추가한 놈의 주소를 뜯어온다. ( 빛을 따라오게 하는 용도 )
	class CLight* Get_LightLastAddress() {
		return m_Lights.back();
	}

	class CLight* Get_DirectionLightAddress() {
		return m_Lights.front();
	}

	// 빛을 초기화 한다.
	void	Clear_Light();

private:
	list<class CLight*>				m_Lights;

	enum STATE { SIZE_UP, SIZE_DOWN, STATE_END };
	STATE m_eState = STATE_END;

public:
	static CLight_Manager* Create();
	virtual void Free() override;
};

END