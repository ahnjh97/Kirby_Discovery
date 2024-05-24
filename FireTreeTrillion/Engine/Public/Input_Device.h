#ifndef InputDev_h__
#define InputDev_h__

#include "Base.h"

/* 키보드와 마우스를 초기화하고 현재 입력장치의 상태를 받아온다. */

BEGIN(Engine)

class CInput_Device : public CBase
{
private:
	CInput_Device(void);
	virtual ~CInput_Device(void) = default;
	
public:
	_bool Get_DIKeyState(_ubyte byKeyID, KEYSTATE eState);
	_bool Get_DIMouseState(MOUSEKEYSTATE eMouse, KEYSTATE eState);

	_byte	Get_DIKeyState(_ubyte byKeyID)			{ 
		return m_byKeyState[byKeyID]; }

	
	_byte	Get_DIMouseState(MOUSEKEYSTATE eMouse) 	{ 	return m_tMouseState.rgbButtons[eMouse]; 	}

	_long	Get_DIMouseMove(MOUSEMOVESTATE eMouseState)	
	{	
		return *(((_long*)&m_tMouseState) + eMouseState);	
	}

	void	Set_WindowActive(_bool _bIsWindowActive) { m_IsWindowActive = _bIsWindowActive; }
	_bool	Get_WindowActive() { return m_IsWindowActive; }
	
public:
	HRESULT Initialize(HINSTANCE hInst, HWND hWnd);
	void	Tick(void);

private:
	LPDIRECTINPUT8			m_pInputSDK = nullptr;	

private:
	LPDIRECTINPUTDEVICE8	m_pKeyBoard = nullptr;
	LPDIRECTINPUTDEVICE8	m_pMouse	= nullptr;
	LPDIRECTINPUTDEVICE8	m_pJoyPad = nullptr;

private:
	_byte					m_byKeyState[256];			// 키보드에 있는 모든 키값을 저장하기 위한 변수
	KEYSTATE				m_eKeyState[256];			// 해당 키의 상태 값 저장

	// 이전 키와 마우스의 상태를 추적하기 위한 변수이다.
	_byte                   m_byPrevKeyState[256];
	DIMOUSESTATE            m_tPrevMouseState;

	DIMOUSESTATE			m_tMouseState;
	KEYSTATE				m_eMouseStates[DIMKS_END];

	// 클라창이 활성화가 되지 않았을 경우 아무것도 작동되지 않게 하는 변수이다.
	_bool					m_IsWindowActive = { true };

public:
	static CInput_Device* Create(HINSTANCE hInst, HWND hWnd);
	virtual void	Free(void);

};
END
#endif // InputDev_h__
